/*
 * Shared_Types.h - 共享数据类型定义
 * 定义系统中使用的通用数据类型和结构体
 * 阶段五：相机模块开发 - 共享类型定义
 */

#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>

// ===============================================
// 基础几何结构体
// ===============================================

/**
 * 二维点坐标结构体
 */
typedef struct {
    int16_t x;
    int16_t y;
} Point;

/**
 * 尺寸结构体
 */
typedef struct {
    uint16_t width;
    uint16_t height;
} Size;

/**
 * 矩形区域结构体
 */
typedef struct {
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
} Rect;

// ===============================================
// 系统状态枚举
// ===============================================

/**
 * 系统状态枚举
 */
typedef enum {
    STATE_MAIN_MENU,
    STATE_SUB_MENU,
    STATE_CAMERA_PREVIEW  // 相机预览模式
} SystemState;

// ===============================================
// 三角形位置枚举
// ===============================================

/**
 * 三角形位置枚举（对应菜单A-F位置）
 */
typedef enum {
    POS_A = 0,
    POS_B = 1,
    POS_C = 2,
    POS_D = 3,
    POS_E = 4,
    POS_F = 5
} TrianglePosition;

// ===============================================
// FreeRTOS任务相关类型
// ===============================================

/**
 * 任务消息结构体
 */
typedef struct {
    uint32_t taskId;
    uint32_t command;
    void* data;
} TaskMessage_t;

/**
 * 任务状态枚举
 */
typedef enum {
    TASK_STATE_INACTIVE = 0,
    TASK_STATE_RUNNING = 1,
    TASK_STATE_BLOCKED = 2,
    TASK_STATE_DELETED = 3
} TaskState_t;

/**
 * 任务管理器结构体
 */
typedef struct {
    TaskHandle_t handle;
    TaskState_t state;
    uint32_t taskId;
    uint32_t stackSize;
    UBaseType_t priority;
    void (*taskFunction)(void*);
} TaskManager_t;

#endif // TYPES_H