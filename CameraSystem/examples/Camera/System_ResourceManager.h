#ifndef SYSTEM_RESOURCEMANAGER_H
#define SYSTEM_RESOURCEMANAGER_H

#include "Utils_BufferManager.h"
#include "Utils_Logger.h"

class ResourceManager {
public:
    typedef enum {
        RESOURCE_IMAGE_BUFFER = 0,
        RESOURCE_JPEG_BUFFER = 1,
        RESOURCE_FONT_BUFFER = 2,
        RESOURCE_TASK_STACK = 3,
        RESOURCE_FILE_BUFFER = 4,
        RESOURCE_MAX
    } ResourceType;
    
    typedef struct {
        uint32_t totalAllocated;
        uint32_t currentlyUsed;
        uint32_t peakUsage;
        uint32_t allocationCount;
        uint32_t freeCount;
    } ResourceStats;
    
    static ResourceManager& getInstance();
    
    bool init();
    void* allocate(ResourceType type, uint32_t size);
    void* allocateAligned(ResourceType type, uint32_t size, uint32_t alignment);
    bool free(ResourceType type, void* ptr);
    bool createPool(ResourceType type, uint32_t blockSize, uint32_t blockCount);
    void* allocateFromPool(ResourceType type);
    bool freeToPool(ResourceType type, void* ptr);
    const ResourceStats* getStats(ResourceType type);
    void printAllStats(char* buffer, uint32_t bufferSize);
    bool checkForLeaks();
    void cleanup();
    
private:
    ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    static ResourceManager m_instance;
    ResourceStats m_stats[RESOURCE_MAX];
    
    void updateStats(ResourceType type, uint32_t size, bool isAllocate);
    const char* getResourceTypeName(ResourceType type);
};

#endif // SYSTEM_RESOURCEMANAGER_H