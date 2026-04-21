/*
 * USB_MassStorageModule.h - USB Mass Storage Module
 * Provides USB MSC functionality for SD card access via USB
 */

#ifndef USB_MASSSTORAGEMODULE_H
#define USB_MASSSTORAGEMODULE_H

#include <Arduino.h>
#include <WiFi.h>
#include "wifi_conf.h"
#include "Display_TFTManager.h"
#include "Display_FontRenderer.h"
#include "Display_font16x16.h"
#include "Utils_Logger.h"
#include "Encoder_Control.h"
#include "USBMassStorage.h"

class USB_MassStorageModule {
public:
    typedef enum {
        STATE_IDLE = 0,
        STATE_INITIALIZING,
        STATE_RUNNING,
        STATE_STOPPING
    } ModuleState;

    USB_MassStorageModule();
    ~USB_MassStorageModule();

    bool init(Display_TFTManager& tftManager, Display_FontRenderer& fontRenderer);
    void cleanup();

    void enter();
    void exit();

    void processLoop();
    void handleRotation(RotationDirection direction);
    void handleButton();

    bool isRunning() const { return m_state == STATE_RUNNING; }
    bool isExitConfirmed() const { return m_exitConfirmed; }
    ModuleState getState() const { return m_state; }

    void showExitDialog();
    void hideExitDialog();
    bool isExitDialogShown() const { return m_exitDialogShown; }

    static USB_MassStorageModule& getInstance();

private:
    Display_TFTManager* m_tftManager;
    Display_FontRenderer* m_fontRenderer;
    USBMassStorage m_usbMassStorage;

    ModuleState m_state;
    bool m_initialized;
    bool m_exitDialogShown;
    bool m_exitDefaultBack;
    bool m_exitConfirmed;

    void updateDisplay();
    void updateExitDialogDisplay();
};

extern USB_MassStorageModule usbMassStorageModule;

#endif // USB_MASSSTORAGEMODULE_H