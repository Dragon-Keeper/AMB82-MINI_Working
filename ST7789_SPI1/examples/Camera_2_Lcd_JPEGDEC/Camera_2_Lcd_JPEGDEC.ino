/*******************************************************
 * 功能增量测试 Step 2：实时预览 + 定时拍照保存至SD卡
 * 在Step 1（预览）成功的基础上增加：
 * 1. 独立高分辨率拍照通道（720p）
 * 2. 定时器（每5秒触发拍照）
 * 3. SD卡存储功能
 * 注意：此版本为清晰逻辑，暂未启用DMA传输优化。
 ******************************************************/

#include <SPI.h>
#include "AmebaST7789_DMA_SPI1.h"
#include "VideoStream.h"
#include <JPEGDEC_Libraries/JPEGDEC.h>
// --- 新增：SD卡文件系统库 ---
#include "AmebaFatFS.h"

// === 硬件引脚定义（保持不变） ===
#define TFT_CS    SPI1_SS
#define TFT_DC    4
#define TFT_RESET 5
#define TFT_LED   6

// === 通道与配置定义 ===
#define PREVIEW_CH  0  // 预览通道：VGA
#define STILL_CH    1  // --- 新增：拍照通道：720p ---
VideoSetting configPreview(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);
// --- 新增：高分辨率拍照配置 ---
VideoSetting configStill(VIDEO_HD, CAM_FPS, VIDEO_JPEG, 1);

// === 全局对象 ===
AmebaST7789_DMA_SPI1 tft(TFT_CS, TFT_DC, TFT_RESET);
JPEGDEC jpeg;
// --- 新增：SD卡文件系统对象 ---
AmebaFatFS fs;

// 图像缓冲区
uint32_t img_addr_preview = 0, img_len_preview = 0;
// --- 新增：拍照图像缓冲区 ---
uint32_t img_addr_still = 0, img_len_still = 0;

// --- 新增：拍照控制与状态变量 ---
const unsigned long CAPTURE_INTERVAL_MS = 5000; // 拍照间隔：5秒
unsigned long lastCaptureTime = 0;               // 上次拍照时间
bool isCapturing = false;                        // 拍照状态锁，防止重入
uint32_t photoCount = 0;                         // 拍照计数（用于生成文件名）

// JPEG解码回调函数（与Step 1相同，用于预览）
int JPEGDraw(JPEGDRAW *pDraw) {
    tft.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    return 1;
}

void setup() {
    // 1. 初始化串口和屏幕背光
    pinMode(TFT_LED, OUTPUT);
    digitalWrite(TFT_LED, HIGH);
    Serial.begin(115200);
    while(!Serial);
    Serial.println("=== 功能测试 Step 2: 预览 + 定时拍照保存 ===");

    // 2. 初始化SPI和显示屏
    SPI1.setDataMode(TFT_CS, SPI_MODE0, SPI_MODE_MASTER);
    SPI1.setDefaultFrequency(25000000);
    SPI1.begin();
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(ST7789_BLACK);
    tft.setCursor(10, 10);
    tft.print("Initializing...");
    Serial.println("显示屏初始化完成。");

    // 3. --- 新增：初始化SD卡 ---
    Serial.print("初始化SD卡...");
    if (fs.begin()) {
        Serial.println("成功");
        tft.setCursor(10, 30);
        tft.print("SD Card OK");
    } else {
        Serial.println("失败！拍照功能将不可用。");
        tft.setCursor(10, 30);
        tft.print("SD Card FAIL");
        // 注意：SD卡初始化失败不影响后续程序运行，但拍照保存会失败。
    }

    // 4. 初始化摄像头（双通道配置）
    Serial.println("正在初始化摄像头（双通道）...");
    Camera.configVideoChannel(PREVIEW_CH, configPreview); // 配置预览通道
    Camera.configVideoChannel(STILL_CH, configStill);     // --- 新增：配置拍照通道 ---
    Camera.videoInit();                                   // 摄像头硬件初始化
    Camera.channelBegin(PREVIEW_CH);                      // 只开启预览通道
    // 注意：拍照通道 `STILL_CH` 暂不开启，仅在拍照时临时开启。
    Serial.println("摄像头初始化完成。预览通道已开启。");

    // 5. 显示就绪信息
    delay(500);
    tft.fillScreen(ST7789_BLACK);
    tft.setCursor(60, 140);
    tft.print("Ready");
    tft.setCursor(40, 160);
    tft.print("Preview Running");
    
    lastCaptureTime = millis(); // 初始化拍照计时器
    Serial.println("系统就绪。预览进行中，每5秒自动拍照。");
}

