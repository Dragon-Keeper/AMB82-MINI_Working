/*
 * Camera_CameraManager.cpp - 相机管理器实现
 * 封装相机设备管理和拍照功能
 * 阶段五：相机模块开发 - 相机设备管理
 */

#include "Camera_CameraManager.h"
#include "Shared_GlobalDefines.h"
#include "System_ConfigManager.h"
#include <cstring>

// 相机配置对象（在Camera.ino中定义）
extern VideoSetting configPreview;
extern VideoSetting configStill;

// 静态指针，用于JPEGDraw回调访问TFTManager
static Display_TFTManager* s_tftManagerForJPEG = nullptr;

// 预览画面显示区域配置（默认值，可通过setPreviewDisplayMode动态修改）
static int s_previewDisplayWidth = 320;     // 屏幕上的显示宽度（默认全屏，旋转后屏幕宽320）
static const int PREVIEW_DISPLAY_HEIGHT = 240;   // 屏幕上的显示高度
static const int FULL_SCREEN_WIDTH = 320;        // 全屏模式宽度（旋转后实际宽度=320）
static const int PANEL_MODE_WIDTH = 195;         // 参数设置面板模式宽度（与PANEL_X对齐，右侧留空间给面板）
static const int MCU_BLOCK_SIZE = 16;             // MCU块大小

// 帧缓冲区 - 使用最大尺寸（全屏）分配
static uint16_t s_frameBuffer[FULL_SCREEN_WIDTH * PREVIEW_DISPLAY_HEIGHT];
static bool s_frameBufferReady = false;

