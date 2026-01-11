/*
 * Camera_CameraManager.cpp - 相机管理器实现
 * 封装相机设备管理和拍照功能
 * 阶段五：相机模块开发 - 相机设备管理
 */

#include "Camera_CameraManager.h"
#include "Shared_GlobalDefines.h"

// 相机配置对象（在Camera.ino中定义）
extern VideoSetting configPreview;
extern VideoSetting configStill;

// 静态指针，用于JPEGDraw回调访问TFTManager
static Display_TFTManager* s_tftManagerForJPEG = nullptr;

// JPEGDraw回调函数
static int CameraManager_JPEGDraw(JPEGDRAW *pDraw) {
    if (s_tftManagerForJPEG != nullptr) {
        s_tftManagerForJPEG->drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    }
    return 1;
}

// SD卡管理器实例（在Camera.ino中定义）
extern SDCardManager sdCardManager;

// DS1307时钟模块实例（在Camera.ino中定义）
extern DS1307_ClockModule clockModule;

CameraManager::CameraManager()
    : m_tftManager(nullptr)
    , m_fontRenderer(nullptr)
    , m_jpegDecoder(nullptr)
    , m_sdCardManager(nullptr)
    , m_state(STATE_IDLE)
    , m_previewActive(false)
    , m_isCapturing(false)
    , m_captureRequested(false)
    , m_initialized(false)
{
    m_previewBuffer.imgAddr = 0;
    m_previewBuffer.imgLen = 0;
    m_stillBuffer.imgAddr = 0;
    m_stillBuffer.imgLen = 0;
}

bool CameraManager::init(Display_TFTManager& tftMgr, Display_FontRenderer& fontRenderer, JPEGDEC& jpegDecoder) {
    if (m_initialized) {
        Utils_Logger::error("CameraManager already initialized");
        return false;
    }

    m_tftManager = &tftMgr;
    m_fontRenderer = &fontRenderer;
    m_jpegDecoder = &jpegDecoder;
    m_sdCardManager = &sdCardManager;

    // 设置静态指针，供JPEGDraw回调使用
    s_tftManagerForJPEG = &tftMgr;

    // 初始化SD卡管理器
    if (!m_sdCardManager->init()) {
        Utils_Logger::error("SD card manager initialization failed");
        // SD卡初始化失败不影响相机功能，继续初始化
    }

    // 初始化相机硬件
    Utils_Logger::info("Initializing camera hardware...");
    Camera.configVideoChannel(PREVIEW_CH, configPreview);
    Camera.configVideoChannel(STILL_CH, configStill);
    Camera.videoInit();
    Utils_Logger::info("Camera hardware initialized");

    m_initialized = true;
    Utils_Logger::info("CameraManager initialized successfully");
    return true;
}

void CameraManager::cleanup() {
    if (m_previewActive) {
        stopPreview();
    }
    
    // 清理SD卡资源
    if (m_sdCardManager != nullptr) {
        m_sdCardManager->cleanup();
    }
    
    m_initialized = false;
}

bool CameraManager::startPreview() {
    if (!m_initialized) {
        Utils_Logger::error("CameraManager not initialized");
        return false;
    }

    if (m_previewActive) {
        Utils_Logger::error("Preview already active");
        return true;
    }

    Utils_Logger::info(">>> Starting camera preview");
    m_state = STATE_PREVIEW;
    m_previewActive = true;

    m_tftManager->fillScreen(ST7789_BLACK);

    int centerX = m_fontRenderer->calculateCenterPosition(240, m_fontRenderer->getRealTimePreviewString());
    m_fontRenderer->drawChineseString(centerX, 150, m_fontRenderer->getRealTimePreviewString(), ST7789_WHITE, ST7789_BLACK);
    Utils_Timer::delayMs(1000);

    Camera.channelEnd(CHANNEL_PREVIEW);
    Camera.channelBegin(CHANNEL_PREVIEW);

    m_tftManager->fillScreen(ST7789_BLACK);

    Utils_Logger::info("Camera preview started");
    return true;
}

void CameraManager::stopPreview() {
    if (!m_previewActive) {
        return;
    }

    Utils_Logger::info("<<< Stopping camera preview");
    m_previewActive = false;
    m_state = STATE_IDLE;

    Camera.channelEnd(CHANNEL_PREVIEW);
    Camera.channelEnd(CHANNEL_STILL);

    m_captureRequested = false;
    m_isCapturing = false;

    Utils_Timer::delayMs(50);

    Utils_Logger::info("Camera preview stopped");
}

