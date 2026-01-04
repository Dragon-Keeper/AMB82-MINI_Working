#ifndef RTOS_TASKFACTORY_H
#define RTOS_TASKFACTORY_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include "RTOS_TaskManager.h"

/**
 * @brief 任务工厂类
 * 
 * 负责任务函数注册、参数传递、资源分配和任务创建
 * 遵循单一职责原则，将任务创建逻辑与任务管理逻辑分离
 */
class TaskFactory {
public:
    /**
     * @brief 任务参数结构体
     * 
     * 用于向任务函数传递自定义参数
     */
    typedef struct {
        uint32_t param1;    // 通用参数1
        uint32_t param2;    // 通用参数2
        void* customParam;  // 自定义参数指针
    } TaskParams;

    /**
     * @brief 初始化任务工厂
     * 
     * @return 初始化成功返回true，失败返回false
     */
    static bool init();

    /**
     * @brief 注册任务函数和参数
     * 
     * @param id 任务ID
     * @param name 任务名称
     * @param func 任务函数指针
     * @param stackSize 任务栈大小
     * @param priority 任务优先级
     * @return 注册成功返回true，失败返回false
     */
    static bool registerTask(TaskManager::TaskID id, const char* name, TaskFunction_t func, 
                            uint32_t stackSize, UBaseType_t priority);

    /**
     * @brief 使用自定义参数创建任务
     * 
     * @param id 任务ID
     * @param params 任务参数指针（可以为NULL）
     * @return 成功返回任务句柄，失败返回NULL
     */
    static TaskHandle_t createTask(TaskManager::TaskID id, void* params = NULL);

    /**
     * @brief 创建带有默认参数的任务
     * 
     * @param id 任务ID
     * @return 成功返回任务句柄，失败返回NULL
     */
    static TaskHandle_t createDefaultTask(TaskManager::TaskID id);

    /**
     * @brief 创建带有TaskParams结构体参数的任务
     * 
     * @param id 任务ID
     * @param params TaskParams结构体参数
     * @return 成功返回任务句柄，失败返回NULL
     */
    static TaskHandle_t createTaskWithParams(TaskManager::TaskID id, const TaskParams& params);

    /**
     * @brief 注销任务
     * 
     * @param id 任务ID
     * @return 注销成功返回true，失败返回false
     */
    static bool unregisterTask(TaskManager::TaskID id);

    /**
     * @brief 清理任务工厂资源
     */
    static void cleanup();

private:
    /**
     * @brief 检查任务ID是否有效
     * 
     * @param id 任务ID
     * @return 有效返回true，无效返回false
     */
    static bool isValidTaskID(TaskManager::TaskID id);
};

// 任务函数声明
extern void taskCameraPreview(void* pvParameters);
extern void taskFunctionB(void* pvParameters);
extern void taskFunctionC(void* pvParameters);
extern void taskFunctionD(void* pvParameters);
extern void taskFunctionE(void* pvParameters);
extern void taskSystemSettings(void* pvParameters);

#endif // RTOS_TASKFACTORY_H