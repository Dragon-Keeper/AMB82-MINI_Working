/*
 * Camera.ino - AMB82-MINI相机控制系统
 * 使用EC11旋转编码器控制三角形在A-F位置间移动
 * 集成实时预览和拍照功能
 * 使用16x16点阵字库显示提示文字

 * FreeRTOS多任务架构：6个独立任务对应菜单A-F位置
 */

#include <Arduino.h>
#include "AmebaST7789_DMA_SPI1.h"
#include "MenuManager.h"
#include "VideoStream.h"
#include <JPEGDEC_Libraries/JPEGDEC.h>
#include "AmebaFatFS.h"

// FreeRTOS头文件
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <event_groups.h>
#include <semphr.h>

// 包含菜单页面数据
#include "MM.h"
#include "SM.h"
#include "ImageConfig.h"

// 包含16x16点阵字库
#include "font16x16.h"

// 预定义字符串常量（基于main.ino实现）
// 每个字符串由字符索引数组组成，以0作为终止符，便于drawChineseString函数遍历绘制
const uint8_t STR_REAL_TIME_PREVIEW[] = {FONT16_IDX_SHI6, FONT16_IDX_SHI5, FONT16_IDX_YU2, FONT16_IDX_LAN2, 0};
const uint8_t STR_PHOTO_SUCCESS[] = {FONT16_IDX_PAI2, FONT16_IDX_ZHAO2, FONT16_IDX_CHENG, FONT16_IDX_GONG, 0};  // 拍照成功
const uint8_t STR_SAVE_FAILED_CN[] = {FONT16_IDX_BAO, FONT16_IDX_CUN, FONT16_IDX_SHI4, FONT16_IDX_BAI, 0};      // 保存失败
const uint8_t STR_CAPTURE_FAILED_CN[] = {FONT16_IDX_PAI2, FONT16_IDX_ZHAO2, FONT16_IDX_SHI4, FONT16_IDX_BAI, 0}; // 拍照失败

// 定义TFT屏幕引脚
#define TFT_CS   SPI1_SS
#define TFT_DC   4
#define TFT_RST  5
#define TFT_BL   6

// 定义EC11旋转编码器引脚
#define ENCODER_CLK 7
#define ENCODER_DT  8  
#define ENCODER_SW  9

// 相机通道与配置定义
#define PREVIEW_CH  0  // 预览通道：VGA
#define STILL_CH    1  // 拍照通道：720p
VideoSetting configPreview(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);
VideoSetting configStill(VIDEO_HD, CAM_FPS, VIDEO_JPEG, 1);

// 全局对象
AmebaST7789_DMA_SPI1 tft(TFT_CS, TFT_DC, TFT_RST);

// 函数前向声明
void clearTriangle(int position);
void switchToMainMenu();

// TaskMessage_t结构体定义
typedef struct {
    uint32_t taskId;
    uint32_t command;
    void* data;
} TaskMessage_t;

// FreeRTOS任务管理器函数声明
void initTaskManager(void);
void createTask(uint32_t taskId);
void deleteTask(uint32_t taskId);
void blockTask(uint32_t taskId);
void unblockTask(uint32_t taskId);
void cleanupTaskResources(uint32_t taskId);
void sendTaskMessage(uint32_t taskId, uint32_t command, void* data);
void handleTaskMessage(TaskMessage_t* message);
void cleanupGlobalState(void);
void safeInterruptDisable(void);
void safeInterruptEnable(void);

// 任务函数声明
void taskCameraPreview(void* pvParameters);
void taskFunctionB(void* pvParameters);
void taskFunctionC(void* pvParameters);
void taskFunctionD(void* pvParameters);
void taskFunctionE(void* pvParameters);
void taskSystemSettings(void* pvParameters);

MenuManager menuManager(tft, ENCODER_CLK, ENCODER_DT, ENCODER_SW);
JPEGDEC jpeg;
AmebaFatFS fs;

// ===============================================
// 全局变量
// ===============================================

// 图像缓冲区
uint32_t img_addr_preview = 0, img_len_preview = 0;
uint32_t img_addr_still = 0, img_len_still = 0;

// 相机控制变量
bool isCapturing = false;        // 拍照状态锁
bool cameraModeActive = false;   // 相机模式激活标志
bool manualCaptureRequested = false; // 手动拍照请求标志

// 旋转编码器变量
volatile int encoderCount = 0;
volatile bool rotationDetected = false;
volatile int rotationDirection = 0;

// 三角形位置变量
int currentTrianglePosition = 0;

// 三角形位置定义
enum TrianglePosition {
    POS_A = 0,
    POS_B = 1,
    POS_C = 2,
    POS_D = 3,
    POS_E = 4,
    POS_F = 5
};

// ===============================================
// FreeRTOS任务管理器架构
// ===============================================

