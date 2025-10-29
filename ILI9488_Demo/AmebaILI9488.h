#ifndef AMEBAILI9488_H
#define AMEBAILI9488_H

#include <Arduino.h>
#include "font5x7.h"

/* 颜色定义18-bit 6-6-6模式 */
#define ILI9488_BLACK   0x0000
#define ILI9488_WHITE   0xFFFF
#define ILI9488_RED     0xF800
#define ILI9488_GREEN   0x07E0
#define ILI9488_BLUE    0x001F
#define ILI9488_YELLOW  0xFFE0
#define ILI9488_CYAN    0x07FF
#define ILI9488_MAGENTA 0xF81F

/* 命令定义 */
#define CMD_SWRESET    0x01
#define CMD_SLPOUT     0x11
#define CMD_INVOFF     0x20
#define CMD_INVON      0x21
#define CMD_DISPON     0x29
#define CMD_DISPOFF    0x28
#define CMD_CASET      0x2A
#define CMD_PASET      0x2B
#define CMD_RAMWR      0x2C
#define CMD_RAMRD      0x2E
#define CMD_MADCTL     0x36
#define CMD_COLMOD     0x3A
#define CMD_PWCTRL1    0xC0
#define CMD_PWCTRL2    0xC1
#define CMD_VMCTRL1    0xC5
#define CMD_PGAMCTRL   0xE0
#define CMD_NGAMCTRL   0xE1

class AmebaILI9488 {
  public:
    AmebaILI9488(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t sck, uint8_t mosi, uint8_t led);
    void begin(void);
    void setRotation(uint8_t r);
    void fillScreen(uint16_t color);
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
    void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void drawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
    void fillCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
    void setCursor(uint16_t x, uint16_t y);
    void setFontSize(uint8_t size);
    void setForeground(uint16_t color);
    void setBackground(uint16_t color);
    size_t write(uint8_t c);
    void print(const char* str);
    void println(const char* str);

  private:
    uint8_t _cs, _dc, _rst, _sck, _mosi, _led;
    uint8_t _madctl;
    uint16_t _cursorX, _cursorY;
    uint8_t _fontSize;
    uint16_t _foreground, _background;
    uint16_t _width, _height;
    
    void hardwareReset(void);
    void initRegisters(void);
    void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void writePixel(uint16_t color);
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeData16(uint16_t data);
    void writeBytes(uint8_t *data, uint32_t len);
    void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
    
    // 软件SPI函数
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