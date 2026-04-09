/*
 * WiFi_WiFiFileServer.h - WiFi File Server Module
 */

#ifndef WIFI_WIFIFILESERVER_H
#define WIFI_WIFIFILESERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AmebaFatFS.h>
#include <ff.h>
#include "Camera_SDCardManager.h"
#include "Display_TFTManager.h"
#include "Display_FontRenderer.h"
#include "Utils_Logger.h"

#define WIFI_FILESERVER_PORT         80
#define WIFI_FILESERVER_MAX_CLIENTS  1
#define WIFI_FILESERVER_MAX_FILES    50
#define WIFI_FILESERVER_BUFFER_SIZE  4096
#define WIFI_FILESERVER_CLIENT_TIMEOUT  30000
#define WIFI_FILESERVER_TRANSFER_TIMEOUT 60000
#define WIFI_FILESERVER_FLUSH_INTERVAL  2048

typedef struct {
    char filename[128];
    uint32_t fileSize;
    uint16_t fdate;
    uint16_t ftime;
} FileInfo;

class WiFiFileServerModule {
public:
    typedef enum {
        STATE_IDLE = 0,
        STATE_STARTING,
        STATE_RUNNING,
        STATE_STOPPING
    } ServerState;

    typedef struct {
        char ssid[32];
        char password[32];
        char channel[4];
    } APConfig;

    typedef struct {
        bool enabled;
        char username[32];
        char password[32];
    } AuthConfig;

    typedef struct {
        unsigned long totalConnections;
        unsigned long totalFilesDownloaded;
        unsigned long totalBytesTransferred;
    } ServerStats;

    WiFiFileServerModule();
    ~WiFiFileServerModule();

    bool init(SDCardManager& sdCardManager, Display_TFTManager& tftManager, Display_FontRenderer& fontRenderer);
    void cleanup();

    bool start();
    void stop();
    void processLoop();

    bool isRunning() const;
    
    void forceShutdown();
    
    ServerState getState() const;
    const ServerStats& getStats() const;

    void setAPConfig(const char* ssid, const char* password, const char* channel);
    void setAuthConfig(bool enabled, const char* username, const char* password);

    void displayInfo();
    void displayConfirmExit();
    void updateExitSelectionDisplay();
    void clearDisplay();

    bool getExitConfirmed() const { return m_exitConfirmed; }
    void setExitConfirmed(bool v) { m_exitConfirmed = v; }
    
    bool getExitSelection() const { return m_exitSelection; }
    void setExitSelection(bool confirm) { m_exitSelection = confirm; }
    void toggleExitSelection() { m_exitSelection = !m_exitSelection; }

    bool canAcceptEncoderInput() const;
    
    bool isShutdownRequested() const { return m_shutdownRequested; }
    unsigned long getStartTime() const { return m_startTimestamp; }
    
    void displayCancelFeedback();
    void displayExitProgress();

private:
    bool setupWiFiAP();
    void shutdownWiFi();

    void handleClientRequest();
    bool authenticateClient(WiFiClient& client);

    void sendLoginPage(WiFiClient& client);
    void sendFileListPage(WiFiClient& client);
    void sendFileDownloadResponse(WiFiClient& client, String filePath);
    void sendErrorResponse(WiFiClient& client, int code, const char* message);
    void sendFileListJSON(WiFiClient& client);
    void sendSystemStatusJSON(WiFiClient& client);
    void sendDeleteFileResponse(WiFiClient& client, String filename);
    void sendShutdownResponse(WiFiClient& client);
    void handleLoginForm(WiFiClient& client);

    String getContentType(String filename);
    String urlDecode(String str);
    String formatFileSize(uint32_t bytes);
    String escapeHTML(String str);
    String base64Decode(String input);

    volatile bool m_exitConfirmed;
    volatile bool m_exitSelection;
    volatile bool m_shutdownRequested;
    bool m_initialized;
    volatile ServerState m_state;
    WiFiServer m_server;
    WiFiClient m_currentClient;
    bool m_clientConnected;
    unsigned long m_clientConnectTime;
    String m_rootPath;
    unsigned long m_startTimestamp;

    SDCardManager* m_sdCardManager;
    Display_TFTManager* m_tftManager;
    Display_FontRenderer* m_fontRenderer;

    APConfig m_apConfig;
    AuthConfig m_authConfig;
    ServerStats m_stats;
};

extern WiFiFileServerModule wifiFileServer;

#endif
