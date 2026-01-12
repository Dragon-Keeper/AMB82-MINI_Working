#ifndef RTOS_TASKMANAGER_H
#define RTOS_TASKMANAGER_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include <event_groups.h>
#include <queue.h>
#include "Shared_Types.h"
#include "Shared_GlobalDefines.h"
#include "Utils_Logger.h"

class TaskManager {
public:
    typedef enum {
        TASK_CAMERA_PREVIEW = 0,
        TASK_FUNCTION_B = 1,
        TASK_FUNCTION_C = 2,
        TASK_FUNCTION_D = 3,
        TASK_FUNCTION_E = 4,
        TASK_SYSTEM_SETTINGS = 5,
        TASK_TIME_SYNC = 6,
        TASK_MAX
    } TaskID;

    typedef struct {
        TaskID id;
        const char* name;
        TaskFunction_t function;
        uint32_t stackSize;
        UBaseType_t priority;
        TaskHandle_t handle;
        TaskState_t state;
    } TaskInfo;

    // 初始化任务管理器
    static bool init();
    
    // 任务创建与删除
    static TaskHandle_t createTask(TaskID id);
    static bool deleteTask(TaskID id);
    
    // 任务状态管理
    static bool suspendTask(TaskID id);
    static bool resumeTask(TaskID id);
    static TaskState_t getTaskState(TaskID id);
    static bool markTaskAsDeleting(TaskID id);
    
    // 任务信息查询
    static const TaskInfo* getTaskInfo(TaskID id);
    static void listAllTasks(char* buffer, uint32_t bufferSize);
    
    // 系统资源监控
    static uint32_t getFreeHeapSize();
    static uint32_t getMinimumEverFreeHeapSize();
    static uint8_t getCPUUsage();
    
    // 任务通信资源获取
    static EventGroupHandle_t getEventGroup();
    static QueueHandle_t getMessageQueue();
    
    // 事件管理功能
    static bool setEvent(uint32_t eventBits);
    static bool clearEvent(uint32_t eventBits);
    static uint32_t waitForEvent(uint32_t eventBits, bool clearOnExit, TickType_t timeout);
    static bool checkEvent(uint32_t eventBits);
    
    // 任务参数管理
    static void setTaskParams(TaskID id, void* params);
    static void* getTaskParams(TaskID id);
    
    // 清理资源
    static void cleanup();

private:
    // 初始化任务信息数组
    static void initTaskInfos();
    
    // 任务信息数组
    static TaskInfo taskInfos[TASK_MAX];
    
    // FreeRTOS通信资源
    static EventGroupHandle_t xEventGroup;
    static QueueHandle_t xTaskMessageQueue;
    
    // 任务参数存储
    static void* taskParams[TASK_MAX];
};

#endif // RTOS_TASKMANAGER_H
