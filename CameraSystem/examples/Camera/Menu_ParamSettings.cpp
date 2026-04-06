/*
 * Menu_ParamSettings.cpp - 参数设置菜单模块实现
 * 纯黑背景纯白文字 · 悬浮感设计 · 无边框 · 横线分隔
 */

#include "Menu_ParamSettings.h"
#include "Camera_CameraManager.h"

static const uint8_t strExposureMode[]   = {FONT16_IDX_PU, FONT16_IDX_GUANG, FONT16_IDX_MO2, FONT16_IDX_SHI8, 0};
static const uint8_t strBrightness[]     = {FONT16_IDX_LIANG, FONT16_IDX_DU, 0};
static const uint8_t strContrast[]       = {FONT16_IDX_DUI, FONT16_IDX_BI, FONT16_IDX_DU, 0};
static const uint8_t strSaturation[]     = {FONT16_IDX_BAO2, FONT16_IDX_HE, FONT16_IDX_DU, 0};
static const uint8_t strResetDefaults[]  = {FONT16_IDX_CHONG, FONT16_IDX_ZHI2, 0};
static const uint8_t strBackToMenu[]     = {FONT16_IDX_FAN2, FONT16_IDX_HUI2, 0};
static const uint8_t strAuto[]           = {FONT16_IDX_ZI, FONT16_IDX_DONG, 0};
static const uint8_t strManual[]         = {FONT16_IDX_SHOU, FONT16_IDX_DONG, 0};

static const uint8_t* menuLabels[] = {
    strExposureMode,
    strBrightness,
    strContrast,
    strSaturation,
    strResetDefaults,
    strBackToMenu
};

static const int MENU_ITEM_COUNT = 6;

ParamSettingsMenu::ParamSettingsMenu(Display_TFTManager &tft, Display_FontRenderer &font)
    : tftManager(tft), fontRenderer(font), m_cameraManager(nullptr),
      currentState(PARAM_STATE_MENU), currentMenuItem(0), exposureSelection(0), 
      adjustingItem(-1), needsRedraw(true) {
}

void ParamSettingsMenu::setCameraManager(CameraManager *cameraManager) {
    m_cameraManager = cameraManager;
}

void ParamSettingsMenu::init() {
    currentState = PARAM_STATE_MENU;
    currentMenuItem = 0;
    adjustingItem = -1;
    exposureSelection = ConfigManager::getValue(ConfigManager::CONFIG_EXPOSURE_MODE);
    needsRedraw = true;
}

void ParamSettingsMenu::show() {
    needsRedraw = true;
}

void ParamSettingsMenu::update() {
    if (needsRedraw) {
        switch (currentState) {
            case PARAM_STATE_MENU:
            case PARAM_STATE_ADJUSTING_VALUE:
                drawParamMenu();
                break;
            case PARAM_STATE_EXPOSURE_SELECT:
                drawExposureSelection();
                break;
            default:
                break;
        }
        needsRedraw = false;
    }
}

void ParamSettingsMenu::drawParamMenu() {
    tftManager.fillRectangle(PANEL_X, PANEL_Y, PANEL_WIDTH, PANEL_HEIGHT, UI_BG);

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        drawMenuItem(i, (i == currentMenuItem));
        
        if (i < MENU_ITEM_COUNT - 1) {
            int y = MENU_START_Y + (i + 1) * MENU_ITEM_HEIGHT - 1;
            tftManager.drawRectangle(PANEL_X + 8, y, PANEL_WIDTH - 16, 1, UI_DIVIDER);
        }
    }
}

void ParamSettingsMenu::drawMenuItem(int index, bool isSelected) {
    int y = MENU_START_Y + index * MENU_ITEM_HEIGHT;

    if (isSelected) {
        tftManager.fillRectangle(PANEL_X + 2, y, PANEL_WIDTH - 4, MENU_ITEM_HEIGHT, UI_PRIMARY);
        
        int indicatorX = PANEL_X + 4;
        tftManager.fillRectangle(indicatorX, y + 4, 4, MENU_ITEM_HEIGHT - 8, UI_ACCENT);
        
        tftManager.drawRectangle(PANEL_X + 1, y - 1, PANEL_WIDTH - 2, 1, UI_GLOW_TOP);
        tftManager.drawRectangle(PANEL_X + 1, y + MENU_ITEM_HEIGHT, PANEL_WIDTH - 2, 1, UI_GLOW_BOTTOM);
    } else {
        tftManager.fillRectangle(PANEL_X + 2, y, PANEL_WIDTH - 4, MENU_ITEM_HEIGHT, UI_PRIMARY);
    }

    int labelX = LABEL_X + 16;
    fontRenderer.drawChineseString(labelX, y + 4, menuLabels[index],
                                     isSelected ? UI_TEXT_SELECTED : UI_TEXT_UNSELECTED,
                                     UI_PRIMARY);

    if (index >= 1 && index <= 3) {
        ConfigManager::ConfigID configId;
        switch (index) {
            case 1: configId = ConfigManager::CONFIG_BRIGHTNESS; break;
            case 2: configId = ConfigManager::CONFIG_CONTRAST; break;
            case 3: configId = ConfigManager::CONFIG_SATURATION; break;
        }
        int value = ConfigManager::getValue(configId);
        drawSlider(SLIDER_X + 5, y + 23, value, index, (isSelected && adjustingItem == index));
    }
}

