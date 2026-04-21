/*
 * USB_MassStorageModule.cpp - USB Mass Storage Module Implementation
 */

#include "USB_MassStorageModule.h"

static const uint8_t strUsdBack[]    = {FONT16_IDX_FAN2, FONT16_IDX_HUI2, 0};
static const uint8_t strUsdConfirm[] = {FONT16_IDX_QUE2, FONT16_IDX_REN2, 0};

USB_MassStorageModule::USB_MassStorageModule()
    : m_tftManager(nullptr),
      m_fontRenderer(nullptr),
      m_state(STATE_IDLE),
      m_initialized(false),
      m_exitDialogShown(false),
      m_exitDefaultBack(true),
      m_exitConfirmed(false) {
}

USB_MassStorageModule::~USB_MassStorageModule() {
    cleanup();
}

USB_MassStorageModule& USB_MassStorageModule::getInstance() {
    static USB_MassStorageModule instance;
    return instance;
}

bool USB_MassStorageModule::init(Display_TFTManager& tftManager, Display_FontRenderer& fontRenderer) {
    if (m_initialized) {
        Utils_Logger::info("[USB_MSD_MODULE] 已经初始化");
        return true;
    }

    Utils_Logger::info("[USB_MSD_MODULE] 初始化");
    m_tftManager = &tftManager;
    m_fontRenderer = &fontRenderer;
    m_initialized = true;
    m_state = STATE_IDLE;

    return true;
}

void USB_MassStorageModule::cleanup() {
    if (!m_initialized) {
        return;
    }

    Utils_Logger::info("[USB_MSD_MODULE] 清理资源");
    exit();
    m_initialized = false;
}

void USB_MassStorageModule::enter() {
    if (m_state == STATE_RUNNING) {
        Utils_Logger::info("[USB_MSD_MODULE] 已经在运行中");
        return;
    }

    Utils_Logger::info("[USB_MSD_MODULE] 进入USB MSC模式");
    m_state = STATE_INITIALIZING;
    m_exitDialogShown = false;
    m_exitDefaultBack = true;
    m_exitConfirmed = false;

    m_tftManager->fillScreen(ST7789_BLACK);

    m_tftManager->setTextColor(ST7789_CYAN, ST7789_BLACK);
    m_tftManager->setTextSize(2);
    m_tftManager->setCursor(70, 20);
    m_tftManager->print("USB MSC Mode");

    m_tftManager->setTextColor(ST7789_WHITE, ST7789_BLACK);
    m_tftManager->setTextSize(1);
    m_tftManager->setCursor(30, 60);
    // m_tftManager->print("[1/6] Release WiFi...");

    Utils_Logger::info("[USB_MSD_MODULE] 释放WiFi资源，避免USB/WiFi冲突");

    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
        Utils_Logger::info("[USB_MSD_MODULE] WiFi已断开连接");
    }

    delay(500);

    wifi_off();
    delay(500);

    Utils_Logger::info("[USB_MSD_MODULE] WiFi硬件已关闭");

    m_tftManager->setCursor(30, 80);
    // m_tftManager->print("[2/6] USB Init...");

    delay(100);

    m_usbMassStorage.USBInit();

    m_tftManager->setCursor(30, 100);
    // m_tftManager->print("[3/6] SDIO Init...");

    m_usbMassStorage.SDIOInit();

    m_tftManager->setCursor(30, 120);
    // m_tftManager->print("[4/6] USB Status...");
    int usbStatus = m_usbMassStorage.USBStatus();
    Utils_Logger::info("[USB_MSD_MODULE] USB状态返回值: %d", usbStatus);

    m_tftManager->setCursor(30, 140);
    // m_tftManager->print("[5/6] Init Disk...");
    m_usbMassStorage.initializeDisk();

    m_tftManager->setCursor(30, 160);
    // m_tftManager->print("[6/6] Load Driver...");
    m_usbMassStorage.loadUSBMassStorageDriver();

    delay(500);

    int connected = m_usbMassStorage.isConnected();
    Utils_Logger::info("[USB_MSD_MODULE] USB连接状态: %d", connected);

    m_state = STATE_RUNNING;
    Utils_Logger::info("[USB_MSD_MODULE] USB MSC初始化完成");

    updateDisplay();
}

