/**
 * @file BasicExample.ino
 * @brief FT6336U_HardI2C 库基本使用示例
 * 
 * 本示例展示FT6336U_HardI2C库的基本使用方法，包括初始化、
 * 触摸检测和坐标读取。
 * 
 * 硬件连接：
 * - SDA: 引脚12 (AMB82-MINI硬件I2C0_SDA)
 * - SCL: 引脚13 (AMB82-MINI硬件I2C0_SCL)
 * - RST: 引脚15 (复位引脚)
 * - INT: 引脚16 (中断引脚，本示例未使用)
 * 
 * 功能：
 * 1. 初始化触摸屏
 * 2. 轮询检测触摸
 * 3. 打印触摸坐标
 * 4. 显示触摸状态
 */

#include <FT6336U_HardI2C.h>

// 创建触摸控制器实例
FT6336U_HardI2C touch;

// 触摸坐标
uint16_t touchX = 0;
uint16_t touchY = 0;

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== FT6336U 基本触摸示例 ===");
  Serial.println("正在初始化触摸控制器...");
  
  // 初始化触摸控制器
  touch.enableDebug(true);  // 启用调试输出
  if (!touch.begin()) {
    Serial.println("触摸控制器初始化失败!");
    while (1) {
      delay(1000);
    }
  }
  
  // 读取芯片信息
  if (!touch.readChipInfo()) {
    Serial.println("读取芯片信息失败!");
  }
  
  // 使用轮询模式（默认）
  touch.setInterruptMode(false);
  touch.setPollingInterval(20);  // 20ms轮询间隔
  
  Serial.println("触摸控制器初始化成功");
  Serial.println("触摸屏幕查看坐标...\n");
}

void loop() {
  // 更新触摸数据
  touch.update();
  
  // 检查屏幕是否被触摸
  if (touch.isTouched()) {
    // 获取触摸点数量
    uint8_t touchCount = touch.getTouchCount();
    
    if (touchCount > 0) {
      // 获取第一个触摸点的坐标
      if (touch.getTouchPoint(0, &touchX, &touchY)) {
        Serial.print("检测到触摸: ");
        Serial.print("X=");
        Serial.print(touchX);
        Serial.print(", Y=");
        Serial.print(touchY);
        Serial.print(" | 点数: ");
        Serial.println(touchCount);
      }
    }
  }
  
  // 可选：定期打印详细状态
  static uint32_t lastStatusTime = 0;
  uint32_t currentTime = millis();
  
  if (currentTime - lastStatusTime > 1000) {
    lastStatusTime = currentTime;
    touch.printStatus();
  }
  
  // 短暂延迟以避免串口数据溢出
  delay(10);
}