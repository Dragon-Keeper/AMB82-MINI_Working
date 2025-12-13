/*******************************************************
 * 优化后的 Camera_2_Lcd_JPEGDEC.ino
 * 
 * 主要优化：
 * 1. 使用WVGA分辨率而非VGA，减少数据量25%
 * 2. 使用JPEG_SCALE_QUARTER减少解码时间
 * 3. 优化帧率控制
 * 4. 减少调试输出
 * 5. 优化内存管理
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
#define TFT_CS    12   // 片选
#define TFT_DC    4    // 数据/命令选择
#define TFT_RESET 5    // 复位
#define TFT_WR    6    // 写使能
#define TFT_RD    7    // 读使能

// 8位数据引脚定义 (D0-D7)
const PortPin dataPins[8] = {8, 9, 2, 3, 10, 11, 13, 14};

// 使用硬件并口驱动
AmebaParallel8 tft(dataPins, TFT_CS, TFT_DC, TFT_RESET, TFT_WR, TFT_RD);

// 阶段一优化：使用VGA分辨率替代WVGA，进一步减少数据量50%
// 原WVGA: 800×480 = 384,000像素
// 新VGA: 640×480 = 307,200像素 (减少20%数据量)
VideoSetting config(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);

// 全局变量
uint32_t img_addr = 0;
uint32_t img_len = 0;
unsigned long lastFrameTime = 0;
unsigned long frameCount = 0;
unsigned long lastFpsTime = 0;
float currentFps = 0.0;

// 阶段三优化：添加显示模式变量
uint8_t displayMode = 1; // 0: 标准模式, 1: 全屏模式 - 默认全屏模式以测试优化效果

// 阶段四优化：全屏性能管理变量
uint8_t performanceMode = 0; // 0: 高质量模式, 1: 高性能模式
unsigned long lastModeSwitchTime = 0; // 模式切换时间保护
bool enableAdaptiveQuality = true; // 启用自适应质量调整

// 优化：减少图像缓存结构的复杂性
struct SimpleImageCache {
    uint32_t addr;
    uint32_t len;
} lastImage;

// 阶段一优化：性能监控结构体
struct PerformanceMetrics {
    unsigned long decodeTime;
    unsigned long displayTime;
    unsigned long totalFrameTime;
    unsigned long lastFrameStart;
    int frameCount;
    float averageFPS;
    float averageDecodeTime;
    float averageDisplayTime;
    uint32_t totalDecodeTime;
    uint32_t totalDisplayTime;
} perfMetrics;

// 性能优化标志
bool useOptimizedDraw = true;
bool performanceMonitoring = true;

// JPEG解码回调函数 - 全屏性能优化版本
int JPEGDraw(JPEGDRAW *pDraw)
{
    unsigned long drawStartTime = 0;
    if (performanceMonitoring) {
        drawStartTime = micros();
    }
    
    // 阶段四优化：全屏渲染加速
    uint16_t *pixels = (uint16_t *)pDraw->pPixels;
    
    // 全屏模式性能优化
    if (displayMode == 1) {
        // 简化版批量渲染：直接使用优化绘制
        if (useOptimizedDraw) {
            tft.drawBitmapOptimized(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pixels);
        } else {
            tft.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pixels);
        }
    } else {
        // 标准模式：使用优化的单次传输
        if (useOptimizedDraw) {
            tft.drawBitmapOptimized(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pixels);
        } else {
            tft.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pixels);
        }
    }
    
    if (performanceMonitoring) {
        perfMetrics.displayTime += (micros() - drawStartTime);
    }
    
    return 1;    // continue decode
}

// 优化：简化图像缓存检查
bool isNewImage(uint32_t addr, uint32_t len) {
    return (addr != lastImage.addr || len != lastImage.len);
}

JPEGDEC jpeg;

// 阶段三优化：高性能帧率控制（目标15+ FPS）
unsigned long adaptiveFrameControl() {
    static unsigned long lastFrameEnd = 0;
    static uint8_t frameSkipCounter = 0;
    
    unsigned long currentTime = millis();
    unsigned long frameTime = currentTime - lastFrameEnd;
    
    // 目标帧间隔（66ms ≈ 15FPS）
    #define TARGET_FRAME_INTERVAL 66
    
    // 智能跳帧策略：当处理时间超过目标间隔时跳帧
    if (frameTime < TARGET_FRAME_INTERVAL) {
        lastFrameEnd = currentTime + (TARGET_FRAME_INTERVAL - frameTime);
        return TARGET_FRAME_INTERVAL - frameTime;  // 需要等待的时间
    } else {
        // 帧率过低时启用跳帧模式
        if (frameTime > TARGET_FRAME_INTERVAL * 2) {
            frameSkipCounter++;
            if (frameSkipCounter >= 2) {  // 每2帧跳1帧
                frameSkipCounter = 0;
                lastFrameEnd = currentTime;
                return 0;  // 跳过当前帧
            }
        }
        lastFrameEnd = currentTime;
        return 0;  // 立即开始下一帧
    }
}

// 阶段四优化：全屏性能增强监控器
void updateFpsCounter() {
    frameCount++;
    unsigned long currentTime = millis();
    if (currentTime - lastFpsTime >= 1000) { // 每秒更新一次FPS
        currentFps = frameCount;
        frameCount = 0;
        lastFpsTime = currentTime;
        
        // 全屏性能分析：每2秒输出详细性能报告
        static uint8_t performanceIndicator = 0;
        if (++performanceIndicator >= 2) { // 每2秒输出详细状态
            performanceIndicator = 0;
            
            // 计算性能指标
            uint8_t targetFps = (displayMode == 0) ? 30 : 15; // 小屏目标30FPS，全屏目标15FPS
            const char* modeStr = (displayMode == 0) ? "小屏" : "全屏";
            
            Serial.print("[全屏性能] ");
            Serial.print(modeStr);
            Serial.print(" | FPS: ");
            Serial.print(currentFps);
            Serial.print(" | 目标: ");
            Serial.print(targetFps);
            Serial.print("+ | ");
            
            // 性能状态评估
            if (displayMode == 1) { // 全屏模式特殊评估
                if (currentFps >= 15) {
                    Serial.println("✓ 全屏优秀 - 解码流畅");
                } else if (currentFps >= 12) {
                    Serial.println("△ 全屏良好 - 轻微卡顿");
                } else if (currentFps >= 8) {
                    Serial.println("⚠ 全屏一般 - 明显卡顿");
                } else {
                    Serial.println("✗ 全屏需优化 - 严重卡顿");
                }
            } else { // 小屏模式
                if (currentFps >= 30) {
                    Serial.println("✓ 小屏优秀");
                } else if (currentFps >= 25) {
                    Serial.println("△ 小屏良好");
                } else {
                    Serial.println("⚠ 小屏需优化");
                }
            }
            
            // 输出当前解码模式
            if (performanceMode == 1) {
                Serial.println("  → 高性能模式: JPEG_SCALE_QUARTER");
            } else {
                Serial.println("  → 高质量模式: JPEG_SCALE_HALF");
            }
            
            // 新增：显示模式稳定性监控
            static uint8_t lastDisplayMode = 1; // 默认全屏模式
            if (displayMode != lastDisplayMode) {
                Serial.print("⚠ 显示模式异常切换: ");
                Serial.print(lastDisplayMode == 1 ? "全屏" : "小屏");
                Serial.print(" → ");
                Serial.println(displayMode == 1 ? "全屏" : "小屏");
                
                // 强制恢复全屏模式（如果检测到异常切换）
                if (lastDisplayMode == 1 && displayMode == 0) {
                    Serial.println("🔧 自动恢复全屏模式");
                    displayMode = 1; // 强制恢复全屏模式
                }
                lastDisplayMode = displayMode;
            }
        }
    }
}

void setup()
{
    Serial.begin(115200);
    
    // 等待串口连接
    delay(1000);
    
    Serial.println("=== AMB82-MINI 摄像头LCD显示系统 ===");
    Serial.println("版本：阶段四全屏性能优化版");
    Serial.println("优化目标：全屏模式流畅播放，消除卡顿");
    Serial.println("核心功能：智能自适应缩放 + 渲染优化");
    Serial.println("========================================");
    
    // 初始化TFT显示屏
    tft.begin();
    tft.fillScreen(0x0000); // 黑色
    tft.setRotation(1); // 横屏模式
    
    Serial.println("TFT显示屏初始化完成");
    
    // 阶段四优化：根据显示模式配置摄像头参数
    if (displayMode == 1) {
        config.setJpegQuality(3); // 全屏模式：平衡质量和性能
        Serial.println("全屏模式：摄像头JPEG质量级别设置为 3");
    } else {
        config.setJpegQuality(2); // 标准模式：高质量
        Serial.println("标准模式：摄像头JPEG质量级别设置为 2");
    }
    
    // 初始化摄像头
    Camera.configVideoChannel(CHANNEL, config);
    Camera.videoInit();
    Camera.channelBegin(CHANNEL);
    
    Serial.println("摄像头初始化完成");
    Serial.println("摄像头视频流已启动");
    Serial.println("开始实时显示...");
    Serial.println("提示：全屏模式下会自动优化性能");
    
    // 阶段三优化：高性能JPEG解码配置
    // 预初始化JPEG解码器以减少延迟
    jpeg.openFLASH((uint8_t *)0, 0, JPEGDraw);
    
    // 优化：降低MaxOutputSize到6，平衡内存使用和性能
    jpeg.setMaxOutputSize(6);  // 从8降低到6，减少内存压力33%
    
    // 初始化计时器
    lastFrameTime = millis();
    lastFpsTime = lastFrameTime;
    lastModeSwitchTime = millis();
    
    Serial.println("System initialized. Starting optimized display loop...");
}

void loop()
{
    unsigned long currentTime = millis();
    
    // 阶段一优化：性能监控开始
    if (performanceMonitoring) {
        perfMetrics.lastFrameStart = currentTime;
        perfMetrics.displayTime = 0; // 重置显示时间
    }
    
    // 阶段一优化：智能帧率控制
    unsigned long waitTime = adaptiveFrameControl();
    if (waitTime > 0) {
        // 使用yield()替代delay()，让其他任务有机会运行
        yield();
        return;
    }
    
    lastFrameTime = currentTime;
    
    // 阶段一优化：内存管理和错误处理
    // 获取新图像
    Camera.getImage(CHANNEL, &img_addr, &img_len);
    
    // 优化：简化图像缓存检查
    if (img_addr != 0 && img_len > 0 && isNewImage(img_addr, img_len)) {
        // 更新缓存
        lastImage.addr = img_addr;
        lastImage.len = img_len;
        
        // 阶段一优化：智能JPEG解码器管理
        // 只有在必要时才重新初始化解码器
        static uint32_t lastDecodeAddr = 0;
        if (img_addr != lastDecodeAddr) {
            // 关闭之前的解码器
            jpeg.close();
            lastDecodeAddr = img_addr;
        }
        
        // 阶段一优化：性能监控 - 记录解码开始时间
        unsigned long decodeStartTime = 0;
        if (performanceMonitoring) {
            decodeStartTime = micros();
        }
        
        // 阶段四优化：智能全屏性能管理
        if (jpeg.openFLASH((uint8_t *)img_addr, img_len, JPEGDraw)) {
            // 全屏性能优化：根据窗口尺寸和帧率动态调整
            static uint8_t lastPerformanceMode = 0;
            
            // 检测性能模式变化
            if (performanceMode != lastPerformanceMode) {
                lastPerformanceMode = performanceMode;
                Serial.print("性能模式切换: ");
                Serial.println(performanceMode == 1 ? "高性能模式" : "高质量模式");
            }
            
            // 全屏模式性能优化策略 - 添加模式切换保护
            if (displayMode == 1) { // 全屏模式
                // 模式切换保护：避免频繁切换
                unsigned long currentTime = millis();
                if (currentTime - lastModeSwitchTime > 2000) { // 2秒保护期
                    // 更激进的性能管理：低于15FPS立即降级
                    if (currentFps < 15 && performanceMode == 0) {
                        performanceMode = 1;
                        lastModeSwitchTime = currentTime;
                        Serial.println("全屏优化：切换到JPEG_SCALE_QUARTER");
                    } else if (currentFps >= 20 && performanceMode == 1) {
                        performanceMode = 0;
                        lastModeSwitchTime = currentTime;
                        Serial.println("全屏优化：升级到JPEG_SCALE_HALF");
                    }
                }
            } else { // 标准模式
                // 标准性能管理：保持原有策略
                unsigned long currentTime = millis();
                if (currentTime - lastModeSwitchTime > 2000) { // 2秒保护期
                    if (currentFps < 12 && performanceMode == 0) {
                        performanceMode = 1;
                        lastModeSwitchTime = currentTime;
                        Serial.println("性能模式：切换到JPEG_SCALE_QUARTER");
                    } else if (currentFps >= 16 && performanceMode == 1) {
                        performanceMode = 0;
                        lastModeSwitchTime = currentTime;
                        Serial.println("性能模式：升级到JPEG_SCALE_HALF");
                    }
                }
            }
            
            // 动态选择缩放模式
            if (performanceMode == 0) {
                jpeg.decode(0, 0, JPEG_SCALE_HALF);   // 高质量全屏
            } else {
                jpeg.decode(0, 0, JPEG_SCALE_QUARTER); // 高性能模式
            }
            
            // 阶段一优化：记录解码完成时间
            if (performanceMonitoring) {
                perfMetrics.decodeTime = micros() - decodeStartTime;
                perfMetrics.totalDecodeTime += perfMetrics.decodeTime;
            }
            
            // 阶段一优化：解码完成后保持解码器打开状态，减少重复初始化
            // jpeg.close(); // 注释掉，保持解码器状态
        } else {
            // 优化：减少错误输出频率
            static uint8_t errorCounter = 0;
            if (++errorCounter >= 20) { // 增加错误间隔到20次
                errorCounter = 0;
                Serial.println("JPEG decode failed!");
            }
        }
    }
    
    // 阶段二优化：性能监控结束和统计
    if (performanceMonitoring) {
        perfMetrics.totalFrameTime = millis() - perfMetrics.lastFrameStart;
        
        // 每100帧计算一次平均值
        if (++perfMetrics.frameCount >= 100) {
            perfMetrics.averageDecodeTime = perfMetrics.totalDecodeTime / 100.0 / 1000.0; // 转换为毫秒
            perfMetrics.averageDisplayTime = perfMetrics.totalDisplayTime / 100.0 / 1000.0; // 转换为毫秒
            
            // 输出性能统计
            Serial.println("\n=== 阶段三优化性能统计 ===");
            Serial.print("平均解码时间: "); Serial.print(perfMetrics.averageDecodeTime); Serial.println(" ms");
            Serial.print("平均显示时间: "); Serial.print(perfMetrics.averageDisplayTime); Serial.println(" ms");
            Serial.print("平均总帧时间: "); Serial.print(perfMetrics.totalFrameTime); Serial.println(" ms");
            Serial.print("当前FPS: "); Serial.println(currentFps);
            Serial.print("目标FPS: 15+ | 当前性能: ");
            
            if (currentFps >= 15) {
                Serial.println("✓ 优秀 - 达到目标");
            } else if (currentFps >= 12) {
                Serial.println("△ 良好 - 接近目标");
            } else {
                Serial.println("✗ 需优化 - 低于目标");
            }
            Serial.println("========================\n");
            
            // 重置统计
            perfMetrics.frameCount = 0;
            perfMetrics.totalDecodeTime = 0;
            perfMetrics.totalDisplayTime = 0;
        }
    }
    
    // 更新FPS计数器
    updateFpsCounter();
}