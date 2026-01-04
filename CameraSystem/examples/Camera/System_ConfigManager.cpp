/*
 * System_ConfigManager.cpp - 配置管理器实现
 * 实现配置项管理、持久化、验证和版本控制
 * 模块化移植：阶段八 - 菜单模块
 */

#include "System_ConfigManager.h"
#include <string.h>

// Static member variables initialization
ConfigManager::ConfigData ConfigManager::m_configData;
const int32_t ConfigManager::m_defaultValues[CONFIG_MAX] = {
    1280,    // CONFIG_CAMERA_RESOLUTION - Default: 1280x720
    80,      // CONFIG_CAMERA_QUALITY - Default: 80%
    150,     // CONFIG_DISPLAY_BRIGHTNESS - Default: 150/255
    2,       // CONFIG_ENCODER_SENSITIVITY - Default: 2 steps per click
    30000    // CONFIG_SYSTEM_TIMEOUT - Default: 30 seconds
};
bool ConfigManager::m_isInitialized = false;

// Private constructor (not used in static class pattern)
ConfigManager::ConfigManager() {
}

// Initialization
bool ConfigManager::init() {
    if (m_isInitialized) {
        Utils_Logger::info("ConfigManager already initialized");
        return true;
    }
    
    Utils_Logger::info("Initializing ConfigManager");
    
    // Try to load configuration from flash
    if (!loadFromFlash()) {
        Utils_Logger::info("Failed to load configuration, using defaults");
        restoreDefaults();
    }
    
    m_isInitialized = true;
    Utils_Logger::info("ConfigManager initialized successfully");
    return true;
}

// Configuration read/write
bool ConfigManager::setValue(ConfigID id, int32_t value) {
    if (!m_isInitialized || !isValidConfigID(id)) {
        Utils_Logger::info("Invalid ConfigID or ConfigManager not initialized");
        return false;
    }
    
    // Store integer value in configuration data
    memcpy(&m_configData.data[id * sizeof(int32_t)], &value, sizeof(int32_t));
    updateLastModified();
    
    Utils_Logger::info("Set configuration value: %d = %d", id, value);
    return true;
}

int32_t ConfigManager::getValue(ConfigID id) {
    if (!m_isInitialized || !isValidConfigID(id)) {
        Utils_Logger::info("Invalid ConfigID or ConfigManager not initialized");
        return 0;
    }
    
    int32_t value;
    memcpy(&value, &m_configData.data[id * sizeof(int32_t)], sizeof(int32_t));
    return value;
}

bool ConfigManager::setString(ConfigID id, const char* value) {
    if (!m_isInitialized || !isValidConfigID(id) || value == nullptr) {
        Utils_Logger::info("Invalid ConfigID, ConfigManager not initialized, or null string");
        return false;
    }
    
    // Calculate offset for string storage
    uint32_t offset = CONFIG_MAX * sizeof(int32_t);
    
    // Store string length first
    uint32_t length = strlen(value);
    memcpy(&m_configData.data[offset + id * 4], &length, sizeof(uint32_t));
    
    // Store string data
    if (length > 0) {
        memcpy(&m_configData.data[offset + CONFIG_MAX * 4 + id * 32], value, length);
    }
    
    updateLastModified();
    
    Utils_Logger::info("Set configuration string: %d = %s", id, value);
    return true;
}

const char* ConfigManager::getString(ConfigID id, char* buffer, uint32_t size) {
    if (!m_isInitialized || !isValidConfigID(id) || buffer == nullptr || size == 0) {
        Utils_Logger::info("Invalid parameters or ConfigManager not initialized");
        return nullptr;
    }
    
    // Calculate offset for string storage
    uint32_t offset = CONFIG_MAX * sizeof(int32_t);
    
    // Get string length
    uint32_t length;
    memcpy(&length, &m_configData.data[offset + id * 4], sizeof(uint32_t));
    
    if (length == 0) {
        buffer[0] = '\0';
        return buffer;
    }
    
    // Copy string data to buffer
    if (length + 1 > size) {
        Utils_Logger::info("Buffer size too small for string");
        return nullptr;
    }
    
    memcpy(buffer, &m_configData.data[offset + CONFIG_MAX * 4 + id * 32], length);
    buffer[length] = '\0';
    
    return buffer;
}

