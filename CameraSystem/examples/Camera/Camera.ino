/*
 * Camera.ino - AMB82-MINI相机控制系统
 * 使用EC11旋转编码器控制三角形在A-F位置间移动
 * 集成实时预览和拍照功能
 * 使用16x16点阵字库显示提示文字

 * FreeRTOS多任务架构：6个独立任务对应菜单A-F位置
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
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

// WiFi网络配置
#define WIFI_CONFIG_COUNT 2
struct WiFiConfig {
  char ssid[20];
  char password[20];
};

WiFiConfig wifiConfigs[WIFI_CONFIG_COUNT] = {
  {"Force", "dd123456"},
  {"Tiger", "Dt5201314"}
};

// 将IPAddress转换为字符串的辅助函数
String ipToString(IPAddress ip) {
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

// 国内NTP服务器列表（优先核心组，后备用组）
String ntpServers[] = {
  "time1.aliyun.com",
  "ntp.ntsc.ac.cn",
  "time2.aliyun.com",
  "ntp1.ntsc.ac.cn",
  "time3.aliyun.com",
  "ntp.10086.cn",
  "ntp.ctyun.com",
  "s1a.time.edu.cn",
  "ntp.unicom.com.cn"
};
// 服务器数量（自动计算，避免手动修改）
int ntpServerCount = sizeof(ntpServers) / sizeof(ntpServers[0]);

// NTP配置
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

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
    TaskFactory::registerTask(TaskManager::TASK_TIME_SYNC, "TimeSync", taskTimeSync, 2048, 1); // 注册后台校时任务
    
    // 创建后台校时任务
    Utils_Logger::info("创建后台校时任务...");
    if (TaskFactory::createDefaultTask(TaskManager::TASK_TIME_SYNC)) {
        Utils_Logger::info("后台校时任务创建成功");
    } else {
        Utils_Logger::error("后台校时任务创建失败");
    }
    
    Utils_Logger::info("系统初始化完成，等待用户交互...");
    Utils_Logger::info("===================================");
}

// 用于控制时间输出频率的计数器
unsigned long timeCounter = 0;

// 后台校时任务函数
void taskTimeSync(void* parameters) {
    Utils_Logger::info("后台校时任务启动");
    
    bool wifiConnected = false;
    const int MAX_RETRIES = 5;
    const int RETRY_DELAY = 3000; // 3秒重试间隔
    
    // 尝试连接WiFi网络
    for (int i = 0; i < WIFI_CONFIG_COUNT; i++) {
        if (wifiConnected) break;
        
        Utils_Logger::info("尝试连接WiFi网络: %s", wifiConfigs[i].ssid);
        
        WiFi.begin(wifiConfigs[i].ssid, wifiConfigs[i].password);
        
        for (int retry = 0; retry < MAX_RETRIES; retry++) {
            if (WiFi.status() == WL_CONNECTED) {
                // 等待获取有效的IP地址
                int ipWaitCount = 0;
                const int MAX_IP_WAIT = 10;
                const int IP_WAIT_DELAY = 1000; // 1秒
                
                while (WiFi.localIP() == IPAddress(0, 0, 0, 0) && ipWaitCount < MAX_IP_WAIT) {
                    Utils_Logger::info("等待获取IP地址... %d/%d", ipWaitCount + 1, MAX_IP_WAIT);
                    vTaskDelay(IP_WAIT_DELAY / portTICK_PERIOD_MS);
                    ipWaitCount++;
                }
                
                if (WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
                    Utils_Logger::info("成功连接到WiFi网络: %s", wifiConfigs[i].ssid);
                    Utils_Logger::info("IP地址: %s", ipToString(WiFi.localIP()).c_str());
                    wifiConnected = true;
                    break;
                } else {
                    Utils_Logger::info("WiFi连接成功但未获取到IP地址");
                    // 重置WiFi状态，继续尝试
                    WiFi.disconnect();
                    vTaskDelay(RETRY_DELAY / portTICK_PERIOD_MS);
                }
            } else {
                Utils_Logger::info("连接中... 尝试 %d/%d", retry + 1, MAX_RETRIES);
                vTaskDelay(RETRY_DELAY / portTICK_PERIOD_MS);
            }
        }
    }
    
    if (wifiConnected) {
        // 获取NTP时间
        Utils_Logger::info("获取NTP时间...");
        bool ntpSync = false;
        const int NTP_MAX_RETRIES = 3;
        const int NTP_TIMEOUT = 2000; // 2秒超时
        
        // 遍历国内NTP服务器列表
        for (int serverIndex = 0; serverIndex < ntpServerCount; serverIndex++) {
            if (ntpSync) break;
            
            String currentServer = ntpServers[serverIndex];
            Utils_Logger::info("尝试使用NTP服务器: %s", currentServer.c_str());
            
            // 初始化NTP客户端
            timeClient.setPoolServerName(currentServer.c_str());
            timeClient.setTimeOffset(8 * 3600); // 设置时区为UTC+8（中国标准时间）
            timeClient.setUpdateInterval(60000); // 设置更新间隔为60秒
            timeClient.begin();
            
            // 尝试多次获取时间
            for (int retry = 0; retry < NTP_MAX_RETRIES; retry++) {
                if (timeClient.update()) {
                    Utils_Logger::info("NTP时间同步成功，使用服务器: %s", currentServer.c_str());
                    Utils_Logger::info("当前时间: %s", timeClient.getFormattedTime().c_str());
                    Utils_Logger::info("当前日期: %s", timeClient.getFormattedDate().c_str());
                    ntpSync = true;
                    
                    // 校准DS1307时钟
                    DS1307_Time ntpTime;
                    ntpTime.seconds = timeClient.getSeconds();
                    ntpTime.minutes = timeClient.getMinutes();
                    ntpTime.hours = timeClient.getHours();
                    ntpTime.day = timeClient.getDay() + 1; // NTPClient返回0-6，DS1307需要1-7
                    ntpTime.date = timeClient.getMonthDay();
                    ntpTime.month = timeClient.getMonth();
                    ntpTime.year = timeClient.getYear();
                    
                    Utils_Logger::info("校准DS1307时钟...");
                    if (clockModule.writeTime(ntpTime)) {
                        Utils_Logger::info("DS1307时钟校准成功");
                        
                        // 读取校准后的时间进行验证（已屏蔽输出，避免显示错误信息）
                        DS1307_Time calibratedTime;
                        if (clockModule.readTime(calibratedTime)) {
                            // 不输出校准后的时间，因为可能存在I2C通信延迟导致的显示错误
                        }
                    } else {
                        Utils_Logger::error("DS1307时钟校准失败");
                    }
                    
                    break;
                }
                
                Utils_Logger::info("NTP时间同步中... 服务器: %s, 尝试 %d/%d", currentServer.c_str(), retry + 1, NTP_MAX_RETRIES);
                vTaskDelay(NTP_TIMEOUT / portTICK_PERIOD_MS); // 2秒后重试
            }
        }
        
        if (!ntpSync) {
            Utils_Logger::error("NTP时间同步失败，已尝试所有国内服务器");
        }
        
        // 断开WiFi连接以节省资源
        Utils_Logger::info("断开WiFi连接以节省资源");
        WiFi.disconnect();
    } else {
        Utils_Logger::error("无法连接到任何WiFi网络");
    }
    
    Utils_Logger::info("后台校时任务完成");
    vTaskDelete(NULL); // 删除任务
}

void loop() {
    // 模块化移植：阶段四 - 使用EncoderControl类检测按钮状态
    encoder.checkButton();
    
    // DS1307时间读取与计数功能控制开关
    // 开关状态可通过修改DS1307_TIME_READ_ENABLED宏来控制（1:启用, 0:禁用）
    if (DS1307_TIME_READ_ENABLED) {
        // 仅在非实时预览阶段每秒读取并输出一次DS1307时间，以提高预览帧率
        if (StateManager::getInstance().getCurrentState() != STATE_CAMERA_PREVIEW) {
            if (timeCounter >= 100) { // 大约每1秒一次（loop每10ms执行一次）
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
            } else {
                timeCounter++;
            }
        }
    } else {
        // 开关关闭时的状态指示，仅在首次进入loop时输出一次
        static bool firstTime = true;
        if (firstTime) {
            Utils_Logger::info("DS1307时间读取与计数功能已禁用");
            firstTime = false;
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



