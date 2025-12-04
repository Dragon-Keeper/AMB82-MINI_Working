/*******************************************************
 * Camera → ST7789 TFT (双通道 DMA 优化版) - 修正版
 * 硬件：BW21-CBV-Kit / AMB82-MINI (基于RTL8735B)
 * 功能：预览通道(VGA)实时显示，拍照通道(720p)按需抓拍保存
 * 优化：按需启停拍照通道，彻底避免VOE队列满错误
 * 修正：适配 AMB82-MINI SDK API (Camera.getImage 返回 void, Serial.printf 不可用)
 ******************************************************/

#include "VideoStream.h"
#include "SPI.h"
#include "AmebaST7789_DMA.h"   // ST7789 DMA驱动
#include <JPEGDEC_Libraries/JPEGDEC.h>
#include "AmebaFatFS.h"

// === 硬件引脚定义 ===
#define TFT_LED   6   // 背光控制
#define TFT_RESET 5
#define TFT_DC    4
#define TFT_CS    SPI_SS  // SPI0片选

// === 通道定义 ===
#define PREVIEW_CH  0  // 预览通道：VGA，持续开启
#define STILL_CH    1  // 拍照通道：720p，按需开启

// === 全局变量 ===
AmebaST7789_DMA tft = AmebaST7789_DMA(TFT_CS, TFT_DC, TFT_RESET);
JPEGDEC jpeg;
AmebaFatFS fs;

// 视频流配置
VideoSetting configPreview(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1); // 预览：VGA@30fps
VideoSetting configStill(VIDEO_HD, CAM_FPS, VIDEO_JPEG, 1);    // 拍照：720p@30fps

// 图像缓冲区
uint32_t img_addr_preview = 0, img_len_preview = 0;
uint32_t img_addr_still = 0, img_len_still = 0;

// SD卡与拍照控制
const char* FILENAME_PREFIX = "HD_";
uint32_t photoCount = 0;
unsigned long lastCaptureTime = 0;
const unsigned long CAPTURE_INTERVAL_MS = 3000; // 3秒拍照间隔
bool isCapturing = false; // 拍照函数防重入标志

// 性能监控
unsigned long lastPrintTime = 0;
int frameCount = 0;

// === 函数声明 ===
void captureHighResolutionPhoto();
bool saveToSDCard(uint32_t addr, uint32_t len);
int JPEGDraw(JPEGDRAW *pDraw);

/*******************************************************
 * JPEG解码回调函数：直接DMA传输到屏幕
 ******************************************************/
int JPEGDraw(JPEGDRAW *pDraw) {
    tft.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    return 1;
}

/*******************************************************
 * 初始化设置
 ******************************************************/
void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10); // 等待串口连接
    Serial.println("\n=== 双通道摄像头应用启动 ===");
    Serial.println("预览通道：VGA (640x480) 实时显示");
    Serial.println("拍照通道：720p (1280x720) 按需抓拍");
    Serial.println("模式：自动定时拍照（每3秒）");

    // 1. 初始化显示屏
    pinMode(TFT_LED, OUTPUT);
    digitalWrite(TFT_LED, HIGH); // 开启背光
    
    SPI.setDefaultFrequency(50000000); // 50MHz SPI
    tft.begin();
    tft.setRotation(1);      // 横屏模式
    tft.setColorOrder(true); // RGB颜色顺序
    tft.fillScreen(0x0000);  // 清屏为黑色
    tft.setCursor(10, 10);
    tft.print("系统启动中...");

    // 2. 初始化SD卡（全局一次）
    if (fs.begin()) {
        Serial.println("SD卡初始化成功");
    } else {
        Serial.println("警告：SD卡初始化失败，拍照将无法保存");
    }

    // 3. 初始化摄像头（配置双通道，但拍照通道不开启）
    Camera.configVideoChannel(PREVIEW_CH, configPreview);
    Camera.configVideoChannel(STILL_CH, configStill); // 仅配置，不开启
    Camera.videoInit();
    Camera.channelBegin(PREVIEW_CH); // 只开启预览通道
    Serial.println("摄像头初始化完成（仅预览通道开启）");

    // 4. 显示就绪信息
    tft.fillScreen(0x0000);
    tft.setCursor(20, 150);
    tft.print("Ready.");
    
    lastCaptureTime = millis();
    lastPrintTime = millis();
}

/*******************************************************
 * 主循环：始终优先保证预览流畅
 ******************************************************/
