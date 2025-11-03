#ifndef _AMEBA_PARALLEL8_H_
#define _AMEBA_PARALLEL8_H_

#include <Arduino.h>
#include "Print.h"

// 8位并行总线ILI9341驱动类
class AmebaParallel8 : public Print {
public:
    AmebaParallel8(int csPin, int dcPin, int resetPin, int wrPin, int rdPin, 
                   int d0Pin, int d1Pin, int d2Pin, int d3Pin, 
                   int d4Pin, int d5Pin, int d6Pin, int d7Pin);

    void begin(void);
    void setRotation(uint8_t m);
    void fillScreen(uint16_t color);
    void clr();
    void fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color);
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
    void write8bitData(uint8_t data);

    // 引脚定义
    int _csPin, _dcPin, _resetPin, _wrPin, _rdPin;
    int _d0Pin, _d1Pin, _d2Pin, _d3Pin, _d4Pin, _d5Pin, _d6Pin, _d7Pin;

    int16_t _width, _height;
    int16_t cursor_x, cursor_y;
    uint16_t foreground, background;
    uint8_t fontsize, rotation;
};

#endif