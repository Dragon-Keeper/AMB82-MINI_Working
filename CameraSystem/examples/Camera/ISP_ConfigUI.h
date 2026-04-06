/*
 * ISP_ConfigUI.h - ISP配置界面头文件
 * 实现ISP配置界面的显示和交互
 * 阶段三：ISP配置界面开发 - UI层
 */

#ifndef ISP_CONFIG_UI_H
#define ISP_CONFIG_UI_H

#include <Arduino.h>
#include "Display_TFTManager.h"
#include "Display_FontRenderer.h"
#include "ISP_ConfigManager.h"
#include "Utils_Logger.h"

// ISP配置菜单项索引
typedef enum {
    ISP_MENU_EXPOSURE = 0,    // A：曝光模式
    ISP_MENU_BRIGHTNESS,       // B：亮度调节
    ISP_MENU_CONTRAST,         // C：对比度调节
    ISP_MENU_SATURATION,       // D：饱和度调节
    ISP_MENU_RESET,            // E：参数重置
    ISP_MENU_BACK,             // F：返回
    ISP_MENU_COUNT
} ISPMenuItem;

// ISP配置界面状态
typedef enum {
    ISP_UI_STATE_MENU = 0,     // 菜单浏览状态
    ISP_UI_STATE_EDITING,       // 参数编辑状态
    ISP_UI_STATE_CONFIRM        // 确认保存状态
} ISPUIState;

class ISPConfigUI {
private:
    Display_TFTManager* m_tftManager;
    Display_FontRenderer* m_fontRenderer;
    ISPConfigManager* m_configManager;
    
    // UI状态
    ISPUIState m_uiState;
    int m_currentMenuIndex;
    int m_editingMenuIndex;
    bool m_initialized;
    
    // 重绘状态跟踪
    ISPUIState m_lastUiState;
    int m_lastMenuIndex;
    int m_lastEditingIndex;
    bool m_forceRedraw;  // 强制重绘标志
    
    // 屏幕尺寸
    static const int SCREEN_WIDTH = 240;
    static const int SCREEN_HEIGHT = 320;
    
    // 颜色定义
    static const uint16_t COLOR_BG = ST7789_BLACK;
    static const uint16_t COLOR_TEXT = ST7789_WHITE;
    static const uint16_t COLOR_HIGHLIGHT = ST7789_RED;
    static const uint16_t COLOR_BAR_BG = ST7789_BLUE;
    static const uint16_t COLOR_BAR_FILL = ST7789_GREEN;
    
    // 内部方法
    void drawMenuBackground();
    void drawMenuItem(int index, bool isSelected, bool isEditing);
    void drawParameterBar(int index, int value, int minValue, int maxValue);
    void drawExposureMode(int mode);
    void drawBrightness(int value);
    void drawContrast(int value);
    void drawSaturation(int value);
    void drawResetButton();
    void drawBackButton();
    
    const char* getMenuItemLabel(int index);
    
public:
    // 构造函数
    ISPConfigUI();
    
    // 初始化
    bool init(Display_TFTManager& tftManager, Display_FontRenderer& fontRenderer, ISPConfigManager& configManager);
    
    // 显示主菜单
    void showMainMenu();
    
    // 选择菜单项
    void selectMenuItem(int index);
    
    // 进入编辑模式
    void enterEditMode(int index);
    
    // 退出编辑模式
    void exitEditMode();
    
    // 更新参数值（编辑模式中）
    void updateParameterValue(int delta);
    
    // 确认保存参数
    void confirmSave();
    
    // 重置参数
    void resetParameters();
    
    // 获取当前状态
    ISPUIState getUIState() const { return m_uiState; }
    int getCurrentMenuIndex() const { return m_currentMenuIndex; }
    int getEditingMenuIndex() const { return m_editingMenuIndex; }
    
    // 重绘UI（保持透明背景）
    void redrawUI();
    
    // 清理
    void cleanup();
};

#endif // ISP_CONFIG_UI_H
