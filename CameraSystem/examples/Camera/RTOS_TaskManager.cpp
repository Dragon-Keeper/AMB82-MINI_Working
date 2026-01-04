#include "RTOS_TaskManager.h"
#include "RTOS_TaskFactory.h"

// 静态成员变量初始化
EventGroupHandle_t TaskManager::xEventGroup = NULL;
QueueHandle_t TaskManager::xTaskMessageQueue = NULL;
void* TaskManager::taskParams[TASK_MAX] = { NULL };

// 任务信息数组将在initTaskInfos()中初始化
TaskManager::TaskInfo TaskManager::taskInfos[TASK_MAX]; // 不在这里初始化，而是在initTaskInfos()中



bool TaskManager::init() {
    // 初始化任务信息
    initTaskInfos();
    
    // 创建事件组
    if (xEventGroup == NULL) {
        xEventGroup = xEventGroupCreate();
        if (xEventGroup == NULL) {
            Utils_Logger::error("Event group creation failed");
            return false;
        }
    }
    
    // 创建消息队列
    if (xTaskMessageQueue == NULL) {
        xTaskMessageQueue = xQueueCreate(10, sizeof(uint32_t));
        if (xTaskMessageQueue == NULL) {
            Utils_Logger::error("Message queue creation failed");
            vEventGroupDelete(xEventGroup);
            xEventGroup = NULL;
            return false;
        }
    }
    
    Utils_Logger::info("TaskManager initialized successfully");
    return true;
}

void TaskManager::initTaskInfos() {
    // 初始化各个任务的信息 - 使用成员逐一赋值的方式
    
    // 相机预览任务
    taskInfos[TASK_CAMERA_PREVIEW].id = TASK_CAMERA_PREVIEW;
    taskInfos[TASK_CAMERA_PREVIEW].name = "CameraPreview";
    taskInfos[TASK_CAMERA_PREVIEW].function = taskCameraPreview;
    taskInfos[TASK_CAMERA_PREVIEW].stackSize = 4096;
    taskInfos[TASK_CAMERA_PREVIEW].priority = 2;
    taskInfos[TASK_CAMERA_PREVIEW].handle = NULL;
    taskInfos[TASK_CAMERA_PREVIEW].state = TASK_STATE_INACTIVE;
    
    // 功能模块B任务
    taskInfos[TASK_FUNCTION_B].id = TASK_FUNCTION_B;
    taskInfos[TASK_FUNCTION_B].name = "FunctionB";
    taskInfos[TASK_FUNCTION_B].function = taskFunctionB;
    taskInfos[TASK_FUNCTION_B].stackSize = 2048;
    taskInfos[TASK_FUNCTION_B].priority = 1;
    taskInfos[TASK_FUNCTION_B].handle = NULL;
    taskInfos[TASK_FUNCTION_B].state = TASK_STATE_INACTIVE;
    
    // 功能模块C任务
    taskInfos[TASK_FUNCTION_C].id = TASK_FUNCTION_C;
    taskInfos[TASK_FUNCTION_C].name = "FunctionC";
    taskInfos[TASK_FUNCTION_C].function = taskFunctionC;
    taskInfos[TASK_FUNCTION_C].stackSize = 2048;
    taskInfos[TASK_FUNCTION_C].priority = 1;
    taskInfos[TASK_FUNCTION_C].handle = NULL;
    taskInfos[TASK_FUNCTION_C].state = TASK_STATE_INACTIVE;
    
    // 功能模块D任务
    taskInfos[TASK_FUNCTION_D].id = TASK_FUNCTION_D;
    taskInfos[TASK_FUNCTION_D].name = "FunctionD";
    taskInfos[TASK_FUNCTION_D].function = taskFunctionD;
    taskInfos[TASK_FUNCTION_D].stackSize = 2048;
    taskInfos[TASK_FUNCTION_D].priority = 1;
    taskInfos[TASK_FUNCTION_D].handle = NULL;
    taskInfos[TASK_FUNCTION_D].state = TASK_STATE_INACTIVE;
    
    // 功能模块E任务
    taskInfos[TASK_FUNCTION_E].id = TASK_FUNCTION_E;
    taskInfos[TASK_FUNCTION_E].name = "FunctionE";
    taskInfos[TASK_FUNCTION_E].function = taskFunctionE;
    taskInfos[TASK_FUNCTION_E].stackSize = 2048;
    taskInfos[TASK_FUNCTION_E].priority = 1;
    taskInfos[TASK_FUNCTION_E].handle = NULL;
    taskInfos[TASK_FUNCTION_E].state = TASK_STATE_INACTIVE;
    
    // 系统设置任务
    taskInfos[TASK_SYSTEM_SETTINGS].id = TASK_SYSTEM_SETTINGS;
    taskInfos[TASK_SYSTEM_SETTINGS].name = "SystemSettings";
    taskInfos[TASK_SYSTEM_SETTINGS].function = taskSystemSettings;
    taskInfos[TASK_SYSTEM_SETTINGS].stackSize = 3072;
    taskInfos[TASK_SYSTEM_SETTINGS].priority = 3;
    taskInfos[TASK_SYSTEM_SETTINGS].handle = NULL;
    taskInfos[TASK_SYSTEM_SETTINGS].state = TASK_STATE_INACTIVE;
    
    // 初始化任务参数数组
    for (uint32_t i = 0; i < TASK_MAX; i++) {
        taskParams[i] = NULL;
    }
}

