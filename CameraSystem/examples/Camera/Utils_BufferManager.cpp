/*
 * Utils_BufferManager.cpp - 缓冲区管理工具实现
 * 提供内存缓冲区的分配、释放和管理功能
 * 阶段五：相机模块开发 - 工具类开发
 */

#include "Utils_BufferManager.h"
#include "Utils_Logger.h"

// 静态成员初始化
Utils_BufferManager::BufferInfo Utils_BufferManager::buffers[MAX_BUFFERS];
size_t Utils_BufferManager::bufferCount = 0;
size_t Utils_BufferManager::totalAllocated = 0;
bool Utils_BufferManager::initialized = false;

bool Utils_BufferManager::init() {
    if (!initialized) {
        // 初始化缓冲区数组
        for (size_t i = 0; i < MAX_BUFFERS; i++) {
            buffers[i].buffer = nullptr;
            buffers[i].size = 0;
            buffers[i].allocated = false;
            buffers[i].name = nullptr;
            buffers[i].timestamp = 0;
        }
        bufferCount = 0;
        totalAllocated = 0;
        initialized = true;
        
        Utils_Logger::info("缓冲区管理器初始化完成，最大缓冲区数: %d", MAX_BUFFERS);
        return true;
    }
    return true;
}

void* Utils_BufferManager::allocate(size_t size, const char* name) {
    if (!initialized) {
        init();
    }
    
    if (size == 0) {
        Utils_Logger::error("尝试分配大小为0的缓冲区");
        return nullptr;
    }
    
    int slot = findFreeSlot();
    if (slot == -1) {
        Utils_Logger::error("缓冲区管理器已满，无法分配新的缓冲区");
        return nullptr;
    }
    
    void* buffer = malloc(size);
    if (buffer == nullptr) {
        Utils_Logger::error("内存分配失败，大小: %d", size);
        return nullptr;
    }
    
    // 记录缓冲区信息
    buffers[slot].buffer = buffer;
    buffers[slot].size = size;
    buffers[slot].allocated = true;
    buffers[slot].name = name;
    buffers[slot].timestamp = millis();
    
    bufferCount++;
    totalAllocated += size;
    
    Utils_Logger::debug("分配缓冲区: %p, 大小: %d, 名称: %s", buffer, size, name ? name : "unnamed");
    
    return buffer;
}

bool Utils_BufferManager::freeBuffer(void* buffer) {
    if (!initialized || buffer == nullptr) {
        return false;
    }
    
    int index = findBufferIndex(buffer);
    if (index == -1) {
        Utils_Logger::error("尝试释放未注册的缓冲区: %p", buffer);
        return false;
    }
    
    // 释放内存
    free(buffer);
    
    // 更新统计信息
    totalAllocated -= buffers[index].size;
    bufferCount--;
    
    Utils_Logger::debug("释放缓冲区: %p, 大小: %d, 名称: %s", 
                       buffer, buffers[index].size, 
                       buffers[index].name ? buffers[index].name : "unnamed");
    
    // 清除缓冲区信息
    buffers[index].buffer = nullptr;
    buffers[index].size = 0;
    buffers[index].allocated = false;
    buffers[index].name = nullptr;
    buffers[index].timestamp = 0;
    
    return true;
}

void* Utils_BufferManager::reallocate(void* buffer, size_t newSize) {
    if (!initialized || buffer == nullptr || newSize == 0) {
        return nullptr;
    }
    
    int index = findBufferIndex(buffer);
    if (index == -1) {
        Utils_Logger::error("尝试重新分配未注册的缓冲区: %p", buffer);
        return nullptr;
    }
    
    void* newBuffer = realloc(buffer, newSize);
    if (newBuffer == nullptr) {
        Utils_Logger::error("重新分配失败，原大小: %d, 新大小: %d", buffers[index].size, newSize);
        return nullptr;
    }
    
    // 更新统计信息
    totalAllocated = totalAllocated - buffers[index].size + newSize;
    
    // 更新缓冲区信息
    buffers[index].buffer = newBuffer;
    buffers[index].size = newSize;
    buffers[index].timestamp = millis();
    
    Utils_Logger::debug("重新分配缓冲区: %p -> %p, 大小: %d -> %d", 
                       buffer, newBuffer, buffers[index].size, newSize);
    
    return newBuffer;
}