void CameraManager::processPreviewFrame() {
    if (!m_previewActive) {
        return;
    }

    // 添加超时处理的相机图像获取
    uint32_t startTime = Utils_Timer::getCurrentTime();
    const uint32_t TIMEOUT_MS = 200; // 200ms超时
    
    Camera.getImage(CHANNEL_PREVIEW, &m_previewBuffer.imgAddr, &m_previewBuffer.imgLen);
    
    // 检查是否超时
    if (Utils_Timer::getCurrentTime() - startTime > TIMEOUT_MS) {
        Utils_Logger::info("Camera.getImage timeout in preview");
        m_previewBuffer.imgLen = 0;
        return;
    }

    if (m_previewBuffer.imgLen > 0) {
        startTime = Utils_Timer::getCurrentTime();
        
        if (m_jpegDecoder->openFLASH((uint8_t *)m_previewBuffer.imgAddr, m_previewBuffer.imgLen, CameraManager_JPEGDraw)) {
            // 添加JPEG解码超时检查
            if (Utils_Timer::getCurrentTime() - startTime > TIMEOUT_MS) {
                Utils_Logger::info("JPEG decode timeout");
                m_jpegDecoder->close();
                return;
            }
            
            m_jpegDecoder->decode(0, 0, JPEG_SCALE_HALF);
            m_jpegDecoder->close();
        }
    }

    // 将拍照请求检查移到任务循环中，减少预览帧处理的延迟
}

bool CameraManager::capturePhoto() {
    if (m_isCapturing) {
        Utils_Logger::error("Warning: Previous capture not finished, skipping");
        return false;
    }

    m_isCapturing = true;
    Utils_Logger::info(">>> Starting photo capture");

    const uint32_t TIMEOUT_MS = 5000; // 5秒总超时
    uint32_t startTime = Utils_Timer::getCurrentTime();

    Camera.channelBegin(CHANNEL_STILL);
    Utils_Logger::info("  1. Still channel opened");
    
    // 带超时检查的延迟
    uint32_t delayStartTime = Utils_Timer::getCurrentTime();
    while (Utils_Timer::getCurrentTime() - delayStartTime < 200) {
        // 检查总超时
        if (Utils_Timer::getCurrentTime() - startTime > TIMEOUT_MS) {
            Utils_Logger::error("Capture timeout during delay");
            goto capture_cleanup;
        }
        vTaskDelay(1); // 让出CPU
    }

    // 带超时的图像获取
    startTime = Utils_Timer::getCurrentTime();
    Camera.getImage(CHANNEL_STILL, &m_stillBuffer.imgAddr, &m_stillBuffer.imgLen);
    
    if (Utils_Timer::getCurrentTime() - startTime > TIMEOUT_MS) {
        Utils_Logger::error("Camera.getImage timeout during capture");
        m_stillBuffer.imgLen = 0;
        goto capture_cleanup;
    }

    if (m_stillBuffer.imgLen > 0) {
        Utils_Logger::info("  2. Image captured successfully, size: %d bytes", m_stillBuffer.imgLen);

        startTime = Utils_Timer::getCurrentTime();
        if (savePhotoToSDCard(m_stillBuffer.imgAddr, m_stillBuffer.imgLen)) {
            // 显示中文成功提示
            int centerX = m_fontRenderer->calculateCenterPosition(240, m_fontRenderer->getPhotoSuccessString());
            m_fontRenderer->drawChineseString(centerX, 150, m_fontRenderer->getPhotoSuccessString(), ST7789_WHITE, ST7789_BLACK);
            
            // 带超时检查的显示延迟
            delayStartTime = Utils_Timer::getCurrentTime();
            while (Utils_Timer::getCurrentTime() - delayStartTime < 500) {
                if (Utils_Timer::getCurrentTime() - startTime > TIMEOUT_MS) {
                    Utils_Logger::error("Timeout during success display");
                    break;
                }
                vTaskDelay(1);
            }
            m_tftManager->fillRectangle(80, 150, 160, 16, ST7789_BLACK);
        } else {
            // 显示中文保存失败提示
            int centerX = m_fontRenderer->calculateCenterPosition(240, m_fontRenderer->getSaveFailedString());
            m_fontRenderer->drawChineseString(centerX, 150, m_fontRenderer->getSaveFailedString(), ST7789_WHITE, ST7789_BLACK);
            
            // 带超时检查的显示延迟
            delayStartTime = Utils_Timer::getCurrentTime();
            while (Utils_Timer::getCurrentTime() - delayStartTime < 1000) {
                if (Utils_Timer::getCurrentTime() - startTime > TIMEOUT_MS) {
                    Utils_Logger::error("Timeout during failure display");
                    break;
                }
                vTaskDelay(1);
            }
            m_tftManager->fillRectangle(80, 150, 160, 16, ST7789_BLACK);
        }
    } else {
        Utils_Logger::error("  2. Error: Failed to capture image data");
        // 显示中文捕获失败提示
        int centerX = m_fontRenderer->calculateCenterPosition(240, m_fontRenderer->getCaptureFailedString());
        m_fontRenderer->drawChineseString(centerX, 150, m_fontRenderer->getCaptureFailedString(), ST7789_WHITE, ST7789_BLACK);
        
        // 带超时检查的显示延迟
        delayStartTime = Utils_Timer::getCurrentTime();
        while (Utils_Timer::getCurrentTime() - delayStartTime < 1000) {
            if (Utils_Timer::getCurrentTime() - startTime > TIMEOUT_MS) {
                Utils_Logger::error("Timeout during error display");
                break;
            }
            vTaskDelay(1);
        }
        m_tftManager->fillRectangle(60, 150, 200, 16, ST7789_BLACK);
    }

capture_cleanup:
    Camera.channelEnd(CHANNEL_STILL);
    Utils_Logger::info("  3. Still channel closed");

    m_isCapturing = false;
    m_captureRequested = false;
    Utils_Logger::info("<<< Photo capture completed");

    return m_stillBuffer.imgLen > 0;
}

