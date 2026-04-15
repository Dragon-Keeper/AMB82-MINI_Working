#include "OTA.h"
#include "ota_drv.h"
#include <osdep_service.h>
#include <wifi_constants.h>
#include "wifi_conf.h"
#include "lwip_netconf.h"

static const char *resource = "api/uploadfile";
static int _firmware_port = 3000;

void http_update_ota_task(void *param)
{
    (void)param;

    int ret = -1;

    OTA::g_otaState = OTA::OtaState[2];

    ret = http_update_ota((char *)OTA::_server, _firmware_port, (char *)resource);

    if (!ret) {
        OTA::g_otaState = OTA::OtaState[3];
        OTA::g_otaState = OTA::OtaState[4];
        ota_platform_reset();
    } else {
        printf("[OTA] 固件下载/写入失败 (ret=%d)，重置OTA状态\n", ret);
        OTA::g_otaState = OTA::OtaState[5];
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        OTA::g_otaState = OTA::OtaState[0];
    }
    vTaskDelete(NULL);
}

void ota_http(void)
{
    if (xTaskCreate(http_update_ota_task, (const char *)"http_update_ota_task", 8192, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
        printf("\n\r[%s] Create update task failed", __FUNCTION__);
    }
}