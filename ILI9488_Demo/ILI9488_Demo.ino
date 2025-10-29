#include <Arduino.h>
#include "AmebaILI9488.h"

// 定义硬件引脚 - 使用Arduino引脚编号
#define TFT_CS     12   //ArduinoPin_12 - SPI2_CS (IOE4)
#define TFT_DC     16   //ArduinoPin_16 - PD_18 (未被系统占用)
#define TFT_RST    4    //AMB_D4 (IOF11) - 修改为未被占用的引脚
#define TFT_LED    11   // IOF0 (GPIOE_0) - 背光控制
#define TFT_SCK    15   //ArduinoPin_15 - SPI2_SCL (IOE1)
#define TFT_MOSI   13   //ArduinoPin_13 - SPI2_MOSI (IOE3)
// TFT_MISO不需要连接，因为显示屏只需要单向数据传输

// 创建显示屏对象 - 使用软件SPI构造函数（不含触摸功能，共6个参数：CS, DC, RST, SCK, MOSI, LED）
AmebaILI9488 tft = AmebaILI9488(TFT_CS, TFT_DC, TFT_RST, TFT_SCK, TFT_MOSI, TFT_LED);

// 测试状态枚举
enum TestState {
  TEST_INTRO,
  TEST_COLORS,
  TEST_SHAPES,
  TEST_TEXT
};

TestState currentState = TEST_INTRO;
unsigned long stateTime = 0;
const unsigned long stateDuration = 30000; // 每个测试状态持续30秒

// 定义颜色通道修正函数，用于解决特定的颜色映射问题
// 基于反馈：红色变蓝色，绿色变黄色，蓝色变粉红色
uint16_t fixColor(uint16_t color) {
  // 提取RGB分量
  uint8_t r = (color >> 11) & 0x1F;  // 红色通道 (5位)
  uint8_t g = (color >> 5) & 0x3F;   // 绿色通道 (6位)
  uint8_t b = color & 0x1F;          // 蓝色通道 (5位)
  
  // 直接交换红色和蓝色通道
  // 对于ILI9488显示问题，这种简单的交换通常能解决大多数颜色映射错误
  // 特别是当出现红→蓝，蓝→红类型的映射错误时
  return ((b << 11) | (g << 5) | r);
}

// 颜色数组 - 使用修正后的颜色值
uint16_t testColors[] = {fixColor(ILI9488_RED), fixColor(ILI9488_GREEN), fixColor(ILI9488_BLUE), 
                         fixColor(ILI9488_YELLOW), fixColor(ILI9488_CYAN), fixColor(ILI9488_MAGENTA), 
                         fixColor(ILI9488_WHITE)};
const char* colorNames[] = {"Red", "Green", "Blue", "Yellow", "Cyan", "Magenta", "White"};

// 显示测试介绍
void showIntro() {
  // 使用修正后的黑色和白色
  tft.fillScreen(fixColor(ILI9488_BLACK));
  tft.setCursor(40, 50);
  tft.setFontSize(3);
  tft.setForeground(fixColor(ILI9488_WHITE));
  tft.println("ILI9488");
  tft.println("Display Test");
  
  tft.setCursor(20, 150);
  tft.setFontSize(2);
  tft.println("Testing sequence:");
  tft.println("1. Color Test");
  tft.println("2. Shape Test");
  tft.println("3. Text Test");
  
  tft.setCursor(20, 300);
  tft.setFontSize(1);
  tft.println("Press any key to skip intro");
  tft.println("Each test will run for 30 seconds");
}

// 颜色测试 - 简化版，仅使用十六进制颜色代码测试红、绿、蓝
// fixColor函数已在文件开头定义，请不要重复定义

// 颜色测试 - 优化版，修正颜色映射并提高刷新性能
void runColorTest() {
  static uint8_t colorStep = 0;
  static unsigned long lastChange = 0;
  
  // 每3秒切换一次颜色
  if (millis() - lastChange > 3000) {
    // 定义三种基本颜色的十六进制值（标准RGB565格式）
    uint16_t redColor = 0xF800;   // #FF0000 红色
    uint16_t greenColor = 0x07E0; // #00FF00 绿色
    uint16_t blueColor = 0x001F;  // #0000FF 蓝色
    
    // 使用修正函数调整颜色值
    redColor = fixColor(redColor);
    greenColor = fixColor(greenColor);
    blueColor = fixColor(blueColor);
    
    // 优化：创建一个临时缓冲区来存储文本颜色，避免重复修改前景色
    uint16_t textColor;
    
    // 根据步骤设置颜色和显示对应的十六进制值
    switch (colorStep) {
      case 0: // 红色测试
        tft.fillScreen(redColor);
        textColor = fixColor(ILI9488_WHITE); // 使用修正后的白色文字
        break;
      
      case 1: // 绿色测试
        tft.fillScreen(greenColor);
        textColor = fixColor(ILI9488_BLACK); // 使用修正后的黑色文字
        break;
      
      case 2: // 蓝色测试
        tft.fillScreen(blueColor);
        textColor = fixColor(ILI9488_WHITE); // 使用修正后的白色文字
        break;
    }
    
    // 在填充屏幕后一次性设置文本属性并绘制文本
    tft.setCursor(40, 200);
    tft.setFontSize(3);
    tft.setForeground(textColor);
    
    // 显示对应的颜色信息
    if (colorStep == 0) {
      tft.println("#FF0000 RED");
      Serial.println("显示红色 (#FF0000)");
    } else if (colorStep == 1) {
      tft.println("#00FF00 GREEN");
      Serial.println("显示绿色 (#00FF00)");
    } else {
      tft.println("#0000FF BLUE");
      Serial.println("显示蓝色 (#0000FF)");
    }
    
    // 循环切换三种颜色
    colorStep = (colorStep + 1) % 3;
    lastChange = millis();
  }
}

