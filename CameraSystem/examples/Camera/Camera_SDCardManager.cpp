/*
 * Camera_SDCardManager.cpp - SD卡文件系统管理器实现
 * 封装SD卡初始化、文件读写和存储空间管理功能
 * 阶段五：相机模块开发 - SD卡管理
 */

#include "Camera_SDCardManager.h"
#include "Utils_Logger.h"
#include "Utils_Timer.h"
#include "DS1307_ClockModule.h"
#include <stdio.h>

// 外部全局DS1307时钟模块实例
extern DS1307_ClockModule clockModule;

SDCardManager::SDCardManager()
    : m_initialized(false)
{
    memset(m_rootPath, 0, sizeof(m_rootPath));
}

bool SDCardManager::init() {
    if (m_initialized) {
        Utils_Logger::error("SDCardManager already initialized");
        return true;
    }

    Utils_Logger::info("Initializing SD card...");

    if (!m_fs.begin()) {
        Utils_Logger::error("SD card initialization failed");
        m_initialized = false;
        return false;
    }

    m_initialized = true;
    Utils_Logger::info("SD card initialized successfully");
    Utils_Logger::info("Root path: %s", m_fs.getRootPath());
    
    strcpy(m_rootPath, m_fs.getRootPath());

    return true;
}

void SDCardManager::cleanup() {
    if (m_initialized) {
        Utils_Logger::info("Cleaning up SD card manager");
        m_fs.end();
        m_initialized = false;
    }
}

bool SDCardManager::isInitialized() const {
    return m_initialized;
}

bool SDCardManager::fileExists(const char* path) {
    if (!m_initialized) {
        Utils_Logger::error("SDCardManager not initialized");
        return false;
    }

    File file = m_fs.open(path);
    if (file) {
        file.close();
        return true;
    }
    return false;
}

bool SDCardManager::createFile(const char* path) {
    if (!m_initialized) {
        Utils_Logger::error("SDCardManager not initialized");
        return false;
    }

    File file = m_fs.open(path);
    if (!file) {
        Utils_Logger::error("Cannot create file: %s", path);
        return false;
    }
    file.close();
    return true;
}

bool SDCardManager::deleteFile(const char* path) {
    if (!m_initialized) {
        Utils_Logger::error("SDCardManager not initialized");
        return false;
    }

    if (m_fs.remove(path)) {
        Utils_Logger::info("File deleted: %s", path);
        return true;
    } else {
        Utils_Logger::error("Failed to delete file: %s", path);
        return false;
    }
}

bool SDCardManager::renameFile(const char* oldPath, const char* newPath) {
    if (!m_initialized) {
        Utils_Logger::error("SDCardManager not initialized");
        return false;
    }

    if (m_fs.rename(oldPath, newPath)) {
        Utils_Logger::info("File renamed: %s -> %s", oldPath, newPath);
        return true;
    } else {
        Utils_Logger::error("Failed to rename file: %s", oldPath);
        return false;
    }
}

int32_t SDCardManager::writeFile(const char* path, const uint8_t* data, uint32_t size, const DS1307_Time* time) {
    if (!m_initialized) {
        Utils_Logger::error("SDCardManager not initialized");
        return -1;
    }

    File file = m_fs.open(path);
    if (!file) {
        Utils_Logger::error("Cannot create file: %s", path);
        return -1;
    }

    int32_t bytesWritten = file.write(data, size);
    file.close();

    // 如果提供了时间参数，设置文件最后修改时间
    if (time && bytesWritten > 0) {
        setLastModTime(path, *time);
    }

    if (bytesWritten == (int32_t)size) {
        Utils_Logger::info("Write successful: %s (%d bytes)", path, bytesWritten);
    } else {
        Utils_Logger::error("Write warning: Written bytes (%d) mismatch expected (%d)", bytesWritten, size);
    }

    return bytesWritten;
}

int32_t SDCardManager::readFile(const char* path, uint8_t* buffer, uint32_t bufferSize) {
    if (!m_initialized) {
        Utils_Logger::error("SDCardManager not initialized");
        return -1;
    }

    File file = m_fs.open(path);
    if (!file) {
        Utils_Logger::error("Cannot open file: %s", path);
        return -1;
    }

    int32_t bytesRead = file.read(buffer, bufferSize);
    file.close();

    Utils_Logger::info("Read successful: %s (%d bytes)", path, bytesRead);
    return bytesRead;
}

