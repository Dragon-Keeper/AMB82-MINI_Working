/*******************************************************
 * Camera → ILI9341 TFT (DMA 版)  2025-06
 * 硬件：AMB82-MINI  Arduino
 * 依赖：JPEGDEC 库（请注释掉 User_Config.h 中的 #define TJPGD_LOAD_SD_LIBRARY）
 ******************************************************/

#include "VideoStream.h"
#include "SPI.h"
#include "AmebaILI9341_DMA.h"   // ← 仅需改这一行
#include <JPEGDEC_Libraries/JPEGDEC.h>

#define CHANNEL 0

#define TFT_LED  6   // PF_13
#define TFT_RESET 5   // PF_12
#define TFT_DC    4   // PF_11
#define TFT_CS    SPI_SS  // SPI0_SS  PE_4

AmebaILI9341_DMA tft = AmebaILI9341_DMA(TFT_CS, TFT_DC, TFT_RESET);

#define ILI9341_SPI_FREQUENCY 50000000   // 50 MHz

VideoSetting config(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);
uint32_t img_addr = 0;
uint32_t img_len = 0;

/* JPEGDEC 回调 → 直接送 DMA */
int JPEGDraw(JPEGDRAW *pDraw) {
    tft.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    return 1;
}

JPEGDEC jpeg;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("=== AMB82-MINI  Camera → ILI9341 DMA ===");

    pinMode(TFT_LED, OUTPUT);
    digitalWrite(TFT_LED, HIGH);   // 点亮 TFT 背光
    
    SPI.setDefaultFrequency(ILI9341_SPI_FREQUENCY); // 50 MHz
    Camera.configVideoChannel(CHANNEL, config);
    Camera.videoInit();
    Camera.channelBegin(CHANNEL);

    tft.begin();
    tft.setRotation(3);          // 横屏
    tft.setColorOrder(false);    // 设置为BGR模式（修正颜色偏差）
    tft.fillScreen(ILI9341_BLACK);
}

void loop() {
    Camera.getImage(CHANNEL, &img_addr, &img_len);
    jpeg.openFLASH((uint8_t *)img_addr, img_len, JPEGDraw);
    jpeg.decode(0, 0, JPEG_SCALE_HALF);
    jpeg.close();
}