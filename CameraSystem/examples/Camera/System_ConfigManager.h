/*
 * System_ConfigManager.h - 配置管理器
 * 实现配置项管理、持久化、验证和版本控制
 * 模块化移植：阶段八 - 菜单模块
 */

#ifndef SYSTEM_CONFIGMANAGER_H
#define SYSTEM_CONFIGMANAGER_H

#include "Shared_Types.h"
#include "Utils_Logger.h"

class ConfigManager {
public:
    typedef enum {
        CONFIG_CAMERA_RESOLUTION = 0,
        CONFIG_CAMERA_QUALITY = 1,
        CONFIG_DISPLAY_BRIGHTNESS = 2,
        CONFIG_ENCODER_SENSITIVITY = 3,
        CONFIG_SYSTEM_TIMEOUT = 4,
        CONFIG_MAX
    } ConfigID;
    
    typedef struct {
        uint32_t version;
        uint32_t checksum;
        uint32_t lastModified;
        uint8_t data[256];  // Configuration data
    } ConfigData;
    
    // Initialization
    static bool init();
    
    // Configuration read/write
    static bool setValue(ConfigID id, int32_t value);
    static int32_t getValue(ConfigID id);
    static bool setString(ConfigID id, const char* value);
    static const char* getString(ConfigID id, char* buffer, uint32_t size);
    
    // Configuration persistence
    static bool saveToFlash();
    static bool loadFromFlash();
    static bool restoreDefaults();
    
    // Configuration validation
    static bool validateConfig();
    static uint32_t calculateChecksum();
    
    // Configuration information
    static uint32_t getConfigVersion();
    static uint32_t getLastModified();
    
    // Cleanup
    static void cleanup();
    
private:
    // Private constructor to prevent instantiation
    ConfigManager();
    
    // Static configuration data
    static ConfigData m_configData;
    
    // Default values
    static const int32_t m_defaultValues[CONFIG_MAX];
    
    // Configuration initialized flag
    static bool m_isInitialized;
    
    // Helper functions
    static void updateLastModified();
    static bool isValidConfigID(ConfigID id);
};

#endif // SYSTEM_CONFIGMANAGER_H