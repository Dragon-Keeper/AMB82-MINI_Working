/*
 * Camera.ino - AMB82-MINI相机控制系统
 * 使用EC11旋转编码器控制三角形在A-F位置间移动
 * 集成实时预览和拍照功能
 * 使用16x16点阵字库显示提示文字

 * FreeRTOS多任务架构：6个独立任务对应菜单A-F位置
 */

#include <Arduino.h>
#include <Wire.h>
#include "Display_AmebaST7789_DMA_SPI1.h"
#include "Menu_MenuManager.h"
#include "VideoStream.h"
#include <JPEGDEC.h>
#include "AmebaFatFS.h"

// FreeRTOS头文件
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <event_groups.h>
#include <semphr.h>

// 包含菜单页面数据
#include "MenuPages.h"
#include "Camera_ImageConfig.h"

// 包含16x16点阵字库
#include "Display_font16x16.h"

// 模块化移植：阶段一 - 共享模块头文件
#include "Shared_GlobalDefines.h"
#include "Shared_Types.h"
#include "Shared_SharedResources.h"

// 模块化移植：阶段二 - 工具模块头文件
#include "Utils_Logger.h"
#include "Utils_Timer.h"
#include "Utils_BufferManager.h"

// 模块化移植：阶段三 - 显示模块头文件
#include "Display_TFTManager.h"
#include "Display_FontRenderer.h"

// 模块化移植：阶段四 - 编码器模块头文件
#include "Encoder_Control.h"

// 模块化移植：阶段五 - 相机管理模块头文件
#include "Camera_CameraManager.h"

// 模块化移植：阶段六 - 菜单模块头文件
#include "Menu_TriangleController.h"
#include "Menu_MenuItems.h"
#include "Menu_MenuContext.h"

// 预定义字符串常量已移动到Display_FontRenderer模块中

// 相机通道与配置定义
VideoSetting configPreview(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);
VideoSetting configStill(VIDEO_HD, CAM_FPS, VIDEO_JPEG, 1);

// 全局对象 - 使用TFT管理器
Display_TFTManager tftManager;

// 函数前向声明
// 编码器回调函数（在Menu_MenuContext.cpp中实现）
void handleEncoderRotation(RotationDirection direction);
void handleEncoderButton();

// 模块化移植：阶段七 - 任务管理模块头文件
#include "RTOS_TaskManager.h"
#include "RTOS_TaskFactory.h"

// 模块化移植：阶段八 - 系统模块头文件
#include "System_StateManager.h"
#include "System_ResourceManager.h"
#include "System_ConfigManager.h"

// 包含DS1307时钟模块头文件
#include "DS1307_ClockModule.h"

// 时间更新开关：设置为1时，刷入固件会更新DS1307时间；设置为0时，不会更新时间
#define UPDATE_DS1307_TIME 0 // 停止时间更新

// 创建DS1307时钟模块实例
DS1307_ClockModule clockModule;

// 时间变量
DS1307_Time currentTime;

// 任务函数现在在RTOS_TaskFactory.cpp中实现

MenuManager menuManager(tftManager);
JPEGDEC jpeg;

// 模块化移植：阶段五 - SD卡管理器实例
SDCardManager sdCardManager;

// 模块化移植：阶段四 - 编码器控制实例
EncoderControl encoder;

// 模块化移植：阶段五 - 相机管理器实例
CameraManager cameraManager;

// 模块化移植：阶段六 - 三角形控制器实例
TriangleController triangleController(tftManager);

// 模块化移植：阶段六 - 菜单上下文实例
MenuContext menuContext(menuManager, triangleController);

// 缓冲区用于存放转换后的RGB565数据
// 16x16字符占用256个像素，每个像素用16位颜色值表示（RGB565格式）
// 字符数据在绘制前会进行顺时针90度旋转，以适应屏幕方向
uint16_t fontBuffer[16 * 16];

/**
 * JPEG解码回调函数
 */
int JPEGDraw(JPEGDRAW *pDraw) {
    tftManager.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    return 1;
}

