/*
 * Utils_BufferManager.h - 缓冲区管理工具头文件
 * 提供内存缓冲区的分配、释放和管理功能
 * 阶段五：相机模块开发 - 工具类开发
 */

#ifndef UTILS_BUFFERMANAGER_H
#define UTILS_BUFFERMANAGER_H

#include <Arduino.h>

class Utils_BufferManager {
public:
    typedef struct {
        void* buffer;           // 缓冲区指针
        size_t size;            // 缓冲区大小
        bool allocated;         // 是否已分配
        const char* name;       // 缓冲区名称（用于调试）
        unsigned long timestamp; // 分配时间戳
    } BufferInfo;
    
    // 初始化缓冲区管理器
    static bool init();
    
    // 分配缓冲区
    static void* allocate(size_t size, const char* name = nullptr);
    
    // 释放缓冲区
    static bool freeBuffer(void* buffer);
    
    // 重新分配缓冲区
    static void* reallocate(void* buffer, size_t newSize);
    
    // 获取缓冲区信息
    static const BufferInfo* getBufferInfo(void* buffer);
    
    // 获取总分配内存
    static size_t getTotalAllocated();
    
    // 获取缓冲区数量
    static size_t getBufferCount();
    
    // 打印缓冲区统计信息
    static void printStatistics();
    
    // 检查缓冲区是否有效
    static bool isValidBuffer(void* buffer);
    
    // 清理所有缓冲区
    static void cleanupAll();
    
    // 获取最大缓冲区大小
    static size_t getMaxBufferSize();
    
    // 获取最小缓冲区大小
    static size_t getMinBufferSize();
    
    // 获取平均缓冲区大小
    static size_t getAverageBufferSize();
    
private:
    static const size_t MAX_BUFFERS = 20;  // 最大缓冲区数量
    static BufferInfo buffers[MAX_BUFFERS];
    static size_t bufferCount;
    static size_t totalAllocated;
    static bool initialized;
    
    // 查找缓冲区索引
    static int findBufferIndex(void* buffer);
    
    // 查找空闲缓冲区槽位
    static int findFreeSlot();
};

#endif // UTILS_BUFFERMANAGER_H