// JPEGDraw回调函数 - 将MCU块写入帧缓冲区（使用动态宽度）
static int CameraManager_JPEGDraw(JPEGDRAW *pDraw) {
    if (s_tftManagerForJPEG != nullptr) {
        int x = pDraw->x;
        int y = pDraw->y;
        int width = pDraw->iWidth;
        int height = pDraw->iHeight;
        
        // 使用动态配置的显示宽度
        if (x >= s_previewDisplayWidth) {
            return 1;
        }
        
        // 简单的边界检查
        if (y >= PREVIEW_DISPLAY_HEIGHT) {
            return 1;
        }
        
        if (x < 0) {
            width += x;
            x = 0;
        }
        
        if (width <= 0 || height <= 0) {
            return 1;
        }
        
        // 确保不超出屏幕边界
        int drawWidth = width;
        int drawHeight = height;
        
        if (x + drawWidth > s_previewDisplayWidth) {
            drawWidth = s_previewDisplayWidth - x;
        }
        
        if (y + drawHeight > PREVIEW_DISPLAY_HEIGHT) {
            drawHeight = PREVIEW_DISPLAY_HEIGHT - y;
        }
        
        // 将MCU块数据拷贝到帧缓冲区（使用动态宽度）
        for (int row = 0; row < drawHeight; row++) {
            int srcOffset = row * width;
            int dstOffset = (y + row) * s_previewDisplayWidth + x;
            
            for (int col = 0; col < drawWidth; col++) {
                s_frameBuffer[dstOffset + col] = pDraw->pPixels[srcOffset + col];
            }
        }
        
        s_frameBufferReady = true;
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
    , m_ispConfig(nullptr)
    , m_currentExposureMode(ISP_DEFAULT_EXPOSURE_MODE)
    , m_currentBrightness(ISP_DEFAULT_BRIGHTNESS)
    , m_currentContrast(ISP_DEFAULT_CONTRAST)
    , m_currentSaturation(ISP_DEFAULT_SATURATION)
    , m_currentAWBMode(ISP_DEFAULT_AWB_MODE)
    , m_ispInitialized(false)
    , m_voeReady(false)
    , m_ispConfigManager(nullptr)
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
    
    // 等待 VOE 硬件完全初始化
    // videoInit() 是异步的，内部会触发 video_pre_init_procedure
    // 从日志分析，该过程需要约 55ms (hal_voe_send2voe too long 54867us)
    // 为确保稳定，等待 200ms 让所有异步操作完成
    Utils_Timer::delayMs(200);
    m_voeReady = true;
    Utils_Logger::info("Camera hardware initialized");

    // 初始化 ISP 配置（只创建对象和加载参数，不立即应用到底层硬件）
    Utils_Logger::info("Initializing ISP configuration...");
    
    // 初始化 ISP 配置管理器
    m_ispConfigManager = new ISPConfigManager();
    if (m_ispConfigManager != nullptr) {
        m_ispConfigManager->init(*m_sdCardManager);
        
        // 优先从ConfigManager读取用户保存的参数（与UI面板保持一致）
        if (ConfigManager::isInitialized()) {
            m_currentExposureMode = ConfigManager::getValue(ConfigManager::CONFIG_EXPOSURE_MODE);
            m_currentBrightness = ConfigManager::getValue(ConfigManager::CONFIG_BRIGHTNESS);
            m_currentContrast = ConfigManager::getValue(ConfigManager::CONFIG_CONTRAST);
            m_currentSaturation = ConfigManager::getValue(ConfigManager::CONFIG_SATURATION);
            m_currentAWBMode = m_ispConfigManager->getAWBMode();
            
            Utils_Logger::info("Loaded parameters from ConfigManager:");
        } else {
            m_currentExposureMode = m_ispConfigManager->getExposureMode();
            m_currentBrightness = m_ispConfigManager->getBrightness();
            m_currentContrast = m_ispConfigManager->getContrast();
            m_currentSaturation = m_ispConfigManager->getSaturation();
            m_currentAWBMode = m_ispConfigManager->getAWBMode();
            
            Utils_Logger::info("Loaded parameters from ISPConfigManager (defaults)");
        }
    }
    
    // 创建 CameraSetting 对象但不应用参数（延迟到 VOE 就绪后）
    initISP();
    
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
    m_voeReady = false;
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

    // 重新配置视频通道，确保使用正确的配置（解决从视频模式返回后的栅格问题）
    // Utils_Logger::info("Reconfiguring camera channels for photo mode...");
    Camera.configVideoChannel(PREVIEW_CH, configPreview);
    Camera.configVideoChannel(STILL_CH, configStill);
    Camera.videoInit();
    
    // 等待 VOE 硬件就绪（videoInit() 异步初始化需要时间）
    Utils_Timer::delayMs(200);
    m_voeReady = true;

    Camera.channelEnd(CHANNEL_PREVIEW);
    Camera.channelBegin(CHANNEL_PREVIEW);

    // 在 channelBegin() 之后应用 ISP 参数，确保 VOE 完全就绪
    applyISPSettings();

    // Utils_Logger::info("Camera preview started");
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

void CameraManager::setPreviewDisplayMode(bool fullWidth) {
    if (fullWidth) {
        s_previewDisplayWidth = FULL_SCREEN_WIDTH;
        Utils_Logger::info("Preview display mode set to FULL SCREEN (%dx%d)", s_previewDisplayWidth, PREVIEW_DISPLAY_HEIGHT);
    } else {
        s_previewDisplayWidth = PANEL_MODE_WIDTH;
        Utils_Logger::info("Preview display mode set to PANEL MODE (%dx%d)", s_previewDisplayWidth, PREVIEW_DISPLAY_HEIGHT);
    }
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
        
        // 清空帧缓冲区（用黑色填充）
        memset(s_frameBuffer, 0, sizeof(s_frameBuffer));
        s_frameBufferReady = false;
        
        if (m_jpegDecoder->openFLASH((uint8_t *)m_previewBuffer.imgAddr, m_previewBuffer.imgLen, CameraManager_JPEGDraw)) {
            // 添加JPEG解码超时检查
            if (Utils_Timer::getCurrentTime() - startTime > TIMEOUT_MS) {
                Utils_Logger::info("JPEG decode timeout");
                m_jpegDecoder->close();
                return;
            }
            
            m_jpegDecoder->decode(0, 0, JPEG_SCALE_HALF);
            m_jpegDecoder->close();
            
            // 解码完成后，一次性将整个帧缓冲区发送到屏幕（使用动态宽度）
            if (s_frameBufferReady && s_tftManagerForJPEG != nullptr) {
                s_tftManagerForJPEG->drawBitmap(0, 0, s_previewDisplayWidth, PREVIEW_DISPLAY_HEIGHT, s_frameBuffer);
            }
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
    uint32_t delayStartTime = 0; // 提前声明，避免被 goto 跨越

    // 阶段二：拍照前应用 ISP 设置
    if (m_ispInitialized && m_ispConfig) {
        Utils_Logger::info("  0. Applying ISP settings...");
        applyISPSettings();
        
        // 等待 ISP 参数生效（100ms）
        uint32_t ispDelayStartTime = Utils_Timer::getCurrentTime();
        while (Utils_Timer::getCurrentTime() - ispDelayStartTime < 100) {
            if (Utils_Timer::getCurrentTime() - startTime > TIMEOUT_MS) {
                Utils_Logger::error("Capture timeout during ISP setup");
                goto capture_cleanup;
            }
            vTaskDelay(1);
        }
        Utils_Logger::info("     ISP settings applied successfully");
    }

    Camera.channelBegin(CHANNEL_STILL);
    Utils_Logger::info("  1. Still channel opened");
    
    // 带超时检查的延迟
    delayStartTime = Utils_Timer::getCurrentTime();
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

// ============================================
// ISP 配置方法实现 - 阶段一基础集成
// ============================================

void CameraManager::initISP() {
    if (m_ispInitialized) {
        // Utils_Logger::info("ISP already initialized");
        return;
    }

    // 创建 CameraSetting 对象（使用默认构造函数）
    m_ispConfig = new CameraSetting();
    if (m_ispConfig == nullptr) {
        Utils_Logger::error("Failed to create CameraSetting object");
        return;
    }

    // 不在此处应用 ISP 参数，延迟到 startPreview() 或 channelBegin() 之后
    // 这样可以确保 VOE 硬件完全就绪，避免 "VOE not init" 警告
    // applyISPSettings();  // 已移除：延迟应用

    m_ispInitialized = true;
    // Utils_Logger::info("ISP initialized (parameters will be applied later)");
}

void CameraManager::applyISPSettings() {
    if (m_ispConfig == nullptr) {
        return;
    }

    if (!m_voeReady) {
        return;
    }

    // 应用所有当前 ISP 设置
    m_ispConfig->setExposureMode(m_currentExposureMode);
    m_ispConfig->setBrightness(m_currentBrightness);
    m_ispConfig->setContrast(m_currentContrast);
    m_ispConfig->setSaturation(m_currentSaturation);
    m_ispConfig->setAWB(m_currentAWBMode);
}

bool CameraManager::isVOEReady() const {
    return m_voeReady;
}

void CameraManager::setVOEReady(bool ready) {
    m_voeReady = ready;
    // 不在此处立即调用 applyISPSettings()
    // 因为 videoInit() 是异步的，返回后 VOE 可能仍在初始化中
    // 应该由调用者在合适的时机（如 channelBegin() 之后）手动调用 applyISPSettings()
}

void CameraManager::setExposureMode(int mode) {
    if (!m_ispInitialized) {
        // Utils_Logger::info("ISP not initialized");
        return;
    }

    if (mode < 0 || mode > 1) {
        Utils_Logger::error("Invalid exposure mode: %d (must be 0 or 1)", mode);
        return;
    }

    m_currentExposureMode = mode;
    if (m_ispConfig && m_voeReady) {
        m_ispConfig->setExposureMode(mode);
    }
    // Utils_Logger::info("Exposure mode set to: %d", mode);
}

int CameraManager::getExposureMode() const {
    return m_currentExposureMode;
}

void CameraManager::setBrightness(int value) {
    if (!m_ispInitialized) {
        // Utils_Logger::info("ISP not initialized");
        return;
    }

    if (value < -64 || value > 64) {
        Utils_Logger::error("Invalid brightness: %d (must be -64~64)", value);
        return;
    }

    m_currentBrightness = value;
    if (m_ispConfig && m_voeReady) {
        m_ispConfig->setBrightness(value);
    }
    // Utils_Logger::info("Brightness set to: %d", value);
}

int CameraManager::getBrightness() const {
    return m_currentBrightness;
}

void CameraManager::setContrast(int value) {
    if (!m_ispInitialized) {
        // Utils_Logger::info("ISP not initialized");
        return;
    }

    if (value < 0 || value > 100) {
        Utils_Logger::error("Invalid contrast: %d (must be 0~100)", value);
        return;
    }

    m_currentContrast = value;
    if (m_ispConfig && m_voeReady) {
        m_ispConfig->setContrast(value);
    }
    // Utils_Logger::info("Contrast set to: %d", value);
}

int CameraManager::getContrast() const {
    return m_currentContrast;
}

void CameraManager::setSaturation(int value) {
    if (!m_ispInitialized) {
        // Utils_Logger::info("ISP not initialized");
        return;
    }

    if (value < 0 || value > 100) {
        Utils_Logger::error("Invalid saturation: %d (must be 0~100)", value);
        return;
    }

    m_currentSaturation = value;
    if (m_ispConfig && m_voeReady) {
        m_ispConfig->setSaturation(value);
    }
    // Utils_Logger::info("Saturation set to: %d", value);
}

int CameraManager::getSaturation() const {
    return m_currentSaturation;
}

void CameraManager::setAWBMode(int mode) {
    if (!m_ispInitialized) {
        // Utils_Logger::info("ISP not initialized");
        return;
    }

    if (mode < 0 || mode > 1) {
        Utils_Logger::error("Invalid AWB mode: %d (must be 0 or 1)", mode);
        return;
    }

    m_currentAWBMode = mode;
    if (m_ispConfig && m_voeReady) {
        m_ispConfig->setAWB(mode);
    }
    // Utils_Logger::info("AWB mode set to: %d", mode);
}

int CameraManager::getAWBMode() const {
    return m_currentAWBMode;
}

void CameraManager::resetISP() {
    if (!m_ispInitialized) {
        Utils_Logger::info("ISP not initialized");
        return;
    }

    // 重置为默认值
    m_currentExposureMode = ISP_DEFAULT_EXPOSURE_MODE;
    m_currentBrightness = ISP_DEFAULT_BRIGHTNESS;
    m_currentContrast = ISP_DEFAULT_CONTRAST;
    m_currentSaturation = ISP_DEFAULT_SATURATION;
    m_currentAWBMode = ISP_DEFAULT_AWB_MODE;

    // 应用默认设置
    applyISPSettings();

    Utils_Logger::info("ISP settings reset to defaults");
}

