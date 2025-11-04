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

#include "VideoStream.h"
#include "SPI.h"
// 使用硬件并口驱动替代SPI驱动
#include "AmebaParallel8.h"
// Include the jpeg decoder library
#include <JPEGDEC_Libraries/JPEGDEC.h>

// 性能优化相关定义
#define CHANNEL 0
#define TARGET_FPS 15  // 目标帧率
#define FRAME_INTERVAL (1000 / TARGET_FPS)  // 帧间隔(ms)

// 硬件并口引脚定义 - 8位数据线 + 5个控制线
// 使用与LCD_Screen_ILI9341_TFT.ino相同的引脚配置
#define TFT_CS    12   // 片选
#define TFT_DC    4    // 数据/命令选择
#define TFT_RESET 5    // 复位
#define TFT_WR    6    // 写使能
#define TFT_RD    7    // 读使能

// 8位数据引脚定义 (D0-D7)
// 使用与LCD_Screen_ILI9341_TFT.ino相同的引脚配置
const PortPin dataPins[8] = {8, 9, 2, 3, 10, 11, 13, 14};

// 使用硬件并口驱动
AmebaParallel8 tft(dataPins, TFT_CS, TFT_DC, TFT_RESET, TFT_WR, TFT_RD);

VideoSetting config(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);

// 全局变量
uint32_t img_addr = 0;
uint32_t img_len = 0;
unsigned long lastFrameTime = 0;
unsigned long frameCount = 0;
unsigned long lastFpsTime = 0;
float currentFps = 0.0;

// 图像缓存结构 - 增强版
struct ImageCache {
    uint32_t addr;
    uint32_t len;
    unsigned long timestamp;
    bool valid;
} imageCache;

// 性能优化标志
bool useOptimizedDraw = true;

// 优化的JPEG绘制回调函数 - 增强版
int JPEGDraw(JPEGDRAW *pDraw)
{
    if (useOptimizedDraw) {
        // 使用高度优化的drawBitmapOptimized函数
        tft.drawBitmapOptimized(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    } else {
        // 备用绘制方法
        tft.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    }
    return 1;    // continue decode
}

// 图像缓存管理函数
void updateImageCache(uint32_t addr, uint32_t len) {
    if (addr != 0 && len > 0) {
        imageCache.addr = addr;
        imageCache.len = len;
        imageCache.timestamp = millis();
        imageCache.valid = true;
    }
}

// 检查图像是否已缓存
bool isImageCached(uint32_t addr, uint32_t len) {
    return imageCache.valid && 
           imageCache.addr == addr && 
           imageCache.len == len &&
           (millis() - imageCache.timestamp) < 100; // 100ms内认为有效
}

JPEGDEC jpeg;

// 性能监控函数
void updateFpsCounter() {
    frameCount++;
    unsigned long currentTime = millis();
    if (currentTime - lastFpsTime >= 1000) { // 每秒更新一次FPS
        currentFps = (frameCount * 1000.0) / (currentTime - lastFpsTime);
        frameCount = 0;
        lastFpsTime = currentTime;
        
        // 输出性能信息
        Serial.print("FPS: ");
        Serial.print(currentFps, 1);
        Serial.print(", Frame Time: ");
        Serial.print(currentTime - lastFrameTime);
        Serial.println("ms");
    }
}

void setup()
{
    Serial.begin(115200);

    Serial.println("TFT ILI9341 - Hardware Parallel 8-bit Interface (Optimized)");
    Serial.println("Optimized for high frame rate display");

    // 初始化摄像头
    Camera.configVideoChannel(CHANNEL, config);
    Camera.videoInit();
    Camera.channelBegin(CHANNEL);

    // 初始化TFT显示屏
    tft.begin();
    tft.setRotation(1);
    
    // 预初始化JPEG解码器以减少延迟
    jpeg.openFLASH((uint8_t *)0, 0, JPEGDraw);
    
    lastFrameTime = millis();
    lastFpsTime = lastFrameTime;
    
    Serial.println("System initialized. Starting optimized display loop...");
}

void loop()
{
    unsigned long currentTime = millis();
    
    // 帧率控制：确保帧间隔
    if (currentTime - lastFrameTime < FRAME_INTERVAL) {
        delay(1); // 短暂延迟，避免过度占用CPU
        return;
    }
    
    lastFrameTime = currentTime;
    
    // 获取新图像
    Camera.getImage(CHANNEL, &img_addr, &img_len);
    
    // 使用图像缓存机制，避免重复解码相同图像
    if (img_addr != 0 && img_len > 0) {
        // 检查是否需要重新解码
        if (!isImageCached(img_addr, img_len)) {
            // 新图像，需要解码
            jpeg.close(); // 关闭之前的解码器
            
            // 使用优化的JPEG解码流程
            if (jpeg.openFLASH((uint8_t *)img_addr, img_len, JPEGDraw)) {
                // 使用优化的解码参数
                jpeg.decode(0, 0, JPEG_SCALE_HALF);
                
                // 更新图像缓存
                updateImageCache(img_addr, img_len);
                
                // 输出解码性能信息（每10帧输出一次）
                if (frameCount % 10 == 0) {
                    Serial.print("Decoded frame: addr=0x");
                    Serial.print(img_addr, HEX);
                    Serial.print(", len=");
                    Serial.println(img_len);
                }
            } else {
                Serial.println("JPEG decode failed!");
            }
        } else {
            // 图像已缓存，跳过解码（理论上不应该发生，因为摄像头持续产生新图像）
            if (frameCount % 30 == 0) {
                Serial.println("Image cached, skipping decode");
            }
        }
    }
    
    // 更新FPS计数器
    updateFpsCounter();
}
