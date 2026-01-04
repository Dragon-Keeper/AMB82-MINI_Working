/*
 * Camera_CameraManager.h - 相机管理器头文件
 * 封装相机设备管理和拍照功能
 * 阶段五：相机模块开发 - 相机设备管理
 */

#ifndef CAMERA_CAMERA_MANAGER_H
#define CAMERA_CAMERA_MANAGER_H

#include <Arduino.h>
#include "VideoStream.h"
#include "Display_TFTManager.h"
#include "Display_FontRenderer.h"
#include "Utils_Logger.h"
#include "Utils_Timer.h"
#include <JPEGDEC.h>
#include "Camera_SDCardManager.h"

class CameraManager {
public:
    typedef enum {
        CHANNEL_PREVIEW = 0,
        CHANNEL_STILL = 1,
        CHANNEL_MAX
    } VideoChannel;

    typedef enum {
        STATE_IDLE = 0,
        STATE_PREVIEW = 1,
        STATE_CAPTURING = 2
    } CameraState;

    typedef struct {
        uint32_t imgAddr;
        uint32_t imgLen;
    } ImageBuffer;

    CameraManager();

    bool init(Display_TFTManager& tftMgr, Display_FontRenderer& fontRenderer, JPEGDEC& jpegDecoder);
    void cleanup();

    bool startPreview();
    void stopPreview();
    void processPreviewFrame();

    bool capturePhoto();
    bool savePhotoToSDCard(uint32_t imgAddr, uint32_t imgLen);

    CameraState getState() const;
    bool isPreviewActive() const;
    bool isCapturing() const;
    bool hasCaptureRequest() const;
    void requestCapture();
    void clearCaptureRequest();

    const ImageBuffer& getPreviewBuffer() const;
    const ImageBuffer& getStillBuffer() const;

    SDCardManager& getSDCardManager();

private:
    Display_TFTManager* m_tftManager;
    Display_FontRenderer* m_fontRenderer;
    JPEGDEC* m_jpegDecoder;
    SDCardManager* m_sdCardManager;

    CameraState m_state;
    bool m_previewActive;
    bool m_isCapturing;
    bool m_captureRequested;

    ImageBuffer m_previewBuffer;
    ImageBuffer m_stillBuffer;

    bool m_initialized;
};

#endif
