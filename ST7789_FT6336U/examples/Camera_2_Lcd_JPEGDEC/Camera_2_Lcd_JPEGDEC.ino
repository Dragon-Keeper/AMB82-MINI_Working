/*******************************************************
 * 功能增量测试 Step 2：实时预览 + 触摸拍照保存至SD卡
 * 在Step 1（预览）成功的基础上增加：
 * 1. 独立高分辨率拍照通道（720p）
 * 2. 触摸拍照按钮（屏幕右侧红色按钮）
 * 3. SD卡存储功能
 * 4. FT6336U触摸屏支持（中断模式）
 ******************************************************/

#include "SPI.h"
#include "AmebaST7789_DMA_SPI1.h"
#include "VideoStream.h"
#include "JPEGDEC_Libraries/JPEGDEC.h"
// --- 新增：SD卡文件系统库 ---
#include "AmebaFatFS.h"
// --- 新增：触摸屏库 ---
#include "FT6336U_HardI2C.h"

// === 硬件引脚定义 ===
// SPI显示屏引脚
#define TFT_CS    SPI1_SS
#define TFT_DC    4
#define TFT_RESET 5
#define TFT_LED   6

// 触摸屏引脚（与TwoPointExample.ino保持一致）
#define TOUCH_SDA 12
#define TOUCH_SCL 13
#define TOUCH_RST 15
#define TOUCH_INT 16

// === 通道与配置定义 ===
#define PREVIEW_CH  0  // 预览通道：VGA
#define STILL_CH    1  // 拍照通道：720p
VideoSetting configPreview(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);
VideoSetting configStill(VIDEO_HD, CAM_FPS, VIDEO_JPEG, 1);

// === 全局对象 ===
AmebaST7789_DMA_SPI1 tft(TFT_CS, TFT_DC, TFT_RESET);
JPEGDEC jpeg;
AmebaFatFS fs;
// --- 新增：触摸屏对象 ---
FT6336U_HardI2C touch(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);

// 图像缓冲区
uint32_t img_addr_preview = 0, img_len_preview = 0;
uint32_t img_addr_still = 0, img_len_still = 0;

// 拍照控制与状态变量
bool isCapturing = false;     // 拍照状态锁，防止重入
uint32_t photoCount = 0;      // 拍照计数（用于生成文件名）

// --- 新增：拍照按钮定义 ---
#define BUTTON_WIDTH   40
#define BUTTON_HEIGHT  80
#define BUTTON_X       (ST7789_TFTWIDTH - BUTTON_WIDTH - 5)  // 右侧边缘
#define BUTTON_Y       ((ST7789_TFTHEIGHT - BUTTON_HEIGHT) / 2)  // 垂直居中
#define BUTTON_COLOR_NORMAL    ST7789_RED     // 常态红色
#define BUTTON_COLOR_PRESSED   ST7789_WHITE   // 按下时白色

bool buttonPressed = false;    // 按钮当前状态
bool buttonLastState = false;  // 按钮上一次状态
bool captureTriggered = false; // 拍照触发标志

// JPEG解码回调函数（用于预览）- 修改版，添加按钮重绘
int JPEGDraw(JPEGDRAW *pDraw) {
    tft.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    return 1;
}

// 新增：专用绘制函数，不干扰预览
void drawButtonOverlay(bool pressed) {
    uint16_t color = pressed ? BUTTON_COLOR_PRESSED : BUTTON_COLOR_NORMAL;
    
    // 绘制按钮主体（使用DMA快速绘制）
    tft.fillRectangle(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, color);
    
    // 绘制按钮边框
    tft.drawRectangle(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 
                     pressed ? ST7789_LIGHTGRAY : ST7789_DARKGRAY);
    
    // 绘制相机图标
    int centerX = BUTTON_X + BUTTON_WIDTH / 2;
    int centerY = BUTTON_Y + BUTTON_HEIGHT / 2;
    
    // 相机镜头（圆形）
    tft.drawCircle(centerX, centerY, 12, 
                   pressed ? ST7789_BLACK : ST7789_WHITE);
    
    // 相机主体（矩形）
    tft.fillRectangle(centerX - 8, centerY - 5, 16, 10, 
                     pressed ? ST7789_BLACK : ST7789_WHITE);
}

