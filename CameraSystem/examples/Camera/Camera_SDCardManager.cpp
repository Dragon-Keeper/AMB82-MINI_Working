/*
 * Camera_SDCardManager.cpp - SD卡文件系统管理器实现
 * 封装SD卡初始化、文件读写和存储空间管理功能
 * 阶段五：相机模块开发 - SD卡管理
 */

#include "Camera_SDCardManager.h"
#include "Utils_Logger.h"
#include "Utils_Timer.h"
#include <stdio.h>

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

int32_t SDCardManager::writeFile(const char* path, const uint8_t* data, uint32_t size) {
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

    unsigned long timestamp = Utils_Timer::getCurrentTime();
    snprintf(buffer, bufferSize, "%sIMG_%lu%s", getRootPath(), timestamp, extension);

    return buffer;
}
