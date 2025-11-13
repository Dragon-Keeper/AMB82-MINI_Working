#ifndef _AMEBA_ST7789_H_
#define _AMEBA_ST7789_H_

#include <Arduino.h>
#include "platform_stdlib.h"
#include "spi_api.h"
#include "spi_ex_api.h"


// 像素类型：RGB565
typedef uint16_t pixel_t;

// ST7789 commands
#define ST7789_NOP     0x00
#define ST7789_SWRESET 0x01
#define ST7789_RDDID   0x04
#define ST7789_RDDST   0x09

#define ST7789_SLPIN   0x10
#define ST7789_SLPOUT  0x11
#define ST7789_PTLON   0x12
#define ST7789_NORON   0x13

#define ST7789_INVOFF  0x20
#define ST7789_INVON   0x21   //
#define ST7789_DISPOFF 0x28
#define ST7789_DISPON  0x29

#define ST7789_CASET   0x2A
#define ST7789_RASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_RAMRD   0x2E

#define ST7789_PTLAR   0x30
#define ST7789_COLMOD  0x3A
#define ST7789_MADCTL  0x36

#define ST7789_MADCTL_MY  0x80
#define ST7789_MADCTL_MX  0x40
#define ST7789_MADCTL_MV  0x20
#define ST7789_MADCTL_ML  0x10
#define ST7789_MADCTL_RGB 0x00
#define ST7789_MADCTL_BGR 0x08
#define ST7789_MADCTL_MH  0x04

// Color definitions (RGB565)
#define ST7789_BLACK       0x0000
#define ST7789_WHITE       0xFFFF
#define ST7789_RED         0xF800
#define ST7789_GREEN       0x07E0
#define ST7789_BLUE        0x001F
#define ST7789_CYAN        0x07FF
#define ST7789_MAGENTA     0xF81F
#define ST7789_YELLOW      0xFFE0
#define ST7789_ORANGE      0xFD20


class AmebaST7789 {
public:
    // 构造函数支持自定义分辨率
    AmebaST7789(int csPin, int dcPin, int resetPin, uint16_t width = 320, uint16_t height = 240);
    void begin();

    // 绘图函数
    void fillScreen(pixel_t color);
    void drawPixel(int16_t x, int16_t y, pixel_t color);
    void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const pixel_t* data);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawChar(unsigned char c);
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t _fontcolor, uint16_t _background, uint8_t _fontsize);
    void drawString(const char *str, int16_t x, int16_t y, uint16_t _fontcolor, uint16_t _background, uint8_t _fontsize);
    void drawString(const char *str);
    void setFrontBufferIndex(uint8_t index);
    void flush();  // 启动 DMA 刷新
    int16_t getWidth() { return _width; }
    int16_t getHeight() { return _height; }
    void setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }
    void setForeground(uint16_t color) { foreground = color; }
    void setBackground(uint16_t color) { background = color; }
    void setFontSize(uint8_t size) { fontsize = size; }
    // 设置旋转方向
    void setRotation(uint8_t m);
    void directWrite(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t *pixels);
private:
    int _csPin, _dcPin, _resetPin;
    uint16_t _width, _height;
    uint8_t rotation = 0;
    uint8_t frontBufferIndex = 0;  // 当前显示的 buffer 索引

    spi_t spi_obj;

    int16_t cursor_x;
    int16_t cursor_y;
    uint16_t foreground;
    uint16_t background;
    uint8_t fontsize;

    bool dmaInProgress = false;  //DMA完成标志
    static void spi_dma_complete_callback(uint32_t id, SpiIrq event);
    void start_dma_transfer(const uint8_t* data, size_t len);
    void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void writecommand(uint8_t command);
    void writedata(uint8_t data);
    void reset(void);
    // 静态帧缓存（最大分辨率为 320x240）
    static const size_t MAX_FRAME_SIZE = 320 * 240;
    static pixel_t __attribute__((aligned(4))) frameBuffer[2][MAX_FRAME_SIZE];
};

#endif