void USB_MassStorageModule::exit() {
    if (m_state == STATE_IDLE) {
        return;
    }

    Utils_Logger::info("[USB_MSD_MODULE] 退出USB MSC模式");
    m_state = STATE_STOPPING;

    m_tftManager->fillScreen(ST7789_BLACK);
    m_tftManager->setTextColor(ST7789_YELLOW, ST7789_BLACK);
    m_tftManager->setTextSize(1);
    m_tftManager->setCursor(30, 60);
    // m_tftManager->print("[1/4] USB MSC Deinit...");

    m_usbMassStorage.USBDeinit();

    Utils_Logger::info("[USB_MSD_MODULE] USB MSC驱动已卸载，USB硬件已反初始化");

    m_tftManager->setCursor(30, 80);
    // m_tftManager->print("[2/4] Wait settle...");
    delay(500);

    m_tftManager->setCursor(30, 100);
    // m_tftManager->print("[3/4] WiFi Restart...");

    wifi_on(RTW_MODE_STA);
    delay(1000);

    Utils_Logger::info("[USB_MSD_MODULE] WiFi硬件已重新启动");

    m_tftManager->setCursor(30, 120);
    // m_tftManager->print("[4/4] Done!");
    delay(500);

    m_state = STATE_IDLE;
    m_exitDialogShown = false;
    m_exitConfirmed = true;
    Utils_Logger::info("[USB_MSD_MODULE] USB MSC已退出，资源已清理，WiFi已恢复");
}

void USB_MassStorageModule::processLoop() {
    if (m_state != STATE_RUNNING) {
        return;
    }
}

void USB_MassStorageModule::handleRotation(RotationDirection direction) {
    if (m_state != STATE_RUNNING) {
        return;
    }

    if (m_exitDialogShown) {
        m_exitDefaultBack = !m_exitDefaultBack;
        Utils_Logger::info("[USB_MSD_MODULE] 退出对话框切换: %s", m_exitDefaultBack ? "返回" : "确认");
        updateExitDialogDisplay();
    } else {
        Utils_Logger::info("[USB_MSD_MODULE] 旋转旋钮弹出退出对话框");
        showExitDialog();
    }
}

void USB_MassStorageModule::handleButton() {
    if (m_state != STATE_RUNNING) {
        return;
    }

    Utils_Logger::info("[USB_MSD_MODULE] handleButton 被调用");

    if (m_exitDialogShown) {
        if (m_exitDefaultBack) {
            Utils_Logger::info("[USB_MSD_MODULE] 用户选择返回");
            hideExitDialog();
            updateDisplay();
        } else {
            Utils_Logger::info("[USB_MSD_MODULE] 用户选择确认退出");
            exit();
        }
    } else {
        showExitDialog();
    }
}

void USB_MassStorageModule::updateDisplay() {
    if (m_state != STATE_RUNNING) {
        return;
    }

    m_tftManager->fillScreen(ST7789_BLACK);

    m_tftManager->setTextColor(ST7789_CYAN, ST7789_BLACK);
    m_tftManager->setTextSize(2);
    m_tftManager->setCursor(70, 40);
    m_tftManager->print("USB MSC Mode");

    m_tftManager->setTextColor(ST7789_GREEN, ST7789_BLACK);
    m_tftManager->setTextSize(1);
    m_tftManager->setCursor(50, 90);
    m_tftManager->print("USB Connected!");

    m_tftManager->setTextColor(ST7789_YELLOW, ST7789_BLACK);
    m_tftManager->setCursor(50, 130);
    m_tftManager->print("PC can access SD card");

    m_tftManager->setTextColor(ST7789_GRAY, ST7789_BLACK);
    m_tftManager->setCursor(50, 170);
    m_tftManager->print("Rotate: Exit dialog");
}

