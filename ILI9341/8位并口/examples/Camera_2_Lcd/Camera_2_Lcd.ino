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
#define FPS_DISPLAY_ENABLED true         // 启用屏幕FPS显示
#define MULTI_FRAME_CACHE_ENABLED true   // 启用多帧缓存机制
#define CACHE_SIZE 3                     // 缓存帧数

// 性能分析参数
#define PERFORMANCE_ANALYSIS true        // 启用详细性能分析
#define MAX_RESPONSE_DELAY 100          // 最大响应延迟(ms)
#define TARGET_UI_FPS 60                // 目标UI帧率
#define MEMORY_MONITOR true             // 启用内存监控
#define CPU_USAGE_MONITOR true          // 启用CPU使用率监控

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

// 性能分析变量
unsigned long frameStartTime = 0;
unsigned long frameEndTime = 0;
unsigned long decodeTime = 0;
unsigned long displayTime = 0;
unsigned long totalFrameTime = 0;
unsigned long maxFrameTime = 0;
unsigned long minFrameTime = 1000;
float averageFrameTime = 0.0;
unsigned long frameTimeSum = 0;
unsigned long frameTimeCount = 0;

// 内存监控变量
#ifdef MEMORY_MONITOR
unsigned long freeMemoryStart = 0;
unsigned long freeMemoryEnd = 0;
unsigned long memoryUsage = 0;
#endif

// CPU使用率监控
#ifdef CPU_USAGE_MONITOR
unsigned long idleTime = 0;
unsigned long busyTime = 0;
float cpuUsage = 0.0;
#endif

// 图像缓存结构
struct ImageCache {
    uint32_t addr;
    uint32_t len;
    bool valid;
} imageCache;

// 多帧缓存结构
struct FrameCache {
    uint32_t addr;
    uint32_t len;
    bool valid;
    unsigned long timestamp;
} frameCache[CACHE_SIZE];