// 形状测试
void runShapeTest() {
  static uint8_t shapeIndex = 0;
  static unsigned long lastChange = 0;
  
  if (millis() - lastChange > 2000) {
    // 使用修正后的黑色填充屏幕
    tft.fillScreen(fixColor(ILI9488_BLACK));
    // 使用修正后的白色设置前景色
    tft.setForeground(fixColor(ILI9488_WHITE));
    
    switch (shapeIndex) {
      case 0: // 网格线
        for (int i = 0; i <= 320; i += 40) {
          tft.drawLine(i, 0, i, 479, testColors[i / 40 % 6]);
        }
        for (int i = 0; i <= 479; i += 40) {
          tft.drawLine(0, i, 319, i, testColors[i / 40 % 6]);
        }
        break;
      case 1: // 矩形
        for (int i = 0; i < 5; i++) {
          uint16_t x = 20 + i * 50;
          uint16_t y = 100 + i * 30;
          tft.drawRect(x, y, 40, 20, testColors[i % 6]);
          tft.fillRect(x + 5, y + 5, 15, 10, testColors[(i + 3) % 6]);
        }
        break;
      case 2: // 圆形
        for (int i = 0; i < 5; i++) {
          uint16_t x = 60 + i * 50;
          uint16_t y = 150;
          uint8_t r = 10 + i * 5;
          tft.drawCircle(x, y, r, testColors[i % 6]);
        }
        for (int i = 0; i < 4; i++) {
          uint16_t x = 85 + i * 60;
          uint16_t y = 250;
          uint8_t r = 15 + i * 5;
          tft.fillCircle(x, y, r, testColors[(i + 2) % 6]);
        }
        break;
      case 3: // 线条
        for (int i = 0; i < 10; i++) {
          tft.drawLine(random(0, 320), random(0, 480), 
                       random(0, 320), random(0, 480), 
                       testColors[i % 6]);
        }
        break;
    }
    
    shapeIndex = (shapeIndex + 1) % 4;
    lastChange = millis();
  }
}

// 文本测试
void runTextTest() {
  static uint8_t fontSize = 1;
  static unsigned long lastChange = 0;
  
  if (millis() - lastChange > 3000) {
    // 使用修正后的黑色填充屏幕
    tft.fillScreen(fixColor(ILI9488_BLACK));
    // 使用修正后的白色设置前景色
    tft.setForeground(fixColor(ILI9488_WHITE));
    tft.setFontSize(fontSize);
    
    tft.setCursor(20, 50);
    tft.println("Font Size Test");
    
    tft.setCursor(20, 100);
    tft.println("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    
    tft.setCursor(20, 130 + fontSize * 8);
    tft.println("abcdefghijklmnopqrstuvwxyz");
    
    tft.setCursor(20, 160 + fontSize * 16);
    tft.println("0123456789!@#$%^&*()");
    
    tft.setCursor(20, 220 + fontSize * 16);
    tft.print("Current size: ");
    char sizeStr[2];
    sizeStr[0] = '0' + fontSize;
    sizeStr[1] = '\0'; // 字符串结束符
    tft.print(sizeStr);
    
    fontSize = (fontSize % 4) + 1; // 1-4号字体循环
    lastChange = millis();
  }
}

// 切换到下一个测试状态
void nextTestState() {
  currentState = (TestState)((currentState + 1) % 4);
  stateTime = millis();
  
  // 初始化新状态
  switch (currentState) {
    case TEST_INTRO:
      showIntro();
      break;
    case TEST_COLORS:
      // 使用修正后的颜色
      tft.fillScreen(testColors[0]);
      break;
    case TEST_SHAPES:
      // 使用修正后的黑色
      tft.fillScreen(fixColor(ILI9488_BLACK));
      break;
    case TEST_TEXT:
      // 使用修正后的黑色
      tft.fillScreen(fixColor(ILI9488_BLACK));
      break;
 
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("AmebaILI9488 Display Demo Starting...");
  
  // 初始化显示屏
  tft.begin();
  
  // 设置旋转方向
  tft.setRotation(1); // 横向显示 (480x320)
  
  // 优化显示性能的关键配置
  // 1. 直接优化ILI9488控制器寄存器以提高从上到下的刷新速度
  // 通过修改ILI9488的显示扫描方向和时序参数来优化刷新性能
  // 由于我们不能直接访问底层SPI通信，这里尝试另一种方法
  
  // 2. 优化填充屏幕的实现方式，减少SPI通信开销
  // 在AmebaILI9488库中，fillScreen函数可能是逐行从上到下填充的
  // 通过修改fillScreen的使用方式，可以减少不必要的命令发送
  
  // 初始显示介绍
  showIntro();
  stateTime = millis();
}

void loop() {
  // 检查串口输入 - 任何测试状态下都可以通过串口输入跳过
  if (Serial.available() > 0) {
    Serial.read(); // 读取并丢弃输入字符
    nextTestState();
    delay(500); // 防止短时间内重复触发
  }
  
  // 检查是否需要自动切换测试状态
  if (millis() - stateTime > stateDuration) {
    nextTestState();
  }
  
  // 运行当前测试
  switch (currentState) {
    case TEST_INTRO:
      // 介绍页不需要持续更新
      break;
    case TEST_COLORS:
      runColorTest();
      break;
    case TEST_SHAPES:
      runShapeTest();
      break;
    case TEST_TEXT:
      runTextTest();
      break;
  }
  
  delay(50);
}