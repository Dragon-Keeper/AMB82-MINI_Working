/*
 * Shared_SharedResources.h - 共享资源定义
 * 管理系统中使用的共享资源和同步机制
 * 阶段五：相机模块开发 - 共享资源管理
 */

#ifndef SHARED_RESOURCES_H
#define SHARED_RESOURCES_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <event_groups.h>
#include <semphr.h>

// ===============================================
// 共享资源声明（外部引用）
// ===============================================

// FreeRTOS任务句柄数组（在Camera.ino中定义）
extern TaskHandle_t taskHandles[6];

// 全局事件组用于任务间通信
extern EventGroupHandle_t xEventGroup;

// 任务消息队列
extern QueueHandle_t xTaskMessageQueue;

// 互斥锁保护全局资源
extern SemaphoreHandle_t xGlobalMutex;
extern SemaphoreHandle_t xCameraMutex;
extern SemaphoreHandle_t xDisplayMutex;

// 任务管理器实例
extern TaskManager_t taskManager[6];

// ===============================================
// 系统状态变量
// ===============================================

// 当前系统状态
extern SystemState currentState;

// 旋转编码器变量
extern volatile int encoderCount;
extern volatile bool rotationDetected;
extern volatile int rotationDirection;

// 三角形位置变量
extern int currentTrianglePosition;

// 相机控制变量（已迁移到CameraManager模块）
// extern bool isCapturing;
extern bool cameraModeActive;
// extern bool manualCaptureRequested;

// 按钮处理相关变量
extern bool lastButtonState;
extern unsigned long lastButtonTime;
extern bool buttonPressDetected;

// SD卡状态（已迁移到CameraManager模块）
// extern bool sdcardInitialized;

// ===============================================
// 图像缓冲区（已迁移到CameraManager模块）
// ===============================================

// extern uint32_t img_addr_preview;
// extern uint32_t img_len_preview;
// extern uint32_t img_addr_still;
// extern uint32_t img_len_still;

// ===============================================
// 字体缓冲区
// ===============================================

extern uint16_t fontBuffer[16 * 16];

#endif // SHARED_RESOURCES_H