#ifndef CAMERA_OTA_H
#define CAMERA_OTA_H

#include <Arduino.h>
#include <httpd/httpd.h>
#include <WiFi.h>

class OTA {
public:
    OTA(void);
    ~OTA(void);

    void start_OTA_threads(int port, char *server);
    void stop_OTA_threads();
    const char* getState() { return g_otaState; }
    bool isIdle() { return g_otaState == OtaState[0]; }

    int getThread1Id() { return thread1_id; }
    int getThread2Id() { return thread2_id; }

    static const char *OtaState[];

    static const char *OTA_STATE_IDLE;
    static const char *OTA_STATE_FAILED;

    static int _port;
    static char *_server;
    static const char *g_otaState;

private:
    static void thread1_task(const void *argument);
    static void thread2_task(const void *argument);
    static void sendPostRequest();

    static uint32_t thread1_id;
    static uint32_t thread2_id;
    static int priority1;
    static uint32_t stack_size1;
    static uint32_t stack_size2;

    static char buffer[1024];
    static String jsonString;

    friend void ota_http(void);
};

void ota_http(void);

#endif