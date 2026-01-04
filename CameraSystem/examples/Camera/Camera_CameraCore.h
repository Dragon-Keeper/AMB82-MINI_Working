/*
 * Camera_CameraCore.h - 相机核心驱动头文件
 * 负责相机硬件初始化和基本操作
 * 阶段五：相机模块开发 - 相机核心驱动
 */

#ifndef CAMERA_CAMERACORE_H
#define CAMERA_CAMERACORE_H

#include <Arduino.h>
#include "VideoStream.h"
#include "Shared_GlobalDefines.h"
#include "Utils_Logger.h"

/**
 * 相机核心驱动模块
 * 负责相机硬件初始化和基本操作
 */
class CameraCore {
public:
    // 视频通道枚举
    typedef enum {
        CHANNEL_PREVIEW = PREVIEW_CH,  // 预览通道：VGA
        CHANNEL_STILL = STILL_CH       // 拍照通道：720p
    } VideoChannel;
    
    // 视频配置结构体
    typedef struct {
        uint32_t resolution;   // 分辨率（VIDEO_VGA, VIDEO_HD等）
        uint32_t fps;          // 帧率（CAM_FPS）
        uint32_t format;       // 格式（VIDEO_JPEG）
        uint32_t bitrate;      // 比特率
    } VideoConfig;
    
    // 构造函数和析构函数
    CameraCore();
    ~CameraCore();
    
    // 初始化函数
    bool init();
    
    // 通道管理
    bool configChannel(VideoChannel ch, const VideoConfig& config);
    bool startChannel(VideoChannel ch);
    bool stopChannel(VideoChannel ch);
    bool isChannelActive(VideoChannel ch) const;
    
    // 图像捕获
    bool captureImage(VideoChannel ch, uint32_t* imgAddr, uint32_t* imgLen);
    
    // 相机参数控制
    bool setExposure(int32_t value);
    bool setWhiteBalance(int32_t value);
    bool setFocus(int32_t value);
    
    // 状态查询
    bool isInitialized() const;
    const char* getLastError() const;
    
    // 清理
    void cleanup();
    
    // 获取默认配置
    static VideoConfig getDefaultPreviewConfig();
    static VideoConfig getDefaultStillConfig();
    
private:
    // 相机状态
    bool m_initialized;
    bool m_previewChannelActive;
    bool m_stillChannelActive;
    
    // 错误信息
    char m_lastError[100];
    
    // 私有方法
    void setError(const char* error);
    bool validateChannel(VideoChannel ch) const;
};

#endif // CAMERA_CAMERACORE_H