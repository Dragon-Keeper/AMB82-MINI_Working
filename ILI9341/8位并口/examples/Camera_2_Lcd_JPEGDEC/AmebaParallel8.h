#ifndef _AMEBA_PARALLEL8_H_
#define _AMEBA_PARALLEL8_H_

#include <Arduino.h>
#include "Print.h"

// 硬件端口配置（根据MCU类型定义）
#if defined(ARDUINO_ARCH_AVR)
  #include <avr/io.h>
  typedef volatile uint8_t* PortReg;
#elif defined(ARDUINO_ARCH_ESP32)
  #include <driver/gpio.h>
  typedef gpio_num_t PortPin; // ESP32用引脚号数组替代端口寄存器
#elif defined(ARDUINO_ARCH_RTL8730) || defined(AMB82_MINI) || defined(ARDUINO_AMEBA)
  // Ameba平台使用标准的Arduino GPIO操作
  typedef int PortPin;
#else
  #warning "Unsupported MCU architecture - Using fallback mode"
  typedef int PortPin;
#endif

class AmebaParallel8 : public Print {
public:
  // 构造函数：新增数据端口寄存器/引脚组参数
  #if defined(ARDUINO_ARCH_AVR)
    AmebaParallel8(PortReg dataPort, uint8_t dataMask,
                   int csPin, int dcPin, int resetPin, int wrPin, int rdPin);
  #elif defined(ARDUINO_ARCH_ESP32)
    AmebaParallel8(const PortPin dataPins[8], // D0-D7引脚数组
                   int csPin, int dcPin, int resetPin, int wrPin, int rdPin);
  #elif defined(ARDUINO_ARCH_RTL8730) || defined(AMB82_MINI) || defined(ARDUINO_AMEBA)
    AmebaParallel8(const PortPin dataPins[8], // D0-D7引脚数组
                   int csPin, int dcPin, int resetPin, int wrPin, int rdPin);
  #else
    AmebaParallel8(int csPin, int dcPin, int resetPin, int wrPin, int rdPin,
                   int d0Pin, int d1Pin, int d2Pin, int d3Pin,
                   int d4Pin, int d5Pin, int d6Pin, int d7Pin);
  #endif

  void begin(void);
  void setRotation(uint8_t m);
  void fillScreen(uint16_t color);
  void clr();
  void fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color);
  void drawBitmapOptimized(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color);
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t fontcolor, uint16_t background, uint8_t fontsize);
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  void drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
  virtual size_t write(uint8_t);

  int16_t getWidth();
  int16_t getHeight();
  void setCursor(int16_t x, int16_t y);
  void setForeground(uint16_t color);
  void setBackground(uint16_t color);
  void setFontSize(uint8_t size);

private:
  void reset(void);
  void writeCommand(uint8_t command);
  void writeData(uint8_t data);
  void writeData16(uint16_t data);
  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
  void write8bitData(uint8_t data); // 硬件并行实现

  // 硬件端口参数
  #if defined(ARDUINO_ARCH_AVR)
    PortReg _dataPort;    // 数据端口寄存器（如&PORTD）
    uint8_t _dataMask;    // 数据引脚掩码（如0xFF，对应D0-D7）
    PortReg _dataDDR;     // 数据端口方向寄存器（如&DDRD）
  #elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_RTL8730) || defined(AMB82_MINI) || defined(ARDUINO_AMEBA)
    PortPin _dataPins[8]; // D0-D7引脚号数组
  #else
    // 原有的引脚定义
    int _d0Pin, _d1Pin, _d2Pin, _d3Pin, _d4Pin, _d5Pin, _d6Pin, _d7Pin;
  #endif

  // 控制引脚
  int _csPin, _dcPin, _resetPin, _wrPin, _rdPin;
  // 状态变量
  int16_t _width, _height;
  int16_t cursor_x, cursor_y;
  uint16_t foreground, background;
  uint8_t fontsize, rotation;
};

#endif