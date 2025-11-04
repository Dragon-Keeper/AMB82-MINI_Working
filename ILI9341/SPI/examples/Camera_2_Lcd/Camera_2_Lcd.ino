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
#include "AmebaParallel8.h"  // 使用8位硬件并口驱动库
// Include the jpeg decoder library
#include "TJpg_Decoder.h"
#include "AmebaFatFS.h"

#define CHANNEL 0

// 8位硬件并口引脚定义 - 与Camera_2_Lcd_JPEGDEC.ino保持一致
#define TFT_RESET 5
#define TFT_DC    4
#define TFT_CS    12
#define TFT_WR    6
#define TFT_RD    7

// 8位数据引脚数组 - D0-D7
PortPin dataPins[8] = {8, 9, 2, 3, 10, 11, 13, 14};

#define FILENAME "ximg_"

// 性能优化定义
#define TARGET_FPS 15                    // 目标帧率
#define FRAME_INTERVAL (1000 / TARGET_FPS) // 帧间隔(ms)
#define CACHE_ENABLED true               // 启用图像缓存
#define PERFORMANCE_MONITOR true         // 启用性能监控

// 使用8位硬件并口驱动
AmebaParallel8 tft(dataPins, TFT_CS, TFT_DC, TFT_RESET, TFT_WR, TFT_RD);

VideoSetting config(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);

uint32_t img_addr = 0;
uint32_t img_len = 0;

// 性能监控变量
unsigned long lastFrameTime = 0;
unsigned long frameCount = 0;
unsigned long lastFpsTime = 0;
float currentFps = 0.0;

// 图像缓存结构
struct ImageCache {
    uint32_t addr;
    uint32_t len;
    bool valid;
} imageCache;

// 性能优化标志
bool useOptimizedDraw = true;

AmebaFatFS fs;
int button = 16;
int button_State = 0;
bool Camer_cap;
uint32_t count;

void button_Handler(uint32_t id, uint32_t event)
{
    if (button_State == 0) {
        button_State = 1;
    }
}

// 性能监控函数
void updateFpsCounter() {
    frameCount++;
    unsigned long currentTime = millis();
    
    // 每秒更新一次FPS
    if (currentTime - lastFpsTime >= 1000) {
        currentFps = (float)frameCount * 1000.0 / (currentTime - lastFpsTime);
        frameCount = 0;
        lastFpsTime = currentTime;
        
        // 每10帧打印一次性能信息
        if (frameCount % 10 == 0) {
            Serial.print("FPS: ");
            Serial.print(currentFps, 1);
            Serial.print(", Image Addr: 0x");
            Serial.print(img_addr, HEX);
            Serial.print(", Len: ");
            Serial.println(img_len);
        }
    }
}

// 图像缓存检查函数
bool isImageCached(uint32_t addr, uint32_t len) {
    if (!CACHE_ENABLED) return false;
    return imageCache.valid && imageCache.addr == addr && imageCache.len == len;
}

// 更新图像缓存
void updateImageCache(uint32_t addr, uint32_t len) {
    if (!CACHE_ENABLED) return;
    imageCache.addr = addr;
    imageCache.len = len;
    imageCache.valid = true;
}

// 优化的TFT输出函数
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    if (useOptimizedDraw) {
        // 使用高度优化的绘制方法
        tft.drawBitmapOptimized(x, y, w, h, bitmap);
    } else {
        // 备用绘制方法
        tft.drawBitmap(x, y, w, h, bitmap);
    }
    
    // Return 1 to decode next block
    return 1;
}

void setup()
{
    Serial.begin(115200);

    Serial.println("TFT ILI9341 - 优化版本");
    Serial.print("目标帧率: ");
    Serial.print(TARGET_FPS);
    Serial.println(" FPS");
    Serial.print("图像缓存: ");
    Serial.println(CACHE_ENABLED ? "启用" : "禁用");

    // 初始化性能监控变量
    lastFrameTime = millis();
    lastFpsTime = millis();
    frameCount = 0;
    currentFps = 0.0;
    
    // 初始化图像缓存
    imageCache.valid = false;
    imageCache.addr = 0;
    imageCache.len = 0;

    pinMode(button, INPUT_IRQ_FALL);
    digitalSetIrqHandler(button, button_Handler);

    Camera.configVideoChannel(CHANNEL, config);
    Camera.videoInit();
    Camera.channelBegin(CHANNEL);

    // 初始化8位硬件并口显示屏
    tft.begin();
    tft.setRotation(1);

    // 预初始化JPEG解码器
    TJpgDec.setJpgScale(2);
    TJpgDec.setCallback(tft_output);
    
    Serial.println("系统初始化完成，开始摄像头视频流显示...");
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
    
    // 获取摄像头图像
    Camera.getImage(CHANNEL, &img_addr, &img_len);
    
    // 检查图像缓存
    if (isImageCached(img_addr, img_len)) {
        // 图像已缓存，跳过解码
        if (PERFORMANCE_MONITOR && frameCount % 20 == 0) {
            Serial.println("图像已缓存，跳过解码");
        }
    } else {
        // 新图像，进行解码和显示
        if (TJpgDec.getJpgSize(0, 0, (uint8_t *)img_addr, img_len) == JDR_OK) {
            // 成功获取图像尺寸，进行解码
            if (TJpgDec.drawJpg(0, 0, (uint8_t *)img_addr, img_len) == JDR_OK) {
                // 解码成功，更新缓存
                updateImageCache(img_addr, img_len);
                
                // 性能监控
                updateFpsCounter();
                
                // 每10帧打印性能信息
                if (PERFORMANCE_MONITOR && frameCount % 10 == 0) {
                    Serial.print("FPS: ");
                    Serial.print(currentFps, 1);
                    Serial.print(", Image Addr: 0x");
                    Serial.print(img_addr, HEX);
                    Serial.print(", Len: ");
                    Serial.println(img_len);
                }
            } else {
                // 解码失败
                if (PERFORMANCE_MONITOR) {
                    Serial.println("JPEG解码失败");
                }
            }
        } else {
            // 获取图像尺寸失败
            if (PERFORMANCE_MONITOR) {
                Serial.println("获取图像尺寸失败");
            }
        }
    }
    
    // 处理SD卡拍照功能
    if (button_State == 1) {
        fs.begin();
        File file = fs.open(String(fs.getRootPath()) + String(FILENAME) + String(count) + String(".jpg"));
        file.write((uint8_t *)img_addr, img_len);
        delay(1);
        file.close();
        fs.end();
        count++;
        button_State = 0;
        
        if (PERFORMANCE_MONITOR) {
            Serial.print("照片已保存: ximg_");
            Serial.print(count - 1);
            Serial.println(".jpg");
        }
    }
}
