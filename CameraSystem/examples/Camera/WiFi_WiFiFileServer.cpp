/*
 * WiFi_WiFiFileServer.cpp - WiFi File Server Module
 */

#include "WiFi_WiFiFileServer.h"
#include "Shared_GlobalDefines.h"

static String ipToString(IPAddress ip) {
    return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

static const char HTML_HEADER[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AmebaPro2 文件服务器</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 16px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.3);
            width: 100%;
            max-width: 900px;
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        .header h1 { font-size: 28px; margin-bottom: 10px; }
        .header p { opacity: 0.9; font-size: 14px; }
        .status-bar {
            background: #f8f9fa;
            padding: 15px 30px;
            border-bottom: 2px solid #e9ecef;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 10px;
        }
        .status-item { font-size: 13px; color: #495057; }
        .status-badge {
            display: inline-block;
            padding: 4px 12px;
            border-radius: 12px;
            font-weight: bold;
            font-size: 12px;
        }
        .status-connected { background: #d4edda; color: #155724; }
        .content { padding: 30px; }
        .file-list {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }
        .file-list th {
            background: #f8f9fa;
            padding: 12px;
            text-align: left;
            font-weight: 600;
            color: #495057;
            border-bottom: 2px solid #dee2e6;
        }
        .file-list td {
            padding: 12px;
            border-bottom: 1px solid #dee2e6;
        }
        .file-list tr:hover { background: #f8f9fa; }
        .file-icon { font-size: 24px; margin-right: 10px; }
        .btn {
            display: inline-block;
            padding: 8px 20px;
            border-radius: 6px;
            text-decoration: none;
            font-weight: 500;
            transition: all 0.3s;
            cursor: pointer;
            border: none;
            font-size: 14px;
        }
        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .btn-primary:hover { transform: translateY(-2px); box-shadow: 0 4px 12px rgba(102,126,234,0.4); }
        .btn-danger { 
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            color: white;
        }
        .btn-danger:hover { transform: translateY(-2px); box-shadow: 0 4px 12px rgba(245,87,108,0.4); }
        .btn-success { background: #28a745; color: white; }
        .btn-sm {
            padding: 6px 12px;
            font-size: 13px;
        }
        .progress-container {
            position: fixed;
            top: 20px;
            right: 20px;
            background: white;
            padding: 20px;
            border-radius: 12px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.2);
            display: none;
            z-index: 1000;
            min-width: 300px;
        }
        .progress-bar-bg {
            width: 100%;
            height: 20px;
            background: #e9ecef;
            border-radius: 10px;
            overflow: hidden;
            margin-top: 10px;
        }
        .progress-bar-fill {
            height: 100%;
            background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
            transition: width 0.3s;
            border-radius: 10px;
        }
        .empty-state {
            text-align: center;
            padding: 60px 20px;
            color: #6c757d;
        }
        .empty-state-icon { font-size: 64px; margin-bottom: 20px; }
        @media (max-width: 768px) {
            .container { margin: 10px; border-radius: 12px; }
            .header { padding: 20px; }
            .content { padding: 20px; }
            .status-bar { padding: 10px 20px; }
        }
    </style>
    <script>
    var JS_OK=true;console.log('[OK] JS LOADED');
    function dl(fn){window.open('/download/'+encodeURIComponent(fn),'_blank');}
    function df(fn,e){
        e&&e.preventDefault();
        e&&e.stopPropagation();
        if(!confirm('删除 "'+fn+'"? '))return;
        (new Image).src='/delete/'+encodeURIComponent(fn)+'?_='+(new Date).getTime();
        setTimeout(function(){location.reload();},800);
    }
    function tsa(cb){var b=document.querySelectorAll('.file-checkbox');for(var i=0;i<b.length;i++)b[i].checked=cb.checked;}
    function bd(){var b=document.querySelectorAll('.file-checkbox:checked');if(b.length==0){alert('请先选择文件!');return;}if(!confirm('确定下载 '+b.length+' 个文件?'))return;var i=0;function n(){if(i>=b.length)return;window.open('/download/'+encodeURIComponent(b[i].value));i++;setTimeout(n,800);}n();}
    function bdel(){
        var b=document.querySelectorAll('.file-checkbox:checked');
        if(b.length==0){alert('请先选择文件!');return;}
        if(!confirm('确定删除 '+b.length+' 个文件?'))return;
        var i=0;
        function n(){
            if(i>=b.length){
                setTimeout(function(){location.reload();},800);
                return;
            }
            (new Image).src='/delete/'+encodeURIComponent(b[i].value)+'?_='+(new Date).getTime();
            i++;
            setTimeout(n,500);
        }
        n();
    }
    console.log('[OK] JS READY');
    </script>
</head>
<body>
<div class="container">
)rawliteral";

static const char LOGIN_FORM[] PROGMEM = R"rawliteral(
<style>
body { font-family: -apple-system, BlinkMacSystemFont, sans-serif; background: linear-gradient(135deg, #667eea, #764ba2); min-height: 100vh; display: flex; justify-content: center; align-items: center; margin: 0; }
.login-box { background: white; padding: 40px; border-radius: 16px; box-shadow: 0 20px 60px rgba(0,0,0,0.3); width: 320px; text-align: center; }
.login-box h2 { color: #333; margin-bottom: 25px; }
.login-box input { width: 100%; padding: 12px 15px; margin: 10px 0; border: 2px solid #e0e0e0; border-radius: 8px; font-size: 14px; outline: none; transition: border-color 0.3s; }
.login-box input:focus { border-color: #667eea; }
.login-box button { width: 100%; padding: 12px; background: linear-gradient(135deg, #667eea, #764ba2); color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; cursor: pointer; margin-top: 15px; transition: opacity 0.3s; }
.login-box button:hover { opacity: 0.9; }
</style>
<form method='POST' action='/login'>
<input type='text' name='username' placeholder='用户名' required><br><br>
<input type='password' name='password' placeholder='密码' required><br><br>
<button type='submit'>登录</button>
</form>
)rawliteral";

WiFiFileServerModule::WiFiFileServerModule()
    : m_exitConfirmed(false)
    , m_exitSelection(false)
    , m_shutdownRequested(false)
    , m_initialized(false)
    , m_state(STATE_IDLE)
    , m_server(WIFI_FILESERVER_PORT)
    , m_currentClient()
    , m_clientConnected(false)
    , m_clientConnectTime(0)
    , m_rootPath("0:/")
    , m_startTimestamp(0)
    , m_sdCardManager(nullptr)
    , m_tftManager(nullptr)
    , m_fontRenderer(nullptr)
{
    strncpy(m_apConfig.ssid, "AmebaPro2_FileServer", sizeof(m_apConfig.ssid) - 1);
    m_apConfig.ssid[sizeof(m_apConfig.ssid) - 1] = '\0';
    strncpy(m_apConfig.password, "12345678", sizeof(m_apConfig.password) - 1);
    m_apConfig.password[sizeof(m_apConfig.password) - 1] = '\0';
    strncpy(m_apConfig.channel, "11", sizeof(m_apConfig.channel) - 1);
    m_apConfig.channel[sizeof(m_apConfig.channel) - 1] = '\0';

    m_authConfig.enabled = false;
    strncpy(m_authConfig.username, "admin", sizeof(m_authConfig.username) - 1);
    m_authConfig.username[sizeof(m_authConfig.username) - 1] = '\0';
    strncpy(m_authConfig.password, "ameba", sizeof(m_authConfig.password) - 1);
    m_authConfig.password[sizeof(m_authConfig.password) - 1] = '\0';

    memset(&m_stats, 0, sizeof(m_stats));
}

WiFiFileServerModule::~WiFiFileServerModule() {
    cleanup();
}

bool WiFiFileServerModule::init(SDCardManager& sdCardManager, Display_TFTManager& tftManager, Display_FontRenderer& fontRenderer) {
    if (m_initialized) return true;

    m_sdCardManager = &sdCardManager;
    m_tftManager = &tftManager;
    m_fontRenderer = &fontRenderer;

    Utils_Logger::info("[WiFiFileServer] Module initialized");
    m_initialized = true;
    return true;
}

void WiFiFileServerModule::cleanup() {
    if (!m_initialized) return;

    stop();

    m_sdCardManager = nullptr;
    m_tftManager = nullptr;
    m_fontRenderer = nullptr;

    m_initialized = false;
    Utils_Logger::info("[WiFiFileServer] Module cleaned up");
}

bool WiFiFileServerModule::start() {
    if (m_state == STATE_RUNNING) {
        Utils_Logger::info("[WiFiFileServer] Already running");
        return true;
    }

    m_state = STATE_STARTING;
    
    m_exitConfirmed = false;
    m_exitSelection = false;
    m_shutdownRequested = false;
    
    Utils_Logger::info("[WiFiFileServer] Starting WiFi server...");

    if (!setupWiFiAP()) {
        Utils_Logger::error("[WiFiFileServer] WiFi AP failed");
        m_state = STATE_IDLE;
        return false;
    }

    m_server.begin();
    m_clientConnected = false;
    memset(&m_stats, 0, sizeof(m_stats));
    m_startTimestamp = millis();

    m_state = STATE_RUNNING;

    Utils_Logger::info("[WiFiFileServer] HTTP server started on port %d", WIFI_FILESERVER_PORT);
    Utils_Logger::info("[WiFiFileServer] AP SSID: %s", m_apConfig.ssid);
    Utils_Logger::info("[WiFiFileServer] AP IP: %s", ipToString(WiFi.localIP()).c_str());

    displayInfo();

    return true;
}

void WiFiFileServerModule::stop() {
    if (m_state == STATE_IDLE) return;

    Utils_Logger::info("[WiFiFileServer] Stopping WiFi server");
    m_state = STATE_STOPPING;

    if (m_clientConnected) {
        m_currentClient.stop();
        m_clientConnected = false;
    }

    m_server.stop();
    delay(100);
    
    shutdownWiFi();

    m_state = STATE_IDLE;
    Utils_Logger::info("[WiFiFileServer] Server stopped");
}

void WiFiFileServerModule::forceShutdown() {
    Utils_Logger::info("[WiFiFileServer] Force shutdown initiated");
    
    m_shutdownRequested = true;
    
    if (m_clientConnected) {
        m_currentClient.stop();
        m_clientConnected = false;
        Utils_Logger::info("[WiFiFileServer] Client connection force-stopped");
    }
    
    m_server.stop();
    m_state = STATE_STOPPING;
    Utils_Logger::info("[WiFiFileServer] State set to STOPPING, processLoop will exit");
}

void WiFiFileServerModule::processLoop() {
    if (m_state != STATE_RUNNING) return;
    if (m_shutdownRequested) return;

    if (!m_clientConnected) {
        WiFiClient newClient = m_server.available();

        if (newClient) {
            if (!newClient.connected()) {
                newClient.stop();
                return;
            }
            
            m_currentClient = newClient;
            m_clientConnected = true;
            m_clientConnectTime = millis();
            m_stats.totalConnections++;

            Utils_Logger::info("[WiFiFileServer] New client connected");

            unsigned long reqStart = millis();
            handleClientRequest();
            unsigned long reqTime = millis() - reqStart;

            Utils_Logger::info("[WiFiFileServer] Request handled in %lums", reqTime);
            delay(5);
            m_currentClient.stop();
            m_clientConnected = false;
        }
    } else {
        if (!m_currentClient.connected() ||
            (millis() - m_clientConnectTime > WIFI_FILESERVER_CLIENT_TIMEOUT) ||
            m_shutdownRequested) {
            Utils_Logger::info("[WiFiFileServer] Client disconnected or timeout or shutdown");
            m_currentClient.stop();
            m_clientConnected = false;
        }
    }
}

bool WiFiFileServerModule::isRunning() const {
    return m_state == STATE_RUNNING;
}

WiFiFileServerModule::ServerState WiFiFileServerModule::getState() const {
    return m_state;
}

const WiFiFileServerModule::ServerStats& WiFiFileServerModule::getStats() const {
    return m_stats;
}

bool WiFiFileServerModule::canAcceptEncoderInput() const {
    if (m_state != STATE_RUNNING) return false;
    if (m_startTimestamp == 0) return false;
    unsigned long elapsed = millis() - m_startTimestamp;
    return (elapsed > 2000);
}

void WiFiFileServerModule::setAPConfig(const char* ssid, const char* password, const char* channel) {
    strncpy(m_apConfig.ssid, ssid, sizeof(m_apConfig.ssid) - 1);
    m_apConfig.ssid[sizeof(m_apConfig.ssid) - 1] = '\0';
    strncpy(m_apConfig.password, password, sizeof(m_apConfig.password) - 1);
    m_apConfig.password[sizeof(m_apConfig.password) - 1] = '\0';
    strncpy(m_apConfig.channel, channel, sizeof(m_apConfig.channel) - 1);
    m_apConfig.channel[sizeof(m_apConfig.channel) - 1] = '\0';
}

void WiFiFileServerModule::setAuthConfig(bool enabled, const char* username, const char* password) {
    m_authConfig.enabled = enabled;
    if (username) {
        strncpy(m_authConfig.username, username, sizeof(m_authConfig.username) - 1);
        m_authConfig.username[sizeof(m_authConfig.username) - 1] = '\0';
    }
    if (password) {
        strncpy(m_authConfig.password, password, sizeof(m_authConfig.password) - 1);
        m_authConfig.password[sizeof(m_authConfig.password) - 1] = '\0';
    }
}

void WiFiFileServerModule::displayInfo() {
    if (m_tftManager == nullptr) return;

    m_tftManager->fillScreen(ST7789_BLACK);

    m_tftManager->setTextColor(ST7789_WHITE, ST7789_BLACK);
    m_tftManager->setTextSize(2);
    m_tftManager->setCursor(5, 8);
    m_tftManager->println("WiFi文件服务器");

    m_tftManager->setTextSize(1);
    m_tftManager->setTextColor(ST7789_CYAN, ST7789_BLACK);

    m_tftManager->setCursor(5, 35);
    m_tftManager->print("SSID: ");
    m_tftManager->println(m_apConfig.ssid);

    m_tftManager->setCursor(5, 50);
    m_tftManager->print("PASS: ");
    m_tftManager->println(m_apConfig.password);

    m_tftManager->setTextColor(ST7789_GREEN, ST7789_BLACK);
    m_tftManager->setCursor(5, 70);
    m_tftManager->print("IP: ");
    m_tftManager->println(ipToString(WiFi.localIP()).c_str());

    m_tftManager->setTextColor(ST7789_WHITE, ST7789_BLACK);
    m_tftManager->setCursor(5, 95);
    m_tftManager->println("浏览器访问:");

    m_tftManager->setTextColor(ST7789_YELLOW, ST7789_BLACK);
    m_tftManager->setCursor(5, 112);
    m_tftManager->print("http://");
    m_tftManager->println(ipToString(WiFi.localIP()).c_str());

    m_tftManager->setTextColor(ST7789_ORANGE, ST7789_BLACK);
    m_tftManager->setCursor(5, 145);
    m_tftManager->println("* 旋转旋钮退出 *");

    m_exitConfirmed = false;
    m_exitSelection = false;
    
    Utils_Logger::info("[WiFiFileServer] Display info updated on TFT");
}

void WiFiFileServerModule::displayConfirmExit() {
    if (m_tftManager == nullptr) return;

    m_tftManager->fillScreen(ST7789_BLACK);

    m_tftManager->setTextColor(ST7789_WHITE, ST7789_BLACK);
    m_tftManager->setTextSize(2);
    m_tftManager->setCursor(10, 15);
    m_tftManager->println("Exit File Transfer?");

    m_tftManager->setTextSize(1);
    m_exitSelection = false;
    m_exitConfirmed = true;
    
    Utils_Logger::info("[WiFiFileServer] Display confirm exit dialog");
    updateExitSelectionDisplay();
}

void WiFiFileServerModule::updateExitSelectionDisplay() {
    if (m_tftManager == nullptr) return;

    int yCancel = 60;
    int yConfirm = 85;

    m_tftManager->fillRectangle(0, yCancel - 5, ST7789_TFTWIDTH, 50, ST7789_BLACK);

    m_tftManager->setTextSize(1);

    if (!m_exitSelection) {
        m_tftManager->setTextColor(ST7789_BLACK, ST7789_YELLOW);
        m_tftManager->fillRectangle(5, yCancel - 2, ST7789_TFTWIDTH - 10, 18, ST7789_YELLOW);
    } else {
        m_tftManager->setTextColor(ST7789_YELLOW, ST7789_BLACK);
    }
    m_tftManager->setCursor(10, yCancel);
    m_tftManager->println("[*] Cancel");

    m_tftManager->setTextColor(ST7789_WHITE, ST7789_BLACK);
    if (m_exitSelection) {
        m_tftManager->setTextColor(ST7789_BLACK, ST7789_GREEN);
        m_tftManager->fillRectangle(5, yConfirm - 2, ST7789_TFTWIDTH - 10, 18, ST7789_GREEN);
    } else {
        m_tftManager->setTextColor(ST7789_GREEN, ST7789_BLACK);
    }
    m_tftManager->setCursor(10, yConfirm);
    m_tftManager->println("[ ] Confirm");

    m_tftManager->setTextColor(ST7789_GRAY, ST7789_BLACK);
    m_tftManager->setCursor(5, 130);
    m_tftManager->println("Rotate: Switch option");
    m_tftManager->setCursor(5, 148);
    m_tftManager->println("Press: Execute action");
}

void WiFiFileServerModule::displayCancelFeedback() {
    if (m_tftManager == nullptr) return;
    
    m_tftManager->fillScreen(ST7789_BLACK);
    m_tftManager->setTextColor(ST7789_GREEN, ST7789_BLACK);
    m_tftManager->setTextSize(2);
    m_tftManager->setCursor(35, 60);
    m_tftManager->println("Cancelled");
    
    m_tftManager->setTextSize(1);
    m_tftManager->setTextColor(ST7789_GRAY, ST7789_BLACK);
    m_tftManager->setCursor(20, 100);
    m_tftManager->println("Returning to info...");
    
    Utils_Logger::info("[WiFiFileServer] Cancel feedback displayed");
}

void WiFiFileServerModule::displayExitProgress() {
    if (m_tftManager == nullptr) return;
    
    m_tftManager->fillScreen(ST7789_BLACK);
    m_tftManager->setTextColor(ST7789_YELLOW, ST7789_BLACK);
    m_tftManager->setTextSize(2);
    m_tftManager->setCursor(20, 60);
    m_tftManager->println("Exiting...");
    
    m_tftManager->setTextSize(1);
    m_tftManager->setTextColor(ST7789_GRAY, ST7789_BLACK);
    m_tftManager->setCursor(10, 100);
    m_tftManager->println("Closing WiFi Server");
    
    Utils_Logger::info("[WiFiFileServer] Exit progress displayed");
}

void WiFiFileServerModule::clearDisplay() {
    if (m_tftManager != nullptr) {
        m_tftManager->fillScreen(ST7789_BLACK);
    }
}

bool WiFiFileServerModule::setupWiFiAP() {
    Utils_Logger::info("[WiFiFileServer] Starting AP... SSID: %s", m_apConfig.ssid);

    WiFi.disconnect();
    delay(200);
    
    int retries = 0;
    const int MAX_RETRIES = 5;

    while (WiFi.apbegin(m_apConfig.ssid, m_apConfig.password, m_apConfig.channel) != WL_CONNECTED) {
        retries++;
        if (retries >= MAX_RETRIES) {
            Utils_Logger::error("[WiFiFileServer] AP start failed after %d attempts", MAX_RETRIES);
            return false;
        }
        delay(500);
    }

    IPAddress localIP(192, 168, 1, 1);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(localIP, gateway, subnet);

    delay(200);

    Utils_Logger::info("[WiFiFileServer] AP started successfully");
    return true;
}

void WiFiFileServerModule::shutdownWiFi() {
    m_server.stop();
    delay(50);
    WiFi.disconnect();
    delay(200);
    Utils_Logger::info("[WiFiFileServer] WiFi shutdown complete (switched to STA mode)");
}

void WiFiFileServerModule::handleClientRequest() {
    String requestLine = "";
    unsigned long timeout = millis();

    while (m_currentClient.connected() && (millis() - timeout < 3000)) {
        if (m_shutdownRequested) return;
        if (m_currentClient.available()) {
            char c = m_currentClient.read();
            if (c == '\n') {
                break;
            }
            if (c != '\r') requestLine += c;
        }
    }

    if (requestLine.length() == 0) {
        Utils_Logger::info("[WiFiFileServer] Empty request");
        return;
    }

    if (m_shutdownRequested) return;

    Utils_Logger::info("[WiFiFileServer] Request: %s", requestLine.c_str());

    int firstSpace = requestLine.indexOf(' ');
    int secondSpace = requestLine.indexOf(' ', firstSpace + 1);

    if (firstSpace == -1 || secondSpace == -1) {
        sendErrorResponse(m_currentClient, 400, "Bad Request");
        return;
    }

    String method = requestLine.substring(0, firstSpace);
    String path = requestLine.substring(firstSpace + 1, secondSpace);
    path = urlDecode(path);

    if (method == "POST") {
        timeout = millis();
        while (m_currentClient.connected() && (millis() - timeout < 2000)) {
            if (m_currentClient.available()) {
                String headerLine = m_currentClient.readStringUntil('\n');
                if (headerLine.length() <= 2) break;
            }
        }
    }

    if (method == "GET") {
        if (path != "/login" && path != "/favicon.ico") {
            if (!authenticateClient(m_currentClient)) return;
        }

        if (path == "/" || path == "/index.html" || path.startsWith("/goform/")) {
            sendFileListPage(m_currentClient);
        } else if (path.startsWith("/download/")) {
            String filename = path.substring(10);
            String fullPath = m_rootPath + filename;
            sendFileDownloadResponse(m_currentClient, fullPath);
        } else if (path == "/login") {
            sendLoginPage(m_currentClient);
        } else if (path == "/api/files") {
            sendFileListJSON(m_currentClient);
        } else if (path == "/api/status") {
            sendSystemStatusJSON(m_currentClient);
        } else if (path == "/shutdown") {
            sendShutdownResponse(m_currentClient);
        } else if (path.startsWith("/delete/")) {
            String filename = path.substring(8);
            int qmark = filename.indexOf('?');
            if (qmark > 0) filename = filename.substring(0, qmark);
            sendDeleteFileResponse(m_currentClient, filename);
        } else {
            sendErrorResponse(m_currentClient, 404, "Not Found");
        }
    } else if (method == "POST") {
        if (path == "/login") {
            handleLoginForm(m_currentClient);
        } else {
            sendErrorResponse(m_currentClient, 405, "Method Not Allowed");
        }
    } else {
        sendErrorResponse(m_currentClient, 405, "Method Not Allowed");
    }
}

bool WiFiFileServerModule::authenticateClient(WiFiClient& client) {
    if (!m_authConfig.enabled) {
        return true;
    }

    String authHeader = "";
    bool authFound = false;

    unsigned long timeout = millis();
    while (client.available() && (millis() - timeout < 1000)) {
        String line = client.readStringUntil('\n');
        if (line.startsWith("Authorization:")) {
            authHeader = line.substring(14);
            authFound = true;
            break;
        }
        if (line.length() == 0 || line == "\r") {
            break;
        }
    }

    if (!authFound || authHeader.length() == 0) {
        client.println("HTTP/1.1 401 Unauthorized");
        client.println("WWW-Authenticate: Basic realm=\"AmebaPro2 File Server\"");
        client.println("Content-Type: text/html; charset=UTF-8");
        client.println("Connection: close");
        client.println();
        client.println(F(HTML_HEADER));
        client.println(F(LOGIN_FORM));
        client.println("</body></html>");
        return false;
    }

    if (authHeader.startsWith("Basic ")) {
        String base64Credentials = authHeader.substring(6);
        String decoded = base64Decode(base64Credentials);

        int colonPos = decoded.indexOf(':');
        if (colonPos > 0) {
            String username = decoded.substring(0, colonPos);
            String password = decoded.substring(colonPos + 1);

            if (username == m_authConfig.username && password == m_authConfig.password) {
                return true;
            }
        }
    }

    client.println("HTTP/1.1 401 Unauthorized");
    client.println("WWW-Authenticate: Basic realm=\"AmebaPro2 File Server\"");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();
    client.println(F(HTML_HEADER));
        client.println("<div class='container'><div class='content' style='text-align:center;padding:40px;'>");
        client.println("<h2 style='color:#dc3545;'>认证失败</h2>");
        client.println("<p>用户名或密码错误</p>");
        client.println("<a href='/' class='btn btn-primary'>重试</a>");
        client.println("</div></div></body></html>");
        return false;
}

void WiFiFileServerModule::sendLoginPage(WiFiClient& client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();
    client.println(F(HTML_HEADER));
    client.println(F(LOGIN_FORM));
    client.println("</body></html>");
}

void WiFiFileServerModule::sendFileListPage(WiFiClient& client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("Cache-Control: no-cache");
    client.println("Connection: close");
    client.println();

    if (m_shutdownRequested) return;

    client.println(F(HTML_HEADER));
    client.println("<div class='header'>");
    client.println("<h1>AmebaPro2 文件服务器</h1>");
    client.println("<p>浏览和下载SD卡文件</p>");
    client.println("</div>");

    IPAddress ip = WiFi.localIP();
    long long freeSpaceBytes = m_sdCardManager->getFileSystem()->get_free_space();
    long long freeSpaceMB = freeSpaceBytes / (1024 * 1024);

    Utils_Logger::info("[WiFiFileServer] Free space (bytes): %lld", freeSpaceBytes);
    Utils_Logger::info("[WiFiFileServer] Free space (MB): %lld", freeSpaceMB);

    client.println("<div class='status-bar'>");
    client.print("<div class='status-item'><strong>状态:</strong> <span class='status-badge status-connected'>在线</span></div>");
    client.print("<div class='status-item'><strong>IP:</strong> ");
    client.print(ipToString(ip).c_str());
    client.println("</div>");
    client.print("<div class='status-item'><strong>下载:</strong> ");
    client.print(m_stats.totalFilesDownloaded);
    client.print(" 个文件 / ");
    client.print(formatFileSize(m_stats.totalBytesTransferred));
    client.println("</div>");
    client.print("<div class='status-item'><strong>剩余空间:</strong> ");
    client.print((unsigned long)freeSpaceMB);
    client.println(" MB</div>");
    client.println("</div>");

    client.println("<div class='content'>");
    client.println("<div style='display:flex;justify-content:space-between;align-items:center;margin-bottom:20px;'>");
    client.println("<h2 style='color:#495057;'>文件列表</h2>");
    client.println("<div style='display:flex;gap:10px;'>");
    client.println("<button onclick='bd()' class='btn btn-primary'>批量下载</button>");
    client.println("<button onclick='bdel()' class='btn btn-danger'>批量删除</button>");
    client.println("<a href='/shutdown' class='btn btn-danger' onclick=\"return confirm('确定要退出服务器吗？');\">退出</a>");
    client.println("</div>");
    client.println("</div>");

    client.println("<table class='file-list'>");
    client.println("<thead><tr>");
    client.println("<th style='width:50px;'><input type='checkbox' id='select-all' onchange='tsa(this)'></th>");
    client.println("<th>文件名</th>");
    client.println("<th style='width:120px;'>类型</th>");
    client.println("<th style='width:120px;'>大小</th>");
    client.println("<th style='width:120px;'>下载</th>");
    client.println("<th style='width:120px;'>删除</th>");
    client.println("</tr></thead>");
    client.println("<tbody>");

    FileInfo* fileList = new FileInfo[WIFI_FILESERVER_MAX_FILES];
    if (fileList == nullptr) {
        client.println("<tr><td colspan='6'><div class='empty-state'>错误</div></td></tr>");
        client.println("</tbody></table></div></div>");
        client.println("<div id='progress' class='progress-container'>");
        client.println("<strong style='color:#495057;'>Downloading:</strong> <span class='filename'></span>");
        client.println("<div id='progress-text' style='font-size:12px;color:#6c757d;margin-top:5px;'>Preparing...</div>");
        client.println("<div class='progress-bar-bg'><div id='progress-fill' class='progress-bar-fill' style='width:0%'></div></div>");
        client.println("</div>");
        client.println("</body></html>");
        return;
    }

    DIR dir;
    FILINFO fno;
    int fileCount = 0;

    Utils_Logger::info("[WiFiFileServer] Opening directory: %s", m_rootPath.c_str());
    
    FRESULT res = f_opendir(&dir, m_rootPath.c_str());
    if (res == FR_OK) {
        while (fileCount < WIFI_FILESERVER_MAX_FILES) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            if ((fno.fattrib & AM_DIR) || fno.fname[0] == '.') continue;

            strncpy(fileList[fileCount].filename, fno.fname, 127);
            fileList[fileCount].filename[127] = '\0';
            fileList[fileCount].fileSize = fno.fsize;
            fileList[fileCount].fdate = fno.fdate;
            fileList[fileCount].ftime = fno.ftime;
            fileCount++;
        }
        f_closedir(&dir);

        for (int i = 0; i < fileCount - 1; i++) {
            for (int j = i + 1; j < fileCount; j++) {
                if (fileList[j].fdate > fileList[i].fdate ||
                    (fileList[j].fdate == fileList[i].fdate && fileList[j].ftime > fileList[i].ftime)) {
                    FileInfo temp = fileList[i];
                    fileList[i] = fileList[j];
                    fileList[j] = temp;
                }
            }
        }
    }

    Utils_Logger::info("[WiFiFileServer] Total files found: %d", fileCount);

    unsigned long sentBytes = 0;
    for (int i = 0; i < fileCount; i++) {
        if (m_shutdownRequested) break;

        String filename = String(fileList[i].filename);
        uint32_t fileSize = fileList[i].fileSize;
        String ext = filename.substring(filename.lastIndexOf('.') + 1);
        ext.toLowerCase();

        String fileType, fileIcon;
        if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "bmp" || ext == "gif") {
            fileType = "图片";
            fileIcon = "\U0001F5BC";
        } else if (ext == "mp4" || ext == "avi" || ext == "mov" || ext == "mkv") {
            fileType = "视频";
            fileIcon = "\U0001F3AC";
        } else if (ext == "mp3" || ext == "wav" || ext == "aac" || ext == "flac") {
            fileType = "音频";
            fileIcon = "\U0001F3B5";
        } else {
            fileType = "文件";
            fileIcon = "\U0001F4C4";
        }

        String fn = escapeHTML(filename);
        client.print("<tr>");
        client.print("<td><input type='checkbox' class='file-checkbox' value='");
        client.print(fn);
        client.print("' data-size='");
        client.print(fileSize);
        client.print("'></td>");
        client.print("<td><span class='file-icon'>");
        client.print(fileIcon);
        client.print("</span><strong>");
        client.print(fn);
        client.println("</strong></td>");
        client.print("<td>");
        client.print(fileType);
        client.println("</td>");
        client.print("<td>");
        client.print(formatFileSize(fileSize));
        client.println("</td>");
        client.print("<td>");
        client.print("<button type='button' class='btn btn-primary btn-sm' onclick='dl(\"");
        client.print(fn);
        client.print("\")'>下载</button>");
        client.println("</td>");
        client.print("<td>");
        client.print("<button type='button' class='btn btn-danger btn-sm' onclick='df(\"");
        client.print(fn);
        client.print("\",event)'>删除</button>");
        client.println("</td>");
        client.println("</tr>");

        Utils_Logger::info("[WiFiFileServer] File #%d: %s (%lu bytes)", i + 1, filename.c_str(), fileSize);

        sentBytes += 128 + fn.length() * 2;
        if (sentBytes >= WIFI_FILESERVER_FLUSH_INTERVAL) {
            sentBytes = 0;
            delay(2);
        }
    }

    delete[] fileList;

    if (fileCount == 0) {
        client.println("<tr><td colspan='6'>");
        client.println("<div class='empty-state'>");
        client.println("<div class='empty-state-icon'>&#x1F4C4;</div>");
        client.println("<h3>暂无文件</h3>");
        client.println("<p>SD卡为空或未找到文件</p>");
        client.println("</div>");
        client.println("</td></tr>");
    }

    client.println("</tbody>");
    client.println("</table>");
    client.println("</div>");
    client.println("</div>");

    client.println("<div id='progress' class='progress-container'>");
    client.println("<strong style='color:#495057;'>正在下载:</strong> <span class='filename'></span>");
    client.println("<div id='progress-text' style='font-size:12px;color:#6c757d;margin-top:5px;'>准备中...</div>");
    client.println("<div class='progress-bar-bg'><div id='progress-fill' class='progress-bar-fill' style='width:0%'></div></div>");
    client.println("</div>");

    client.println("</body></html>");
}

void WiFiFileServerModule::sendFileDownloadResponse(WiFiClient& client, String filePath) {
    AmebaFatFS* fs = m_sdCardManager->getFileSystem();
    File file = fs->open(filePath.c_str());

    if (!file) {
        sendErrorResponse(client, 404, "File not found");
        return;
    }

    uint32_t fileSize = file.size();
    String contentType = getContentType(filePath);
    String filename = filePath.substring(filePath.lastIndexOf('/') + 1);

    uint32_t rangeStart = 0;
    uint32_t rangeEnd = fileSize - 1;
    bool isRangeRequest = false;

    String headers = "";
    unsigned long timeout = millis();
    while (client.available() && (millis() - timeout < 1000)) {
        String line = client.readStringUntil('\n');
        headers += line + "\n";
        if (line.startsWith("Range: bytes=")) {
            String rangeValue = line.substring(13);
            int dashPos = rangeValue.indexOf('-');
            if (dashPos > 0) {
                rangeStart = rangeValue.substring(0, dashPos).toInt();
                if (dashPos < (int)rangeValue.length() - 1) {
                    rangeEnd = rangeValue.substring(dashPos + 1).toInt();
                }
                isRangeRequest = true;
            }
        }
        if (line.length() == 0 || line == "\r") break;
    }

    if (isRangeRequest) {
        file.seek(rangeStart);
    }

    uint32_t contentLength = rangeEnd - rangeStart + 1;

    if (isRangeRequest) {
        client.print("HTTP/1.1 206 Partial Content\r\n");
        client.print("Content-Range: bytes ");
        client.print(rangeStart);
        client.print("-");
        client.print(rangeEnd);
        client.print("/");
        client.print(fileSize);
        client.print("\r\n");
    } else {
        client.println("HTTP/1.1 200 OK");
    }

    client.print("Content-Type: ");
    client.println(contentType);
    client.print("Content-Length: ");
    client.println(contentLength);
    client.print("Content-Disposition: attachment; filename=\"");
    client.print(filename);
    client.println("\"");
    client.println("Accept-Ranges: bytes");
    client.println("Cache-Control: no-cache");
    client.println("Connection: close");
    client.println();

    uint8_t buffer[WIFI_FILESERVER_BUFFER_SIZE];
    uint32_t remaining = contentLength;
    uint32_t totalSent = 0;

    while (remaining > 0 && client.connected()) {
        if (m_shutdownRequested) break;
        uint32_t chunkSize = min(remaining, (uint32_t)WIFI_FILESERVER_BUFFER_SIZE);
        int bytesRead = file.read(buffer, chunkSize);

        if (bytesRead > 0) {
            int bytesWritten = client.write(buffer, bytesRead);
            if (bytesWritten > 0) {
                remaining -= bytesWritten;
                totalSent += bytesWritten;
            } else {
                break;
            }
        } else {
            break;
        }

        delay(1);
    }

    file.close();

    m_stats.totalFilesDownloaded++;
    m_stats.totalBytesTransferred += totalSent;

    Utils_Logger::info("[WiFiFileServer] Downloaded: %s (%lu bytes)", filename.c_str(), totalSent);
}

void WiFiFileServerModule::sendFileListJSON(WiFiClient& client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json; charset=UTF-8");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Cache-Control: no-cache");
    client.println("Connection: close");
    client.println();

    client.println("{\"files\": [");

    DIR dir;
    FILINFO fno;
    int fileCount = 0;
    bool first = true;

    FRESULT res = f_opendir(&dir, m_rootPath.c_str());
    if (res == FR_OK) {
        while (fileCount < WIFI_FILESERVER_MAX_FILES) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            if (fno.fattrib & AM_DIR) continue;

            String filename = String(fno.fname);
            if (filename.length() == 0 || filename[0] == '.') continue;

            if (!first) client.println(",");
            first = false;

            client.println("{");
            client.print("\"name\": \"");
            client.print(filename);
            client.println("\",");
            client.print("\"size\": ");
            client.println(fno.fsize);
            client.print(",\"type\": \"");

            String ext = filename.substring(filename.lastIndexOf('.') + 1);
            ext.toLowerCase();
            if (ext == "jpg" || ext == "jpeg" || ext == "png") client.print("image");
            else if (ext == "mp4" || ext == "avi") client.print("video");
            else if (ext == "mp3" || ext == "wav") client.print("audio");
            else client.print("unknown");

            client.println("\"");
            client.print("}");
            fileCount++;
        }
        f_closedir(&dir);
    }

    client.println("],");
    client.print("\"count\": ");
    client.println(fileCount);
    client.println("}");
}

void WiFiFileServerModule::sendSystemStatusJSON(WiFiClient& client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json; charset=UTF-8");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close");
    client.println();

    client.println("{");
    client.print("\"status\": \"online\",");
    client.print("\"ip\": \"");
    client.print(ipToString(WiFi.localIP()).c_str());
    client.print("\",");
    client.print("\"ssid\": \"");
    client.print(m_apConfig.ssid);
    client.print("\",");
    client.print("\"rssi\": ");
    client.print(WiFi.RSSI());
    client.print(",");
    client.print("\"freeSpaceMB\": ");
    client.print(m_sdCardManager->getFileSystem()->get_free_space() / (1024 * 1024));
    client.print(",");
    client.print("\"totalDownloads\": ");
    client.print(m_stats.totalFilesDownloaded);
    client.print(",");
    client.print("\"totalTransferredBytes\": ");
    client.print(m_stats.totalBytesTransferred);
    client.print(",");
    client.print("\"uptimeSeconds\": ");
    client.print(millis() / 1000);
    client.println("}");
}

void WiFiFileServerModule::sendDeleteFileResponse(WiFiClient& client, String filename) {
    String fullPath = m_rootPath + filename;

    AmebaFatFS* fs = m_sdCardManager->getFileSystem();

    if (!fs->exists(fullPath.c_str())) {
        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-Type: application/json; charset=UTF-8");
        client.println("Connection: close");
        client.println();
        client.println("{\"success\":false,\"error\":\"File not found\"}");
        return;
    }

    bool success = fs->remove(fullPath.c_str());

    if (success) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json; charset=UTF-8");
        client.println("Connection: close");
        client.println();
        client.print("{\"success\":true,\"message\":\"Deleted: ");
        client.print(filename);
        client.println("\"}");
        Utils_Logger::info("[WiFiFileServer] File deleted: %s", filename.c_str());
    } else {
        client.println("HTTP/1.1 500 Internal Server Error");
        client.println("Content-Type: application/json; charset=UTF-8");
        client.println("Connection: close");
        client.println();
        client.println("{\"success\":false,\"error\":\"Delete failed\"}");
        Utils_Logger::info("[WiFiFileServer] Delete failed: %s", filename.c_str());
    }
}

void WiFiFileServerModule::sendShutdownResponse(WiFiClient& client) {
    Utils_Logger::info("[WiFiFileServer] Shutdown request received");
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();
    
    client.println(F(HTML_HEADER));
    client.println("<div class='container' style='text-align:center;padding:50px;'>");
    client.println("<div style='font-size:48px;margin-bottom:20px;'>&#x1F6D1;</div>");
    client.println("<h2>服务器正在关闭</h2>");
    client.println("<p>AmebaPro2 文件服务器即将退出...</p>");
    client.println("<p style='margin-top:30px;color:#868e96;'>请等待安全关机</p>");
    client.println("</div>");
    client.println("</body></html>");
    
    delay(100);
    
    m_shutdownRequested = true;
    m_exitConfirmed = true;
    
    Utils_Logger::info("[WiFiFileServer] Server marked for exit");
}

void WiFiFileServerModule::handleLoginForm(WiFiClient& client) {
    String body = "";
    unsigned long timeout = millis();

    while (client.connected() && (millis() - timeout < 5000)) {
        if (client.available()) {
            body += client.readStringUntil('\r');
            if (body.endsWith("\n\n") || body.length() > 512) break;
        }
    }

    String username = "";
    String password = "";

    int userStart = body.indexOf("username=");
    int passStart = body.indexOf("&password=");

    if (userStart != -1 && passStart != -1) {
        username = body.substring(userStart + 9, passStart);
        password = body.substring(passStart + 10);
        username = urlDecode(username);
        password = urlDecode(password);
    }

    if (username == m_authConfig.username && password == m_authConfig.password) {
        client.println("HTTP/1.1 302 Found");
        client.println("Location: /");
        client.println("Set-Cookie: session=authenticated; Path=/; Max-Age=3600");
        client.println("Connection: close");
        client.println();
        return;
    } else {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html; charset=UTF-8");
        client.println("Connection: close");
        client.println();
        client.println(F(HTML_HEADER));
        client.println("<div class='container'><div class='content' style='text-align:center;padding:40px;'>");
        client.println("<h2 style='color:#dc3545;'>Auth Failed</h2>");
        client.println("<p>Invalid credentials, please try again</p>");
        client.println("<a href='/' class='btn btn-primary'>Back to Login</a>");
        client.println("</div></div></body></html>");
    }
}

void WiFiFileServerModule::sendErrorResponse(WiFiClient& client, int code, const char* message) {
    client.print("HTTP/1.1 ");
    client.print(code);
    client.print(" ");
    client.println(message);
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();
    
    client.println(F(HTML_HEADER));
    client.println("<div class='container'><div class='content' style='text-align:center;padding:60px;'>");
    client.print("<h1 style='font-size:72px;color:#dc3545;margin-bottom:20px;'>");
    client.print(code);
    client.println("</h1>");
    client.print("<h2 style='color:#495057;margin-bottom:20px;'>");
    client.print(message);
    client.println("</h2>");
    client.println("<p style='color:#6c757d;margin-bottom:30px;'>发生错误</p>");
    client.println("<a href='/' class='btn btn-primary'>返回</a>");
    client.println("</div></div></body></html>");
    
    Utils_Logger::error("[WiFiFileServer] Error: %d - %s", code, message);
}

String WiFiFileServerModule::getContentType(String filename) {
    String ext = filename.substring(filename.lastIndexOf('.') + 1);
    ext.toLowerCase();

    if (ext == "htm" || ext == "html") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "png") return "image/png";
    if (ext == "gif") return "image/gif";
    if (ext == "mp4") return "video/mp4";
    if (ext == "avi") return "video/x-msvideo";
    if (ext == "mp3") return "audio/mpeg";
    if (ext == "wav") return "audio/wav";
    if (ext == "pdf") return "application/pdf";
    if (ext == "zip") return "application/zip";
    if (ext == "txt") return "text/plain";
    if (ext == "json") return "application/json";
    if (ext == "xml") return "application/xml";
    return "application/octet-stream";
}

String WiFiFileServerModule::urlDecode(String str) {
    String decoded = "";
    char temp[] = "0x00";
    unsigned int len = str.length();
    unsigned int i = 0;
    while (i < len) {
        char c = str.charAt(i);
        if (c == '+' && i < len) {
            decoded += ' ';
        } else if (c == '%' && i + 2 < len) {
            temp[2] = str.charAt(i + 1);
            temp[3] = str.charAt(i + 2);
            decoded += char(strtol(temp, NULL, 16));
            i += 2;
        } else {
            decoded += c;
        }
        i++;
    }
    return decoded;
}

String WiFiFileServerModule::formatFileSize(uint32_t bytes) {
    if (bytes < 1024) return String(bytes) + " B";
    if (bytes < 1024 * 1024) return String(bytes / 1024) + "." + String((bytes % 1024) * 10 / 1024) + " KB";
    if (bytes < 1024UL * 1024 * 1024) return String(bytes / (1024 * 1024)) + "." + String((bytes % (1024 * 1024)) * 100 / (1024 * 1024)) + " MB";
    return String(bytes / (1024UL * 1024 * 1024)) + "." + String((bytes % (1024UL * 1024 * 1024)) * 100 / (1024UL * 1024 * 1024)) + " GB";
}

String WiFiFileServerModule::escapeHTML(String str) {
    String result = "";
    for (unsigned int i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (c == '&') result += "&amp;";
        else if (c == '<') result += "&lt;";
        else if (c == '>') result += "&gt;";
        else if (c == '"') result += "&quot;";
        else if (c == '\'') result += "&#39;";
        else result += c;
    }
    return result;
}

String WiFiFileServerModule::base64Decode(String input) {
    static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    String output = "";
    int val = 0, valb = -8;
    for (unsigned int i = 0; i < input.length(); i++) {
        char c = input.charAt(i);
        if (c == '=') break;
        const char *pos = strchr(base64_chars, c);
        if (pos == nullptr) continue;
        val = (val << 6) | (pos - base64_chars);
        valb += 6;
        if (valb >= 0) {
            output += char((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return output;
}