void loop() {
    // ========== 第一部分：实时预览（与Step 1完全相同） ==========
    Camera.getImage(PREVIEW_CH, &img_addr_preview, &img_len_preview);
    if (img_len_preview > 0) {
        if (jpeg.openFLASH((uint8_t *)img_addr_preview, img_len_preview, JPEGDraw)) {
            jpeg.decode(0, 0, JPEG_SCALE_HALF);
            jpeg.close();
        }
    }

    // ========== --- 新增：第二部分：定时拍照逻辑 --- ==========
    unsigned long currentTime = millis();
    
    // 检查是否到达拍照时间且当前没有正在进行的拍照任务
    if (!isCapturing && (currentTime - lastCaptureTime >= CAPTURE_INTERVAL_MS)) {
        captureAndSavePhoto(); // 执行拍照并保存
        lastCaptureTime = currentTime; // 更新上次拍照时间
    }
    
    // 此处可以添加一个小的延时来控制预览的最大帧率，如 delay(10);
}

// --- 新增：核心拍照保存函数 ---
void captureAndSavePhoto() {
    if (isCapturing) {
        Serial.println("警告：上一次拍照尚未结束，跳过。");
        return;
    }
    isCapturing = true; // 上锁
    Serial.println(">>> 开始自动拍照流程");
    
    // 步骤1：临时开启高分辨率拍照通道
    Camera.channelBegin(STILL_CH);
    Serial.println("  1. 拍照通道已开启");
    delay(120); // 重要：给传感器一点时间稳定到高分辨率模式

    // 步骤2：捕获单帧720p图像
    Camera.getImage(STILL_CH, &img_addr_still, &img_len_still);
    if (img_len_still > 0) {
        Serial.print("  2. 图像捕获成功，大小: ");
        Serial.print(img_len_still);
        Serial.println(" 字节");
        
        // 步骤3：保存到SD卡
        if (saveToSDCard(img_addr_still, img_len_still)) {
            photoCount++;
            // 在屏幕上给出视觉反馈（可选）
            tft.setCursor(100, 10);
            tft.print("SAVED");
            delay(200);
            tft.setCursor(100, 10);
            tft.print("     "); // 清除提示
        }
    } else {
        Serial.println("  2. 错误：未能捕获图像数据");
    }
    
    // 步骤4：立即关闭拍照通道，释放资源
    Camera.channelEnd(STILL_CH);
    Serial.println("  3. 拍照通道已关闭");
    
    isCapturing = false; // 解锁
    Serial.println("<<< 拍照流程结束");
    Serial.println();
}

// --- 新增：SD卡保存函数 ---
bool saveToSDCard(uint32_t img_addr, uint32_t img_len) {
    // 每次操作前重新初始化文件系统，确保稳定性
    if (!fs.begin()) {
        Serial.println("  保存失败：文件系统初始化错误");
        return false;
    }

    // 生成带时间戳的唯一文件名 (例如: IMG_20250101_120305.jpg)
    char filename[50];
    sprintf(filename, "%sIMG_%lu.jpg", fs.getRootPath(), millis());
    // 注：使用毫秒时间戳简单保证唯一性。更完善的做法是使用RTC时间。
    
    Serial.print("  正在保存至: ");
    Serial.println(filename);

    // 打开文件进行写入
    File file = fs.open(filename);
    if (!file) {
        Serial.println("  保存失败：无法创建文件");
        fs.end();
        return false;
    }

    // 写入JPEG数据
    int bytesWritten = file.write((uint8_t *)img_addr, img_len);
    file.close(); // 关闭文件
    fs.end();     // 结束本次文件系统会话

    // 验证结果
    if (bytesWritten == (int)img_len) {
        Serial.print("  保存成功！写入 ");
        Serial.print(bytesWritten);
        Serial.println(" 字节");
        return true;
    } else {
        Serial.print("  保存警告：写入字节数 (");
        Serial.print(bytesWritten);
        Serial.print(") 与预期 (");
        Serial.print(img_len);
        Serial.println(") 不符");
        return false;
    }
}