// 缓存索引
int currentCacheIndex = 0;
int lastDisplayedIndex = -1;

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
    
    // 每300ms更新一次FPS（符合用户要求）
    if (currentTime - lastFpsTime >= 300) {
        currentFps = (float)frameCount * 1000.0 / (currentTime - lastFpsTime);
        
        // 限制FPS范围在0-120之间
        if (currentFps < 0) currentFps = 0;
        if (currentFps > 120) currentFps = 120;
        
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

// 屏幕FPS显示函数
void displayFPS() {
    if (!FPS_DISPLAY_ENABLED) return;
    
    static float lastDisplayedFps = -1.0; // 上次显示的FPS值
    static unsigned long lastDisplayTime = 0; // 上次显示时间
    
    // 每300ms更新一次显示，避免闪烁
    unsigned long currentTime = millis();
    if (currentTime - lastDisplayTime < 300 && abs(currentFps - lastDisplayedFps) < 0.5) {
        return; // 避免频繁更新导致的闪烁
    }
    
    // 设置文本颜色和字体大小
    tft.setForeground(0xFFFF); // 白色
    tft.setBackground(0x0000);  // 黑色
    tft.setFontSize(2);
    
    // 清除之前的显示区域（黑色背景）
    tft.fillRectangle(5, 5, 80, 20, 0x0000);
    
    // 在屏幕左上角显示FPS，精度为小数点后一位
    tft.setCursor(5, 5);
    tft.print("FPS: ");
    tft.print(currentFps, 1);
    
    // 更新显示状态
    lastDisplayedFps = currentFps;
    lastDisplayTime = currentTime;
}

// 多帧缓存管理函数
void updateFrameCache(uint32_t addr, uint32_t len) {
    if (!MULTI_FRAME_CACHE_ENABLED) return;
    
    // 更新当前缓存帧
    frameCache[currentCacheIndex].addr = addr;
    frameCache[currentCacheIndex].len = len;
    frameCache[currentCacheIndex].valid = true;
    frameCache[currentCacheIndex].timestamp = millis();
    
    // 移动到下一个缓存位置
    currentCacheIndex = (currentCacheIndex + 1) % CACHE_SIZE;
}

// 性能分析函数
void analyzePerformance() {
    if (!PERFORMANCE_ANALYSIS) return;
    
    // 计算帧时间统计
    totalFrameTime = frameEndTime - frameStartTime;
    
    // 更新最大和最小帧时间
    if (totalFrameTime > maxFrameTime) maxFrameTime = totalFrameTime;
    if (totalFrameTime < minFrameTime) minFrameTime = totalFrameTime;
    
    // 计算平均帧时间
    frameTimeSum += totalFrameTime;
    frameTimeCount++;
    averageFrameTime = (float)frameTimeSum / frameTimeCount;
    
    // 每30帧输出详细性能报告
    if (frameCount % 30 == 0) {
        Serial.println("=== 详细性能分析报告 ===");
        Serial.print("当前FPS: ");
        Serial.print(currentFps, 1);
        Serial.print(" / 目标FPS: ");
        Serial.println(TARGET_FPS);
        
        Serial.print("帧时间统计 - 当前: ");
        Serial.print(totalFrameTime);
        Serial.print("ms, 最小: ");
        Serial.print(minFrameTime);
        Serial.print("ms, 最大: ");
        Serial.print(maxFrameTime);
        Serial.print("ms, 平均: ");
        Serial.print(averageFrameTime, 1);
        Serial.println("ms");
        
        Serial.print("解码时间: ");
        Serial.print(decodeTime);
        Serial.print("ms, 显示时间: ");
        Serial.print(displayTime);
        Serial.println("ms");
        
        // 检查响应延迟
        if (totalFrameTime > MAX_RESPONSE_DELAY) {
            Serial.println("⚠️ 警告: 响应延迟超过100ms阈值!");
        }
        
        // 检查UI帧率
        float uiFps = 1000.0 / totalFrameTime;
        if (uiFps < TARGET_UI_FPS) {
            Serial.print("⚠️ 警告: UI帧率(");
            Serial.print(uiFps, 1);
            Serial.print("FPS)低于目标值(");
            Serial.print(TARGET_UI_FPS);
            Serial.println("FPS)");
        }
        
        Serial.println("========================");
    }
}

// 内存监控函数
#ifdef MEMORY_MONITOR
void monitorMemory() {
    // 这里可以添加内存监控逻辑
    // 注意：Arduino环境下的内存监控需要特定实现
}
#endif

// CPU使用率监控函数
#ifdef CPU_USAGE_MONITOR
void monitorCpuUsage() {
    // 这里可以添加CPU使用率监控逻辑
    // 注意：Arduino环境下的CPU监控需要特定实现
}
#endif

// 检查多帧缓存
bool isFrameCached(uint32_t addr, uint32_t len) {
    if (!MULTI_FRAME_CACHE_ENABLED) return false;
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (frameCache[i].valid && frameCache[i].addr == addr && frameCache[i].len == len) {
            return true;
        }
    }
    return false;
}

// 获取最佳缓存帧
int getBestCachedFrame() {
    if (!MULTI_FRAME_CACHE_ENABLED) return -1;
    
    int bestIndex = -1;
    unsigned long oldestTime = millis();
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (frameCache[i].valid && frameCache[i].timestamp < oldestTime) {
            oldestTime = frameCache[i].timestamp;
            bestIndex = i;
        }
    }
    
    return bestIndex;
}

