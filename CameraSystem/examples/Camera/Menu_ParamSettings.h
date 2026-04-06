/*
 * Menu_ParamSettings.h - 参数设置菜单模块
 * 实现参数设置菜单的显示、交互和状态管理
 * 包括曝光模式选择、亮度/对比度/饱和度调节等功能
 */

#ifndef MENU_PARAMSETTINGS_H
#define MENU_PARAMSETTINGS_H

#include <Arduino.h>
#include "Display_TFTManager.h"
#include "Display_FontRenderer.h"
#include "Display_font16x16.h"
#include "System_ConfigManager.h"
#include "Encoder_Control.h"
#include "RTOS_TaskManager.h"

// 参数设置菜单状态枚举
typedef enum {
    PARAM_STATE_MENU = 0,           // 主菜单状态
    PARAM_STATE_EXPOSURE_SELECT,    // 曝光模式选择状态
    PARAM_STATE_ADJUSTING_VALUE     // 调节数值状态
} ParamSettingState;

class CameraManager;

class ParamSettingsMenu {
private:
    Display_TFTManager &tftManager;
    Display_FontRenderer &fontRenderer;
    CameraManager *m_cameraManager;
    
    ParamSettingState currentState;
    int currentMenuItem;            // 当前选中的菜单项 (0-5)
    int exposureSelection;          // 曝光模式选择 (0=自动, 1=手动)
    int adjustingItem;              // 正在调节的项目 (1=亮度, 2=对比度, 3=饱和度)
    bool needsRedraw;               // 是否需要重绘界面

    // 菜单布局常量（侧边栏模式 - 分层显示）
    static const int PANEL_X = 195;          // 参数面板X起始位置（右侧面板）
    static const int PANEL_WIDTH = 125;      // 参数面板宽度
    static const int PANEL_Y = 5;            // 参数面板Y起始位置
    static const int PANEL_HEIGHT = 230;     // 参数面板高度
    
    static const int MENU_ITEM_HEIGHT = 38;  // 菜单项高度（增加间距避免标签和色条重叠）
    static const int MENU_START_Y = 15;      // 菜单项Y起始位置（移除了标题区域）
    static const int SLIDER_HEIGHT = 6;      // 色条高度
    static const int SLIDER_WIDTH = 105;     // 色条宽度（适配面板）
    static const int SLIDER_X = 205;         // 色条X位置
    static const int LABEL_X = 200;          // 标签X位置
    
    // 绘制色条控件
    void drawSlider(int x, int y, int value, int paramIndex, bool isActive);
    
    // 调节参数值（支持循环）
    void adjustValue(int delta);
    
    // 绘制曝光模式选择界面
    void drawExposureSelection();
    
    // 绘制参数设置主菜单
    void drawParamMenu();
    
    // 绘制单个菜单项
    void drawMenuItem(int index, bool isSelected);
    
public:
    // 构造函数
    ParamSettingsMenu(Display_TFTManager &tft, Display_FontRenderer &font);
    
    // 初始化
    void init();
    
    // 设置相机管理器引用
    void setCameraManager(CameraManager *cameraManager);
    
    // 显示参数设置菜单
    void show();
    
    // 处理向上移动事件
    void handleUp();
    
    // 处理向下移动事件
    void handleDown();
    
    // 处理选择/确认事件
    void handleSelect();
    
    // 处理返回事件
    void handleBack();
    
    // 获取当前状态
    ParamSettingState getState() const { return currentState; }
    
    // 获取当前菜单项
    int getCurrentMenuItem() const { return currentMenuItem; }
    
    // 重置状态
    void reset();
    
    // 处理编码器旋转事件
    void handleEncoderRotation(RotationDirection direction);
    
    // 处理编码器按钮事件
    void handleEncoderButton();
    
    // 更新界面（每帧调用）
    void update();
};

#endif // MENU_PARAMSETTINGS_H
