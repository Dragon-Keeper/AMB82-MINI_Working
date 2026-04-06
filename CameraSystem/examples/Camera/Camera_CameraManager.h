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
#include "ISP_ConfigManager.h"

// ISP 参数默认值定义
#define ISP_DEFAULT_EXPOSURE_MODE 1       // 1: 自动曝光
#define ISP_DEFAULT_BRIGHTNESS 0          // 0: 默认亮度 (-64~64)
#define ISP_DEFAULT_CONTRAST 50           // 50: 默认对比度 (0~100)
#define ISP_DEFAULT_SATURATION 50         // 50: 默认饱和度 (0~100)
#define ISP_DEFAULT_AWB_MODE 1             // 1: 自动白平衡

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

    // 设置预览显示模式（全屏或左侧2/3）
    // fullWidth: true=全屏显示(240x240), false=左侧2/3显示(195x240，用于参数设置面板)
    void setPreviewDisplayMode(bool fullWidth);

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
    bool isInitialized() const; // 新增方法：检查相机管理器是否已初始化

    // ISP 配置方法 - 阶段一基础集成
    void initISP();
    void setExposureMode(int mode);
    int getExposureMode() const;
    void setBrightness(int value);
    int getBrightness() const;
    void setContrast(int value);
    int getContrast() const;
    void setSaturation(int value);
    int getSaturation() const;
    void setAWBMode(int mode);
    int getAWBMode() const;
    void resetISP();
    void applyISPSettings();

    // VOE 状态管理
    bool isVOEReady() const;
    void setVOEReady(bool ready);

private:

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

    // ISP 配置成员变量 - 阶段一基础集成
    CameraSetting* m_ispConfig;
    int m_currentExposureMode;
    int m_currentBrightness;
    int m_currentContrast;
    int m_currentSaturation;
    int m_currentAWBMode;
    bool m_ispInitialized;
    bool m_voeReady;

    // ISP 配置管理器 - 阶段三集成
    ISPConfigManager* m_ispConfigManager;
};

#endif
