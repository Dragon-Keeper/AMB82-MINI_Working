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

    Camera.getImage(CHANNEL_PREVIEW, &m_previewBuffer.imgAddr, &m_previewBuffer.imgLen);

    if (m_previewBuffer.imgLen > 0) {
        if (m_jpegDecoder->openFLASH((uint8_t *)m_previewBuffer.imgAddr, m_previewBuffer.imgLen, CameraManager_JPEGDraw)) {
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

    Camera.channelBegin(CHANNEL_STILL);
    Utils_Logger::info("  1. Still channel opened");
    Utils_Timer::delayMs(200);

    Camera.getImage(CHANNEL_STILL, &m_stillBuffer.imgAddr, &m_stillBuffer.imgLen);

    if (m_stillBuffer.imgLen > 0) {
        Utils_Logger::info("  2. Image captured successfully, size: %d bytes", m_stillBuffer.imgLen);

        if (savePhotoToSDCard(m_stillBuffer.imgAddr, m_stillBuffer.imgLen)) {
            m_fontRenderer->showCameraStatus(m_fontRenderer->getPhotoSuccessString(), 120, 150);
            Utils_Timer::delayMs(500);
            m_tftManager->fillRectangle(80, 150, 160, 16, ST7789_BLACK);
        } else {
            m_fontRenderer->showCameraStatus(m_fontRenderer->getSaveFailedString(), 80, 150);
            Utils_Timer::delayMs(1000);
            m_tftManager->fillRectangle(80, 150, 160, 16, ST7789_BLACK);
        }
    } else {
        Utils_Logger::error("  2. Error: Failed to capture image data");
        m_fontRenderer->showCameraStatus(m_fontRenderer->getCaptureFailedString(), 60, 150);
        Utils_Timer::delayMs(1000);
        m_tftManager->fillRectangle(60, 150, 200, 16, ST7789_BLACK);
    }

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

    Utils_Logger::info("Saving to: %s", filename);

    int32_t bytesWritten = m_sdCardManager->writeFile(filename, (uint8_t *)imgAddr, imgLen);

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

const CameraManager::ImageBuffer& CameraManager::getPreviewBuffer() const {
    return m_previewBuffer;
}

const CameraManager::ImageBuffer& CameraManager::getStillBuffer() const {
    return m_stillBuffer;
}

SDCardManager& CameraManager::getSDCardManager() {
    return *m_sdCardManager;
}