void setup() {
    // 1. 初始化串口和屏幕背光
    pinMode(TFT_LED, OUTPUT);
    digitalWrite(TFT_LED, HIGH);
    Serial.begin(115200);
    while(!Serial);
    Serial.println("=== 功能测试 Step 2: 预览 + 触摸拍照保存 ===");

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

    // 3. 初始化SD卡
    Serial.print("初始化SD卡...");
    if (fs.begin()) {
        Serial.println("成功");
        tft.setCursor(10, 30);
        tft.print("SD Card OK");
    } else {
        Serial.println("失败！拍照功能将不可用。");
        tft.setCursor(10, 30);
        tft.print("SD Card FAIL");
    }

    // 4. --- 新增：初始化触摸屏（中断模式）---
    Serial.println("初始化触摸屏...");
    touch.enableDebug(false);  // 禁用调试输出（避免干扰）
    if (!touch.begin()) {
        Serial.println("❌ 触摸屏初始化失败!");
        tft.setCursor(10, 50);
        tft.print("Touch FAIL");
        // 触摸屏初始化失败不影响后续程序运行，但触摸拍照功能不可用
    } else {
        // 设置中断模式
        touch.setInterruptMode(true);
        // 设置轮询间隔（用于备用轮询模式）
        touch.setPollingInterval(20);
        Serial.println("✅ 触摸屏初始化成功（中断模式）");
        tft.setCursor(10, 50);
        tft.print("Touch OK");
    }

    // 5. 初始化摄像头（双通道配置）
    Serial.println("正在初始化摄像头（双通道）...");
    Camera.configVideoChannel(PREVIEW_CH, configPreview); // 配置预览通道
    Camera.configVideoChannel(STILL_CH, configStill);     // 配置拍照通道
    Camera.videoInit();                                   // 摄像头硬件初始化
    Camera.channelBegin(PREVIEW_CH);                      // 只开启预览通道
    Serial.println("摄像头初始化完成。预览通道已开启。");

    // 6. 显示就绪信息
    delay(500);
    tft.fillScreen(ST7789_BLACK);
    tft.setCursor(60, 140);
    tft.print("Ready");
    tft.setCursor(40, 160);
    tft.print("Touch to Capture");
    
    // 绘制初始按钮
    drawButtonOverlay(false);
    
    Serial.println("系统就绪。触摸右侧红色按钮进行拍照。");
}

void loop() {
    // ========== 第一部分：实时预览 ==========
    Camera.getImage(PREVIEW_CH, &img_addr_preview, &img_len_preview);
    if (img_len_preview > 0) {
        if (jpeg.openFLASH((uint8_t *)img_addr_preview, img_len_preview, JPEGDraw)) {
            jpeg.decode(0, 0, JPEG_SCALE_HALF);
            jpeg.close();
            
            // 重要：在预览图像显示后立即重绘按钮
            drawButtonOverlay(buttonPressed);
        }
    }

    // ========== 第二部分：触摸检测与按钮处理 ==========
    // 更新触摸数据（中断模式下会自动更新）
    touch.update();
    
    // 检查是否有触摸事件
    if (checkTouchOnButton()) {
        // 按钮被触摸，更新按钮状态
        if (!buttonPressed) {
            buttonPressed = true;
            // 立即更新按钮显示状态
            drawButtonOverlay(true);
            Serial.println("🟢 拍照按钮按下");
        }
    } else {
        // 按钮未被触摸
        if (buttonPressed) {
            buttonPressed = false;
            // 立即更新按钮显示状态
            drawButtonOverlay(false);
            Serial.println("🔴 拍照按钮释放");
            
            // 触发拍照（仅在按钮释放时触发）
            if (!captureTriggered) {
                captureTriggered = true;
            }
        }
    }
    
    // ========== 第三部分：拍照处理 ==========
    // 检查是否触发拍照
    if (captureTriggered && !isCapturing) {
        captureTriggered = false;
        captureAndSavePhoto();
    }
    
    // 短暂延迟，控制循环频率
    delay(10);
}