// 优化的TFT输出函数
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    if (useOptimizedDraw) {
        // 使用高度优化的绘制方法
        tft.drawBitmapOptimized(x, y, w, h, (const unsigned short *)bitmap);
    } else {
        // 备用绘制方法
        tft.drawBitmap(x, y, w, h, (const unsigned short *)bitmap);
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
    
    // 初始化多帧缓存
    for (int i = 0; i < CACHE_SIZE; i++) {
        frameCache[i].valid = false;
        frameCache[i].addr = 0;
        frameCache[i].len = 0;
        frameCache[i].timestamp = 0;
    }

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
    
    // 记录帧开始时间
    frameStartTime = millis();
    lastFrameTime = currentTime;
    
    // 获取摄像头图像
    Camera.getImage(CHANNEL, &img_addr, &img_len);
    
    // 智能缓存策略：优先检查多帧缓存
    bool useCachedFrame = false;
    int cachedFrameIndex = -1;
    
    if (MULTI_FRAME_CACHE_ENABLED && isFrameCached(img_addr, img_len)) {
        // 当前帧已在多帧缓存中
        useCachedFrame = true;
        if (PERFORMANCE_MONITOR && frameCount % 30 == 0) {
            Serial.println("多帧缓存命中，跳过解码");
        }
    } else if (CACHE_ENABLED && isImageCached(img_addr, img_len)) {
        // 单帧缓存命中
        useCachedFrame = true;
        if (PERFORMANCE_MONITOR && frameCount % 20 == 0) {
            Serial.println("单帧缓存命中，跳过解码");
        }
    }
    
    if (!useCachedFrame) {
        // 新图像，进行解码和显示
        if (TJpgDec.getJpgSize(0, 0, (uint8_t *)img_addr, img_len) == JDR_OK) {
            // 成功获取图像尺寸，进行解码
            if (TJpgDec.drawJpg(0, 0, (uint8_t *)img_addr, img_len) == JDR_OK) {
                // 解码成功，更新所有缓存
                updateImageCache(img_addr, img_len);
                updateFrameCache(img_addr, img_len);
                
                if (PERFORMANCE_MONITOR && frameCount % 15 == 0) {
                    Serial.println("新图像解码成功，已更新缓存");
                }
            } else {
                // 解码失败，尝试使用缓存帧
                if (MULTI_FRAME_CACHE_ENABLED) {
                    cachedFrameIndex = getBestCachedFrame();
                    if (cachedFrameIndex != -1) {
                        // 使用最佳缓存帧
                        img_addr = frameCache[cachedFrameIndex].addr;
                        img_len = frameCache[cachedFrameIndex].len;
                        useCachedFrame = true;
                        if (PERFORMANCE_MONITOR) {
                            Serial.println("解码失败，使用缓存帧替代");
                        }
                    }
                }
                
                if (!useCachedFrame && PERFORMANCE_MONITOR) {
                    Serial.println("JPEG解码失败");
                }
            }
        } else {
            // 获取图像尺寸失败，尝试使用缓存帧
            if (MULTI_FRAME_CACHE_ENABLED) {
                cachedFrameIndex = getBestCachedFrame();
                if (cachedFrameIndex != -1) {
                    // 使用最佳缓存帧
                    img_addr = frameCache[cachedFrameIndex].addr;
                    img_len = frameCache[cachedFrameIndex].len;
                    useCachedFrame = true;
                    if (PERFORMANCE_MONITOR) {
                        Serial.println("获取尺寸失败，使用缓存帧替代");
                    }
                }
            }
            
            if (!useCachedFrame && PERFORMANCE_MONITOR) {
                Serial.println("获取图像尺寸失败");
            }
        }
    }
    
    // 记录帧结束时间
    frameEndTime = millis();
    
    // 性能监控和FPS显示
    updateFpsCounter();
    
    // 在屏幕上显示实时FPS
    displayFPS();
    
    // 性能分析
    analyzePerformance();
    
    // 每10帧打印性能信息
    if (PERFORMANCE_MONITOR && frameCount % 10 == 0) {
        Serial.print("FPS: ");
        Serial.print(currentFps, 1);
        Serial.print(", Image Addr: 0x");
        Serial.print(img_addr, HEX);
        Serial.print(", Len: ");
        Serial.print(img_len);
        Serial.print(", 缓存状态: ");
        Serial.println(useCachedFrame ? "命中" : "未命中");
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