TaskHandle_t TaskManager::createTask(TaskID id) {
    if (id >= TASK_MAX) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return NULL;
    }
    
    TaskInfo& taskInfo = taskInfos[id];
    
    // 检查任务是否已存在
    if (taskInfo.handle != NULL && taskInfo.state != TASK_STATE_DELETED) {
        Utils_Logger::info("Task %s already exists", taskInfo.name);
        return taskInfo.handle;
    }
    
    // 创建任务，传递任务参数
    BaseType_t result = xTaskCreate(
        taskInfo.function,
        taskInfo.name,
        taskInfo.stackSize,
        taskParams[id],  // 使用保存的任务参数
        taskInfo.priority,
        &taskInfo.handle
    );
    
    if (result != pdPASS) {
        Utils_Logger::error("Failed to create task %s", taskInfo.name);
        return NULL;
    }
    
    taskInfo.state = TASK_STATE_RUNNING;
    Utils_Logger::info("Created task %s successfully with params: %p", taskInfo.name, taskParams[id]);
    
    return taskInfo.handle;
}

bool TaskManager::deleteTask(TaskID id) {
    if (id >= TASK_MAX) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return false;
    }
    
    TaskInfo& taskInfo = taskInfos[id];
    
    if (taskInfo.handle == NULL || taskInfo.state == TASK_STATE_DELETED) {
        Utils_Logger::info("Task %s does not exist or already deleted", taskInfo.name);
        return true;
    }
    
    // 删除任务
    vTaskDelete(taskInfo.handle);
    
    taskInfo.handle = NULL;
    taskInfo.state = TASK_STATE_DELETED;
    
    Utils_Logger::info("Deleted task %s successfully", taskInfo.name);
    return true;
}

bool TaskManager::suspendTask(TaskID id) {
    if (id >= TASK_MAX) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return false;
    }
    
    TaskInfo& taskInfo = taskInfos[id];
    
    if (taskInfo.handle == NULL || taskInfo.state != TASK_STATE_RUNNING) {
        Utils_Logger::info("Task %s is not running", taskInfo.name);
        return false;
    }
    
    vTaskSuspend(taskInfo.handle);
    taskInfo.state = TASK_STATE_BLOCKED;
    
    Utils_Logger::info("Suspended task %s", taskInfo.name);
    return true;
}

bool TaskManager::resumeTask(TaskID id) {
    if (id >= TASK_MAX) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return false;
    }
    
    TaskInfo& taskInfo = taskInfos[id];
    
    if (taskInfo.handle == NULL) {
        Utils_Logger::info("Task %s does not exist", taskInfo.name);
        return false;
    }
    
    if (taskInfo.state == TASK_STATE_RUNNING) {
        Utils_Logger::info("Task %s is already running", taskInfo.name);
        return true;
    }
    
    vTaskResume(taskInfo.handle);
    taskInfo.state = TASK_STATE_RUNNING;
    
    Utils_Logger::info("Resumed task %s", taskInfo.name);
    return true;
}

TaskState_t TaskManager::getTaskState(TaskID id) {
    if (id >= TASK_MAX) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return TASK_STATE_INACTIVE;
    }
    
    return taskInfos[id].state;
}

bool TaskManager::markTaskAsDeleting(TaskID id) {
    if (id >= TASK_MAX) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return false;
    }
    
    TaskInfo& taskInfo = taskInfos[id];
    
    if (taskInfo.handle == NULL) {
        Utils_Logger::info("Task %s does not exist", taskInfo.name);
        return false;
    }
    
    // 更新任务状态为正在删除
    taskInfo.state = TASK_STATE_DELETED;
    taskInfo.handle = NULL;
    
    Utils_Logger::info("Marked task %s as deleting", taskInfo.name);
    return true;
}

const TaskManager::TaskInfo* TaskManager::getTaskInfo(TaskID id) {
    if (id >= TASK_MAX) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return NULL;
    }
    
    return &taskInfos[id];
}

