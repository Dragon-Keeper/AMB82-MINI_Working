#define ARDUINOJSON_STRING_LENGTH_SIZE_4

#include "OTA.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include "ota_drv.h"
#include "wiring_os.h"

uint32_t OTA::thread1_id = 0;
uint32_t OTA::thread2_id = 0;
int OTA::_port = 0;
char* OTA::_server = nullptr;
const char* OTA::g_otaState = nullptr;
char OTA::buffer[1024] = {0};
String OTA::jsonString;

int OTA::priority1 = 0;
uint32_t OTA::stack_size1 = 0;
uint32_t OTA::stack_size2 = 0;

const char* OTA::OtaState[] = {
    "OTA_STATE_IDLE",
    "OTA_STATE_RECEIVED_START_SIGNAL",
    "OTA_STATE_DOWNLOAD_FIRMWARE_IN_PROGRESS",
    "OTA_STATE_DOWNLOAD_FIRMWARE_COMPLETED",
    "OTA_STATE_REBOOT",
    "OTA_STATE_FAILED"
};

const char* OTA::OTA_STATE_IDLE = OTA::OtaState[0];
const char* OTA::OTA_STATE_FAILED = OTA::OtaState[5];

OTA::OTA() {
    g_otaState = OtaState[0];
};

OTA::~OTA(){};

void OTA::sendPostRequest()
{
    static JsonDocument doc;

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[OTA] WiFi未连接，跳过POST请求");
        delay(3000);
        return;
    }

    doc["OTA_state"] = g_otaState;
    jsonString = "";
    serializeJson(doc, jsonString);

    Serial.print("[OTA] 发送POST请求到: ");
    Serial.print(_server);
    Serial.print(":");
    Serial.println(_port);
    Serial.print("[OTA] JSON数据: ");
    Serial.println(jsonString);

#undef connect
#undef read
#undef write
    WiFiClient wifiClient;

    if (wifiClient.connect(_server, _port) && wifiClient.connected()) {
        Serial.println("[OTA] 服务器连接成功，发送HTTP请求...");
        
        wifiClient.println("POST /api/connectedclients HTTP/1.1");
        wifiClient.println("Host: " + String(_server));
        wifiClient.println("Content-Type: application/json");
        wifiClient.println("Content-Length: " + String(jsonString.length()));
        wifiClient.println("Connection: keep-alive");
        wifiClient.println();
        wifiClient.print(jsonString);
        
        Serial.println("[OTA] POST请求已发送，等待响应...");
        
        unsigned long timeout = millis();
        while (wifiClient.available() == 0 && millis() - timeout < 5000) {
            delay(10);
        }
        
        if (wifiClient.available()) {
            Serial.println("[OTA] 收到服务器响应:");
            while (wifiClient.available()) {
                char c = wifiClient.read();
                Serial.print(c);
            }
            Serial.println();
        } else {
            Serial.println("[OTA] 服务器无响应（5秒超时）");
        }

        wifiClient.stop();
        Serial.println("[OTA] 连接已关闭，3秒后重试...");
    } else {
        Serial.println("[OTA] 连接服务器失败，3秒后重试...");
    }
    delay(3000);
}

void OTA::thread1_task(const void *argument)
{
    while (1) {
        sendPostRequest();
    }
}

void OTA::thread2_task(const void *argument)
{
    (void)argument;
    static WiFiServer server(5000);
    static bool serverInitialized = false;
    
    if (serverInitialized) {
        server.stop();
        delay(100);
    }
    server.begin();
    serverInitialized = true;

    Serial.println("[OTA] OTA服务器已启动，监听端口5000...");

    while (1) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[OTA] WiFi断开，等待重连...");
            delay(1000);
            continue;
        }

#undef read
#undef write
        WiFiClient client = server.available();

        if (client && client.connected()) {
            Serial.println("[OTA] 客户端已连接");
            unsigned long connectTime = millis();

            while (client.connected()) {
                memset(buffer, 0, 1024);
                int n = client.read((uint8_t *)(&buffer[0]), sizeof(buffer));
                if (n > 0) {
                    n = client.write(buffer, n);
                    if (n <= 0) {
                        break;
                    }
                    if (strstr(buffer, "start_ota")) {
                        Serial.println("[OTA] 收到OTA启动信号");
                        if (g_otaState == OtaState[0] || g_otaState == OtaState[5]) {
                            ota_http();
                        } else {
                            Serial.println("[OTA] OTA已在进行中，忽略重复请求");
                        }
                    }
                }

                if (millis() - connectTime > 30000) {
                    Serial.println("[OTA] 客户端连接超时（30秒）");
                    break;
                }

                delay(100);
            }
            client.stop();
            Serial.println("[OTA] 客户端连接已关闭");
        } else {
            delay(100);
        }
    }
}

void OTA::start_OTA_threads(int port, char *server)
{
    _port = port;
    _server = server;
    g_otaState = OtaState[0];

    Serial.println("[OTA] ========================================");
    Serial.println("[OTA] 启动OTA升级服务");
    Serial.print("[OTA] 服务器地址: ");
    Serial.print(_server);
    Serial.print(":");
    Serial.println(_port);
    Serial.println("[OTA] ========================================");

    priority1 = osPriorityNormal;
    stack_size1 = 8192;
    thread1_id = os_thread_create_arduino(thread1_task, NULL, priority1, stack_size1);

    if (thread1_id) {
        Serial.println("[OTA] ✅ Keep-alive连接线程创建成功");
    } else {
        Serial.println("[OTA] ❌ Keep-alive连接线程创建失败！");
    }

    stack_size2 = 8192;
    thread2_id = os_thread_create_arduino(thread2_task, NULL, priority1, stack_size2);

    if (thread2_id) {
        Serial.println("[OTA] ✅ OTA服务器线程创建成功");
        Serial.println("[OTA] 等待OTA服务器连接...");
    } else {
        Serial.println("[OTA] ❌ OTA服务器线程创建失败！");
    }

    Serial.println("[OTA] OTA服务初始化完成");
}

void OTA::stop_OTA_threads()
{
    Serial.println("[OTA] 正在停止OTA线程...");

    if (thread1_id) {
        os_thread_terminate_arduino(thread1_id);
        Serial.println("[OTA] Keep-alive连接线程已终止");
        thread1_id = 0;
    }

    if (thread2_id) {
        os_thread_terminate_arduino(thread2_id);
        Serial.println("[OTA] OTA服务器线程已终止");
        thread2_id = 0;
    }

    g_otaState = OtaState[0];

    Serial.println("[OTA] OTA线程已全部停止");
}