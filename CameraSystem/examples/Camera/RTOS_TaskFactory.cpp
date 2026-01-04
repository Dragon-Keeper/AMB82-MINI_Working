#include "RTOS_TaskFactory.h"
#include "Utils_Logger.h"
#include "Camera_ImageConfig.h"
#include "Menu_TriangleController.h"
#include "Menu_MenuContext.h"
#include "Menu_MenuManager.h"
#include "Display_TFTManager.h"
#include "Display_FontRenderer.h"
#include "Camera_CameraManager.h"
#include "System_StateManager.h"

// 外部全局对象声明
extern Display_TFTManager tftManager;
extern MenuManager menuManager;
extern CameraManager cameraManager;
extern MenuContext menuContext;
extern JPEGDEC jpeg;

// 系统状态通过StateManager管理，不再使用全局变量



// 任务函数实现

/**
 * 相机预览任务 (位置A)
 */
void taskCameraPreview(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 0;
    
    Utils_Logger::info("相机预览任务 %c 启动", (char)('A' + taskId));
    
    // 任务初始化 - 使用CameraManager
    cameraManager.startPreview();
    
    // 任务主循环
    while (1) {
        // 处理相机预览 - 使用CameraManager
        cameraManager.processPreviewFrame();
        
        // 检查是否有拍照请求
        if (cameraManager.hasCaptureRequest() && !cameraManager.isCapturing()) {
            // 只在空闲时处理拍照请求，减少对预览的影响
            Utils_Logger::info("Capture request detected, starting capture...");
            cameraManager.clearCaptureRequest();
            cameraManager.capturePhoto();
        }
        
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            1 / portTICK_PERIOD_MS  // 减少超时时间，提高响应速度
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            Utils_Logger::info("相机预览任务 %c 收到退出信号", (char)('A' + taskId));
            break;
        }
        
        // 移除不必要的延迟，最大化帧率
    }
    
    // 任务清理 - 使用CameraManager
    cameraManager.stopPreview();
    
    // 立即更新系统状态为STATE_MAIN_MENU，防止编码器事件继续被相机预览模式处理
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 切换回主菜单
    menuContext.switchToMainMenu();
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_CAMERA_PREVIEW);
    
    Utils_Logger::info("相机预览任务 %c 退出", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块B任务 (位置B)
 */
void taskFunctionB(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 1;
    
    Utils_Logger::info("功能模块B任务 %c 启动", (char)('A' + taskId));
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 任务功能实现
        // TODO: 实现功能模块B的具体逻辑
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_B);
    
    Utils_Logger::info("功能模块B任务 %c 退出", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块C任务 (位置C)
 */
void taskFunctionC(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 2;
    
    Utils_Logger::info("功能模块C任务 %c 启动", (char)('A' + taskId));
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 任务功能实现
        // TODO: 实现功能模块C的具体逻辑
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_C);
    
    Utils_Logger::info("功能模块C任务 %c 退出", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块D任务 (位置D)
 */
void taskFunctionD(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 3;
    
    Utils_Logger::info("功能模块D任务 %c 启动", (char)('A' + taskId));
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 任务功能实现
        // TODO: 实现功能模块D的具体逻辑
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_D);
    
    Utils_Logger::info("功能模块D任务 %c 退出", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块E任务 (位置E)
 */
void taskFunctionE(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 4;
    
    Utils_Logger::info("功能模块E任务 %c 启动", (char)('A' + taskId));
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 任务功能实现
        // TODO: 实现功能模块E的具体逻辑
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_E);
    
    Utils_Logger::info("功能模块E任务 %c 退出", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 系统设置任务 (位置F)
 */
void taskSystemSettings(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 5;
    
    Utils_Logger::info("系统设置任务 %c 启动", (char)('A' + taskId));
    
    // 任务初始化
    menuContext.switchToSubMenu();
    
    // 任务主循环
    while (1) {
        // 处理子菜单
        menuContext.handleSubMenu();
        
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            Utils_Logger::info("系统设置任务 %c 收到退出信号", (char)('A' + taskId));
            break;
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
    // 任务清理
    menuContext.switchToMainMenu();
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_SYSTEM_SETTINGS);
    
    Utils_Logger::info("系统设置任务 %c 退出", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

bool TaskFactory::init() {
    Utils_Logger::info("TaskFactory initialized successfully");
    return true;
}

bool TaskFactory::registerTask(TaskManager::TaskID id, const char* name, TaskFunction_t func, 
                             uint32_t stackSize, UBaseType_t priority) {
    if (!isValidTaskID(id)) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return false;
    }
    
    if (name == NULL || func == NULL) {
        Utils_Logger::error("Invalid task parameters");
        return false;
    }
    
    // 获取任务信息指针
    TaskManager::TaskInfo* taskInfo = const_cast<TaskManager::TaskInfo*>(TaskManager::getTaskInfo(id));
    if (taskInfo == NULL) {
        Utils_Logger::error("Failed to get task info for ID: %d", id);
        return false;
    }
    
    // 更新任务信息
    taskInfo->name = name;
    taskInfo->function = func;
    taskInfo->stackSize = stackSize;
    taskInfo->priority = priority;
    
    Utils_Logger::info("Registered task %s (ID: %d) with stack size %lu, priority %lu", 
                      name, id, stackSize, priority);
    
    return true;
}

TaskHandle_t TaskFactory::createTask(TaskManager::TaskID id, void* params) {
    if (!isValidTaskID(id)) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return NULL;
    }
    
    // 使用TaskManager的公共接口设置任务参数
    TaskManager::setTaskParams(id, params);
    
    // 使用TaskManager创建任务
    TaskHandle_t handle = TaskManager::createTask(id);
    
    if (handle == NULL) {
        Utils_Logger::error("Failed to create task with ID: %d", id);
        TaskManager::setTaskParams(id, NULL); // 清理参数
    } else {
        Utils_Logger::info("Created task with ID: %d, handle: %p", id, handle);
    }
    
    return handle;
}

TaskHandle_t TaskFactory::createDefaultTask(TaskManager::TaskID id) {
    // 使用默认参数创建任务
    return createTask(id, NULL);
}

TaskHandle_t TaskFactory::createTaskWithParams(TaskManager::TaskID id, const TaskParams& params) {
    // 创建参数副本
    TaskParams* taskParamsCopy = new TaskParams(params);
    if (taskParamsCopy == NULL) {
        Utils_Logger::error("Failed to allocate memory for task parameters");
        return NULL;
    }
    
    // 创建任务
    TaskHandle_t handle = createTask(id, taskParamsCopy);
    
    if (handle == NULL) {
        delete taskParamsCopy; // 清理参数
    }
    
    return handle;
}

bool TaskFactory::unregisterTask(TaskManager::TaskID id) {
    if (!isValidTaskID(id)) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return false;
    }
    
    // 获取任务信息指针
    TaskManager::TaskInfo* taskInfo = const_cast<TaskManager::TaskInfo*>(TaskManager::getTaskInfo(id));
    if (taskInfo == NULL) {
        Utils_Logger::error("Failed to get task info for ID: %d", id);
        return false;
    }
    
    // 清理任务参数
    void* params = TaskManager::getTaskParams(id);
    if (params != NULL) {
        // 将void*转换为TaskParams*后再删除
        delete static_cast<TaskFactory::TaskParams*>(params);
        TaskManager::setTaskParams(id, NULL);
    }
    
    // 重置任务信息
    taskInfo->function = NULL;
    
    Utils_Logger::info("Unregistered task with ID: %d", id);
    
    return true;
}

void TaskFactory::cleanup() {
    // 任务参数由TaskManager统一管理和清理
    Utils_Logger::info("TaskFactory cleaned up successfully");
}

bool TaskFactory::isValidTaskID(TaskManager::TaskID id) {
    return (id >= 0 && id < TaskManager::TASK_MAX);
}