// 任务句柄定义
TaskHandle_t taskHandles[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

// 任务状态枚举
typedef enum {
    TASK_STATE_INACTIVE = 0,
    TASK_STATE_RUNNING = 1,
    TASK_STATE_BLOCKED = 2,
    TASK_STATE_DELETED = 3
} TaskState_t;

// 任务管理器结构体
typedef struct {
    TaskHandle_t handle;
    TaskState_t state;
    uint32_t taskId;
    uint32_t stackSize;
    UBaseType_t priority;
    void (*taskFunction)(void*);
} TaskManager_t;

// 任务管理器实例
TaskManager_t taskManager[6];

// 全局事件组用于任务间通信
EventGroupHandle_t xEventGroup;
#define EVENT_TASK_A_REQUESTED    (1 << 0)
#define EVENT_TASK_B_REQUESTED    (1 << 1)
#define EVENT_TASK_C_REQUESTED    (1 << 2)
#define EVENT_TASK_D_REQUESTED    (1 << 3)
#define EVENT_TASK_E_REQUESTED    (1 << 4)
#define EVENT_TASK_F_REQUESTED    (1 << 5)
#define EVENT_RETURN_TO_MENU      (1 << 6)
#define EVENT_ALL_TASKS_CLEAR     (0x7F) // 清除所有任务事件（包括RETURN_TO_MENU）

// 任务消息队列
QueueHandle_t xTaskMessageQueue;

// 互斥锁保护全局资源
SemaphoreHandle_t xGlobalMutex;
SemaphoreHandle_t xCameraMutex;
SemaphoreHandle_t xDisplayMutex;

// 系统状态枚举（扩展相机模式）
enum SystemState {
    STATE_MAIN_MENU,
    STATE_SUB_MENU,
    STATE_CAMERA_PREVIEW  // 新增：相机预览模式
};

SystemState currentState = STATE_MAIN_MENU;

// 按钮处理相关变量
bool lastButtonState = HIGH;
unsigned long lastButtonTime = 0;
const unsigned long BUTTON_DEBOUNCE_DELAY = 50;
bool buttonPressDetected = false;

// SD卡状态
bool sdcardInitialized = false;

// ===============================================
// 16x16字体显示函数（基于main.ino实现）
// ===============================================

// 缓冲区用于存放转换后的RGB565数据
// 16x16字符占用256个像素，每个像素用16位颜色值表示（RGB565格式）
// 字符数据在绘制前会进行顺时针90度旋转，以适应屏幕方向
uint16_t fontBuffer[16 * 16];

/**
 * 绘制16x16中文字符（基于main.ino实现）
 * 该函数绘制单个字符到屏幕指定位置，支持颜色和背景色设置
 * 使用字库索引从font16x16.h中获取字符数据
 * 字库数据以16x16点阵格式存储，每个字符占用32字节（16行×2字节/行）
 * 每行用两个字节表示，高位字节在前，低位字节在后
 * 字符顺时针旋转90度后显示，以适应屏幕方向
 * @param x 字符绘制的X坐标
 * @param y 字符绘制的Y坐标
 * @param charIndex 字符在字库中的索引
 * @param color 字符颜色（RGB565格式）
 * @param bgColor 背景颜色（RGB565格式）
 */
void drawChineseChar(int16_t x, int16_t y, uint8_t charIndex, uint16_t color, uint16_t bgColor) {
    // 检查字库索引是否有效，防止数组越界访问
    if (charIndex >= sizeof(font16x16)/sizeof(font16x16[0])) {
        return;
    }
    
    // 获取字库数据，font16x16数组中存储了预定义的字符点阵数据
    const uint8_t* fontData = font16x16[charIndex];
    
    // 实现方式：将原数据按顺时针90度旋转后填入缓冲区
    for (int row = 0; row < 16; row++) {
        // 获取字库数据中的高字节和低字节
        uint8_t highByte = fontData[row * 2];      // 高8位，表示每行的前8个像素
        uint8_t lowByte = fontData[row * 2 + 1];   // 低8位，表示每行的后8个像素
        // 合并为16位数据，表示该行16个像素的点阵信息
        uint16_t rowBits = (highByte << 8) | lowByte;
            
        for (int col = 0; col < 16; col++) {
            // 检查对应位是否为1（从高位到低位），判断该像素是否为字符像素
            // (1 << (15 - col)) 生成第(15-col)位为1的掩码，与rowBits进行与运算
            // 必须使用下面的"col * 16 + row;"才能正常方向显示文字
            int bufferIndex = col * 16 + row;
            if (rowBits & (1 << (15 - col))) {
                fontBuffer[bufferIndex] = color;  // 字符像素：使用前景色
            } else {
                fontBuffer[bufferIndex] = bgColor;  // 背景像素：使用背景色
            }
        }
    }
    
    // 将缓冲区数据绘制到屏幕指定位置
    // 使用tft.drawBitmap函数将预处理好的字符像素数据一次性绘制到屏幕
    tft.drawBitmap(x, y, 16, 16, fontBuffer);
}

/**
 * 绘制中文字符串（基于main.ino实现）
 * 该函数绘制多个字符组成的字符串到屏幕指定位置
 * 按照字符间距16像素依次绘制每个字符，形成完整的字符串显示
 * @param x 字符串绘制的起始X坐标
 * @param y 字符串绘制的起始Y坐标
 * @param charIndices 字符索引数组，以0作为终止符
 * @param color 字符颜色（RGB565格式）
 * @param bgColor 背景颜色（RGB565格式）
 */
void drawChineseString(int16_t x, int16_t y, const uint8_t* charIndices, 
                      uint16_t color, uint16_t bgColor) {
    int i = 0;
    // 遍历字符索引数组直到遇到终止符（索引值为0）
    while (charIndices[i] != 0) {
        // 绘制单个字符，每个字符之间间隔16像素（与字符宽度一致）
        // x + i * 16 计算第i个字符的X坐标位置
        drawChineseChar(x + i * 16, y, charIndices[i], color, bgColor);
        i++;
    }
}

/**
 * 显示拍照状态提示（基于main.ino实现）
 * 该函数显示状态提示信息到屏幕指定位置
 * 封装了字符串绘制功能，使用固定的白色前景色和黑色背景色
 * @param message 提示消息的字符索引数组
 * @param x 提示信息绘制的X坐标
 * @param y 提示信息绘制的Y坐标
 */
void showCameraStatus(const uint8_t* message, int x, int y) {
    // 绘制状态提示信息，使用白色字符和黑色背景的标准配色
    drawChineseString(x, y, message, ST7789_WHITE, ST7789_BLACK);
}

/**
 * 在指定位置显示英文字符串（已移除）
 * 原功能：在指定位置显示英文字符串
 * @param x 起始X坐标
 * @param y 起始Y坐标
 * @param text 要显示的文本
 * @param color 颜色
 * 功能已移除 to implement new approach based on main.ino
 */

/**
 * 显示拍照状态提示（已移除）
 * 原功能：显示拍照状态提示
 * @param message 提示消息
 * @param x 起始X坐标
 * @param y 起始Y坐标
 * 功能已移除 to implement new approach based on main.ino
 */

// ===============================================
// 旋转编码器中断处理函数
// ===============================================

void encoderRotation_handler(uint32_t id, uint32_t event) {
    if (id != ENCODER_CLK) {
        return;
    }
    
    // 硬件消抖：检查时间间隔
    static unsigned long lastInterruptTime = 0;
    unsigned long currentTime = micros();
    if (currentTime - lastInterruptTime < 100000) {
        return;
    }
    lastInterruptTime = currentTime;
    
    // 如果在拍照中，忽略旋转事件
    if (isCapturing) {
        return;
    }

    // 读取当前CLK和DT引脚状态
    int clkState = digitalRead(ENCODER_CLK);
    int dtState = digitalRead(ENCODER_DT);
    
    // 确认是下降沿（CLK为低电平）
    if (clkState == LOW) {
        // 检测旋转方向（在下降沿时检测DT引脚状态）
        if (dtState == HIGH) {
            // 顺时针旋转：向下移动三角形
            rotationDirection = 1;
            rotationDetected = true;
        } else if (dtState == LOW) {
            // 逆时针旋转：向上移动三角形
            rotationDirection = -1;
            rotationDetected = true;
        }
    }
}

// ===============================================
// 三角形位置精确调整系统
// ===============================================

void adjustTrianglePosition(int position, int xOffset = 40) {
    if (position < 0 || position > 5) {
        return;
    }
    
    // 清除当前位置三角形
    clearTriangle(currentTrianglePosition);
    
    // 更新当前位置
    currentTrianglePosition = position;
    
    // 定义固定Y轴坐标值（像素）
    int fixedYPositions[6] = {60, 90, 120, 150, 180, 210};
    
    // 使用固定数值定位方式
    int centerY = fixedYPositions[position];
    int triangleSizeValue = 7;  // 调整三角形尺寸为原来的70%（从10改为7）
    
    // 更新三角形顶点坐标（基于新的xOffset）
    int x1 = xOffset + triangleSizeValue * 2;  // 右侧顶点
    int y1 = centerY;
    int x2 = xOffset;                          // 左侧顶点1
    int y2 = centerY - triangleSizeValue;
    int x3 = xOffset;                          // 左侧顶点2
    int y3 = centerY + triangleSizeValue;
    
    // 绘制新位置的三角形（白色）
    tft.drawLine(x1, y1, x2, y2, ST7789_WHITE);
    tft.drawLine(x1, y1, x3, y3, ST7789_WHITE);
    tft.drawLine(x2, y2, x3, y3, ST7789_WHITE);
    
    Serial.print("三角形移动到位置: ");
    Serial.println((char)('A' + position));
}

void clearTriangle(int position) {
    if (position < 0 || position > 5) {
        return;
    }
    
    // 定义固定Y轴坐标值（像素）
    int fixedYPositions[6] = {60, 90, 120, 150, 180, 210};
    
    // 使用固定数值定位方式
    int centerY = fixedYPositions[position];
    int triangleSizeValue = 7;  // 调整三角形尺寸为原来的70%（从10改为7）
    
    // 三角形顶点坐标（与drawTriangle一致）
    int x1 = 40 + triangleSizeValue * 2;  // 右侧顶点（向左移动40像素）
    int y1 = centerY;
    
    int x2 = 40;                          // 左侧顶点1（向左移动40像素）
    int y2 = centerY - triangleSizeValue;
    
    int x3 = 40;                          // 左侧顶点2（向左移动40像素）
    int y3 = centerY + triangleSizeValue;
    
    // 使用黑色线条重新绘制三角形边框来清除（覆盖原线条）
    tft.drawLine(x1, y1, x2, y2, ST7789_BLACK);  // 清除右侧到左上
    tft.drawLine(x1, y1, x3, y3, ST7789_BLACK);  // 清除右侧到左下
    tft.drawLine(x2, y2, x3, y3, ST7789_BLACK);  // 清除左上到左下
    
    // 额外清除三角形内部可能残留的像素（使用矩形清除）
    int x = 40;  // 从屏幕左侧开始，向左移动40像素
    int y = centerY - triangleSizeValue;
    int width = triangleSizeValue * 3;
    int height = triangleSizeValue * 2;
    tft.fillRectangle(x, y, width, height, ST7789_BLACK);
}

void updateTriangleDisplay() {
    // 清除所有可能的位置
    for (int i = 0; i < 6; i++) {
        clearTriangle(i);
    }
    
    // 绘制当前位置的三角形
    adjustTrianglePosition(currentTrianglePosition, 40);
}

// 移动三角形到指定位置
void moveTriangleToPosition(int newPosition) {
    if (newPosition >= 0 && newPosition <= 5) {
        // 使用主控制函数进行精确移动
        adjustTrianglePosition(newPosition, 40);
        // 更新当前三角形位置
        currentTrianglePosition = newPosition;
    }
}

// 向上移动三角形
void moveTriangleUp() {
    int newPosition = (currentTrianglePosition - 1 + 6) % 6;
    Serial.print("三角形从位置 ");
    Serial.print((char)('A' + currentTrianglePosition));
    Serial.print(" 向上移动到位置: ");
    Serial.println((char)('A' + newPosition));
    moveTriangleToPosition(newPosition);
}

// 向下移动三角形
void moveTriangleDown() {
    int newPosition = (currentTrianglePosition + 1) % 6;
    Serial.print("三角形从位置 ");
    Serial.print((char)('A' + currentTrianglePosition));
    Serial.print(" 向下移动到位置: ");
    Serial.println((char)('A' + newPosition));
    moveTriangleToPosition(newPosition);
}

// ===============================================
// 相机功能模块
// ===============================================

/**
 * JPEG解码回调函数
 */
int JPEGDraw(JPEGDRAW *pDraw) {
    tft.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    return 1;
}

/**
 * 初始化SD卡（单次初始化版）
 */
bool initSDCard() {
    if (sdcardInitialized) {
        return true;
    }
    
    Serial.print("初始化SD卡...");
    if (fs.begin()) {
        sdcardInitialized = true;
        Serial.println("成功");
        // 获取并显示根路径
        const char* rootPath = fs.getRootPath();
        Serial.print("SD卡根路径: ");
        Serial.println(rootPath);
        // 注意：这里不调用 fs.end()，保持SD卡挂载状态
        return true;
    } else {
        Serial.println("失败");
        sdcardInitialized = false;
        return false;
    }
}

/**
 * 初始化相机系统
 */
bool initCameraSystem() {
    Serial.println("初始化相机系统...");
    
    // 配置相机通道
    Camera.configVideoChannel(PREVIEW_CH, configPreview);
    Camera.configVideoChannel(STILL_CH, configStill);
    
    // 初始化摄像头硬件
    Camera.videoInit();
    
    Serial.println("相机系统初始化完成");
    return true;
}

/**
 * SD卡保存函数 - 参照Camera_2_Lcd_JPEGDEC.ino项目
 */
bool savePhotoToSDCard(uint32_t img_addr, uint32_t img_len) {
    // 确保SD卡已初始化
    if (!initSDCard()) {
        Serial.println("保存失败：SD卡未初始化");
        return false;
    }

    // 生成带时间戳的唯一文件名（参照Camera_2_Lcd_JPEGDEC.ino）
    char filename[50];
    // 获取当前时间作为文件名
    unsigned long timestamp = millis();
    sprintf(filename, "%sIMG_%lu.jpg", fs.getRootPath(), timestamp);
    
    Serial.print("正在保存至: ");
    Serial.println(filename);

    // 打开文件进行写入
    File file = fs.open(filename);
    if (!file) {
        Serial.println("  保存失败：无法创建文件");
        return false;
    }

    // 写入JPEG数据
    int bytesWritten = file.write((uint8_t *)img_addr, img_len);
    file.close(); // 关闭文件

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

/**
 * 开始相机预览模式
 * 初始化相机预览界面，显示操作提示信息
 */
void startCameraPreview() {
    Serial.println(">>> 进入相机预览模式");
    cameraModeActive = true;
    currentState = STATE_CAMERA_PREVIEW;
    
    // 清除屏幕显示
    tft.fillScreen(ST7789_BLACK);
    
    // 显示相机模式提示（基于main.ino实现）
    // 使用预定义的字符串常量和新的文本渲染函数显示提示信息
    // 显示中文"实时预览"提示，居中显示（4个字符×16像素=64像素，屏幕宽度240像素，居中位置=(240-64)/2=88）
    drawChineseString(88, 150, STR_REAL_TIME_PREVIEW, ST7789_WHITE, ST7789_BLACK);
    delay(1000);
    
    // 确保相机通道已开启（先关闭再重新开启，确保状态正确）
    Camera.channelEnd(PREVIEW_CH);
    Camera.channelBegin(PREVIEW_CH);
    
    // 清除提示，开始实时预览
    tft.fillScreen(ST7789_BLACK);
    
    Serial.println("相机预览模式已启动");
}

/**
 * 退出相机预览模式
 */
void exitCameraPreview() {
    Serial.println("<<< 退出相机预览模式");
    cameraModeActive = false;
    currentState = STATE_MAIN_MENU;
    
    // 关闭相机通道
    Camera.channelEnd(PREVIEW_CH);
    Camera.channelEnd(STILL_CH);
    
    // 重新显示主菜单
    menuManager.switchToPage(0);
    updateTriangleDisplay();
    
    // 关键修复：清除所有可能残留的事件标志
    rotationDetected = false;
    buttonPressDetected = false;
    manualCaptureRequested = false;
    isCapturing = false;
    
    // 添加短暂延迟，确保状态稳定
    delay(50);

    Serial.println("已返回主菜单");
}

/**
 * 执行手动拍照并保存 - 参照Camera_2_Lcd_JPEGDEC.ino项目
 */
void captureAndSavePhoto() {
    if (isCapturing) {
        Serial.println("警告：上一次拍照尚未结束，跳过");
        return;
    }
    
    isCapturing = true; // 上锁
    Serial.println(">>> 开始手动拍照");
    
    // 移除"正在拍照..."提示
    
    // 临时开启高分辨率拍照通道
    Camera.channelBegin(STILL_CH);
    Serial.println("  1. 拍照通道已开启");
    delay(200); // 给传感器稳定时间（比原来更长一些）
    
    // 捕获单帧720p图像
    Camera.getImage(STILL_CH, &img_addr_still, &img_len_still);
    if (img_len_still > 0) {
        Serial.print("  2. 图像捕获成功，大小: ");
        Serial.print(img_len_still);
        Serial.println(" 字节");
        
        // 移除"正在保存..."提示
        
        // 保存到SD卡
        if (savePhotoToSDCard(img_addr_still, img_len_still)) {
            // 在屏幕上显示拍照成功提示（中文）
            showCameraStatus(STR_PHOTO_SUCCESS, 120, 150);
            delay(500);
            // 清除提示（基于main.ino实现）
            tft.fillRectangle(80, 150, 160, 16, ST7789_BLACK);
        } else {
            // 显示保存失败提示（中文）
            showCameraStatus(STR_SAVE_FAILED_CN, 80, 150);
            delay(1000);
            // 清除提示（基于main.ino实现）
            tft.fillRectangle(80, 150, 160, 16, ST7789_BLACK);
        }
    } else {
        Serial.println("  2. 错误：未能捕获图像数据");
        // 显示拍照失败提示（中文）
        showCameraStatus(STR_CAPTURE_FAILED_CN, 60, 150);
        delay(1000);
        // 清除提示（基于main.ino实现）
        tft.fillRectangle(60, 150, 200, 16, ST7789_BLACK);
    }
    
    // 关闭拍照通道
    Camera.channelEnd(STILL_CH);
    Serial.println("  3. 拍照通道已关闭");
    
    isCapturing = false; // 解锁
    manualCaptureRequested = false; // 清除请求标志
    Serial.println("<<< 拍照完成");
}

/**
 * 相机预览循环处理 - 优化帧率版本
 */
void handleCameraPreview() {
    // 实时预览：获取并显示预览图像（无帧率限制，连续处理）
    Camera.getImage(PREVIEW_CH, &img_addr_preview, &img_len_preview);
    if (img_len_preview > 0) {
        if (jpeg.openFLASH((uint8_t *)img_addr_preview, img_len_preview, JPEGDraw)) {
            jpeg.decode(0, 0, JPEG_SCALE_HALF);
            jpeg.close();
        }
    }
    
    // 检查是否有手动拍照请求
    if (manualCaptureRequested && !isCapturing) {
        Serial.println("检测到拍照请求，开始拍照...");
        captureAndSavePhoto();
    }
}

// ===============================================
// 系统设置子菜单处理
// ===============================================

/**
 * 处理系统设置子菜单
 */
void handleSubMenu() {
    // 处理旋转事件（在子菜单中移动三角形）- 添加严格的消抖处理
    if (rotationDetected) {
        static unsigned long lastValidRotation = 0;
        unsigned long currentTime = millis();
        
        // 严格的软件消抖：确保两次旋转事件之间有足够的时间间隔
        if (currentTime - lastValidRotation > 300) { // 300ms消抖（增加消抖时间）
            lastValidRotation = currentTime;
            rotationDetected = false; // 重置标志
            
            // 验证旋转方向并移动三角形
            if (rotationDirection == 1) {
                moveTriangleDown();
                Serial.println("子菜单：编码器旋转: 顺时针（向下移动三角形）");
                encoderCount++;
            } else if (rotationDirection == -1) {
                moveTriangleUp();
                Serial.println("子菜单：编码器旋转: 逆时针（向上移动三角形）");
                encoderCount++;
            } else {
                Serial.println("子菜单警告：未知的旋转方向");
            }
            
            // 显示当前三角形位置
            Serial.print("子菜单三角形当前位置: ");
            Serial.println((char)('A' + currentTrianglePosition));
            Serial.print("子菜单编码器计数: ");
            Serial.println(encoderCount);
        } else {
            // 忽略过快的连续旋转（可能是抖动）
            rotationDetected = false;
            Serial.println("子菜单忽略快速旋转事件（严格消抖处理）");
        }
    }
    
    // 处理按钮事件（只在F位置返回主菜单）
    if (buttonPressDetected) {
        buttonPressDetected = false;
        
        if (currentTrianglePosition == POS_F) {
            // F位置按下：返回主菜单
            Serial.println("子菜单F位置：返回主菜单");
            xEventGroupSetBits(xEventGroup, EVENT_RETURN_TO_MENU);
        } else {
            // 其他位置按下：无操作，仅显示位置信息
            Serial.print("子菜单非F位置按下：位置");
            Serial.println((char)('A' + currentTrianglePosition));
        }
    }
}

// ===============================================
// 事件处理函数
// ===============================================

/**
 * 处理按钮按下事件
 */
void handleButtonPress() {
    buttonPressDetected = true;
}

/**
 * 处理编码器旋转事件
 */
void handleEncoderRotation() {
    // 空函数，实际由中断处理
}

/**
 * 检测和处理按钮事件 - 重新设计按钮处理逻辑
 */
void checkButton() {
    bool currentButtonState = digitalRead(ENCODER_SW);
    unsigned long currentTime = millis();
    
    // 检测按钮按下（下降沿）
    if (currentButtonState == LOW && lastButtonState == HIGH) {
        // 消抖处理
        if (currentTime - lastButtonTime >= BUTTON_DEBOUNCE_DELAY) {
            lastButtonTime = currentTime;
            
            // 根据当前状态处理按钮事件
            switch (currentState) {
                case STATE_MAIN_MENU:
                    // 主菜单模式：处理任务创建请求
                    handleButtonPress();
                    Serial.println("主菜单：按钮按下事件已触发");
                    break;
                    
                case STATE_SUB_MENU:
                    // 系统设置子菜单：处理子菜单按钮事件
                    handleButtonPress();
                    Serial.println("系统设置子菜单：按钮按下事件已触发");
                    break;
                    
                case STATE_CAMERA_PREVIEW:
                    // 相机预览模式：设置拍照请求
                    manualCaptureRequested = true;
                    Serial.println("相机预览模式：设置拍照请求标志");
                    break;
                    
                default:
                    Serial.println("未知状态：按钮按下事件已触发");
                    break;
            }
        }
    }
    
    lastButtonState = currentButtonState;
}

// ===============================================
// 菜单切换函数
// ===============================================

// 切换到系统设置子菜单
void switchToSubMenu() {
    Serial.println("切换到系统设置子菜单");
    currentState = STATE_SUB_MENU;
    
    // 切换到子菜单页面（索引1）
    menuManager.switchToPage(1);
    
    // 重置三角形位置到A
    currentTrianglePosition = POS_A;
    updateTriangleDisplay();
    
    Serial.println("已切换到系统设置子菜单");
}

// 切换回主菜单
void switchToMainMenu() {
    Serial.println("切换回主菜单");
    currentState = STATE_MAIN_MENU;
    
    // 切换回主菜单页面（索引0）
    menuManager.switchToPage(0);
    
    // 重置三角形位置到A
    currentTrianglePosition = POS_A;
    updateTriangleDisplay();
    
    Serial.println("已切换回主菜单");
}

// ===============================================
// 主程序
// ===============================================

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== AMB82-MINI相机控制系统启动 ===");
    Serial.println("使用16x16点阵字库显示提示文字");
    
    // 初始化TFT背光
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    // 初始化TFT屏幕
    Serial.println("初始化TFT屏幕...");
    SPI1.setDataMode(TFT_CS, SPI_MODE0, SPI_MODE_MASTER);
    SPI1.setDefaultFrequency(25000000);
    SPI1.begin();
    
    // 初始化TFT显示屏
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(ST7789_BLACK);
    
    // 初始化菜单管理器
    Serial.println("初始化菜单管理器...");
    if (!menuManager.init()) {
        Serial.println("菜单管理器初始化失败!");
        while(1);
    }
    
    // 配置旋转编码器引脚
    Serial.println("配置旋转编码器引脚...");
    pinMode(ENCODER_CLK, INPUT_IRQ_FALL);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    digitalSetIrqHandler(ENCODER_CLK, encoderRotation_handler);
    
    Serial.println("编码器引脚配置完成");
    
    // 添加菜单页面
    Serial.println("添加菜单页面...");
    menuManager.addMenuPage(MM_IMAGE_DATA, MM_IMAGE_SIZE, MM_IMAGE_WIDTH, MM_IMAGE_HEIGHT);
    menuManager.addMenuPage(SM_IMAGE_DATA, SM_IMAGE_SIZE, SM_IMAGE_WIDTH, SM_IMAGE_HEIGHT);
    
    Serial.print("成功添加 ");
    Serial.print(menuManager.getTotalPages());
    Serial.println(" 个菜单页面");
    
    // 显示初始菜单页面
    Serial.println("显示初始菜单页面...");
    menuManager.showCurrentMenu();
    
    // 绘制初始三角形
    updateTriangleDisplay();
    
    // 初始化相机系统（但不启动预览）
    initCameraSystem();
    
    // 初始化SD卡
    initSDCard();
    
    // 初始化FreeRTOS任务管理器
    initTaskManager();
    
    Serial.println("系统初始化完成，等待用户交互...");
    Serial.println("===================================");
}

void loop() {
    // 检测按钮状态
    checkButton();
    
    // 处理任务消息队列
    TaskMessage_t message;
    if (xQueueReceive(xTaskMessageQueue, &message, 0) == pdTRUE) {
        handleTaskMessage(&message);
    }
    
    // 根据当前状态执行不同的处理逻辑
    switch (currentState) {
        case STATE_CAMERA_PREVIEW:
            // 相机预览模式 - 现在由任务处理
            // 旋转事件处理已移至任务内部，这里不再处理
            break;
            
        case STATE_SUB_MENU:
            // 系统设置子菜单 - 现在由任务处理
            break;
            
        case STATE_MAIN_MENU:
        default:
            // 主菜单模式 - 处理任务创建请求
            // 处理旋转事件（移动三角形）- 添加严格的消抖处理
            if (rotationDetected) {
                static unsigned long lastValidRotation = 0;
                unsigned long currentTime = millis();
                
                // 严格的软件消抖：确保两次旋转事件之间有足够的时间间隔
                if (currentTime - lastValidRotation > 300) { // 300ms消抖（增加消抖时间）
                    lastValidRotation = currentTime;
                    rotationDetected = false; // 重置标志
                    
                    // 验证旋转方向并移动三角形
                    if (rotationDirection == 1) {
                        moveTriangleDown();
                        Serial.println("编码器旋转: 顺时针（向下移动三角形）");
                        encoderCount++;
                    } else if (rotationDirection == -1) {
                        moveTriangleUp();
                        Serial.println("编码器旋转: 逆时针（向上移动三角形）");
                        encoderCount++;
                    } else {
                        Serial.println("警告：未知的旋转方向");
                    }
                    
                    // 显示当前三角形位置
                    Serial.print("三角形当前位置: ");
                    Serial.println((char)('A' + currentTrianglePosition));
                    Serial.print("编码器计数: ");
                    Serial.println(encoderCount);
                } else {
                    // 忽略过快的连续旋转（可能是抖动）
                    rotationDetected = false;
                    Serial.println("忽略快速旋转事件（严格消抖处理）");
                }
            }
            
            // 处理按钮事件（创建对应任务）
            if (buttonPressDetected) {
                buttonPressDetected = false;
                
                if (currentTrianglePosition == POS_A) {
                    // A位置按下：创建相机预览任务
                    Serial.println("主菜单A位置：创建相机预览任务");
                    createTask(POS_A);
                    currentState = STATE_CAMERA_PREVIEW;
                } else if (currentTrianglePosition == POS_F) {
                    // F位置按下：创建系统设置任务
                    Serial.println("主菜单F位置：创建系统设置任务");
                    createTask(POS_F);
                    currentState = STATE_SUB_MENU;
                } else {
                    // 其他位置：无操作，仅显示位置信息
                    Serial.print("主菜单位置 ");
                    Serial.print((char)('A' + currentTrianglePosition));
                    Serial.println("：无操作，仅显示位置信息");
                }
            }
            break;
    }
    
    // 减少延迟以提高响应速度
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

// ===============================================
// FreeRTOS任务管理器实现
// ===============================================

/**
 * 初始化任务管理器
 * 创建事件组、消息队列和互斥锁
 */
void initTaskManager(void) {
    Serial.println("初始化FreeRTOS任务管理器...");
    
    // 创建全局事件组
    xEventGroup = xEventGroupCreate();
    if (xEventGroup == NULL) {
        Serial.println("错误：无法创建事件组");
        return;
    }
    
    // 创建任务消息队列
    xTaskMessageQueue = xQueueCreate(10, sizeof(TaskMessage_t));
    if (xTaskMessageQueue == NULL) {
        Serial.println("错误：无法创建消息队列");
        return;
    }
    
    // 创建互斥锁
    xGlobalMutex = xSemaphoreCreateMutex();
    xCameraMutex = xSemaphoreCreateMutex();
    xDisplayMutex = xSemaphoreCreateMutex();
    
    if (xGlobalMutex == NULL || xCameraMutex == NULL || xDisplayMutex == NULL) {
        Serial.println("错误：无法创建互斥锁");
        return;
    }
    
    // 初始化任务管理器数组
    for (int i = 0; i < 6; i++) {
        taskManager[i].handle = NULL;
        taskManager[i].state = TASK_STATE_INACTIVE;
        taskManager[i].taskId = i;
        taskManager[i].stackSize = 2048; // 2KB堆栈
        taskManager[i].priority = tskIDLE_PRIORITY + 1;
        
        // 设置任务函数指针
        switch (i) {
            case 0: taskManager[i].taskFunction = taskCameraPreview; break;
            case 1: taskManager[i].taskFunction = taskFunctionB; break;
            case 2: taskManager[i].taskFunction = taskFunctionC; break;
            case 3: taskManager[i].taskFunction = taskFunctionD; break;
            case 4: taskManager[i].taskFunction = taskFunctionE; break;
            case 5: taskManager[i].taskFunction = taskSystemSettings; break;
        }
    }
    
    Serial.println("FreeRTOS任务管理器初始化完成");
}

/**
 * 创建指定任务
 * @param taskId 任务ID (0-5对应A-F位置)
 */
void createTask(uint32_t taskId) {
    if (taskId >= 6) {
        Serial.println("错误：无效的任务ID");
        return;
    }
    
    // 检查任务是否已存在且正在运行
    if (taskManager[taskId].state == TASK_STATE_RUNNING || 
        taskManager[taskId].state == TASK_STATE_BLOCKED) {
        Serial.print("任务 ");
        Serial.print((char)('A' + taskId));
        Serial.println(" 已存在，跳过创建");
        return;
    }
    
    // 如果任务状态是已删除，允许重新创建
    if (taskManager[taskId].state == TASK_STATE_DELETED) {
        Serial.print("任务 ");
        Serial.print((char)('A' + taskId));
        Serial.println(" 已删除，重新创建");
        // 关键修复：确保任务句柄为NULL，防止残留状态影响
        taskManager[taskId].handle = NULL;
        // 重置状态为INACTIVE，允许重新创建
        taskManager[taskId].state = TASK_STATE_INACTIVE;
    }
    
    // 清理之前任务的资源
    cleanupTaskResources(taskId);
    
    // 创建任务
    BaseType_t xReturn = xTaskCreate(
        taskManager[taskId].taskFunction,       // 任务函数
        getTaskName(taskId),                    // 任务名称
        taskManager[taskId].stackSize,          // 堆栈大小
        (void*)taskId,                          // 参数
        taskManager[taskId].priority,           // 优先级
        &taskManager[taskId].handle             // 任务句柄
    );
    
    if (xReturn == pdPASS) {
        taskManager[taskId].state = TASK_STATE_RUNNING;
        Serial.print("任务 ");
        Serial.print((char)('A' + taskId));
        Serial.println(" 创建成功");
    } else {
        Serial.print("错误：无法创建任务 ");
        Serial.println((char)('A' + taskId));
    }
}

/**
 * 删除指定任务
 * @param taskId 任务ID (0-5对应A-F位置)
 */
void deleteTask(uint32_t taskId) {
    if (taskId >= 6) {
        Serial.println("错误：无效的任务ID");
        return;
    }
    
    if (taskManager[taskId].state == TASK_STATE_RUNNING || 
        taskManager[taskId].state == TASK_STATE_BLOCKED) {
        
        // 清理任务资源
        cleanupTaskResources(taskId);
        
        // 删除任务
        vTaskDelete(taskManager[taskId].handle);
        taskManager[taskId].state = TASK_STATE_DELETED;
        taskManager[taskId].handle = NULL;
        
        Serial.print("任务 ");
        Serial.print((char)('A' + taskId));
        Serial.println(" 已删除");
    }
}

/**
 * 阻塞指定任务
 * @param taskId 任务ID (0-5对应A-F位置)
 */
void blockTask(uint32_t taskId) {
    if (taskId >= 6 || taskManager[taskId].state != TASK_STATE_RUNNING) {
        return;
    }
    
    // 使用事件组阻塞任务
    (void)xEventGroupWaitBits(
        xEventGroup,
        EVENT_RETURN_TO_MENU,
        pdTRUE,
        pdFALSE,
        portMAX_DELAY
    );
    
    taskManager[taskId].state = TASK_STATE_BLOCKED;
    Serial.print("任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 已阻塞");
}

/**
 * 解除阻塞指定任务
 * @param taskId 任务ID (0-5对应A-F位置)
 */
void unblockTask(uint32_t taskId) {
    if (taskId >= 6 || taskManager[taskId].state != TASK_STATE_BLOCKED) {
        return;
    }
    
    // 设置事件标志唤醒任务
    xEventGroupSetBits(xEventGroup, EVENT_RETURN_TO_MENU);
    taskManager[taskId].state = TASK_STATE_RUNNING;
    Serial.print("任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 已唤醒");
}

/**
 * 清理任务资源
 * @param taskId 任务ID (0-5对应A-F位置)
 */
void cleanupTaskResources(uint32_t taskId) {
    Serial.print("清理任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 的资源...");
    
    // 根据任务ID执行特定的清理操作
    switch (taskId) {
        case 0: // 相机预览任务
            // 清理相机相关资源
            if (xSemaphoreTake(xCameraMutex, portMAX_DELAY) == pdTRUE) {
                Camera.channelEnd(PREVIEW_CH);
                Camera.channelEnd(STILL_CH);
                xSemaphoreGive(xCameraMutex);
            }
            break;
        
        case 5: // 系统设置任务
            // 清理系统设置相关资源
            break;
        
        default:
            // 其他任务的通用清理
            break;
    }
    
    // 清理全局状态
    cleanupGlobalState();
}

/**
 * 获取任务名称
 * @param taskId 任务ID
 * @return 任务名称字符串
 */
const char* getTaskName(uint32_t taskId) {
    static const char* taskNames[] = {
        "Task_CameraPreview",
        "Task_FunctionB",
        "Task_FunctionC", 
        "Task_FunctionD",
        "Task_FunctionE",
        "Task_SystemSettings"
    };
    
    if (taskId < 6) {
        return taskNames[taskId];
    }
    return "UnknownTask";
}

/**
 * 发送任务消息
 * @param taskId 目标任务ID
 * @param command 命令
 * @param data 数据指针
 */
void sendTaskMessage(uint32_t taskId, uint32_t command, void* data) {
    TaskMessage_t message;
    message.taskId = taskId;
    message.command = command;
    message.data = data;
    
    if (xQueueSend(xTaskMessageQueue, &message, portMAX_DELAY) != pdTRUE) {
        Serial.println("错误：无法发送任务消息");
    }
}

/**
 * 处理任务消息
 * @param message 消息指针
 */
void handleTaskMessage(TaskMessage_t* message) {
    switch (message->command) {
        case 1: // 退出任务命令
            deleteTask(message->taskId);
            break;
        case 2: // 阻塞任务命令
            blockTask(message->taskId);
            break;
        default:
            Serial.println("未知任务消息命令");
            break;
    }
}

/**
 * 清理全局状态
 */
void cleanupGlobalState(void) {
    Serial.println("清理全局状态...");
    
    // 重置所有全局标志
    isCapturing = false;
    cameraModeActive = false;
    manualCaptureRequested = false;
    rotationDetected = false;
    buttonPressDetected = false;
    
    // 重置编码器状态
    encoderCount = 0;
    rotationDirection = 0;
    
    // 清除事件组中的任务事件
    xEventGroupClearBits(xEventGroup, EVENT_ALL_TASKS_CLEAR);
    
    Serial.println("全局状态清理完成");
}

/**
 * 安全禁用中断
 */
void safeInterruptDisable(void) {
    // 获取互斥锁保护中断操作
    if (xSemaphoreTake(xGlobalMutex, portMAX_DELAY) == pdTRUE) {
        // 禁用相关中断
        digitalSetIrqHandler(ENCODER_CLK, NULL);
    }
}

/**
 * 安全启用中断
 */
void safeInterruptEnable(void) {
    // 重新启用中断
    digitalSetIrqHandler(ENCODER_CLK, encoderRotation_handler);
    
    // 释放互斥锁
    xSemaphoreGive(xGlobalMutex);
}

// ===============================================
// 任务函数实现
// ===============================================

/**
 * 相机预览任务 (位置A)
 */
void taskCameraPreview(void* pvParameters) {
    uint32_t taskId = (uint32_t)pvParameters;
    Serial.print("相机预览任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 启动");
    
    // 任务初始化
    startCameraPreview();
    
    // 任务主循环
    while (1) {
        // 处理相机预览
        handleCameraPreview();
        
        // 处理旋转事件（返回主菜单）- 添加严格的消抖处理
        if (rotationDetected) {
            static unsigned long lastValidRotation = 0;
            unsigned long currentTime = millis();
            
            // 严格的软件消抖：确保两次旋转事件之间有足够的时间间隔
            if (currentTime - lastValidRotation > 300) { // 300ms消抖（增加消抖时间）
                lastValidRotation = currentTime;
                rotationDetected = false; // 重置标志
                
                Serial.println("相机模式：检测到旋转事件，准备退出");
                
                // 防止在拍照过程中退出
                if (isCapturing) {
                    Serial.println("拍照中，忽略旋转事件");
                    continue;
                }
                
                Serial.println("相机模式：旋转返回主菜单");
                
                // 发送退出信号给当前任务
                xEventGroupSetBits(xEventGroup, EVENT_RETURN_TO_MENU);
                
                // 额外清除一次，确保无残留
                rotationDetected = false;
                rotationDirection = 0;
            } else {
                // 忽略过快的连续旋转（可能是抖动）
                rotationDetected = false;
                Serial.println("相机模式忽略快速旋转事件（严格消抖处理）");
            }
        }
        
        // 检查是否需要退出任务（使用等待事件，确保不会错过旋转事件）
        EventBits_t uxBits = xEventGroupWaitBits(
            xEventGroup,
            EVENT_RETURN_TO_MENU,
            pdTRUE,
            pdFALSE,
            10 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            Serial.print("相机预览任务 ");
            Serial.print((char)('A' + taskId));
            Serial.println(" 收到退出信号");
            break;
        }
        
        // 最小化任务延迟，提高帧率
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    
    // 任务清理
    exitCameraPreview();
    
    // 关键修复：首先清除退出事件标志，防止残留影响新任务
    xEventGroupClearBits(xEventGroup, EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    currentState = STATE_MAIN_MENU;
    
    // 清除所有事件标志，确保系统状态干净
    xEventGroupClearBits(xEventGroup, EVENT_ALL_TASKS_CLEAR);
    
    // 关键修复：更新任务管理器状态为已删除，允许重新创建
    // 必须在删除任务之前更新状态，确保状态同步
    taskManager[taskId].state = TASK_STATE_DELETED;
    taskManager[taskId].handle = NULL;
    
    Serial.print("相机预览任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 退出");
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块B任务 (位置B)
 */
void taskFunctionB(void* pvParameters) {
    uint32_t taskId = (uint32_t)pvParameters;
    Serial.print("功能模块B任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 启动");
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        EventBits_t uxBits = xEventGroupWaitBits(
            xEventGroup,
            EVENT_RETURN_TO_MENU,
            pdTRUE,
            pdFALSE,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 任务功能实现
        // TODO: 实现功能模块B的具体逻辑
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 关键修复：首先清除退出事件标志，防止残留影响新任务
    xEventGroupClearBits(xEventGroup, EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    currentState = STATE_MAIN_MENU;
    
    // 清除所有事件标志，确保系统状态干净
    xEventGroupClearBits(xEventGroup, EVENT_ALL_TASKS_CLEAR);
    
    // 关键修复：更新任务管理器状态为已删除，允许重新创建
    taskManager[taskId].state = TASK_STATE_DELETED;
    taskManager[taskId].handle = NULL;
    
    Serial.print("功能模块B任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 退出");
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块C任务 (位置C)
 */
void taskFunctionC(void* pvParameters) {
    uint32_t taskId = (uint32_t)pvParameters;
    Serial.print("功能模块C任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 启动");
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        EventBits_t uxBits = xEventGroupWaitBits(
            xEventGroup,
            EVENT_RETURN_TO_MENU,
            pdTRUE,
            pdFALSE,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 任务功能实现
        // TODO: 实现功能模块C的具体逻辑
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 关键修复：首先清除退出事件标志，防止残留影响新任务
    xEventGroupClearBits(xEventGroup, EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    currentState = STATE_MAIN_MENU;
    
    // 清除所有事件标志，确保系统状态干净
    xEventGroupClearBits(xEventGroup, EVENT_ALL_TASKS_CLEAR);
    
    // 关键修复：更新任务管理器状态为已删除，允许重新创建
    taskManager[taskId].state = TASK_STATE_DELETED;
    taskManager[taskId].handle = NULL;
    
    Serial.print("功能模块C任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 退出");
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块D任务 (位置D)
 */
void taskFunctionD(void* pvParameters) {
    uint32_t taskId = (uint32_t)pvParameters;
    Serial.print("功能模块D任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 启动");
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        EventBits_t uxBits = xEventGroupWaitBits(
            xEventGroup,
            EVENT_RETURN_TO_MENU,
            pdTRUE,
            pdFALSE,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 任务功能实现
        // TODO: 实现功能模块D的具体逻辑
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 关键修复：首先清除退出事件标志，防止残留影响新任务
    xEventGroupClearBits(xEventGroup, EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    currentState = STATE_MAIN_MENU;
    
    // 清除所有事件标志，确保系统状态干净
    xEventGroupClearBits(xEventGroup, EVENT_ALL_TASKS_CLEAR);
    
    // 关键修复：更新任务管理器状态为已删除，允许重新创建
    taskManager[taskId].state = TASK_STATE_DELETED;
    taskManager[taskId].handle = NULL;
    
    Serial.print("功能模块D任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 退出");
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块E任务 (位置E)
 */
void taskFunctionE(void* pvParameters) {
    uint32_t taskId = (uint32_t)pvParameters;
    Serial.print("功能模块E任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 启动");
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        EventBits_t uxBits = xEventGroupWaitBits(
            xEventGroup,
            EVENT_RETURN_TO_MENU,
            pdTRUE,
            pdFALSE,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 任务功能实现
        // TODO: 实现功能模块E的具体逻辑
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 关键修复：首先清除退出事件标志，防止残留影响新任务
    xEventGroupClearBits(xEventGroup, EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    currentState = STATE_MAIN_MENU;
    
    // 清除所有事件标志，确保系统状态干净
    xEventGroupClearBits(xEventGroup, EVENT_ALL_TASKS_CLEAR);
    
    // 关键修复：更新任务管理器状态为已删除，允许重新创建
    taskManager[taskId].state = TASK_STATE_DELETED;
    taskManager[taskId].handle = NULL;
    
    Serial.print("功能模块E任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 退出");
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 系统设置任务 (位置F)
 */
void taskSystemSettings(void* pvParameters) {
    uint32_t taskId = (uint32_t)pvParameters;
    Serial.print("系统设置任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 启动");
    
    // 任务初始化
    switchToSubMenu();
    
    // 任务主循环
    while (1) {
        // 处理子菜单
        handleSubMenu();
        
        // 检查是否需要退出任务
        EventBits_t uxBits = xEventGroupWaitBits(
            xEventGroup,
            EVENT_RETURN_TO_MENU,
            pdTRUE,
            pdFALSE,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            Serial.print("系统设置任务 ");
            Serial.print((char)('A' + taskId));
            Serial.println(" 收到退出信号");
            break;
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
    // 任务清理
    switchToMainMenu();
    
    // 关键修复：首先清除退出事件标志，防止残留影响新任务
    xEventGroupClearBits(xEventGroup, EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    currentState = STATE_MAIN_MENU;
    
    // 清除所有事件标志，确保系统状态干净
    xEventGroupClearBits(xEventGroup, EVENT_ALL_TASKS_CLEAR);
    
    // 关键修复：更新任务管理器状态为已删除，允许重新创建
    // 必须在删除任务之前更新状态，确保状态同步
    taskManager[taskId].state = TASK_STATE_DELETED;
    taskManager[taskId].handle = NULL;
    
    Serial.print("系统设置任务 ");
    Serial.print((char)('A' + taskId));
    Serial.println(" 退出");
    
    // 删除任务
    vTaskDelete(NULL);
}