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
#include "Camera_CameraManager.h"

// 前向声明
extern CameraManager cameraManager;

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

// 编码器回调函数声明
void handleEncoderRotation(RotationDirection direction);
void handleEncoderButton();

// 菜单上下文类，协调菜单管理器和三角形控制器
class MenuContext {
private:
    MenuManager &menuManager;         // 菜单管理器引用
    TriangleController &triangleController; // 三角形控制器引用
    bool isInitialized = false;
    
    // 菜单项数量配置
    int maxMenuItems = 6; // A-F共6个选项
    
public:
    // 构造函数
    MenuContext(MenuManager &menuManager, TriangleController &triangleController)
        : menuManager(menuManager), triangleController(triangleController) {}
    
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
};

#endif // MENU_MENU_CONTEXT_H