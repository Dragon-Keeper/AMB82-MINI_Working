/*
 * ISP_ConfigTask.h - ISP配置任务头文件
 * 实现ISP配置界面的完整交互逻辑
 * 阶段三：ISP配置界面开发 - 任务层
 */

#ifndef ISP_CONFIG_TASK_H
#define ISP_CONFIG_TASK_H

#include <Arduino.h>
#include "ISP_ConfigUI.h"
#include "ISP_ConfigManager.h"
#include "Camera_CameraManager.h"
#include "Encoder_Control.h"
#include "System_StateManager.h"
#include "RTOS_TaskManager.h"
#include "Utils_Logger.h"
#include "Utils_Timer.h"

// ISP配置任务参数
typedef struct {
    int dummy;
} ISPConfigTaskParams;

class ISPConfigTask {
private:
    ISPConfigUI* m_ui;
    ISPConfigManager* m_configManager;
    CameraManager* m_cameraManager;
    
    bool m_initialized;
    bool m_taskRunning;
    
    // 内部状态管理
    void processMenuNavigation(RotationDirection direction);
    void processParameterEdit(RotationDirection direction);
    void processButtonPress();
    
    // 应用参数到相机
    void applyParametersToCamera();
    
public:
    // 构造函数
    ISPConfigTask();
    
    // 编码器事件处理（public 供外部调用）
    void handleEncoderRotation(RotationDirection direction);
    void handleEncoderButton();
    
    // 初始化
    bool init(ISPConfigUI& ui, ISPConfigManager& configManager, CameraManager& cameraManager);
    
    // 任务主循环
    void run();
    
    // 启动/停止
    void start();
    void stop();
    
    // 清理
    void cleanup();
    
    // 重绘UI（用于确保参数设置始终显示在预览画面之上）
    void redrawUI();
};

// 全局ISP配置任务实例声明
extern ISPConfigTask g_ispConfigTask;

// RTOS任务函数
void taskISPConfig(void* pvParameters);

#endif // ISP_CONFIG_TASK_H
