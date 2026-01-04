#include "System_ResourceManager.h"

ResourceManager ResourceManager::m_instance;

ResourceManager& ResourceManager::getInstance() {
    return m_instance;
}

ResourceManager::ResourceManager() {
    for (int i = 0; i < RESOURCE_MAX; i++) {
        m_stats[i].totalAllocated = 0;
        m_stats[i].currentlyUsed = 0;
        m_stats[i].peakUsage = 0;
        m_stats[i].allocationCount = 0;
        m_stats[i].freeCount = 0;
    }
}

bool ResourceManager::init() {
    // Utils_BufferManager是静态类，不需要为每个资源类型创建实例
    if (!Utils_BufferManager::init()) {
        Utils_Logger::error("ResourceManager: Failed to initialize buffer manager");
        return false;
    }
    Utils_Logger::info("ResourceManager: Initialized successfully");
    return true;
}

void* ResourceManager::allocate(ResourceType type, uint32_t size) {
    if (type >= RESOURCE_MAX || size == 0) {
        Utils_Logger::error("ResourceManager: Invalid allocation parameters - type: %d, size: %d", type, size);
        return nullptr;
    }
    
    // 创建资源名称
    char resourceName[32];
    snprintf(resourceName, sizeof(resourceName), "%s_%lu", getResourceTypeName(type), size);
    
    void* ptr = Utils_BufferManager::allocate(size, resourceName);
    if (ptr) {
        updateStats(type, size, true);
        Utils_Logger::debug("ResourceManager: Allocated %d bytes for %s at 0x%08X", 
                          size, getResourceTypeName(type), (uint32_t)ptr);
    } else {
        Utils_Logger::error("ResourceManager: Failed to allocate %d bytes for %s", 
                          size, getResourceTypeName(type));
    }
    
    return ptr;
}

void* ResourceManager::allocateAligned(ResourceType type, uint32_t size, uint32_t alignment) {
    // Utils_BufferManager不支持对齐分配，直接调用普通分配
    Utils_Logger::info("ResourceManager: Aligned allocation not supported, using regular allocation");
    return allocate(type, size);
}

bool ResourceManager::free(ResourceType type, void* ptr) {
    if (type >= RESOURCE_MAX || !ptr) {
        Utils_Logger::error("ResourceManager: Invalid free parameters - type: %d, ptr: 0x%08X", type, (uint32_t)ptr);
        return false;
    }
    
    // 获取分配大小
    const Utils_BufferManager::BufferInfo* info = Utils_BufferManager::getBufferInfo(ptr);
    uint32_t size = info ? info->size : 0;
    
    if (Utils_BufferManager::freeBuffer(ptr)) {
        updateStats(type, size, false);
        Utils_Logger::debug("ResourceManager: Freed %d bytes for %s at 0x%08X", 
                          size, getResourceTypeName(type), (uint32_t)ptr);
        return true;
    } else {
        Utils_Logger::error("ResourceManager: Failed to free memory at 0x%08X for %s", 
                          (uint32_t)ptr, getResourceTypeName(type));
        return false;
    }
}

bool ResourceManager::createPool(ResourceType type, uint32_t blockSize, uint32_t blockCount) {
    // Utils_BufferManager不支持内存池，返回不支持
    Utils_Logger::info("ResourceManager: Memory pool not supported by underlying buffer manager");
    return false;
}

void* ResourceManager::allocateFromPool(ResourceType type) {
    // Utils_BufferManager不支持内存池，返回nullptr
    Utils_Logger::info("ResourceManager: Memory pool not supported by underlying buffer manager");
    return nullptr;
}

bool ResourceManager::freeToPool(ResourceType type, void* ptr) {
    // Utils_BufferManager不支持内存池，调用普通free
    return free(type, ptr);
}

const ResourceManager::ResourceStats* ResourceManager::getStats(ResourceType type) {
    if (type >= RESOURCE_MAX) {
        Utils_Logger::error("ResourceManager: Invalid stats request - type: %d", type);
        return nullptr;
    }
    return &m_stats[type];
}

