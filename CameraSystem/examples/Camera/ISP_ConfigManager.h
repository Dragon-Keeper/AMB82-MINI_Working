/*
 * ISP_ConfigManager.h - ISP配置管理器头文件
 * 管理ISP参数配置和配置文件读写
 * 阶段三：ISP配置界面开发 - 参数管理
 */

#ifndef ISP_CONFIG_MANAGER_H
#define ISP_CONFIG_MANAGER_H

#include <Arduino.h>
#include "Utils_Logger.h"
#include "Camera_SDCardManager.h"

// ISP 参数配置结构
typedef struct {
    int exposureMode;      // 曝光模式：0=手动, 1=自动
    int brightness;        // 亮度：-64 ~ 64
    int contrast;          // 对比度：0 ~ 100
    int saturation;        // 饱和度：0 ~ 100
    int awbMode;           // 白平衡模式：0=手动, 1=自动
} ISPConfig;

// ISP 参数范围定义
#define ISP_EXPOSURE_MODE_MIN 0
#define ISP_EXPOSURE_MODE_MAX 1
#define ISP_EXPOSURE_MODE_DEFAULT 1

#define ISP_BRIGHTNESS_MIN -64
#define ISP_BRIGHTNESS_MAX 64
#define ISP_BRIGHTNESS_DEFAULT 0

#define ISP_CONTRAST_MIN 0
#define ISP_CONTRAST_MAX 100
#define ISP_CONTRAST_DEFAULT 50

#define ISP_SATURATION_MIN 0
#define ISP_SATURATION_MAX 100
#define ISP_SATURATION_DEFAULT 50

#define ISP_AWB_MODE_MIN 0
#define ISP_AWB_MODE_MAX 1
#define ISP_AWB_MODE_DEFAULT 1

// 配置文件路径
#define ISP_CONFIG_FILE_PATH "0:/ISPControl.ini"

class ISPConfigManager {
private:
    ISPConfig currentConfig;      // 当前配置
    ISPConfig defaultConfig;      // 默认配置
    bool initialized = false;
    SDCardManager* m_sdCardManager; // SD卡管理器指针

    // 初始化默认配置
    void initDefaultConfig();

public:
    // 构造函数
    ISPConfigManager();

    // 初始化配置管理器
    bool init(SDCardManager& sdCardManager);

    // 从文件加载配置
    bool loadConfig();

    // 保存配置到文件
    bool saveConfig();

    // 重置为默认配置
    void resetToDefault();

    // 获取当前配置
    const ISPConfig& getConfig() const { return currentConfig; }

    // 设置完整配置
    void setConfig(const ISPConfig& config);

    // 设置单个参数
    void setExposureMode(int mode);
    void setBrightness(int value);
    void setContrast(int value);
    void setSaturation(int value);
    void setAWBMode(int mode);

    // 获取单个参数
    int getExposureMode() const { return currentConfig.exposureMode; }
    int getBrightness() const { return currentConfig.brightness; }
    int getContrast() const { return currentConfig.contrast; }
    int getSaturation() const { return currentConfig.saturation; }
    int getAWBMode() const { return currentConfig.awbMode; }

    // 验证参数范围
    static bool validateExposureMode(int mode);
    static bool validateBrightness(int value);
    static bool validateContrast(int value);
    static bool validateSaturation(int value);
    static bool validateAWBMode(int mode);

    // 获取参数名称
    static const char* getExposureModeLabel(int mode);
    static const char* getAWBModeLabel(int mode);

    // 清理资源
    void cleanup();
};

#endif // ISP_CONFIG_MANAGER_H