bool CameraManager::savePhotoToSDCard(uint32_t imgAddr, uint32_t imgLen) {
    if (!m_sdCardManager || !m_sdCardManager->isInitialized()) {
        Utils_Logger::error("Save failed: SD card not initialized");
        return false;
    }

    char filename[50];
    m_sdCardManager->generatePhotoFileName(filename, sizeof(filename));

    // 从DS1307模块获取当前时间，用于设置文件最后修改时间
    DS1307_Time captureTime;
    bool timeSuccess = clockModule.readTime(captureTime);
    
    if (!timeSuccess) {
        Utils_Logger::error("Failed to read capture time from DS1307");
        // 如果读取失败，使用默认时间
        captureTime = {0, 0, 0, 1, 1, 1, 2026};
    }

    Utils_Logger::info("Saving to: %s", filename);

    // 添加SD卡写入超时处理
    const uint32_t TIMEOUT_MS = 3000; // 3秒超时
    uint32_t startTime = Utils_Timer::getCurrentTime();
    
    // 写入文件并设置最后修改时间
    int32_t bytesWritten = m_sdCardManager->writeFile(filename, (uint8_t *)imgAddr, imgLen, &captureTime);
    
    if (Utils_Timer::getCurrentTime() - startTime > TIMEOUT_MS) {
        Utils_Logger::error("SD card write timeout");
        return false;
    }

    if (bytesWritten == (int32_t)imgLen) {
        Utils_Logger::info("Save successful! Wrote %d bytes", bytesWritten);
        return true;
    } else {
        Utils_Logger::error("Save warning: Written bytes (%d) mismatch expected (%d)", bytesWritten, imgLen);
        return false;
    }
}

CameraManager::CameraState CameraManager::getState() const {
    return m_state;
}

bool CameraManager::isPreviewActive() const {
    return m_previewActive;
}

bool CameraManager::isCapturing() const {
    return m_isCapturing;
}

bool CameraManager::hasCaptureRequest() const {
    return m_captureRequested;
}

void CameraManager::requestCapture() {
    m_captureRequested = true;
}

void CameraManager::clearCaptureRequest() {
    m_captureRequested = false;
}

bool CameraManager::isInitialized() const {
    return m_initialized;
}

const CameraManager::ImageBuffer& CameraManager::getPreviewBuffer() const {
    return m_previewBuffer;
}

const CameraManager::ImageBuffer& CameraManager::getStillBuffer() const {
    return m_stillBuffer;
}

SDCardManager& CameraManager::getSDCardManager() {
    return *m_sdCardManager;
}