void ResourceManager::printAllStats(char* buffer, uint32_t bufferSize) {
    if (!buffer || bufferSize == 0) {
        Utils_Logger::error("ResourceManager: Invalid stats buffer");
        return;
    }
    
    uint32_t offset = 0;
    offset += snprintf(buffer + offset, bufferSize - offset, "Resource Manager Statistics:\n");
    offset += snprintf(buffer + offset, bufferSize - offset, "===================================\n");
    
    for (int i = 0; i < RESOURCE_MAX; i++) {
        const ResourceStats& stats = m_stats[i];
        offset += snprintf(buffer + offset, bufferSize - offset, 
                          "%s:\n", getResourceTypeName((ResourceType)i));
        offset += snprintf(buffer + offset, bufferSize - offset, 
                          "  Total Allocated: %lu bytes\n", stats.totalAllocated);
        offset += snprintf(buffer + offset, bufferSize - offset, 
                          "  Currently Used: %lu bytes\n", stats.currentlyUsed);
        offset += snprintf(buffer + offset, bufferSize - offset, 
                          "  Peak Usage: %lu bytes\n", stats.peakUsage);
        offset += snprintf(buffer + offset, bufferSize - offset, 
                          "  Allocations: %lu\n", stats.allocationCount);
        offset += snprintf(buffer + offset, bufferSize - offset, 
                          "  Frees: %lu\n\n", stats.freeCount);
        
        if (offset >= bufferSize) {
            break;
        }
    }
    
    offset += snprintf(buffer + offset, bufferSize - offset, "===================================\n");
    
    // 添加Utils_BufferManager的全局统计
    offset += snprintf(buffer + offset, bufferSize - offset, "Global Buffer Statistics:\n");
    offset += snprintf(buffer + offset, bufferSize - offset, "  Total Allocated: %d bytes\n", Utils_BufferManager::getTotalAllocated());
    offset += snprintf(buffer + offset, bufferSize - offset, "  Buffer Count: %d\n", Utils_BufferManager::getBufferCount());
    
    Utils_Logger::info("ResourceManager: %s", buffer);
}

bool ResourceManager::checkForLeaks() {
    bool hasLeaks = false;
    
    for (int i = 0; i < RESOURCE_MAX; i++) {
        const ResourceStats& stats = m_stats[i];
        if (stats.currentlyUsed > 0 || stats.allocationCount != stats.freeCount) {
            hasLeaks = true;
            Utils_Logger::error("ResourceManager: Memory leak detected in %s - Used: %d bytes, Allocations: %d, Frees: %d", 
                              getResourceTypeName((ResourceType)i), 
                              stats.currentlyUsed, 
                              stats.allocationCount, 
                              stats.freeCount);
        }
    }
    
    if (!hasLeaks) {
        Utils_Logger::info("ResourceManager: No memory leaks detected");
    }
    
    return !hasLeaks;
}

void ResourceManager::cleanup() {
    checkForLeaks();
    Utils_Logger::info("ResourceManager: Cleaned up successfully");
}

void ResourceManager::updateStats(ResourceType type, uint32_t size, bool isAllocate) {
    ResourceStats& stats = m_stats[type];
    
    if (isAllocate) {
        stats.totalAllocated += size;
        stats.currentlyUsed += size;
        stats.allocationCount++;
        if (stats.currentlyUsed > stats.peakUsage) {
            stats.peakUsage = stats.currentlyUsed;
        }
    } else {
        if (stats.currentlyUsed >= size) {
            stats.currentlyUsed -= size;
        }
        stats.freeCount++;
    }
}

const char* ResourceManager::getResourceTypeName(ResourceType type) {
    switch (type) {
        case RESOURCE_IMAGE_BUFFER:
            return "IMAGE_BUFFER";
        case RESOURCE_JPEG_BUFFER:
            return "JPEG_BUFFER";
        case RESOURCE_FONT_BUFFER:
            return "FONT_BUFFER";
        case RESOURCE_TASK_STACK:
            return "TASK_STACK";
        case RESOURCE_FILE_BUFFER:
            return "FILE_BUFFER";
        default:
            return "UNKNOWN";
    }
}