// Configuration persistence
bool ConfigManager::saveToFlash() {
    if (!m_isInitialized) {
        Utils_Logger::info("ConfigManager not initialized");
        return false;
    }
    
    // Calculate checksum before saving
    m_configData.checksum = calculateChecksum();
    
    // In a real implementation, this would write to flash memory
    // For this example, we'll simulate writing to flash
    Utils_Logger::info("Saving configuration to flash (simulated)");
    Utils_Logger::info("Configuration version: %d", m_configData.version);
    Utils_Logger::info("Configuration checksum: %d", m_configData.checksum);
    Utils_Logger::info("Last modified: %d", m_configData.lastModified);
    
    return true;
}

bool ConfigManager::loadFromFlash() {
    // In a real implementation, this would read from flash memory
    // For this example, we'll simulate reading from flash
    Utils_Logger::info("Loading configuration from flash (simulated)");
    
    // Simulate successful load
    if (m_configData.version == 0) {
        Utils_Logger::info("No valid configuration found in flash");
        return false;
    }
    
    // Validate configuration
    if (!validateConfig()) {
        Utils_Logger::info("Invalid configuration found in flash");
        return false;
    }
    
    Utils_Logger::info("Configuration loaded successfully");
    return true;
}

bool ConfigManager::restoreDefaults() {
    if (!m_isInitialized) {
        Utils_Logger::info("ConfigManager not initialized");
        return false;
    }
    
    Utils_Logger::info("Restoring default configuration");
    
    // Reset configuration data
    m_configData.version = 1;
    m_configData.checksum = 0;
    m_configData.lastModified = 0;
    memset(m_configData.data, 0, sizeof(m_configData.data));
    
    // Set default values
    for (uint32_t i = 0; i < CONFIG_MAX; i++) {
        setValue(static_cast<ConfigID>(i), m_defaultValues[i]);
    }
    
    updateLastModified();
    
    // Save defaults to flash
    return saveToFlash();
}

// Configuration validation
bool ConfigManager::validateConfig() {
    if (m_configData.version == 0) {
        return false;
    }
    
    uint32_t calculatedChecksum = calculateChecksum();
    if (m_configData.checksum != calculatedChecksum) {
        Utils_Logger::info("Configuration checksum mismatch: expected %d, got %d", m_configData.checksum, calculatedChecksum);
        return false;
    }
    
    return true;
}

uint32_t ConfigManager::calculateChecksum() {
    uint32_t checksum = 0;
    
    // Calculate checksum for configuration data (excluding checksum field itself)
    const uint8_t* data = reinterpret_cast<const uint8_t*>(&m_configData);
    uint32_t size = sizeof(m_configData);
    
    for (uint32_t i = 0; i < size; i++) {
        // Skip the checksum field
        if (i >= offsetof(ConfigData, checksum) && i < offsetof(ConfigData, checksum) + sizeof(m_configData.checksum)) {
            continue;
        }
        
        checksum += data[i];
    }
    
    return checksum;
}

// Configuration information
uint32_t ConfigManager::getConfigVersion() {
    if (!m_isInitialized) {
        Utils_Logger::info("ConfigManager not initialized");
        return 0;
    }
    
    return m_configData.version;
}

uint32_t ConfigManager::getLastModified() {
    if (!m_isInitialized) {
        Utils_Logger::info("ConfigManager not initialized");
        return 0;
    }
    
    return m_configData.lastModified;
}

// Cleanup
void ConfigManager::cleanup() {
    if (!m_isInitialized) {
        return;
    }
    
    Utils_Logger::info("Cleaning up ConfigManager");
    
    // Save configuration before cleanup
    saveToFlash();
    
    m_isInitialized = false;
}

// Helper functions
void ConfigManager::updateLastModified() {
    // In a real implementation, this would use a real timestamp
    // For this example, we'll use a simple counter
    static uint32_t counter = 0;
    m_configData.lastModified = ++counter;
}

bool ConfigManager::isValidConfigID(ConfigID id) {
    return (id >= 0 && id < CONFIG_MAX);
}