/*
 * ISP_ConfigUI_Simple.h - ISP配置界面简化版头文件
 * 最小可行性版本，提供基础的ISP配置界面
 * 阶段三：ISP配置界面开发 - 简化版
 */

#ifndef ISP_CONFIG_UI_SIMPLE_H
#define ISP_CONFIG_UI_SIMPLE_H

#include <Arduino.h>
#include "Display_TFTManager.h"
#include "Display_FontRenderer.h"
#include "ISP_ConfigManager.h"
#include "Utils_Logger.h"

class ISPConfigUISimple {
private:
    Display_TFTManager* m_tftManager;
    Display_FontRenderer* m_fontRenderer;
    ISPConfigManager* m_configManager;
    bool m_initialized;
    
    int m_currentSelection;  // 0-5: A-F
    
    void drawBackground();
    void drawMenuItems();
    void drawSelection();
    
public:
    ISPConfigUISimple();
    
    bool init(Display_TFTManager& tftManager, Display_FontRenderer& fontRenderer, ISPConfigManager& configManager);
    
    void show();
    void moveUp();
    void moveDown();
    int getSelection() const { return m_currentSelection; }
    
    void cleanup();
};

#endif // ISP_CONFIG_UI_SIMPLE_H
