/*
 * Camera_SDCardManager.h - SD卡文件系统管理器模块
 * 封装SD卡初始化、文件读写和存储空间管理功能
 * 阶段五：相机模块开发 - SD卡管理
 */

#ifndef CAMERA_SD_CARD_MANAGER_H
#define CAMERA_SD_CARD_MANAGER_H

#include <Arduino.h>
#include "AmebaFatFS.h"
#include "DS1307_ClockModule.h"

class SDCardManager {
public:
    typedef struct {
        uint64_t totalBytes;
        uint64_t freeBytes;
        uint64_t usedBytes;
        uint8_t percentUsed;
    } StorageInfo;

    SDCardManager();

    bool init();
    void cleanup();

    bool isInitialized() const;

    bool fileExists(const char* path);
    bool createFile(const char* path);
    bool deleteFile(const char* path);
    bool renameFile(const char* oldPath, const char* newPath);

    // 写入文件，可选参数指定文件最后修改时间
    int32_t writeFile(const char* path, const uint8_t* data, uint32_t size, const DS1307_Time* time = nullptr);
    int32_t readFile(const char* path, uint8_t* buffer, uint32_t bufferSize);

    bool createDirectory(const char* path);
    bool listDirectory(const char* path, char* buffer, uint32_t bufferSize);

    bool getStorageInfo(StorageInfo* info);
    char* getRootPath();

    // 设置文件最后修改时间
    bool setLastModTime(const char* path, uint16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t minute, uint16_t second);
    bool setLastModTime(const char* path, const DS1307_Time& time);

    char* generatePhotoFileName(char* buffer, uint32_t bufferSize);
    char* generateTimestampFileName(char* buffer, uint32_t bufferSize, const char* extension);

private:
    bool m_initialized;
    AmebaFatFS m_fs;
    char m_rootPath[64];
};

#endif
