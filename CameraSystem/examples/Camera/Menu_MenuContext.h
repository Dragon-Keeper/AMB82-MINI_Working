/*
 * Menu_MenuContext.h - 菜单上下文头文件
 * 管理菜单事件和上下文状态
 * 阶段六：菜单模块优化 - 菜单上下文管理
 */

#ifndef MENU_MENU_CONTEXT_H
#define MENU_MENU_CONTEXT_H

#include <Arduino.h>
#include "Encoder_Control.h"
#include "Display_TFTManager.h"
#include "Menu_MenuManager.h"
#include "Menu_TriangleController.h"
#include "Utils_Logger.h"
#include "Utils_Timer.h"
#include "System_StateManager.h"
#include "RTOS_TaskManager.h"
#include "RTOS_TaskFactory.h"
#include "OTA.h"

// 前向声明
class CameraManager;
extern CameraManager cameraManager;

// 前向声明参数设置菜单类
class ParamSettingsMenu;

// 菜单事件类型
typedef enum {
    MENU_EVENT_NONE = 0,
    MENU_EVENT_UP,           // 向上滚动
    MENU_EVENT_DOWN,         // 向下滚动
    MENU_EVENT_SELECT,       // 选择当前项
    MENU_EVENT_BACK,         // 返回上一级
    MENU_EVENT_NEXT_PAGE,    // 下一页
    MENU_EVENT_PREV_PAGE,    // 上一页
    MENU_EVENT_RESET         // 重置菜单
} MenuEventType;

// OTA服务器IP配置状态机状态
typedef enum {
    OTA_IP_STATE_IDLE = 0,       // 空闲，未进入IP配置
    OTA_IP_STATE_FIELD_1,        // 正在编辑第1个字段（如192）
    OTA_IP_STATE_FIELD_2,        // 正在编辑第2个字段（如168）
    OTA_IP_STATE_FIELD_3,        // 正在编辑第3个字段（如1）
    OTA_IP_STATE_FIELD_4,        // 正在编辑第4个字段（如50）
    OTA_IP_STATE_SUBMITTED       // IP已提交，配置完成
} OtaIpConfigState;

// 编码器回调函数声明
void handleEncoderRotation(RotationDirection direction);
void handleEncoderButton();

// 前向声明
class Display_FontRenderer;
extern Display_FontRenderer fontRenderer;

// 菜单上下文类，协调菜单管理器和三角形控制器
class MenuContext {
private:
    MenuManager &menuManager;         // 菜单管理器引用
    TriangleController &triangleController; // 三角形控制器引用
    ParamSettingsMenu *paramSettingsMenu; // 参数设置菜单指针
    bool isInitialized = false;
    bool inParamSettings = false;     // 是否在参数设置菜单中
    bool inRebootConfirm = false;     // 是否在重启确认对话框中
    bool confirmDefaultBack = true;    // 确认对话框默认选中"返回"选项
    int rebootConfirmPosB = -1;        // 保存进入确认对话框时的B位置
    
    // OTA确认对话框状态
    bool inOtaConfirm = false;         // 是否在OTA确认对话框中
    bool otaConfirmDefaultBack = true; // OTA对话框默认选中"返回"选项
    int otaConfirmPosD = -1;           // 保存进入OTA确认对话框时的D位置
    volatile bool otaInProgress = false; // OTA升级进行中标志，防止重复启动

    // OTA升级中界面退出确认状态
    bool inOtaProgress = false;         // 是否在OTA升级中界面（区别于OTA确认对话框）
    bool otaProgressDefaultBack = true;  // 升级中界面默认选中"返回"选项
    bool otaExitDialogShown = false;     // 退出确认对话框是否正在显示
    
    // OTA服务器IP配置状态
    OtaIpConfigState otaIpState = OTA_IP_STATE_IDLE; // IP配置状态机当前状态
    uint8_t otaServerIp[4] = {192, 168, 1, 50};      // OTA服务器IP地址（4个字段，0~255）
    char otaServerIpStr[16] = "192.168.1.50";         // OTA服务器IP字符串形式（用于WiFi连接）
    
    // 菜单项数量配置
    int maxMenuItems = 6; // A-F共6个选项

public:
    // 确认对话框绘制（需要在回调函数中调用）
    void showRebootConfirmDialog();
    void hideRebootConfirmDialog();
    void executeReboot();
    void executeShutdown();
    
    // OTA确认对话框绘制
    void showOtaConfirmDialog();
    void hideOtaConfirmDialog();
    void executeOTA();
    void showVersionInfo();
    const char* getOtaStateText();
    void updateOtaDisplay();

    // OTA升级中退出确认对话框
    void showOtaProgressScreen();
    void showOtaProgressExitDialog();
    void handleOtaProgressExitRotation(RotationDirection direction);
    void handleOtaProgressExitButton();
    
    // OTA服务器IP配置
    void showOtaIpConfig();                                    // 显示IP配置界面
    void updateOtaIpDisplay();                                 // 刷新IP配置界面显示
    void handleOtaIpRotation(RotationDirection direction);     // 处理IP配置中的旋转事件
    void handleOtaIpButton();                                  // 处理IP配置中的按钮事件
    bool isInOtaIpConfig() const { return otaIpState != OTA_IP_STATE_IDLE && otaIpState != OTA_IP_STATE_SUBMITTED; }
    void commitOtaServerIp();                                  // 提交IP地址，更新字符串形式

    // 构造函数
    MenuContext(MenuManager &menuManager, TriangleController &triangleController)
        : menuManager(menuManager), triangleController(triangleController),
          paramSettingsMenu(nullptr), inParamSettings(false), inRebootConfirm(false),
          confirmDefaultBack(true), rebootConfirmPosB(-1),
          inOtaConfirm(false), otaConfirmDefaultBack(true), otaConfirmPosD(-1) {}
    
    // 初始化菜单上下文
    bool init();
    
    // 处理菜单事件
    bool handleEvent(MenuEventType event);
    
    // 显示菜单
    void showMenu();
    
    // 获取当前菜单项
    int getCurrentMenuItem() const { return triangleController.getCurrentPosition(); }
    
    // 获取当前页面类型
    MenuPageType getCurrentPageType() const {
        return menuManager.getCurrentPageType();
    }
    
    // 获取确认对话框默认选中状态
    bool isConfirmDefaultBack() const { return confirmDefaultBack; }
    
    // 设置确认对话框选中状态
    void setConfirmDefaultBack(bool value) { confirmDefaultBack = value; }
    
    // 设置OTA确认对话框选中状态
    void setOtaConfirmDefaultBack(bool value) { otaConfirmDefaultBack = value; }
    
    // 获取重启确认对话框状态
    bool isInRebootConfirm() const { return inRebootConfirm; }
    
    // 获取OTA确认对话框状态
    bool isInOtaConfirm() const { return inOtaConfirm; }
    bool isInOtaProgress() const { return inOtaProgress; }
    bool isOtaExitDialogShown() const { return otaExitDialogShown; }
    void setOtaProgressDefaultBack(bool value) { otaProgressDefaultBack = value; }
    
    // 获取当前选中的菜单项信息
    const MenuItem *getCurrentMenuItemInfo() const;
    
    // 重置菜单状态
    void reset();
    
    // 处理系统设置子菜单
    void handleSubMenu();
    
    // 切换到系统设置子菜单
    void switchToSubMenu();
    
    // 切换回主菜单
    void switchToMainMenu();
    
    // 清理菜单上下文资源
    void cleanup();
};

#endif // MENU_MENU_CONTEXT_H