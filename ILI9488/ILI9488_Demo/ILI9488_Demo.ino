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
// 适配RGB666模式的颜色修正 - 现在配合MADCTL_BGR设置
uint32_t fixColor(uint32_t color) {
  // 在RGB666模式下，由于我们在驱动中设置了MADCTL_BGR，
  // 这里需要保持RGB顺序，因为控制器会根据MADCTL自动调整内部映射
  // 提取RGB分量 (每个通道6位)
  uint8_t r = (color >> 16) & 0xFC;  // 红色通道 (6位，R5-R0)
  uint8_t g = (color >> 8) & 0xFC;   // 绿色通道 (6位，G5-G0)
  uint8_t b = color & 0xFC;          // 蓝色通道 (6位，B5-B0)
  
  // 保持原始RGB顺序，因为MADCTL_BGR会在控制器内部处理颜色通道交换
  // 这样可以确保红色显示为红色，绿色显示为绿色，蓝色显示为蓝色
  return ((r << 16) | (g << 8) | b);
}

// 颜色数组 - 使用修正后的颜色值
uint32_t testColors[] = {fixColor(ILI9488_RED), fixColor(ILI9488_GREEN), fixColor(ILI9488_BLUE), 
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

// 颜色测试 - 18位RGB666优化版
void runColorTest() {
  static uint8_t colorStep = 0;
  static unsigned long lastChange = 0;
  
  // 每3秒切换一次颜色
  if (millis() - lastChange > 3000) {
    // 测试不同颜色的显示 - 18位RGB666优化版
    // 使用修正后的颜色值，确保正确的RGB666颜色表示
    const uint32_t colors[] = {
      fixColor(ILI9488_BLACK),
      fixColor(ILI9488_RED),
      fixColor(ILI9488_GREEN),
      fixColor(ILI9488_BLUE),
      fixColor(ILI9488_YELLOW),
      fixColor(ILI9488_CYAN),
      fixColor(ILI9488_MAGENTA),
      fixColor(ILI9488_WHITE)
    };
    
    const char* colorNames[] = {
      "BLACK", "RED", "GREEN", "BLUE",
      "YELLOW", "CYAN", "MAGENTA", "WHITE"
    };
    
    tft.fillScreen(colors[colorStep]);
    tft.setCursor(40, 200);
    tft.setFontSize(3);
    
    // 根据背景颜色选择合适的文本颜色
    if (colorStep == 0 || colorStep == 7) {  // 黑色背景显示白色文字，白色背景显示黑色文字
      tft.setForeground(colorStep == 0 ? fixColor(ILI9488_WHITE) : fixColor(ILI9488_BLACK));
    } else {
      tft.setForeground(fixColor(ILI9488_BLACK));
    }
    
    tft.println(colorNames[colorStep]);
    Serial.print("显示");
    Serial.println(colorNames[colorStep]);
    
    // 循环切换所有8种颜色
    colorStep = (colorStep + 1) % 8;
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
  tft.setRotation(3); // 横向显示 (480x320)
  
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
  
  // 运行当前测试 - 完整的RGB666功能验证
  switch (currentState) {
    case TEST_INTRO:
      // 介绍页不需要持续更新
      break;
    case TEST_COLORS:
      runColorTest();    // 测试所有颜色显示，使用18位RGB666优化版
      break;
    case TEST_SHAPES:
      runShapeTest();    // 测试形状绘制功能
      break;
    case TEST_TEXT:
      runTextTest();     // 测试文本显示功能
      break;
  }
  
  delay(50);
}