void setup() {
    // 初始化串口
    Serial.begin(115200);
    
    // 初始化DS1307时钟模块
    if (clockModule.initialize()) {
        Serial.println("DS1307时钟模块初始化成功");
    } else {
        Serial.println("DS1307时钟模块初始化失败");
    }
    
    // 仅在UPDATE_DS1307_TIME为1时更新DS1307时间
    #if UPDATE_DS1307_TIME
    // 设置DS1307初始时间为2026年1月8日 12:00:00
    DS1307_Time initTime;
    initTime.seconds = 0;
    initTime.minutes = 0;
    initTime.hours = 12;
    initTime.day = 5; // 星期五
    initTime.date = 8;
    initTime.month = 1;
    initTime.year = 2026;
    
    if (clockModule.writeTime(initTime)) {
        Serial.println("DS1307初始时间已设置为2026-01-08 12:00:00");
    } else {
        Serial.println("DS1307初始时间设置失败");
    }
    #else
    Serial.println("DS1307时间更新功能已禁用");
    #endif
    
    // 初始化工具模块
    Utils_Logger::init(Utils_Logger::LEVEL_INFO);
    Utils_BufferManager::init();
    
    // 初始化资源管理器
    Utils_Logger::info("初始化资源管理器...");
    if (!ResourceManager::getInstance().init()) {
        Utils_Logger::error("资源管理器初始化失败!");
        while(1);
    }
    
    // 初始化配置管理器
    Utils_Logger::info("初始化配置管理器...");
    if (!ConfigManager::init()) {
        Utils_Logger::error("配置管理器初始化失败!");
        while(1);
    }
    
    Utils_Logger::info("\n=== AMB82-MINI相机控制系统启动 ===");
    Utils_Logger::info("使用16x16点阵字库显示提示文字");
    
    // 初始化TFT屏幕（使用TFT管理器）
    tftManager.setBacklight(true);
    if (!tftManager.begin()) {
        Utils_Logger::error("TFT管理器初始化失败!");
        while(1);
    }
    
    // 初始化字体渲染器
    Utils_Logger::info("初始化字体渲染器...");
    if (!fontRenderer.begin(&tftManager)) {
        Utils_Logger::error("字体渲染器初始化失败!");
        while(1);
    }
    
    // 初始化菜单管理器
    Utils_Logger::info("初始化菜单管理器...");
    if (!menuManager.init()) {
        Utils_Logger::error("菜单管理器初始化失败!");
        while(1);
    }
    
    // 初始化三角形控制器
    Utils_Logger::info("初始化三角形控制器...");
    if (!triangleController.init()) {
        Utils_Logger::error("三角形控制器初始化失败!");
        while(1);
    }
    
    // 初始化菜单上下文
    Utils_Logger::info("初始化菜单上下文...");
    if (!menuContext.init()) {
        Utils_Logger::error("菜单上下文初始化失败!");
        while(1);
    }
    
    // 配置旋转编码器引脚
    Utils_Logger::info("配置旋转编码器引脚...");
    
    // 模块化移植：阶段四 - 使用EncoderControl类初始化编码器
    if (encoder.init(ENCODER_CLK, ENCODER_DT, ENCODER_SW)) {
        // 设置旋转和按钮回调函数
        encoder.setRotationCallback(handleEncoderRotation);
        encoder.setButtonCallback(handleEncoderButton);
        Utils_Logger::info("编码器模块初始化成功");
    } else {
        Utils_Logger::error("编码器模块初始化失败");
    }
    
    Utils_Logger::info("编码器引脚配置完成");
    
    // 模块化移植：阶段五 - 初始化相机管理器
    Utils_Logger::info("初始化相机管理器...");
    if (!cameraManager.init(tftManager, fontRenderer, jpeg)) {
        Utils_Logger::error("相机管理器初始化失败!");
        while(1);
    }
    Utils_Logger::info("相机管理器初始化成功");
    
    // 设置菜单页面
    Utils_Logger::info("设置菜单页面...");
    menuManager.setMenuPages(menuPages, MENU_PAGE_COUNT);
    
    Utils_Logger::info("成功设置 %d 个菜单页面", menuManager.getTotalPages());
    
    // 显示初始菜单和三角形
    Utils_Logger::info("显示初始菜单和三角形...");
    menuContext.showMenu();
    
    // 初始化FreeRTOS任务管理器
    if (!TaskManager::init()) {
        Utils_Logger::error("TaskManager初始化失败!");
        while(1);
    }
    
    // 初始化任务工厂
    if (!TaskFactory::init()) {
        Utils_Logger::error("TaskFactory初始化失败!");
        while(1);
    }
    
    // 初始化系统状态管理器
    if (!StateManager::getInstance().init()) {
        Utils_Logger::error("StateManager初始化失败!");
        while(1);
    }
    
    // 注册所有任务
    TaskFactory::registerTask(TaskManager::TASK_CAMERA_PREVIEW, "CameraPreview", taskCameraPreview, 1024, 1);
    TaskFactory::registerTask(TaskManager::TASK_FUNCTION_B, "FunctionB", taskFunctionB, 1024, 1);
    TaskFactory::registerTask(TaskManager::TASK_FUNCTION_C, "FunctionC", taskFunctionC, 4096, 1); // 增加堆栈大小以避免溢出
    TaskFactory::registerTask(TaskManager::TASK_FUNCTION_D, "FunctionD", taskFunctionD, 1024, 1);
    TaskFactory::registerTask(TaskManager::TASK_FUNCTION_E, "FunctionE", taskFunctionE, 1024, 1);
    TaskFactory::registerTask(TaskManager::TASK_SYSTEM_SETTINGS, "SystemSettings", taskSystemSettings, 1024, 1);
    
    Utils_Logger::info("系统初始化完成，等待用户交互...");
    Utils_Logger::info("===================================");
}