void ParamSettingsMenu::drawSlider(int x, int y, int value, int paramIndex, bool isActive) {
    tftManager.fillRectangle(x, y, SLIDER_WIDTH, SLIDER_HEIGHT, UI_TRACK);

    int fillWidth = 0;
    if (paramIndex == 1) {
        fillWidth = ((value + 64) * SLIDER_WIDTH) / 128;
    } else {
        fillWidth = (value * SLIDER_WIDTH) / 100;
    }

    if (fillWidth < 0) fillWidth = 0;
    if (fillWidth > SLIDER_WIDTH) fillWidth = SLIDER_WIDTH;

    if (fillWidth > 0) {
        tftManager.fillRectangle(x, y, fillWidth, SLIDER_HEIGHT,
                                  isActive ? UI_ACCENT : UI_FILL);
    }

    if (isActive) {
        int knobX = x + fillWidth - 3;
        if (knobX < x) knobX = x;
        if (knobX > x + SLIDER_WIDTH - 6) knobX = x + SLIDER_WIDTH - 6;
        
        tftManager.fillRectangle(knobX - 1, y - 2, 8, SLIDER_HEIGHT + 4, UI_GLOW_TOP);
        tftManager.fillRectangle(knobX, y - 1, 6, SLIDER_HEIGHT + 2, UI_ACCENT);
        tftManager.fillRectangle(knobX + 1, y, 4, SLIDER_HEIGHT, UI_TEXT_SELECTED);
    }
}

void ParamSettingsMenu::drawExposureSelection() {
    tftManager.fillRectangle(PANEL_X, PANEL_Y, PANEL_WIDTH, PANEL_HEIGHT, UI_BG);

    fontRenderer.drawChineseString(LABEL_X + 8, MENU_START_Y + 4, strExposureMode, UI_TEXT_SELECTED, UI_BG);
    
    int dividerY = MENU_START_Y + MENU_ITEM_HEIGHT - 2;
    tftManager.drawRectangle(PANEL_X + 8, dividerY, PANEL_WIDTH - 16, 1, UI_DIVIDER);

    int yAuto = MENU_START_Y + MENU_ITEM_HEIGHT;
    if (exposureSelection == 0) {
        tftManager.fillRectangle(PANEL_X + 4, yAuto, PANEL_WIDTH - 8, MENU_ITEM_HEIGHT, UI_PRIMARY);
        
        int indicatorX = PANEL_X + 6;
        tftManager.fillRectangle(indicatorX, yAuto + 4, 4, MENU_ITEM_HEIGHT - 8, UI_ACCENT);
        
        tftManager.drawRectangle(PANEL_X + 3, yAuto - 1, PANEL_WIDTH - 6, 1, UI_GLOW_TOP);
        tftManager.drawRectangle(PANEL_X + 3, yAuto + MENU_ITEM_HEIGHT, PANEL_WIDTH - 6, 1, UI_GLOW_BOTTOM);
        
        fontRenderer.drawChineseString(LABEL_X + 18, yAuto + 4, strAuto, UI_TEXT_SELECTED, UI_PRIMARY);
    } else {
        tftManager.fillRectangle(PANEL_X + 4, yAuto, PANEL_WIDTH - 8, MENU_ITEM_HEIGHT, UI_PRIMARY);
        fontRenderer.drawChineseString(LABEL_X + 18, yAuto + 4, strAuto, UI_TEXT_UNSELECTED, UI_PRIMARY);
    }
    
    int midDividerY = yAuto + MENU_ITEM_HEIGHT - 1;
    tftManager.drawRectangle(PANEL_X + 8, midDividerY, PANEL_WIDTH - 16, 1, UI_DIVIDER);

    int yManual = yAuto + MENU_ITEM_HEIGHT;
    if (exposureSelection == 1) {
        tftManager.fillRectangle(PANEL_X + 4, yManual, PANEL_WIDTH - 8, MENU_ITEM_HEIGHT, UI_PRIMARY);
        
        int indicatorX = PANEL_X + 6;
        tftManager.fillRectangle(indicatorX, yManual + 4, 4, MENU_ITEM_HEIGHT - 8, UI_ACCENT);
        
        tftManager.drawRectangle(PANEL_X + 3, yManual - 1, PANEL_WIDTH - 6, 1, UI_GLOW_TOP);
        tftManager.drawRectangle(PANEL_X + 3, yManual + MENU_ITEM_HEIGHT, PANEL_WIDTH - 6, 1, UI_GLOW_BOTTOM);
        
        fontRenderer.drawChineseString(LABEL_X + 18, yManual + 4, strManual, UI_TEXT_SELECTED, UI_PRIMARY);
    } else {
        tftManager.fillRectangle(PANEL_X + 4, yManual, PANEL_WIDTH - 8, MENU_ITEM_HEIGHT, UI_PRIMARY);
        fontRenderer.drawChineseString(LABEL_X + 18, yManual + 4, strManual, UI_TEXT_UNSELECTED, UI_PRIMARY);
    }
}

