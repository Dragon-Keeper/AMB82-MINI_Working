#ifndef _AMEBA_ILI9341_H_
#define _AMEBA_ILI9341_H_

#include "Arduino.h"
#include "Print.h"
#include "SPI.h"

/* 屏幕分辨率 */
#define ILI9341_TFTWIDTH  240
#define ILI9341_TFTHEIGHT 320

/* ILI9341 命令集 */
#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13

#define ILI9341_INVOFF  0x20
#define ILI9341_INVON   0x21
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON  0x29

#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C
#define ILI9341_RAMRD   0x2E

#define ILI9341_PTLAR   0x30
#define ILI9341_MADCTL  0x36
#define ILI9341_PIXFMT  0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_RGB 0x00

/* RGB565 颜色表 */
#define ILI9341_BLACK       0x0000
#define ILI9341_WHITE       0xFFFF
#define ILI9341_RED         0xF800
#define ILI9341_GREEN       0x07E0
#define ILI9341_BLUE        0x001F
#define ILI9341_CYAN        0x07FF
#define ILI9341_MAGENTA     0xF81F
#define ILI9341_YELLOW      0xFFE0

class AmebaILI9341 : public Print {
public:
    AmebaILI9341(int csPin, int dcPin, int resetPin);
    void begin(void);

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

    void reset(void);
    /* 底层辅助 */
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
};

#endif