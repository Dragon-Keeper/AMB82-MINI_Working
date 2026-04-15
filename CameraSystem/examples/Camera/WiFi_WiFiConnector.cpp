#include "WiFi_WiFiConnector.h"

const uint32_t WiFiConnector::_flashMagic = 0x57494649;
const unsigned int WiFiConnector::_flashBaseAddr = 0xFE000;

WiFiConnector::WiFiConnector()
    : _state(WIFI_CONN_STATE_IDLE), _cancelled(false), _bleConfigRunning(false)
{
    memset(_savedAPs, 0, sizeof(_savedAPs));
    for (int i = 0; i < WIFI_CONN_MAX_SAVED_AP; i++) {
        _savedAPs[i].valid = false;
    }
    _loadFromFlash();
}

WiFiConnector::~WiFiConnector()
{
    if (_bleConfigRunning) {
        stopBLEConfig();
    }
}

WiFiConnectResult WiFiConnector::checkCurrentConnection(uint8_t targetIpSegment3)
{
    if (WiFi.status() == WL_CONNECTED) {
        IPAddress ip = WiFi.localIP();
        if (ip[2] == targetIpSegment3) {
            Utils_Logger::info("[WiFiConn] Already connected, IP: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
            return WIFI_CONN_ALREADY_CONNECTED;
        } else {
            Utils_Logger::info("[WiFiConn] Connected but wrong IP segment: %d != %d", ip[2], targetIpSegment3);
            return WIFI_CONN_WRONG_IP_SEGMENT;
        }
    }
    return WIFI_CONN_ALL_FAILED;
}

WiFiConnectResult WiFiConnector::connectWithStrategy(uint8_t targetIpSegment3)
{
    _state = WIFI_CONN_STATE_CHECKING;
    _cancelled = false;

    WiFiConnectResult checkResult = checkCurrentConnection(targetIpSegment3);
    if (checkResult == WIFI_CONN_ALREADY_CONNECTED) {
        _state = WIFI_CONN_STATE_DONE;
        return WIFI_CONN_ALREADY_CONNECTED;
    }

    if (checkResult == WIFI_CONN_WRONG_IP_SEGMENT) {
        Utils_Logger::info("[WiFiConn] Wrong IP segment, disconnect and reconnect");
        wifi_config_autoreconnect(0, 0, 0);
        WiFi.disconnect();
        delay(500);
    }

    const char* knownSSIDs[] = {"Force", "Tiger"};
    const char* knownPasswords[] = {"dd123456", "Dt5201314"};
    const int knownCount = 2;

    for (int i = 0; i < knownCount; i++) {
        if (_cancelled) {
            Utils_Logger::info("[WiFiConn] Cancelled by user");
            _state = WIFI_CONN_STATE_DONE;
            return WIFI_CONN_CANCELLED;
        }

        _state = (i == 0) ? WIFI_CONN_STATE_CONNECTING_FORCE : WIFI_CONN_STATE_CONNECTING_TIGER;

        for (int retry = 0; retry < WIFI_CONN_SAME_SSID_RETRIES; retry++) {
            if (_cancelled) {
                Utils_Logger::info("[WiFiConn] Cancelled by user");
                _state = WIFI_CONN_STATE_DONE;
                return WIFI_CONN_CANCELLED;
            }

            Utils_Logger::info("[WiFiConn] Known SSID[%d]=%s attempt %d/%d",
                              i, knownSSIDs[i], retry + 1, WIFI_CONN_SAME_SSID_RETRIES);

            if (_connectToAP(knownSSIDs[i], knownPasswords[i])) {
                if (_validateIpSegment(targetIpSegment3)) {
                    _state = WIFI_CONN_STATE_DONE;
                    return WIFI_CONN_SUCCESS;
                }
                Utils_Logger::info("[WiFiConn] IP mismatch, disconnect retry");
                wifi_config_autoreconnect(0, 0, 0);
                WiFi.disconnect();
                delay(500);
            } else {
                wifi_config_autoreconnect(0, 0, 0);
                WiFi.disconnect();
                delay(500);
            }
        }
    }

    _state = WIFI_CONN_STATE_CONNECTING_SAVED;
    for (int i = 0; i < WIFI_CONN_MAX_SAVED_AP; i++) {
        if (_cancelled) {
            Utils_Logger::info("[WiFiConn] Cancelled by user");
            _state = WIFI_CONN_STATE_DONE;
            return WIFI_CONN_CANCELLED;
        }

        if (!_savedAPs[i].valid) continue;

        bool isKnown = false;
        for (int j = 0; j < knownCount; j++) {
            if (strcmp(_savedAPs[i].ssid, knownSSIDs[j]) == 0) {
                isKnown = true;
                break;
            }
        }
        if (isKnown) continue;

        Utils_Logger::info("[WiFiConn] Trying saved SSID: %s", _savedAPs[i].ssid);

        for (int retry = 0; retry < WIFI_CONN_SAME_SSID_RETRIES; retry++) {
            if (_cancelled) {
                Utils_Logger::info("[WiFiConn] Cancelled by user");
                _state = WIFI_CONN_STATE_DONE;
                return WIFI_CONN_CANCELLED;
            }

            if (_connectToAP(_savedAPs[i].ssid, _savedAPs[i].password)) {
                if (_validateIpSegment(targetIpSegment3)) {
                    _state = WIFI_CONN_STATE_DONE;
                    return WIFI_CONN_SUCCESS;
                }
                wifi_config_autoreconnect(0, 0, 0);
                WiFi.disconnect();
                delay(500);
            } else {
                wifi_config_autoreconnect(0, 0, 0);
                WiFi.disconnect();
                delay(500);
            }
        }
    }

    _state = WIFI_CONN_STATE_BLE_CONFIG;
    Utils_Logger::info("[WiFiConn] All known/saved SSIDs failed, starting BLE WiFi Config");

    if (_cancelled) {
        Utils_Logger::info("[WiFiConn] Cancelled by user");
        _state = WIFI_CONN_STATE_DONE;
        return WIFI_CONN_CANCELLED;
    }

    if (!startBLEConfig()) {
        Utils_Logger::error("[WiFiConn] BLE config start failed");
        _state = WIFI_CONN_STATE_DONE;
        return WIFI_CONN_BLE_FAILED;
    }

    unsigned long bleStartTime = millis();
    const unsigned long bleTimeout = 120000;

    while (!_cancelled && (millis() - bleStartTime < bleTimeout)) {
        if (WiFi.status() == WL_CONNECTED) {
            IPAddress ip = WiFi.localIP();
            Utils_Logger::info("[WiFiConn] BLE config connected! SSID: %s, IP: %d.%d.%d.%d",
                              WiFi.SSID(), ip[0], ip[1], ip[2], ip[3]);

            stopBLEConfig();

            if (ip[2] == targetIpSegment3) {
                String ssidStr = WiFi.SSID();
                bool isKnownSSID = false;
                for (int j = 0; j < knownCount; j++) {
                    if (ssidStr == knownSSIDs[j]) {
                        isKnownSSID = true;
                        break;
                    }
                }
                if (!isKnownSSID) {
                    Utils_Logger::info("[WiFiConn] Saving new SSID: %s", ssidStr.c_str());
                }

                _state = WIFI_CONN_STATE_DONE;
                return WIFI_CONN_SUCCESS;
            } else {
                Utils_Logger::info("[WiFiConn] BLE config connected but wrong IP segment");
                wifi_config_autoreconnect(0, 0, 0);
                WiFi.disconnect();
                delay(500);
                _state = WIFI_CONN_STATE_DONE;
                return WIFI_CONN_WRONG_IP_SEGMENT;
            }
        }
        delay(500);
    }

    stopBLEConfig();

    if (_cancelled) {
        Utils_Logger::info("[WiFiConn] Cancelled by user during BLE config");
        _state = WIFI_CONN_STATE_DONE;
        return WIFI_CONN_CANCELLED;
    }

    Utils_Logger::info("[WiFiConn] BLE config timeout");
    _state = WIFI_CONN_STATE_DONE;
    return WIFI_CONN_BLE_FAILED;
}

bool WiFiConnector::_connectToAP(const char* ssid, const char* password)
{
    Utils_Logger::info("[WiFiConn] Connecting to %s...", ssid);

    char ssidBuf[WIFI_CONN_SSID_MAX_LEN];
    char passBuf[WIFI_CONN_PASS_MAX_LEN];
    strncpy(ssidBuf, ssid, sizeof(ssidBuf) - 1);
    strncpy(passBuf, password, sizeof(passBuf) - 1);
    ssidBuf[sizeof(ssidBuf) - 1] = '\0';
    passBuf[sizeof(passBuf) - 1] = '\0';

    WiFi.begin(ssidBuf, passBuf);

    int retry = 0;
    while (retry < WIFI_CONN_CONNECT_TIMEOUT && WiFi.status() != WL_CONNECTED) {
        delay(500);
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        IPAddress ip = WiFi.localIP();
        Utils_Logger::info("[WiFiConn] Connected to %s, IP: %d.%d.%d.%d",
                          ssid, ip[0], ip[1], ip[2], ip[3]);
        return true;
    }

    Utils_Logger::error("[WiFiConn] Failed to connect to %s", ssid);
    return false;
}

bool WiFiConnector::_validateIpSegment(uint8_t targetSegment3)
{
    if (WiFi.status() != WL_CONNECTED) return false;
    IPAddress ip = WiFi.localIP();
    return ip[2] == targetSegment3;
}

int WiFiConnector::_findSavedAPIndex(const char* ssid)
{
    for (int i = 0; i < WIFI_CONN_MAX_SAVED_AP; i++) {
        if (_savedAPs[i].valid && strcmp(_savedAPs[i].ssid, ssid) == 0) {
            return i;
        }
    }
    return -1;
}

void WiFiConnector::saveAP(const char* ssid, const char* password)
{
    int existingIdx = _findSavedAPIndex(ssid);
    if (existingIdx >= 0) {
        strncpy(_savedAPs[existingIdx].password, password, WIFI_CONN_PASS_MAX_LEN - 1);
        _savedAPs[existingIdx].password[WIFI_CONN_PASS_MAX_LEN - 1] = '\0';
        Utils_Logger::info("[WiFiConn] Updated saved AP: %s", ssid);
    } else {
        int emptyIdx = -1;
        for (int i = 0; i < WIFI_CONN_MAX_SAVED_AP; i++) {
            if (!_savedAPs[i].valid) {
                emptyIdx = i;
                break;
            }
        }
        if (emptyIdx < 0) {
            emptyIdx = 0;
            Utils_Logger::info("[WiFiConn] Saved AP full, overwriting first entry");
        }

        strncpy(_savedAPs[emptyIdx].ssid, ssid, WIFI_CONN_SSID_MAX_LEN - 1);
        _savedAPs[emptyIdx].ssid[WIFI_CONN_SSID_MAX_LEN - 1] = '\0';
        strncpy(_savedAPs[emptyIdx].password, password, WIFI_CONN_PASS_MAX_LEN - 1);
        _savedAPs[emptyIdx].password[WIFI_CONN_PASS_MAX_LEN - 1] = '\0';
        _savedAPs[emptyIdx].valid = true;
        Utils_Logger::info("[WiFiConn] Saved new AP: %s at index %d", ssid, emptyIdx);
    }

    _saveToFlash();
}

void WiFiConnector::clearSavedAPs()
{
    memset(_savedAPs, 0, sizeof(_savedAPs));
    for (int i = 0; i < WIFI_CONN_MAX_SAVED_AP; i++) {
        _savedAPs[i].valid = false;
    }
    _saveToFlash();
    Utils_Logger::info("[WiFiConn] Cleared all saved APs");
}

void WiFiConnector::loadSavedAPs()
{
    _loadFromFlash();
}

const char* WiFiConnector::getCurrentSSID()
{
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.SSID();
    }
    return "";
}

