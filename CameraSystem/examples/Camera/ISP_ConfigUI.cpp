/*
 * ISP_ConfigUI.cpp - ISP配置界面实现
 * 实现ISP配置界面的显示和交互
 * 阶段三：ISP配置界面开发 - UI层
 */

#include "ISP_ConfigUI.h"

ISPConfigUI::ISPConfigUI() 
    : m_tftManager(nullptr)
    , m_fontRenderer(nullptr)
    , m_configManager(nullptr)
    , m_uiState(ISP_UI_STATE_MENU)
    , m_currentMenuIndex(0)
    , m_editingMenuIndex(-1)
    , m_initialized(false)
    , m_lastUiState(ISP_UI_STATE_MENU)
    , m_lastMenuIndex(-1)
    , m_lastEditingIndex(-2)
    , m_forceRedraw(true)
{
}

bool ISPConfigUI::init(Display_TFTManager& tftManager, Display_FontRenderer& fontRenderer, ISPConfigManager& configManager)
{
    if (m_initialized) {
        Utils_Logger::info("ISPConfigUI already initialized");
        return true;
    }
    
    m_tftManager = &tftManager;
    m_fontRenderer = &fontRenderer;
    m_configManager = &configManager;
    m_initialized = true;
    
    Utils_Logger::info("ISPConfigUI initialized successfully");
    return true;
}

void ISPConfigUI::showMainMenu()
{
    if (!m_initialized) {
        Utils_Logger::error("ISPConfigUI not initialized");
        return;
    }
    
    m_uiState = ISP_UI_STATE_MENU;
    m_forceRedraw = true;  // 设置强制重绘标志
    drawMenuBackground();
    
    for (int i = 0; i < ISP_MENU_COUNT; i++) {
        drawMenuItem(i, (i == m_currentMenuIndex), false);
    }
}

void ISPConfigUI::selectMenuItem(int index)
{
    if (!m_initialized || index < 0 || index >= ISP_MENU_COUNT) {
        return;
    }
    
    m_currentMenuIndex = index;
    showMainMenu();
}

void ISPConfigUI::enterEditMode(int index)
{
    if (!m_initialized || index < 0 || index >= ISP_MENU_COUNT) {
        return;
    }
    
    m_editingMenuIndex = index;
    m_uiState = ISP_UI_STATE_EDITING;
    m_forceRedraw = true;  // 设置强制重绘标志
    
    drawMenuBackground();
    for (int i = 0; i < ISP_MENU_COUNT; i++) {
        drawMenuItem(i, (i == index), (i == index));
    }
}

void ISPConfigUI::exitEditMode()
{
    if (!m_initialized) {
        return;
    }
    
    m_editingMenuIndex = -1;
    m_forceRedraw = true;  // 设置强制重绘标志
    showMainMenu();
}

void ISPConfigUI::updateParameterValue(int delta)
{
    if (!m_initialized || m_editingMenuIndex < 0) {
        return;
    }
    
    ISPConfig config = m_configManager->getConfig();
    int step = 10; // 设置步长为10
    
    switch (m_editingMenuIndex) {
        case ISP_MENU_EXPOSURE:
            config.exposureMode = (config.exposureMode + delta) % 2;
            if (config.exposureMode < 0) config.exposureMode = 1;
            break;
        case ISP_MENU_BRIGHTNESS:
            config.brightness += delta * step;
            if (config.brightness > 64) config.brightness = 64;
            if (config.brightness < -64) config.brightness = -64;
            break;
        case ISP_MENU_CONTRAST:
            config.contrast += delta * step;
            if (config.contrast > 100) config.contrast = 100;
            if (config.contrast < 0) config.contrast = 0;
            break;
        case ISP_MENU_SATURATION:
            config.saturation += delta * step;
            if (config.saturation > 100) config.saturation = 100;
            if (config.saturation < 0) config.saturation = 0;
            break;
        default:
            break;
    }
    
    m_configManager->setConfig(config);
    drawMenuItem(m_editingMenuIndex, true, true);
    
    Utils_Logger::info("ISP参数更新: 曝光=%d, 亮度=%d, 对比度=%d, 饱和度=%d", 
                     config.exposureMode, config.brightness, config.contrast, config.saturation);
}

void ISPConfigUI::confirmSave()
{
    if (!m_initialized) {
        return;
    }
    
    if (m_configManager->saveConfig()) {
        Utils_Logger::info("ISP config saved successfully");
    } else {
        Utils_Logger::error("Failed to save ISP config");
    }
    
    exitEditMode();
}

void ISPConfigUI::resetParameters()
{
    if (!m_initialized) {
        return;
    }
    
    m_configManager->resetToDefault();
    showMainMenu();
    Utils_Logger::info("ISP parameters reset to default");
}

void ISPConfigUI::cleanup()
{
    m_tftManager = nullptr;
    m_fontRenderer = nullptr;
    m_configManager = nullptr;
    m_initialized = false;
    m_uiState = ISP_UI_STATE_MENU;
    m_currentMenuIndex = 0;
    m_editingMenuIndex = -1;
}

void ISPConfigUI::drawMenuBackground()
{
    // 去除黑色背景，改为透明，让参数设置直接显示在实时预览画面之上
    // 不再调用fillRectangle，使背景保持透明
    Utils_Logger::debug("ISP配置界面背景已设为透明");
}