const Utils_BufferManager::BufferInfo* Utils_BufferManager::getBufferInfo(void* buffer) {
    if (!initialized || buffer == nullptr) {
        return nullptr;
    }
    
    int index = findBufferIndex(buffer);
    if (index == -1) {
        return nullptr;
    }
    
    return &buffers[index];
}

size_t Utils_BufferManager::getTotalAllocated() {
    return totalAllocated;
}

size_t Utils_BufferManager::getBufferCount() {
    return bufferCount;
}

void Utils_BufferManager::printStatistics() {
    if (!initialized) {
        Utils_Logger::info("缓冲区管理器未初始化");
        return;
    }
    
    Utils_Logger::info("=== 缓冲区统计信息 ===");
    Utils_Logger::info("总分配内存: %d 字节", totalAllocated);
    Utils_Logger::info("缓冲区数量: %d", bufferCount);
    Utils_Logger::info("最大缓冲区大小: %d 字节", getMaxBufferSize());
    Utils_Logger::info("最小缓冲区大小: %d 字节", getMinBufferSize());
    Utils_Logger::info("平均缓冲区大小: %d 字节", getAverageBufferSize());
    
    if (bufferCount > 0) {
        Utils_Logger::info("缓冲区详细信息:");
        for (size_t i = 0; i < MAX_BUFFERS; i++) {
            if (buffers[i].allocated) {
                Utils_Logger::info("  [%d] %p - 大小: %d, 名称: %s, 时间: %lu", 
                                  i, buffers[i].buffer, buffers[i].size,
                                  buffers[i].name ? buffers[i].name : "unnamed",
                                  buffers[i].timestamp);
            }
        }
    }
}

bool Utils_BufferManager::isValidBuffer(void* buffer) {
    return findBufferIndex(buffer) != -1;
}

void Utils_BufferManager::cleanupAll() {
    if (!initialized) {
        return;
    }
    
    Utils_Logger::info("清理所有缓冲区...");
    
    size_t freedCount = 0;
    for (size_t i = 0; i < MAX_BUFFERS; i++) {
        if (buffers[i].allocated) {
            free(buffers[i].buffer);
            buffers[i].buffer = nullptr;
            buffers[i].size = 0;
            buffers[i].allocated = false;
            buffers[i].name = nullptr;
            buffers[i].timestamp = 0;
            freedCount++;
        }
    }
    
    bufferCount = 0;
    totalAllocated = 0;
    
    Utils_Logger::info("已清理 %d 个缓冲区", freedCount);
}

size_t Utils_BufferManager::getMaxBufferSize() {
    size_t maxSize = 0;
    for (size_t i = 0; i < MAX_BUFFERS; i++) {
        if (buffers[i].allocated && buffers[i].size > maxSize) {
            maxSize = buffers[i].size;
        }
    }
    return maxSize;
}

size_t Utils_BufferManager::getMinBufferSize() {
    size_t minSize = SIZE_MAX;
    for (size_t i = 0; i < MAX_BUFFERS; i++) {
        if (buffers[i].allocated && buffers[i].size < minSize) {
            minSize = buffers[i].size;
        }
    }
    return minSize == SIZE_MAX ? 0 : minSize;
}

size_t Utils_BufferManager::getAverageBufferSize() {
    if (bufferCount == 0) {
        return 0;
    }
    return totalAllocated / bufferCount;
}

int Utils_BufferManager::findBufferIndex(void* buffer) {
    for (size_t i = 0; i < MAX_BUFFERS; i++) {
        if (buffers[i].allocated && buffers[i].buffer == buffer) {
            return i;
        }
    }
    return -1;
}

int Utils_BufferManager::findFreeSlot() {
    for (size_t i = 0; i < MAX_BUFFERS; i++) {
        if (!buffers[i].allocated) {
            return i;
        }
    }
    return -1;
}