void USB_MassStorageModule::showExitDialog() {
    Utils_Logger::info("[USB_MSD_MODULE] 显示退出确认对话框");
    m_exitDialogShown = true;
    m_exitDefaultBack = true;

    const int screenWidth = 320;
    const int screenHeight = 240;

    const int dialogWidth = 220;
    const int dialogHeight = 100;
    const int dialogX = (screenWidth - dialogWidth) / 2;
    const int dialogY = (screenHeight - dialogHeight) / 2;

    m_tftManager->fillRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_BLACK);
    m_tftManager->drawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_WHITE);

    const int titleY = dialogY + 20;
    m_tftManager->setTextColor(ST7789_WHITE, ST7789_BLACK);
    m_tftManager->setTextSize(1);
    m_tftManager->setCursor(dialogX + 60, titleY);
    m_tftManager->print("Exit USB MSC?");

    updateExitDialogDisplay();
}

void USB_MassStorageModule::hideExitDialog() {
    m_exitDialogShown = false;
    m_exitDefaultBack = true;
}

void USB_MassStorageModule::updateExitDialogDisplay() {
    const int screenWidth = 320;
    const int screenHeight = 240;

    const int dialogWidth = 220;
    const int dialogHeight = 100;
    const int dialogX = (screenWidth - dialogWidth) / 2;
    const int dialogY = (screenHeight - dialogHeight) / 2;

    const int btnY = dialogY + 55;
    const int btnAreaWidth = dialogWidth - 20;

    const int backBtnWidth = m_fontRenderer->getStringLength(strUsdBack) * 16;
    const int confirmBtnWidth = m_fontRenderer->getStringLength(strUsdConfirm) * 16;
    const int totalBtnWidth = backBtnWidth + 20 + confirmBtnWidth;
    const int btnStartX = dialogX + (btnAreaWidth - totalBtnWidth) / 2;

    const int backTextX = btnStartX + backBtnWidth;
    const int confirmTextX = backTextX + 20 + confirmBtnWidth;

    m_tftManager->fillRectangle(dialogX + 10, btnY - 5, dialogWidth - 20, 30, ST7789_BLACK);

    if (m_exitDefaultBack) {
        m_tftManager->setCursor(btnStartX - 12, btnY);
        m_tftManager->setTextColor(ST7789_YELLOW, ST7789_BLACK);
        m_tftManager->print(">");
        m_fontRenderer->drawChineseString(btnStartX, btnY, strUsdBack, ST7789_YELLOW, ST7789_BLACK);

        m_tftManager->setCursor(backTextX + 8, btnY);
        m_tftManager->setTextColor(ST7789_WHITE, ST7789_BLACK);
        m_tftManager->print(">");
        m_fontRenderer->drawChineseString(confirmTextX, btnY, strUsdConfirm, ST7789_WHITE, ST7789_BLACK);
    } else {
        m_tftManager->setCursor(btnStartX - 12, btnY);
        m_tftManager->setTextColor(ST7789_WHITE, ST7789_BLACK);
        m_tftManager->print(">");
        m_fontRenderer->drawChineseString(btnStartX, btnY, strUsdBack, ST7789_WHITE, ST7789_BLACK);

        m_tftManager->setCursor(backTextX + 8, btnY);
        m_tftManager->setTextColor(ST7789_YELLOW, ST7789_BLACK);
        m_tftManager->print(">");
        m_fontRenderer->drawChineseString(confirmTextX, btnY, strUsdConfirm, ST7789_YELLOW, ST7789_BLACK);
    }
}

USB_MassStorageModule usbMassStorageModule;
