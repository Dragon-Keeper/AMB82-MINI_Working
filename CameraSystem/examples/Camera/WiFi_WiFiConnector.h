#ifndef WIFI_WIFICONNECTOR_H
#define WIFI_WIFICONNECTOR_H

#include <Arduino.h>
#include <WiFi.h>
#include "wifi_conf.h"
#include "FlashMemory.h"
#include "BLEDevice.h"
#include "BLEWifiConfigService.h"
#include "Utils_Logger.h"

#define WIFI_CONN_MAX_SAVED_AP      5
#define WIFI_CONN_SAME_SSID_RETRIES 3
#define WIFI_CONN_CONNECT_TIMEOUT   10
#define WIFI_CONN_SSID_MAX_LEN      33
#define WIFI_CONN_PASS_MAX_LEN      64

typedef enum {
    WIFI_CONN_SUCCESS = 0,
    WIFI_CONN_ALREADY_CONNECTED,
    WIFI_CONN_WRONG_IP_SEGMENT,
    WIFI_CONN_ALL_FAILED,
    WIFI_CONN_SAVED_FAILED,
    WIFI_CONN_BLE_FAILED,
    WIFI_CONN_CANCELLED
} WiFiConnectResult;

typedef enum {
    WIFI_CONN_STATE_IDLE = 0,
    WIFI_CONN_STATE_CHECKING,
    WIFI_CONN_STATE_CONNECTING_FORCE,
    WIFI_CONN_STATE_CONNECTING_TIGER,
    WIFI_CONN_STATE_CONNECTING_SAVED,
    WIFI_CONN_STATE_BLE_CONFIG,
    WIFI_CONN_STATE_DONE
} WiFiConnectorState;

typedef struct {
    char ssid[WIFI_CONN_SSID_MAX_LEN];
    char password[WIFI_CONN_PASS_MAX_LEN];
    bool valid;
} SavedAP;

class WiFiConnector {
public:
    WiFiConnector();
    ~WiFiConnector();

    WiFiConnectResult connectWithStrategy(uint8_t targetIpSegment3);
    WiFiConnectResult checkCurrentConnection(uint8_t targetIpSegment3);

    bool isCancelled() const { return _cancelled; }
    void cancel() { _cancelled = true; }

    void loadSavedAPs();
    void saveAP(const char* ssid, const char* password);
    void clearSavedAPs();

    const char* getCurrentSSID();
    const char* getCurrentPassword();
    WiFiConnectorState getState() const { return _state; }
    const char* getResultString(WiFiConnectResult result);

    bool startBLEConfig();
    void stopBLEConfig();
    bool isBLEConfigRunning() const { return _bleConfigRunning; }

    const SavedAP* getSavedAPs() const { return _savedAPs; }
    SavedAP& getSavedAP(int index) { return _savedAPs[index]; }

private:
    WiFiConnectorState _state;
    bool _cancelled;
    SavedAP _savedAPs[WIFI_CONN_MAX_SAVED_AP];
    bool _bleConfigRunning;
    BLEWifiConfigService _bleConfigService;

    bool _connectToAP(const char* ssid, const char* password);
    bool _validateIpSegment(uint8_t targetSegment3);
    int _findSavedAPIndex(const char* ssid);
    void _loadFromFlash();
    void _saveToFlash();

    static const uint32_t _flashMagic;
    static const unsigned int _flashBaseAddr;
};

#endif