void ISPConfigUI::drawMenuItem(int index, bool isSelected, bool isEditing)
{
    if (!m_tftManager || index < 0 || index >= ISP_MENU_COUNT) {
        return;
    }
    
    int yPos = 30 + index * 45;
    const char* label = getMenuItemLabel(index);
    ISPConfig config = m_configManager->getConfig();
    
    uint16_t textColor = isSelected ? COLOR_HIGHLIGHT : COLOR_TEXT;
    int menuX = SCREEN_WIDTH / 2 + 10;
    
    // 绘制选中高亮框
    if (isSelected) {
        m_tftManager->drawRectangle(menuX - 5, yPos - 5, SCREEN_WIDTH / 2 - 10, 40, COLOR_HIGHLIGHT);
    }
    
    // 设置字号为2（默认是1，增加一倍）
    m_tftManager->setTextSize(2);
    m_tftManager->setTextColor(textColor);
    m_tftManager->setCursor(menuX, yPos);
    m_tftManager->print(label);
    
    if (isEditing) {
        char valueStr[32];
        m_tftManager->setCursor(menuX, yPos + 20);
        
        switch (index) {
            case ISP_MENU_EXPOSURE:
                m_tftManager->print((config.exposureMode == 0) ? "Manual" : "Auto");
                break;
            case ISP_MENU_BRIGHTNESS:
                sprintf(valueStr, "%d", config.brightness);
                m_tftManager->print(valueStr);
                drawParameterBar(index, config.brightness, -64, 64);
                break;
            case ISP_MENU_CONTRAST:
                sprintf(valueStr, "%d", config.contrast);
                m_tftManager->print(valueStr);
                drawParameterBar(index, config.contrast, 0, 100);
                break;
            case ISP_MENU_SATURATION:
                sprintf(valueStr, "%d", config.saturation);
                m_tftManager->print(valueStr);
                drawParameterBar(index, config.saturation, 0, 100);
                break;
            default:
                break;
        }
    }
    
    // 恢复默认字号
    m_tftManager->setTextSize(1);
}

void ISPConfigUI::drawParameterBar(int index, int value, int minValue, int maxValue)
{
    if (!m_tftManager) return;
    
    int yPos = 30 + index * 45;
    int menuX = SCREEN_WIDTH / 2 + 10;
    int barX = menuX;
    int barY = yPos + 40;
    int barWidth = SCREEN_WIDTH / 2 - 20;
    int barHeight = 10;
    
    // 绘制色条背景
    m_tftManager->fillRectangle(barX, barY, barWidth, barHeight, COLOR_BAR_BG);
    
    // 计算填充宽度
    int range = maxValue - minValue;
    int normalizedValue = value - minValue;
    int fillWidth = (normalizedValue * barWidth) / range;
    
    // 绘制填充部分
    if (fillWidth > 0) {
        m_tftManager->fillRectangle(barX, barY, fillWidth, barHeight, COLOR_BAR_FILL);
    }
}

void ISPConfigUI::drawExposureMode(int mode)
{
}

void ISPConfigUI::drawBrightness(int value)
{
}

void ISPConfigUI::drawContrast(int value)
{
}

void ISPConfigUI::drawSaturation(int value)
{
}

void ISPConfigUI::drawResetButton()
{
}

void ISPConfigUI::drawBackButton()
{
}

const char* ISPConfigUI::getMenuItemLabel(int index)
{
    switch (index) {
        case ISP_MENU_EXPOSURE:
            return "A: Exposure";
        case ISP_MENU_BRIGHTNESS:
            return "B: Brightness";
        case ISP_MENU_CONTRAST:
            return "C: Contrast";
        case ISP_MENU_SATURATION:
            return "D: Saturation";
        case ISP_MENU_RESET:
            return "E: Reset";
        case ISP_MENU_BACK:
            return "F: Back";
        default:
            return "";
    }
}

void ISPConfigUI::redrawUI()
{
    if (!m_initialized) {
        return;
    }
    
    // 检查是否需要重绘
    bool needRedraw = m_forceRedraw;
    
    // 检查状态是否变化，只有变化时才重绘，避免不必要的重绘导致闪烁
    if (!needRedraw) {
        if (m_uiState != m_lastUiState) {
            needRedraw = true;
            m_lastUiState = m_uiState;
        }
        
        if (m_currentMenuIndex != m_lastMenuIndex) {
            needRedraw = true;
            m_lastMenuIndex = m_currentMenuIndex;
        }
        
        if (m_editingMenuIndex != m_lastEditingIndex) {
            needRedraw = true;
            m_lastEditingIndex = m_editingMenuIndex;
        }
    }
    
    // 如果状态没有变化，直接返回，不重绘
    // 编辑模式下参数值变化由updateParameterValue直接调用drawMenuItem处理
    if (!needRedraw) {
        return;
    }
    
    // 清除强制重绘标志
    m_forceRedraw = false;
    
    // 重绘菜单，保持透明背景
    // 不调用drawMenuBackground，使背景保持透明
    
    if (m_uiState == ISP_UI_STATE_MENU) {
        // 菜单模式：直接绘制所有菜单项
        for (int i = 0; i < ISP_MENU_COUNT; i++) {
            drawMenuItem(i, (i == m_currentMenuIndex), false);
        }
    } else if (m_uiState == ISP_UI_STATE_EDITING) {
        // 编辑模式：绘制所有菜单项，其中当前项处于编辑状态
        for (int i = 0; i < ISP_MENU_COUNT; i++) {
            drawMenuItem(i, (i == m_editingMenuIndex), (i == m_editingMenuIndex));
        }
    }
}
