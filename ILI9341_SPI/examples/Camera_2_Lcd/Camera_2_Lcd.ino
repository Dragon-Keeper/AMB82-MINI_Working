/*******************************************************
 * Camera_2_Lcd.ino - Camera to TFT LCD
 *
 * Author: Kevin Chen
 * Date: 2023/11/16
 *
 * Version: 1.0.0
 *
 * This code was written by Kevin Chen.
 *
 * This is an open-source program, and it can be freely used, modified, and distributed under the following conditions:
 *
 * 1. The original copyright notice must not be removed or modified.
 * 2. Projects or products using this code must acknowledge the original author's name and the source in applicable documentation, websites, or other related materials.
 * 3. Any derivative work based on this code must state its origin and retain the original copyright notice in its documentation.
 * 4. This code must not be used for any activities that may infringe upon the rights of others, be unlawful, or harmful, whether in a commercial or non-commercial environment.
 *
 * This code is provided "as is" with no warranty, expressed or implied. The author is not liable for any losses or damages caused by using the code.
 *
 * Users of Camera_2_Lcd.ino assume all risks associated with its use, and the author shall not be held responsible for any consequences.
 *
 * For more information about our company, please visit: www.makdev.net
 *
 *  1.Based on AmebaILI9341.h, implement the functionality to display Camera output on TFT LCD and utilize the SD Card feature to achieve basic photo capture.
 *
 *  2.Before starting the project, please install the TJpg_Decoder library. In the library's configuration file, User_Config.h, comment out line 5 which reads: #define TJPGD_LOAD_SD_LIBRARY
 *******************************************************/

#include "VideoStream.h"
#include "SPI.h"
#include "AmebaILI9341_DMA.h"   // DMA版本
// Include the jpeg decoder library
#include <JPEGDEC_Libraries/JPEGDEC.h>
#include "AmebaFatFS.h"

#define CHANNEL 0

#define TFT_LED  6   // PF_13 背光控制
#define TFT_RESET 5   // PF_12
#define TFT_DC    4   // PF_11
#define TFT_CS    SPI_SS  // SPI0_SS  PE_4

#define FILENAME "Image_"

AmebaILI9341_DMA tft = AmebaILI9341_DMA(TFT_CS, TFT_DC, TFT_RESET);

#define ILI9341_SPI_FREQUENCY 50000000   // 50 MHz

VideoSetting config(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);

uint32_t img_addr = 0;
uint32_t img_len = 0;

AmebaFatFS fs;
uint32_t count = 0;
unsigned long lastCaptureTime = 0;
const unsigned long captureInterval = 5000; // 5秒拍照间隔

JPEGDEC jpeg;

/* JPEGDEC 回调 → 直接送 DMA */
int JPEGDraw(JPEGDRAW *pDraw) {
    tft.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    return 1;
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("=== AMB82-MINI Camera → ILI9341 DMA (JPEGDEC + 5秒自动拍照) ===");

    pinMode(TFT_LED, OUTPUT);
    digitalWrite(TFT_LED, HIGH);   // 点亮 TFT 背光
    
    SPI.setDefaultFrequency(ILI9341_SPI_FREQUENCY); // 50 MHz

    Camera.configVideoChannel(CHANNEL, config);
    Camera.videoInit();
    Camera.channelBegin(CHANNEL);

    tft.begin();
    tft.setRotation(3);
    tft.setColorOrder(false);    // 设置为BGR模式（修正颜色偏差）
    tft.fillScreen(ILI9341_BLACK);

    lastCaptureTime = millis(); // 初始化拍照计时器
}

void loop()
{
    Camera.getImage(CHANNEL, &img_addr, &img_len);
    
    // 使用JPEGDEC库显示图像
    jpeg.openFLASH((uint8_t *)img_addr, img_len, JPEGDraw);
    jpeg.decode(0, 0, JPEG_SCALE_HALF);
    jpeg.close();
    
    // 5秒自动拍照功能
    unsigned long currentTime = millis();
    if (currentTime - lastCaptureTime >= captureInterval) {
        // 保存图像到SD卡
        fs.begin();
        File file = fs.open(String(fs.getRootPath()) + String(FILENAME) + String(count) + String(".jpg"));
        if (file) {
            file.write((uint8_t *)img_addr, img_len);
            delay(1);
            file.close();
            Serial.println("照片已保存: " + String(FILENAME) + String(count) + String(".jpg"));
            count++;
        } else {
            Serial.println("SD卡写入失败");
        }
        fs.end();
        
        lastCaptureTime = currentTime; // 更新拍照时间
    }
}
