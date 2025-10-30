/***************************************************
  ILI9488 TFT Driver for Ameba Platform
  Based on Adafruit ILI9488 Library
  This is our library for the ILI9488 Breakout and Shield
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#ifndef AMEBAILI9488_H
#define AMEBAILI9488_H

#include <Arduino.h>
#include "font5x7.h"

#define ILI9488_TFTWIDTH  320
#define ILI9488_TFTHEIGHT 480

// Command definitions
#define ILI9488_NOP     0x00
#define ILI9488_SWRESET 0x01
#define ILI9488_RDDID   0x04
#define ILI9488_RDDST   0x09

#define ILI9488_SLPIN   0x10
#define ILI9488_SLPOUT  0x11
#define ILI9488_PTLON   0x12
#define ILI9488_NORON   0x13

#define ILI9488_RDMODE  0x0A
#define ILI9488_RDMADCTL  0x0B
#define ILI9488_RDPIXFMT  0x0C
#define ILI9488_RDIMGFMT  0x0D
#define ILI9488_RDSELFDIAG  0x0F

#define ILI9488_INVOFF  0x20
#define ILI9488_INVON   0x21
#define ILI9488_GAMMASET 0x26
#define ILI9488_DISPOFF 0x28
#define ILI9488_DISPON  0x29

#define ILI9488_CASET   0x2A
#define ILI9488_PASET   0x2B
#define ILI9488_RAMWR   0x2C
#define ILI9488_RAMRD   0x2E

#define ILI9488_PTLAR   0x30
#define ILI9488_MADCTL  0x36
#define ILI9488_PIXFMT  0x3A

#define ILI9488_FRMCTR1 0xB1
#define ILI9488_FRMCTR2 0xB2
#define ILI9488_FRMCTR3 0xB3
#define ILI9488_INVCTR  0xB4
#define ILI9488_DFUNCTR 0xB6

#define ILI9488_PWCTR1  0xC0
#define ILI9488_PWCTR2  0xC1
#define ILI9488_PWCTR3  0xC2
#define ILI9488_PWCTR4  0xC3
#define ILI9488_PWCTR5  0xC4
#define ILI9488_VMCTR1  0xC5
#define ILI9488_VMCTR2  0xC7

#define ILI9488_RDID1   0xDA
#define ILI9488_RDID2   0xDB
#define ILI9488_RDID3   0xDC
#define ILI9488_RDID4   0xDD

#define ILI9488_GMCTRP1 0xE0
#define ILI9488_GMCTRN1 0xE1

// MADCTL flags
#define MADCTL_MY  0x80  ///< Bottom to top
#define MADCTL_MX  0x40  ///< Right to left
#define MADCTL_MV  0x20  ///< Reverse Mode
#define MADCTL_ML  0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00  ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08  ///< Blue-Green-Red pixel order
#define MADCTL_MH  0x04  ///< LCD refresh right to left

// Color definitions - 18-bit RGB666 mode
// 反转颜色定义以解决颜色显示相反的问题
#define ILI9488_BLACK   0xFCFCFC  ///< 255, 255, 255 (实际显示为黑色)
#define ILI9488_WHITE   0x000000  ///<   0,   0,   0 (实际显示为白色)
#define ILI9488_RED     0x00FCFC  ///<   0, 255, 255 (实际显示为红色)
#define ILI9488_GREEN   0xFC00FC  ///< 255,   0, 255 (实际显示为绿色)
#define ILI9488_BLUE    0xFCFC00  ///< 255, 255,   0 (实际显示为蓝色)
#define ILI9488_YELLOW  0x0000FC  ///<   0,   0, 255 (实际显示为黄色)
#define ILI9488_CYAN    0xFC0000  ///< 255,   0,   0 (实际显示为青色)
#define ILI9488_MAGENTA 0x00FC00  ///<   0, 255,   0 (实际显示为洋红色)

// RGB888 to RGB666 conversion macro - 调整为与颜色反转设置兼容
// 由于我们已经反转了颜色定义，这里保持原始转换逻辑
#define rgb888_to_rgb666(r, g, b) (((r & 0xFC) << 16) | ((g & 0xFC) << 8) | (b & 0xFC))

// RGB666 channel extraction macros
#define getRed666(color)    ((color >> 16) & 0xFC)
#define getGreen666(color)  ((color >> 8) & 0xFC)
#define getBlue666(color)   (color & 0xFC)

class AmebaILI9488 {
  public:
    AmebaILI9488(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t sck, uint8_t mosi, uint8_t led);
    void begin(void);
    void setRotation(uint8_t r);
    void invertDisplay(boolean i);
    
    // Drawing and filling functions
    void fillScreen(uint32_t color);
    void drawPixel(uint16_t x, uint16_t y, uint32_t color);
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color);
    void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
    void drawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint32_t color);
    void fillCircle(uint16_t x0, uint16_t y0, uint8_t r, uint32_t color);
    void drawFastHLine(uint16_t x, uint16_t y, uint16_t w, uint32_t color);
    void drawFastVLine(uint16_t x, uint16_t y, uint16_t h, uint32_t color);
    
    // Text functions
    void setCursor(uint16_t x, uint16_t y);
    void setFontSize(uint8_t size);
    void setForeground(uint32_t color);
    void setBackground(uint32_t color);
    size_t write(uint8_t c);
    void print(const char* str);
    void println(const char* str);
    
    // Color functions
    uint32_t color666(uint8_t r, uint8_t g, uint8_t b);
    
    // Address and scrolling functions
    void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void scrollTo(uint16_t y);
    void setScrollMargins(uint16_t top, uint16_t bottom);
    
    // 公开writeData24方法，用于JPEG图像绘制
    void writeData24(uint32_t data);

  private:
    uint8_t _cs, _dc, _rst, _sck, _mosi, _led;
    uint8_t _rotation;
    uint16_t _cursorX, _cursorY;
    uint8_t _fontSize;
    uint32_t _foreground, _background;
    uint16_t _width, _height;
    
    // Internal functions
    void hardwareReset(void);
    void initRegisters(void);
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeBytes(uint8_t *data, uint32_t len);
    void writePixelData(uint32_t color);
    void drawChar(uint16_t x, uint16_t y, char c, uint32_t color, uint32_t bg, uint8_t size);
    
    // SPI functions
    inline void SPI_Transfer(uint8_t data);
    inline void CS_LOW(void)  { digitalWrite(_cs, LOW); }
    inline void CS_HIGH(void) { digitalWrite(_cs, HIGH); }
    inline void DC_LOW(void)  { digitalWrite(_dc, LOW); }
    inline void DC_HIGH(void) { digitalWrite(_dc, HIGH); }
    inline void SCK_LOW(void) { digitalWrite(_sck, LOW); }
    inline void SCK_HIGH(void) { digitalWrite(_sck, HIGH); }
    inline void MOSI_LOW(void) { digitalWrite(_mosi, LOW); }
    inline void MOSI_HIGH(void) { digitalWrite(_mosi, HIGH); }
};

#endif