const char* WiFiConnector::getCurrentPassword()
{
    return "";
}

const char* WiFiConnector::getResultString(WiFiConnectResult result)
{
    switch (result) {
        case WIFI_CONN_SUCCESS:            return "Connected";
        case WIFI_CONN_ALREADY_CONNECTED:  return "Already Connected";
        case WIFI_CONN_WRONG_IP_SEGMENT:   return "Wrong IP Segment";
        case WIFI_CONN_ALL_FAILED:         return "All Failed";
        case WIFI_CONN_SAVED_FAILED:       return "Saved SSID Failed";
        case WIFI_CONN_BLE_FAILED:         return "BLE Config Failed";
        case WIFI_CONN_CANCELLED:          return "Cancelled";
        default:                           return "Unknown";
    }
}

bool WiFiConnector::startBLEConfig()
{
    if (_bleConfigRunning) {
        Utils_Logger::info("[WiFiConn] BLE config already running");
        return true;
    }

    Utils_Logger::info("[WiFiConn] Starting BLE WiFi Config...");

    BLE.init();

    BLE.configServer(1);
    _bleConfigService.addService();
    _bleConfigService.begin();

    BLE.beginPeripheral();

    BLE.configAdvert()->stopAdv();
    BLE.configAdvert()->setAdvData(_bleConfigService.advData());
    BLE.configAdvert()->updateAdvertParams();
    delay(100);
    BLE.configAdvert()->startAdv();

    _bleConfigRunning = true;
    Utils_Logger::info("[WiFiConn] BLE WiFi Config started, waiting for connection...");
    return true;
}