// 用于控制时间输出频率的计数器
unsigned long timeCounter = 0;

void loop() {
    // 模块化移植：阶段四 - 使用EncoderControl类检测按钮状态
    encoder.checkButton();
    
    // 仅在实时预览阶段每秒读取并输出一次DS1307时间
    if (StateManager::getInstance().getCurrentState() == STATE_CAMERA_PREVIEW) {
        if (timeCounter++ >= 100) { // 大约每秒一次（loop每10ms执行一次）
            timeCounter = 0;
            
            // 读取DS1307时间
            bool timeValid = clockModule.readTime(currentTime);
            
            // 仅当时间读取成功且数据有效时，才打印时间到串口监视器
            if (timeValid) {
              // 使用formatTime函数格式化时间输出
              char timeStr[20];
              clockModule.formatTime(currentTime, timeStr, sizeof(timeStr));
              Serial.println(timeStr);
            }
        }
    }
    
    // 根据当前状态执行不同的处理逻辑
    switch (StateManager::getInstance().getCurrentState()) {
        case STATE_CAMERA_PREVIEW:
            // 相机预览模式 - 由任务处理
            break;
            
        case STATE_SUB_MENU:
            // 系统设置子菜单 - 由MenuContext处理
            menuContext.handleSubMenu();
            break;
            
        case STATE_MAIN_MENU:
        default:
            // 主菜单模式 - 处理任务创建请求
            // 旋转事件和按钮事件通过EncoderControl回调函数处理
            
            // 处理按钮事件（创建对应任务）
            if (StateManager::getInstance().isButtonPressDetected()) {
                StateManager::getInstance().setButtonPressDetected(false);
                
                // 保存当前位置，避免handleEvent重置位置后影响判断
                int currentMenuItem = menuContext.getCurrentMenuItem();
                
                // 调用MenuContext的事件处理函数
                menuContext.handleEvent(MENU_EVENT_SELECT);
                
                // 根据保存的位置执行相应操作
                if (currentMenuItem == POS_A) {
                    // A位置按下：创建相机预览任务
                    Utils_Logger::info("主菜单A位置：创建相机预览任务");
                    // 只有当任务创建成功时，才更新系统状态
                    if (TaskFactory::createDefaultTask(TaskManager::TASK_CAMERA_PREVIEW)) {
                        StateManager::getInstance().setCurrentState(STATE_CAMERA_PREVIEW);
                    }
                } else if (currentMenuItem == POS_C) {
                    // C位置按下：创建图片回放任务
                    Utils_Logger::info("主菜单C位置：创建图片回放任务");
                    // 只有当任务创建成功时，才更新系统状态
                    if (TaskFactory::createDefaultTask(TaskManager::TASK_FUNCTION_C)) {
                        StateManager::getInstance().setCurrentState(STATE_CAMERA_PREVIEW);
                    }
                } else if (currentMenuItem == POS_F) {
                    // F位置按下：进入系统设置子菜单
                    Utils_Logger::info("主菜单F位置：进入系统设置子菜单");
                    menuContext.switchToSubMenu();
                } else {
                    // 其他位置：无操作，仅显示位置信息
                    Utils_Logger::info("主菜单位置 %c：无操作，仅显示位置信息", (char)('A' + currentMenuItem));
                }
            }
            break;
    }
    
    // 减少延迟以提高响应速度
    vTaskDelay(10 / portTICK_PERIOD_MS);
}