void loop() {
    // 移除未使用的 loopStart 变量以避免警告
    frameCount++;

    // === 1. 实时预览（最高优先级） ===
    // 修正：Camera.getImage 返回 void，不能与 0 比较。通过 img_len_preview > 0 判断是否有效。
    Camera.getImage(PREVIEW_CH, &img_addr_preview, &img_len_preview);
    if (img_len_preview > 0) {
        if (jpeg.openFLASH((uint8_t *)img_addr_preview, img_len_preview, JPEGDraw)) {
            jpeg.decode(0, 0, JPEG_SCALE_HALF); // VGA缩放到320x240显示
            jpeg.close();
        }
    }

    // === 2. 检查并触发拍照（非阻塞方式） ===
    unsigned long now = millis();
    if (!isCapturing && (now - lastCaptureTime >= CAPTURE_INTERVAL_MS)) {
        captureHighResolutionPhoto();
    }

    // === 3. 性能监控（每秒打印一次） ===
    if (now - lastPrintTime >= 1000) {
        int fps = frameCount; // 过去一秒的预览帧数
        frameCount = 0;
        lastPrintTime = now;
        
        // 计算平均每帧耗时(ms)
        int avgLoopTime = 0;
        if (fps > 0) {
            avgLoopTime = 1000 / fps;
        } else {
            avgLoopTime = 999; // 如果fps为0，表示非常慢
        }
        
        // 修正：使用 Serial.print 代替 Serial.printf
        Serial.print("[状态] 预览FPS: ");
        Serial.print(fps);
        Serial.print(", 平均每帧耗时: ");
        Serial.print(avgLoopTime);
        Serial.print("ms");
        if (avgLoopTime > 33) {
            Serial.print(" (注意：接近30fps极限)"); // 30fps对应33ms/帧
        }
        Serial.println();
    }
}

/*******************************************************
 * 高分辨率拍照函数（按需启停通道）
 ******************************************************/
void captureHighResolutionPhoto() {
    if (isCapturing) {
        Serial.println("警告：上一次拍照仍在进行，跳过。");
        return;
    }
    isCapturing = true;
    Serial.println(">>> 开始拍照流程");
    
    // 步骤1：开启拍照通道（硬件开始生产720p帧）
    Camera.channelBegin(STILL_CH);
    Serial.println("  1. 拍照通道已开启");
    
    // 关键：给传感器时间稳定到高分辨率模式
    delay(120);
    
    // 可选：在抓拍前快速清空几帧预览，保持预览响应
    for(int i = 0; i < 2; i++) {
        Camera.getImage(PREVIEW_CH, &img_addr_preview, &img_len_preview);
        if (img_len_preview > 0 && jpeg.openFLASH((uint8_t *)img_addr_preview, img_len_preview, JPEGDraw)) {
            jpeg.decode(0, 0, JPEG_SCALE_HALF);
            jpeg.close();
        }
    }
    
    // 步骤2：捕获单张720p帧
    // 修正：Camera.getImage 返回 void，不能与 0 比较。
    Camera.getImage(STILL_CH, &img_addr_still, &img_len_still);
    if (img_len_still > 0) {
        // 修正：使用 Serial.print 代替 Serial.printf
        Serial.print("  2. 已捕获帧，大小: ");
        Serial.print(img_len_still);
        Serial.println(" bytes");
        
        // 步骤3：保存到SD卡
        if (saveToSDCard(img_addr_still, img_len_still)) {
            photoCount++;
        }
    } else {
        Serial.println("  2. 错误：未能捕获图像数据");
    }
    
    // 步骤4：立即关闭拍照通道（停止生产，释放VOE队列）
    Camera.channelEnd(STILL_CH);
    Serial.println("  3. 拍照通道已关闭，资源释放");
    
    lastCaptureTime = millis();
    isCapturing = false;
    Serial.println("<<< 拍照流程结束\n");
}

/*******************************************************
 * SD卡保存函数（修正版 - 完全按原成功模式重构）
 ******************************************************/
bool saveToSDCard(uint32_t img_addr, uint32_t img_len) {
    // 0. 每次操作前初始化文件系统 (与你的成功代码模式一致)
    if (!fs.begin()) {
        Serial.println("  保存失败：文件系统初始化错误");
        return false;
    }

    // 1. 构建文件路径 (确保与成功代码格式一致)
    String filename = String(fs.getRootPath()) + String(FILENAME_PREFIX) + String(photoCount) + String(".jpg");
    
    // 2. 打开文件 (使用单参数调用，与你原成功代码完全一致)
    File file = fs.open(filename);
    
    if (!file) {
        Serial.println("  保存失败：无法创建或打开文件");
        fs.end(); // 结束本次会话
        return false;
    }

    // 3. 写入数据 (保留你原代码中的 delay(1)，有时能提高稳定性)
    int bytesWritten = file.write((uint8_t *)img_addr, img_len);
    delay(1); // 关键：与原成功代码保持一致，确保数据稳定写入
    file.close();
    
    // 4. 结束本次文件系统会话 (与fs.begin()配对)
    fs.end();

    // 5. 检查结果
    if (bytesWritten == (int)img_len) {
        Serial.print("  保存成功：");
        Serial.print(filename);
        Serial.print(" (");
        Serial.print(bytesWritten);
        Serial.println(" bytes)");
        return true;
    } else {
        Serial.print("  保存警告：写入字节数(");
        Serial.print(bytesWritten);
        Serial.print(")与预期(");
        Serial.print(img_len);
        Serial.println(")不符");
        // 注意：由于已调用fs.end()，此处无法直接删除文件。
        // 可考虑在下次初始化后删除，或暂时忽略不完整文件。
        return false;
    }
}