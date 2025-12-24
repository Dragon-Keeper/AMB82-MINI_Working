#ifndef _AMEBA_ST7789_SPI1_H_
#define _AMEBA_ST7789_SPI1_H_

#include "Arduino.h"
#include "Print.h"
#include "SPI.h"

/* 屏幕分辨率 - ST7789通常为240x320 */
#define ST7789_TFTWIDTH  240
#define ST7789_TFTHEIGHT 320

/* ST7789 命令集 */
#define ST7789_NOP     0x00
#define ST7789_SWRESET 0x01
#define ST7789_RDDID   0x04
#define ST7789_RDDST   0x09

#define ST7789_SLPIN   0x10
#define ST7789_SLPOUT  0x11
#define ST7789_PTLON   0x12
#define ST7789_NORON   0x13

#define ST7789_INVOFF  0x20
#define ST7789_INVON   0x21
#define ST7789_DISPOFF 0x28
#define ST7789_DISPON  0x29

#define ST7789_CASET   0x2A
#define ST7789_PASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_RAMRD   0x2E

#define ST7789_PTLAR   0x30
#define ST7789_MADCTL  0x36
#define ST7789_COLMOD  0x3A

#define ST7789_FRMCTR1 0xB1
#define ST7789_FRMCTR2 0xB2
#define ST7789_FRMCTR3 0xB3
#define ST7789_INVCTR  0xB4
#define ST7789_DISSET5 0xB6

#define ST7789_PWCTR1  0xC0
#define ST7789_PWCTR2  0xC1
#define ST7789_PWCTR3  0xC2
#define ST7789_PWCTR4  0xC3
#define ST7789_PWCTR5  0xC4
#define ST7789_VMCTR1  0xC5
#define ST7789_VMOFCTR 0xC7

#define ST7789_WRDISBV 0x51
#define ST7789_WRCTRLD 0x53

#define ST7789_RDID1   0xDA
#define ST7789_RDID2   0xDB
#define ST7789_RDID3   0xDC
#define ST7789_RDID4   0xDD

#define ST7789_GMCTRP1 0xE0
#define ST7789_GMCTRN1 0xE1
#define ST7789_EXTCTRL 0xE9

/* MADCTL 位定义 */
#define ST7789_MADCTL_MY  0x80
#define ST7789_MADCTL_MX  0x40
#define ST7789_MADCTL_MV  0x20
#define ST7789_MADCTL_ML  0x10
#define ST7789_MADCTL_RGB 0x00
#define ST7789_MADCTL_BGR 0x08
#define ST7789_MADCTL_MH  0x04

/* RGB565 颜色表 */
#define ST7789_BLACK       0x0000
#define ST7789_WHITE       0xFFFF
#define ST7789_RED         0xF800
#define ST7789_GREEN       0x07E0
#define ST7789_BLUE        0x001F
#define ST7789_CYAN        0x07FF
#define ST7789_MAGENTA     0xF81F
#define ST7789_YELLOW      0xFFE0

class AmebaST7789_SPI1 : public Print {
public:
    AmebaST7789_SPI1(int csPin, int dcPin, int resetPin);

    /* 基础功能 */
    void begin(void);
    void reset(void);

    /* 底层接口 */
    void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void writecommand(uint8_t c);
    void writedata(uint8_t d);
    void writedata(uint8_t *d, int len);
    void setRotation(uint8_t r);
    void setColorOrder(bool rgbOrder);  // true=RGB, false=BGR

    /* 绘图 API */
    void fillScreen(uint16_t color);
    void clr(void);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    void drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h);
    void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color);
    void fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r);
    void drawChar(unsigned char c);
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);

    /* Print 接口 */
    virtual size_t write(uint8_t);

    /* 属性 */
    int16_t getWidth(void)  { return _width;  }
    int16_t getHeight(void) { return _height; }
    void setCursor(int16_t x, int16_t y);
    void setForeground(uint16_t c) { foreground = c; }
    void setBackground(uint16_t c) { background = c; }
    void setFontSize(uint8_t s)    { fontsize = s;    }

protected:
    int _csPin, _dcPin, _resetPin;
    int16_t _width, _height;
    int16_t cursor_x, cursor_y;
    uint16_t foreground, background;
    uint8_t  fontsize, rotation;

    /* 底层辅助 */
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
};

#endif