void TaskManager::listAllTasks(char* buffer, uint32_t bufferSize) {
    if (buffer == NULL || bufferSize == 0) {
        return;
    }
    
    uint32_t offset = 0;
    
    offset += snprintf(buffer + offset, bufferSize - offset, "=== Task List ===\n");
    
    for (uint32_t i = 0; i < TASK_MAX && offset < bufferSize; i++) {
        const TaskInfo& task = taskInfos[i];
        const char* stateStr = "UNKNOWN";
        
        switch (task.state) {
            case TASK_STATE_INACTIVE: stateStr = "INACTIVE"; break;
            case TASK_STATE_RUNNING: stateStr = "RUNNING"; break;
            case TASK_STATE_BLOCKED: stateStr = "BLOCKED"; break;
            case TASK_STATE_DELETED: stateStr = "DELETED"; break;
            default: stateStr = "UNKNOWN"; break;
        }
        
        offset += snprintf(buffer + offset, bufferSize - offset, 
                          "ID: %d, Name: %s, State: %s, Priority: %lu\n",
                          task.id, task.name, stateStr, task.priority);
    }
    
    if (offset < bufferSize) {
        buffer[offset] = '\0';
    } else {
        buffer[bufferSize - 1] = '\0';
    }
}

uint32_t TaskManager::getFreeHeapSize() {
    return xPortGetFreeHeapSize();
}

uint32_t TaskManager::getMinimumEverFreeHeapSize() {
    return xPortGetMinimumEverFreeHeapSize();
}

uint8_t TaskManager::getCPUUsage() {
    // 简化实现，实际项目中可能需要更复杂的CPU使用率计算
    return 0;
}

EventGroupHandle_t TaskManager::getEventGroup() {
    return xEventGroup;
}

QueueHandle_t TaskManager::getMessageQueue() {
    return xTaskMessageQueue;
}

bool TaskManager::setEvent(uint32_t eventBits) {
    if (xEventGroup == NULL) {
        Utils_Logger::error("Event group not initialized");
        return false;
    }
    
    xEventGroupSetBits(xEventGroup, eventBits);
    Utils_Logger::debug("Set event bits: 0x%08lx", eventBits);
    return true;
}

bool TaskManager::clearEvent(uint32_t eventBits) {
    if (xEventGroup == NULL) {
        Utils_Logger::error("Event group not initialized");
        return false;
    }
    
    xEventGroupClearBits(xEventGroup, eventBits);
    Utils_Logger::debug("Cleared event bits: 0x%08lx", eventBits);
    return true;
}

uint32_t TaskManager::waitForEvent(uint32_t eventBits, bool clearOnExit, TickType_t timeout) {
    if (xEventGroup == NULL) {
        Utils_Logger::error("Event group not initialized");
        return 0;
    }
    

    uint32_t result = xEventGroupWaitBits(
        xEventGroup,
        eventBits,
        clearOnExit,
        pdFALSE,  // 不需要等待所有位设置
        timeout
    );
    
    if (result != 0) {
        Utils_Logger::debug("Received event bits: 0x%08lx", result);
    }
    
    return result;
}

bool TaskManager::checkEvent(uint32_t eventBits) {
    if (xEventGroup == NULL) {
        Utils_Logger::error("Event group not initialized");
        return false;
    }
    
    uint32_t currentBits = xEventGroupGetBits(xEventGroup);
    return ((currentBits & eventBits) == eventBits);
}

void TaskManager::setTaskParams(TaskID id, void* params) {
    if (id >= TASK_MAX) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return;
    }
    
    taskParams[id] = params;
    Utils_Logger::info("Set task %d params to: %p", id, params);
}

void* TaskManager::getTaskParams(TaskID id) {
    if (id >= TASK_MAX) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return NULL;
    }
    
    return taskParams[id];
}

void TaskManager::cleanup() {
    // 删除所有正在运行的任务
    for (uint32_t i = 0; i < TASK_MAX; i++) {
        deleteTask(static_cast<TaskID>(i));
        
        // 清理任务参数
        if (taskParams[i] != NULL) {
            // 将void*转换为TaskParams*后再删除
            delete static_cast<TaskFactory::TaskParams*>(taskParams[i]);
            taskParams[i] = NULL;
        }
    }
    
    // 删除消息队列
    if (xTaskMessageQueue != NULL) {
        vQueueDelete(xTaskMessageQueue);
        xTaskMessageQueue = NULL;
    }
    
    // 删除事件组
    if (xEventGroup != NULL) {
        vEventGroupDelete(xEventGroup);
        xEventGroup = NULL;
    }
    
    Utils_Logger::info("TaskManager cleaned up successfully");
}