void WiFiConnector::stopBLEConfig()
{
    if (!_bleConfigRunning) return;

    Utils_Logger::info("[WiFiConn] Stopping BLE WiFi Config...");

    _bleConfigService.end();
    BLE.configAdvert()->stopAdv();
    BLE.end();
    BLE.deinit();

    _bleConfigRunning = false;
    Utils_Logger::info("[WiFiConn] BLE WiFi Config stopped");
}

void WiFiConnector::_loadFromFlash()
{
    FlashMemoryClass flash;
    flash.begin(_flashBaseAddr, FLASH_SECTOR_SIZE);

    uint8_t* buf = flash.buf;
    flash.read();

    uint32_t magic = *(uint32_t*)buf;
    if (magic != _flashMagic) {
        Utils_Logger::info("[WiFiConn] Flash: no saved AP data (magic mismatch)");
        flash.end();
        return;
    }

    int offset = sizeof(uint32_t);
    for (int i = 0; i < WIFI_CONN_MAX_SAVED_AP; i++) {
        if (offset + sizeof(SavedAP) > flash.buf_size) break;

        memcpy(&_savedAPs[i], buf + offset, sizeof(SavedAP));
        offset += sizeof(SavedAP);

        if (_savedAPs[i].valid) {
            Utils_Logger::info("[WiFiConn] Flash: loaded AP[%d] = %s", i, _savedAPs[i].ssid);
        }
    }

    flash.end();
}

void WiFiConnector::_saveToFlash()
{
    FlashMemoryClass flash;
    flash.begin(_flashBaseAddr, FLASH_SECTOR_SIZE);

    uint8_t* buf = flash.buf;
    flash.read();

    memset(buf, 0, flash.buf_size);

    *(uint32_t*)buf = _flashMagic;
    int offset = sizeof(uint32_t);

    for (int i = 0; i < WIFI_CONN_MAX_SAVED_AP; i++) {
        if (offset + sizeof(SavedAP) > flash.buf_size) break;
        memcpy(buf + offset, &_savedAPs[i], sizeof(SavedAP));
        offset += sizeof(SavedAP);
    }

    flash.write();

    flash.end();
    Utils_Logger::info("[WiFiConn] Saved APs to flash");
}