// --- 新增：检测按钮触摸函数 ---
bool checkTouchOnButton() {
    // 获取触摸点数量
    uint8_t touchCount = touch.getTouchCount();
    
    if (touchCount == 0) {
        return false;
    }
    
    // 检查所有触摸点是否在按钮区域内
    for (uint8_t i = 0; i < touchCount; i++) {
        uint16_t touchX, touchY;
        if (touch.getTouchPoint(i, &touchX, &touchY)) {
            // 将触摸坐标转换为屏幕坐标（FT6336U通常返回0-2047，需要缩放到屏幕分辨率）
            uint16_t screenX = map(touchX, 0, 2047, 0, ST7789_TFTWIDTH);
            uint16_t screenY = map(touchY, 0, 2047, 0, ST7789_TFTHEIGHT);
            
            // 检查是否在按钮区域内
            if (screenX >= BUTTON_X && screenX <= BUTTON_X + BUTTON_WIDTH &&
                screenY >= BUTTON_Y && screenY <= BUTTON_Y + BUTTON_HEIGHT) {
                return true;
            }
        }
    }
    
    return false;
}

// --- 核心拍照保存函数（修改版）---
void captureAndSavePhoto() {
    if (isCapturing) {
        Serial.println("警告：上一次拍照尚未结束，跳过。");
        return;
    }
    isCapturing = true; // 上锁
    
    // 在屏幕上显示拍照提示（临时覆盖部分预览）
    tft.fillRectangle(0, 0, 200, 20, ST7789_BLACK);
    tft.setCursor(10, 10);
    tft.setForeground(ST7789_WHITE);
    tft.print("Capturing...");
    tft.setForeground(ST7789_WHITE); // 恢复默认前景色
    
    Serial.println(">>> 开始手动拍照流程");
    
    // 步骤1：临时开启高分辨率拍照通道
    Camera.channelBegin(STILL_CH);
    Serial.println("  1. 拍照通道已开启");
    delay(120); // 重要：给传感器一点时间稳定到高分辨率模式

    // 步骤2：捕获单帧720p图像
    Camera.getImage(STILL_CH, &img_addr_still, &img_len_still);
    if (img_addr_still > 0 && img_len_still > 0) {
        Serial.print("  2. 图像捕获成功，大小: ");
        Serial.print(img_len_still);
        Serial.println(" 字节");
        
        // 步骤3：保存到SD卡
        if (saveToSDCard(img_addr_still, img_len_still)) {
            photoCount++;
            Serial.print("  3. 照片保存成功，总计: ");
            Serial.print(photoCount);
            Serial.println(" 张");
            
            // 在屏幕上显示保存成功提示
            tft.fillRectangle(0, 0, 200, 20, ST7789_BLACK);
            tft.setCursor(10, 10);
            tft.setForeground(ST7789_GREEN);
            tft.print("Saved!");
            tft.setForeground(ST7789_WHITE);
            delay(500);
        } else {
            // 保存失败提示
            tft.fillRectangle(0, 0, 200, 20, ST7789_BLACK);
            tft.setCursor(10, 10);
            tft.setForeground(ST7789_RED);
            tft.print("Save Failed");
            tft.setForeground(ST7789_WHITE);
            delay(500);
        }
    } else {
        Serial.println("  2. 错误：未能捕获图像数据");
        tft.fillRectangle(0, 0, 200, 20, ST7789_BLACK);
        tft.setCursor(10, 10);
        tft.setForeground(ST7789_RED);
        tft.print("Capture Failed");
        tft.setForeground(ST7789_WHITE);
        delay(500);
    }
    
    // 步骤4：立即关闭拍照通道，释放资源
    Camera.channelEnd(STILL_CH);
    Serial.println("  4. 拍照通道已关闭");
    
    // 清除拍照提示
    tft.fillRectangle(0, 0, 200, 20, ST7789_BLACK);
    
    isCapturing = false; // 解锁
    Serial.println("<<< 拍照流程结束");
    Serial.println();
}

// --- SD卡保存函数（保持不变）---
bool saveToSDCard(uint32_t img_addr, uint32_t img_len) {
    // 每次操作前重新初始化文件系统，确保稳定性
    if (!fs.begin()) {
        Serial.println("  保存失败：文件系统初始化错误");
        return false;
    }

    // 生成带时间戳的唯一文件名
    char filename[50];
    sprintf(filename, "%sIMG_%lu.jpg", fs.getRootPath(), millis());
    
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