bool SDCardManager::createDirectory(const char* path) {
    if (!m_initialized) {
        Utils_Logger::error("SDCardManager not initialized");
        return false;
    }

    if (m_fs.mkdir(path)) {
        Utils_Logger::info("Directory created: %s", path);
        return true;
    } else {
        Utils_Logger::error("Failed to create directory: %s", path);
        return false;
    }
}

bool SDCardManager::listDirectory(const char* path, char* buffer, uint32_t bufferSize) {
    if (!m_initialized) {
        Utils_Logger::error("SDCardManager not initialized");
        return false;
    }

    if (!m_fs.isDir((char*)path)) {
        Utils_Logger::error("Path is not a directory: %s", path);
        return false;
    }

    int result = m_fs.readDir((char*)path, buffer, bufferSize);
    
    if (result > 0) {
        Utils_Logger::info("Directory listed: %s (%d items)", path, result);
        return true;
    } else {
        Utils_Logger::error("Failed to list directory: %s", path);
        return false;
    }
}

bool SDCardManager::getStorageInfo(StorageInfo* info) {
    if (!m_initialized) {
        Utils_Logger::error("SDCardManager not initialized");
        return false;
    }

    if (!info) {
        Utils_Logger::error("Invalid StorageInfo pointer");
        return false;
    }

    long long int freeSpace = m_fs.get_free_space();
    long long int usedSpace = m_fs.get_used_space();
    
    info->freeBytes = freeSpace;
    info->usedBytes = usedSpace;
    info->totalBytes = freeSpace + usedSpace;
    
    if (info->totalBytes > 0) {
        info->percentUsed = (uint8_t)((info->usedBytes * 100) / info->totalBytes);
    } else {
        info->percentUsed = 0;
    }

    Utils_Logger::info("Storage info: Total=%llu, Free=%llu, Used=%llu (%d%%)",
                     info->totalBytes, info->freeBytes, info->usedBytes, info->percentUsed);
    return true;
}

char* SDCardManager::getRootPath() {
    return m_rootPath;
}

char* SDCardManager::generatePhotoFileName(char* buffer, uint32_t bufferSize) {
    return generateTimestampFileName(buffer, bufferSize, ".jpg");
}

char* SDCardManager::generateTimestampFileName(char* buffer, uint32_t bufferSize, const char* extension) {
    if (!buffer || bufferSize < 50) {
        Utils_Logger::error("Invalid buffer or buffer size too small");
        return nullptr;
    }

    // 从DS1307模块获取当前时间
    DS1307_Time currentTime;
    bool success = clockModule.readTime(currentTime);
    
    if (!success) {
        Utils_Logger::error("Failed to read time from DS1307");
        // 如果读取失败，使用默认时间
        currentTime = {0, 0, 0, 1, 1, 1, 2026};
    }

    // 获取系统毫秒时间（0-999）
    uint32_t milliseconds = millis() % 1000;
    
    // 生成格式为YYYYMMDD_HHMMSSXXX的文件名，XXX为毫秒级时间
    snprintf(buffer, bufferSize, "%sIMG_%04u%02u%02u_%02u%02u%02u%03lu%s", 
             getRootPath(), 
             currentTime.year, currentTime.month, currentTime.date, 
             currentTime.hours, currentTime.minutes, currentTime.seconds,
             milliseconds,
             extension);

    return buffer;
}

// 设置文件最后修改时间 - 接受单独的时间参数
bool SDCardManager::setLastModTime(const char* path, uint16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t minute, uint16_t second) {
    if (!m_initialized) {
        Utils_Logger::error("SDCardManager not initialized");
        return false;
    }

    if (!path) {
        Utils_Logger::error("Invalid path");
        return false;
    }

    // 使用const_cast将const char*转换为char*，因为AmebaFatFS::setLastModTime期望char*参数
    bool result = m_fs.setLastModTime(const_cast<char*>(path), year, month, day, hour, minute, second);
    
    if (result) {
        Utils_Logger::info("Set file modification time: %s -> %04u-%02u-%02u %02u:%02u:%02u", 
                         path, year, month, day, hour, minute, second);
    } else {
        Utils_Logger::error("Failed to set file modification time: %s", path);
    }
    
    return result;
}

// 设置文件最后修改时间 - 接受DS1307_Time结构体
bool SDCardManager::setLastModTime(const char* path, const DS1307_Time& time) {
    return setLastModTime(path, time.year, time.month, time.date, time.hours, time.minutes, time.seconds);
}
