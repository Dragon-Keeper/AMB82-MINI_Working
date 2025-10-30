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

/*******************************************************
 * This example has been modified by Realtek. The original example is written by Kevin Chen.
 *
 * This example uses another JPEG Decoder library which can also work as well to display video stream from AMB82 Mini.
 *
 * Date Modified: 19 March 2024
 *
 *******************************************************/

#include <Arduino.h>
#include "VideoStream.h"
#include "SPI.h"
#include "AmebaILI9488.h"
// Include the jpeg decoder library
#include <JPEGDEC_Libraries/JPEGDEC.h>

// 引脚定义 - 与ILI9488_Demo保持一致
#define TFT_CS     12   //ArduinoPin_12 - SPI2_CS (IOE4)
#define TFT_DC     16   //ArduinoPin_16 - PD_18 (未被系统占用)
#define TFT_RST    4    //AMB_D4 (IOF11)
#define TFT_LED    11   // IOF0 (GPIOE_0) - 背光控制
#define TFT_SCK    15   //ArduinoPin_15 - SPI2_SCL (IOE1)
#define TFT_MOSI   13   //ArduinoPin_13 - SPI2_MOSI (IOE3)

// 屏幕尺寸
#define LCD_WIDTH  320
#define LCD_HEIGHT 480

// 创建ILI9488显示屏对象
AmebaILI9488 tft = AmebaILI9488(TFT_CS, TFT_DC, TFT_RST, TFT_SCK, TFT_MOSI, TFT_LED);

#define ILI9488_SPI_FREQUENCY 40000000  // ILI9488建议的SPI频率

#define CHANNEL 0
VideoSetting config(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);

uint32_t img_addr = 0;
uint32_t img_len = 0;

// 颜色转换函数：将RGB565转换为RGB666格式，并考虑颜色反转
uint32_t rgb565to666(uint16_t rgb565)
{
    // 提取RGB565各通道
    uint8_t r5 = (rgb565 >> 11) & 0x1F;
    uint8_t g6 = (rgb565 >> 5) & 0x3F;
    uint8_t b5 = rgb565 & 0x1F;
    
    // 转换为RGB666格式（每个通道6位）
    uint8_t r6 = (r5 * 0xFF / 0x1F) & 0xFC;  // 扩展到8位然后取高6位
    g6 &= 0xFC;                              // 已经是6位，确保低2位为0
    uint8_t b6 = (b5 * 0xFF / 0x1F) & 0xFC;  // 扩展到8位然后取高6位
    
    // 考虑颜色反转（与AmebaILI9488.h中的颜色定义保持一致）
    // 反转每个通道的高位（6位）
    r6 = 0xFC - r6;
    g6 = 0xFC - g6;
    b6 = 0xFC - b6;
    
    // 返回18位RGB666格式（打包为32位整数）
    return ((uint32_t)r6 << 16) | ((uint32_t)g6 << 8) | b6;
}

// JPEG绘制回调函数 - 适配ILI9488的RGB666格式
int JPEGDraw(JPEGDRAW *pDraw)
{
    // 确保JPEG绘制不超出屏幕边界
    if (pDraw->x >= LCD_WIDTH || pDraw->y >= LCD_HEIGHT) return 1;
    
    int16_t w = pDraw->iWidth;
    int16_t h = pDraw->iHeight;
    
    // 计算绘制区域，确保在屏幕范围内
    if (pDraw->x + w > LCD_WIDTH) w = LCD_WIDTH - pDraw->x;
    if (pDraw->y + h > LCD_HEIGHT) h = LCD_HEIGHT - pDraw->y;
    if (w <= 0 || h <= 0) return 1;
    
    // 设置绘制窗口 - 使用AmebaILI9488类的公共方法
    tft.setAddrWindow(pDraw->x, pDraw->y, pDraw->x + w - 1, pDraw->y + h - 1);
    
    // 提取像素数据
    uint16_t *pixels = pDraw->pPixels;
    
    // 批量处理像素数据，使用AmebaILI9488类的公共API - writeData24
    for (int16_t row = 0; row < h; row++) {
        for (int16_t col = 0; col < w; col++) {
            // 获取RGB565格式的像素值
            uint16_t rgb565 = pixels[row * pDraw->iWidth + col];
            
            // 转换为RGB666格式（已包含颜色反转处理）
            uint32_t rgb666 = rgb565to666(rgb565);
            
            // 使用AmebaILI9488类的公共方法writeData24发送24位颜色数据
            tft.writeData24(rgb666);
        }
    }
    
    return 1;    // continue decode
}

JPEGDEC jpeg;

void setup()
{
    Serial.begin(115200);
    Serial.println("Camera_2_Lcd_JPEGDEC Demo with ILI9488");

    // 设置SPI频率
    SPI.setDefaultFrequency(ILI9488_SPI_FREQUENCY);

    // 初始化显示屏 - 使用18位RGB666模式
    tft.begin();
    tft.fillScreen(ILI9488_BLACK);  // 清除屏幕
    tft.setRotation(0);  // 设置屏幕方向
    
    // 显示初始化信息
    tft.setCursor(10, 10);
    tft.setFontSize(2);
    tft.print("ILI9488 Display Ready");

    // 初始化摄像头
    Serial.println("Initializing camera...");
    Camera.configVideoChannel(CHANNEL, config);
    Camera.videoInit();
    Camera.channelBegin(CHANNEL);
    Serial.println("Camera initialized successfully");
}

void loop()
{
    // 获取图像数据
    Camera.getImage(CHANNEL, &img_addr, &img_len);
    
    // 检查图像数据是否有效
      if (img_addr > 0 && img_len > 0) {
        Serial.print("Received image data: ");
        Serial.print(img_len);
        Serial.println(" bytes");
        
        // 打开JPEG数据并设置回调函数
        if (jpeg.openFLASH((uint8_t *)img_addr, img_len, JPEGDraw)) {
          Serial.print("JPEG image size: ");
          Serial.print(jpeg.getWidth());
          Serial.print(" x ");
          Serial.println(jpeg.getHeight());
            
            // 计算居中显示的坐标
            int x_offset = (LCD_WIDTH - jpeg.getWidth()) / 2;
            int y_offset = (LCD_HEIGHT - jpeg.getHeight()) / 2;
            
            // 解码JPEG图像，使用缩放以适应屏幕
            jpeg.decode(x_offset, y_offset, JPEG_SCALE_HALF);
            jpeg.close();
            
            Serial.println("Image displayed successfully");
        } else {
            Serial.println("JPEG打开失败");
        }
    }
    
    // 添加短暂延迟以降低CPU使用率
    delay(10);
}