void ParamSettingsMenu::handleUp() {
    switch (currentState) {
        case PARAM_STATE_MENU:
            currentMenuItem = (currentMenuItem <= 0) ? (MENU_ITEM_COUNT - 1) : (currentMenuItem - 1);
            needsRedraw = true;
            break;

        case PARAM_STATE_EXPOSURE_SELECT:
            exposureSelection = (exposureSelection <= 0) ? 1 : 0;
            needsRedraw = true;
            break;

        case PARAM_STATE_ADJUSTING_VALUE:
            adjustValue(-1);
            break;
    }
}

void ParamSettingsMenu::handleDown() {
    switch (currentState) {
        case PARAM_STATE_MENU:
            currentMenuItem = (currentMenuItem >= MENU_ITEM_COUNT - 1) ? 0 : (currentMenuItem + 1);
            needsRedraw = true;
            break;

        case PARAM_STATE_EXPOSURE_SELECT:
            exposureSelection = (exposureSelection >= 1) ? 0 : 1;
            needsRedraw = true;
            break;

        case PARAM_STATE_ADJUSTING_VALUE:
            adjustValue(1);
            break;
    }
}

void ParamSettingsMenu::adjustValue(int delta) {
    if (adjustingItem < 1 || adjustingItem > 3) return;

    ConfigManager::ConfigID configId;
    switch (adjustingItem) {
        case 1: configId = ConfigManager::CONFIG_BRIGHTNESS; break;
        case 2: configId = ConfigManager::CONFIG_CONTRAST; break;
        case 3: configId = ConfigManager::CONFIG_SATURATION; break;
        default: return;
    }

    int value = ConfigManager::getValue(configId);
    value += delta * 5;

    if (adjustingItem == 1) {
        if (value < -64) value = 64;
        if (value > 64) value = -64;
    } else {
        if (value < 0) value = 100;
        if (value > 100) value = 0;
    }

    ConfigManager::setValue(configId, value);

    if (m_cameraManager != nullptr) {
        if (adjustingItem == 1) m_cameraManager->setBrightness(value);
        else if (adjustingItem == 2) m_cameraManager->setContrast(value);
        else if (adjustingItem == 3) m_cameraManager->setSaturation(value);
    }

    needsRedraw = true;
}

void ParamSettingsMenu::handleSelect() {
    switch (currentState) {
        case PARAM_STATE_MENU:
            switch (currentMenuItem) {
                case 0:
                    currentState = PARAM_STATE_EXPOSURE_SELECT;
                    exposureSelection = ConfigManager::getValue(ConfigManager::CONFIG_EXPOSURE_MODE);
                    needsRedraw = true;
                    break;

                case 1:
                case 2:
                case 3:
                    currentState = PARAM_STATE_ADJUSTING_VALUE;
                    adjustingItem = currentMenuItem;
                    needsRedraw = true;
                    break;

                case 4:
                    ConfigManager::restoreDefaults();
                    needsRedraw = true;
                    break;

                case 5:
                    reset();
                    break;
            }
            break;

        case PARAM_STATE_EXPOSURE_SELECT:
            ConfigManager::setValue(ConfigManager::CONFIG_EXPOSURE_MODE, exposureSelection);
            currentState = PARAM_STATE_MENU;
            needsRedraw = true;
            break;

        case PARAM_STATE_ADJUSTING_VALUE:
            currentState = PARAM_STATE_MENU;
            adjustingItem = -1;
            needsRedraw = true;
            break;
    }
}

void ParamSettingsMenu::handleBack() {
    switch (currentState) {
        case PARAM_STATE_EXPOSURE_SELECT:
        case PARAM_STATE_ADJUSTING_VALUE:
            currentState = PARAM_STATE_MENU;
            adjustingItem = -1;
            needsRedraw = true;
            break;

        case PARAM_STATE_MENU:
            reset();
            break;
    }
}

void ParamSettingsMenu::reset() {
    currentState = PARAM_STATE_MENU;
    currentMenuItem = 0;
    adjustingItem = -1;
    TaskManager::setEvent(EVENT_RETURN_TO_MENU);
}

void ParamSettingsMenu::handleEncoderRotation(RotationDirection direction) {
    if (direction == ROTATION_CW) {
        handleDown();
    } else {
        handleUp();
    }
}

void ParamSettingsMenu::handleEncoderButton() {
    handleSelect();
}
