/*
 * Camera_CameraCore.cpp - 相机核心驱动实现
 * 负责相机硬件初始化和基本操作
 * 阶段五：相机模块开发 - 相机核心驱动
 */

#include "Camera_CameraCore.h"

/**
 * 构造函数
 */
CameraCore::CameraCore() :
    m_initialized(false),
    m_previewChannelActive(false),
    m_stillChannelActive(false)
{
    m_lastError[0] = '\0';
}

/**
 * 析构函数
 */
CameraCore::~CameraCore()
{
    cleanup();
}

/**
 * 初始化相机系统
 * @return 初始化成功返回true，失败返回false
 */
bool CameraCore::init()
{
    Utils_Logger::info("初始化相机系统...");
    
    // 配置默认的预览通道
    VideoConfig previewConfig = getDefaultPreviewConfig();
    if (!configChannel(CHANNEL_PREVIEW, previewConfig)) {
        setError("配置预览通道失败");
        return false;
    }
    
    // 配置默认的拍照通道
    VideoConfig stillConfig = getDefaultStillConfig();
    if (!configChannel(CHANNEL_STILL, stillConfig)) {
        setError("配置拍照通道失败");
        return false;
    }
    
    // 初始化摄像头硬件
    Camera.videoInit();
    
    m_initialized = true;
    Utils_Logger::info("相机系统初始化完成");
    return true;
}

/**
 * 配置视频通道
 * @param ch 通道ID
 * @param config 通道配置
 * @return 配置成功返回true，失败返回false
 */
bool CameraCore::configChannel(VideoChannel ch, const VideoConfig& config)
{
    if (!validateChannel(ch)) {
        setError("无效的视频通道");
        return false;
    }
    
    Utils_Logger::info("配置视频通道 %d: 分辨率=%d, 帧率=%d, 格式=%d", 
                      ch, config.resolution, config.fps, config.format);
    
    // 创建VideoSetting对象并配置通道
    VideoSetting setting(config.resolution, config.fps, config.format, config.bitrate);
    Camera.configVideoChannel(ch, setting);
    
    Utils_Logger::info("视频通道 %d 配置完成", ch);
    return true;
}

/**
 * 启动视频通道
 * @param ch 通道ID
 * @return 启动成功返回true，失败返回false
 */
bool CameraCore::startChannel(VideoChannel ch)
{
    if (!validateChannel(ch)) {
        setError("无效的视频通道");
        return false;
    }
    
    if (!m_initialized) {
        setError("相机系统未初始化");
        return false;
    }
    
    Utils_Logger::info("启动视频通道 %d", ch);
    Camera.channelBegin(ch);
    
    // 更新通道状态
    if (ch == CHANNEL_PREVIEW) {
        m_previewChannelActive = true;
    } else if (ch == CHANNEL_STILL) {
        m_stillChannelActive = true;
    }
    
    Utils_Logger::info("视频通道 %d 启动成功", ch);
    return true;
}

/**
 * 停止视频通道
 * @param ch 通道ID
 * @return 停止成功返回true，失败返回false
 */
bool CameraCore::stopChannel(VideoChannel ch)
{
    if (!validateChannel(ch)) {
        setError("无效的视频通道");
        return false;
    }
    
    Utils_Logger::info("停止视频通道 %d", ch);
    Camera.channelEnd(ch);
    
    // 更新通道状态
    if (ch == CHANNEL_PREVIEW) {
        m_previewChannelActive = false;
    } else if (ch == CHANNEL_STILL) {
        m_stillChannelActive = false;
    }
    
    Utils_Logger::info("视频通道 %d 停止成功", ch);
    return true;
}

/**
 * 检查通道是否处于活动状态
 * @param ch 通道ID
 * @return 通道活动返回true，否则返回false
 */
bool CameraCore::isChannelActive(VideoChannel ch) const
{
    if (ch == CHANNEL_PREVIEW) {
        return m_previewChannelActive;
    } else if (ch == CHANNEL_STILL) {
        return m_stillChannelActive;
    }
    return false;
}

/**
 * 捕获图像
 * @param ch 通道ID
 * @param imgAddr 图像地址指针
 * @param imgLen 图像长度指针
 * @return 捕获成功返回true，失败返回false
 */
bool CameraCore::captureImage(VideoChannel ch, uint32_t* imgAddr, uint32_t* imgLen)
{
    if (!validateChannel(ch)) {
        setError("无效的视频通道");
        return false;
    }
    
    if (!isChannelActive(ch)) {
        setError("视频通道未启动");
        return false;
    }
    
    Camera.getImage(ch, imgAddr, imgLen);
    
    if (*imgLen > 0) {
        return true;
    } else {
        setError("未能捕获图像数据");
        return false;
    }
}

/**
 * 设置曝光参数
 * @param value 曝光值
 * @return 设置成功返回true，失败返回false
 */
bool CameraCore::setExposure(int32_t value)
{
    // 预留接口，当前版本暂不实现
    Utils_Logger::info("设置曝光参数: %d", value);
    return true;
}

/**
 * 设置白平衡参数
 * @param value 白平衡值
 * @return 设置成功返回true，失败返回false
 */
bool CameraCore::setWhiteBalance(int32_t value)
{
    // 预留接口，当前版本暂不实现
    Utils_Logger::info("设置白平衡参数: %d", value);
    return true;
}

/**
 * 设置对焦参数
 * @param value 对焦值
 * @return 设置成功返回true，失败返回false
 */
bool CameraCore::setFocus(int32_t value)
{
    // 预留接口，当前版本暂不实现
    Utils_Logger::info("设置对焦参数: %d", value);
    return true;
}

/**
 * 检查相机是否已初始化
 * @return 已初始化返回true，否则返回false
 */
bool CameraCore::isInitialized() const
{
    return m_initialized;
}

/**
 * 获取最后一条错误信息
 * @return 错误信息字符串
 */
const char* CameraCore::getLastError() const
{
    return m_lastError;
}

/**
 * 清理相机资源
 */
void CameraCore::cleanup()
{
    if (m_previewChannelActive) {
        stopChannel(CHANNEL_PREVIEW);
    }
    
    if (m_stillChannelActive) {
        stopChannel(CHANNEL_STILL);
    }
    
    m_initialized = false;
    Utils_Logger::info("相机资源清理完成");
}

/**
 * 获取默认预览配置
 * @return 预览通道配置
 */
CameraCore::VideoConfig CameraCore::getDefaultPreviewConfig()
{
    VideoConfig config;
    config.resolution = VIDEO_VGA;
    config.fps = CAM_FPS;
    config.format = VIDEO_JPEG;
    config.bitrate = 1;
    return config;
}

/**
 * 获取默认拍照配置
 * @return 拍照通道配置
 */
CameraCore::VideoConfig CameraCore::getDefaultStillConfig()
{
    VideoConfig config;
    config.resolution = VIDEO_HD;
    config.fps = CAM_FPS;
    config.format = VIDEO_JPEG;
    config.bitrate = 1;
    return config;
}

/**
 * 设置错误信息
 * @param error 错误信息
 */
void CameraCore::setError(const char* error)
{
    strncpy(m_lastError, error, sizeof(m_lastError) - 1);
    m_lastError[sizeof(m_lastError) - 1] = '\0';
    Utils_Logger::error("相机错误: %s", error);
}

/**
 * 验证通道ID是否有效
 * @param ch 通道ID
 * @return 有效返回true，无效返回false
 */
bool CameraCore::validateChannel(VideoChannel ch) const
{
    return (ch == CHANNEL_PREVIEW || ch == CHANNEL_STILL);
}