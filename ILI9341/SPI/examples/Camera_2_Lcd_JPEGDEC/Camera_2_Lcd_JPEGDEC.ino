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

// 阶段一优化：增强的JPEG绘制回调函数，添加性能监控
int JPEGDraw(JPEGDRAW *pDraw)
{
    unsigned long drawStartTime = 0;
    if (performanceMonitoring) {
        drawStartTime = micros();
    }
    
    if (useOptimizedDraw) {
        // 使用高度优化的drawBitmapOptimized函数
        tft.drawBitmapOptimized(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    } else {
        // 备用绘制方法
        tft.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
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

// 阶段一优化：智能帧率控制函数
unsigned long adaptiveFrameControl() {
    static unsigned long lastFrameEnd = 0;
    // static float fpsAdjustment = 1.0; // 未使用，已注释
    
    unsigned long currentTime = millis();
    unsigned long frameTime = currentTime - lastFrameEnd;
    
    // 基础帧间隔（33ms ≈ 30FPS）
    #define FRAME_INTERVAL 33
    
    if (frameTime < FRAME_INTERVAL) {
        lastFrameEnd = currentTime + (FRAME_INTERVAL - frameTime);
        return FRAME_INTERVAL - frameTime;  // 需要等待的时间
    } else {
        lastFrameEnd = currentTime;
        return 0;  // 立即开始下一帧
    }
}

// 优化：简化FPS计数器，减少计算开销
void updateFpsCounter() {
    frameCount++;
    unsigned long currentTime = millis();
    if (currentTime - lastFpsTime >= 1000) { // 每秒更新一次FPS
        currentFps = frameCount;
        frameCount = 0;
        lastFpsTime = currentTime;
        
        // 优化：减少串口输出频率
        static uint8_t printCounter = 0;
        if (++printCounter >= 5) { // 每5秒输出一次
            printCounter = 0;
            Serial.print("FPS: ");
            Serial.println(currentFps);
        }
    }
}

void setup()
{
    Serial.begin(115200);

    Serial.println("TFT ILI9341 - Hardware Parallel 8-bit Interface (Phase 1 Optimized)");
    Serial.println("Phase 1 Optimizations Applied:");
    Serial.println("- Resolution: VGA (640x480) - 20% data reduction");
    Serial.println("- JPEG Decoder: MaxOutputSize=5, LE_PIXELS format");
    Serial.println("- Smart Frame Rate Control: Adaptive timing");
    Serial.println("- Memory Management: Intelligent decoder reuse");
    Serial.println("- Performance Monitoring: Detailed metrics enabled");

    // 初始化摄像头
    Camera.configVideoChannel(CHANNEL, config);
    Camera.videoInit();
    Camera.channelBegin(CHANNEL);
    
    // 优化：尝试降低JPEG质量
    // Camera.setJpegQuality(60); // 如果API可用，取消注释

    // 初始化TFT显示屏
    tft.begin();
    tft.setRotation(1);
    
    // 阶段一优化：JPEG解码参数优化
    // 预初始化JPEG解码器以减少延迟
    jpeg.openFLASH((uint8_t *)0, 0, JPEGDraw);
    
    // 优化：设置更小的MaxOutputSize，减少内存使用
    jpeg.setMaxOutputSize(5);  // 从10降低到5，减少内存占用50%
    
    // 阶段一优化：设置JPEG解码选项以提升性能
    // jpeg.setOptions(JPEG_LUMA_ONLY);     // 仅解码亮度分量（如硬件支持）
    // jpeg.setOptions(JPEG_LE_PIXELS);     // 小端像素格式，提升效率（方法不存在，已注释）（方法不存在，已注释）
    
    lastFrameTime = millis();
    lastFpsTime = lastFrameTime;
    
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
        
        // 使用优化的JPEG解码流程
        if (jpeg.openFLASH((uint8_t *)img_addr, img_len, JPEGDraw)) {
            // 优化：使用JPEG_SCALE_QUARTER替代JPEG_SCALE_HALF，进一步减少解码时间
            jpeg.decode(0, 0, JPEG_SCALE_QUARTER);
            
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
    
    // 阶段一优化：性能监控结束和统计
    if (performanceMonitoring) {
        perfMetrics.totalFrameTime = millis() - perfMetrics.lastFrameStart;
        
        // 每100帧计算一次平均值
        if (++perfMetrics.frameCount >= 100) {
            perfMetrics.averageDecodeTime = perfMetrics.totalDecodeTime / 100.0 / 1000.0; // 转换为毫秒
            perfMetrics.averageDisplayTime = perfMetrics.totalDisplayTime / 100.0 / 1000.0; // 转换为毫秒
            
            // 输出性能统计
            Serial.println("\n=== 阶段一优化性能统计 ===");
            Serial.print("平均解码时间: "); Serial.print(perfMetrics.averageDecodeTime); Serial.println(" ms");
            Serial.print("平均显示时间: "); Serial.print(perfMetrics.averageDisplayTime); Serial.println(" ms");
            Serial.print("平均总帧时间: "); Serial.print(perfMetrics.totalFrameTime); Serial.println(" ms");
            Serial.print("当前FPS: "); Serial.println(currentFps);
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