#include "AmebaST7789.h"
#include "font5x7.h"
#include <SPI.h>
#include <stdlib.h>

/* 构造函数 */
AmebaST7789::AmebaST7789(int csPin, int dcPin, int resetPin) {
    _csPin    = csPin;
    _dcPin    = dcPin;
    _resetPin = resetPin;
    _width    = ST7789_TFTWIDTH;
    _height   = ST7789_TFTHEIGHT;
    cursor_x  = 0;
    cursor_y  = 0;
    foreground = ST7789_WHITE;
    background = ST7789_BLACK;
    fontsize   = 1;
    rotation   = 0;
}

/* ST7789 初始化序列 */
void AmebaST7789::begin(void) {
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, LOW);
    delay(10);
    digitalWrite(_resetPin, HIGH);
    delay(120);

    pinMode(_dcPin, OUTPUT);
    SPI.begin();          // Arduino SPI0
    reset();

    // ST7789 初始化序列
    writecommand(ST7789_SLPOUT);    // Sleep out
    delay(120);

    // 注意：先设置像素格式，再设置MADCTL
    writecommand(ST7789_COLMOD);    // Interface Pixel Format
    writedata(0x55);               // 16位/pixel (RGB565)

    writecommand(ST7789_MADCTL);    // Memory Data Access Control
    writedata(0x00);               // 默认方向设置

    // Porch Setting
    writecommand(0xB2);
    writedata(0x0C);
    writedata(0x0C);
    writedata(0x00);
    writedata(0x33);
    writedata(0x33);

    // Gate Control
    writecommand(0xB7);
    writedata(0x74);      // VGH=14.97V, VGL=-7.67V

    // VCOM Setting
    writecommand(0xBB);
    writedata(0x1F);

    // LCM Control
    writecommand(0xC0);
    writedata(0x2C);

    // VDV and VRH Command Enable
    writecommand(0xC2);
    writedata(0x01);

    // VRH Set
    writecommand(0xC3);
    writedata(0x10);

    // VDV Set
    writecommand(0xC4);
    writedata(0x20);

    // Frame Rate Control in Normal Mode
    writecommand(0xC6);
    writedata(0x0F);

    // Power Control 1
    writecommand(0xD0);
    writedata(0xA4);
    writedata(0xA1);

    // Positive Voltage Gamma Control
    writecommand(0xE0);
    writedata(0xD0);
    writedata(0x07);
    writedata(0x0E);
    writedata(0x0B);
    writedata(0x0A);
    writedata(0x14);
    writedata(0x38);
    writedata(0x33);
    writedata(0x4F);
    writedata(0x37);
    writedata(0x16);
    writedata(0x16);
    writedata(0x2A);
    writedata(0x2E);

    // Negative Voltage Gamma Control
    writecommand(0xE1);
    writedata(0xD0);
    writedata(0x0B);
    writedata(0x10);
    writedata(0x08);
    writedata(0x08);
    writedata(0x06);
    writedata(0x35);
    writedata(0x54);
    writedata(0x4D);
    writedata(0x0A);
    writedata(0x14);
    writedata(0x14);
    writedata(0x2C);
    writedata(0x2F);

    // Set Image Function
    writecommand(0xE9);
    writedata(0x11);
    writedata(0x11);
    writedata(0x03);

    writecommand(ST7789_INVON);     // Display Inversion On
    writecommand(ST7789_DISPON);    // Display On
    delay(120);
    // 在初始化最后设置默认颜色顺序
    setColorOrder(true);  // 默认使用RGB模式
}

void AmebaST7789::reset(void) {
    digitalWrite(_resetPin, HIGH);
    delay(5);
    digitalWrite(_resetPin, LOW);
    delay(20);
    digitalWrite(_resetPin, HIGH);
    delay(150);
}

/* 底层命令/数据 */
inline void AmebaST7789::writecommand(uint8_t c) {
    digitalWrite(_dcPin, LOW);
    SPI.transfer(c);
}
inline void AmebaST7789::writedata(uint8_t d) {
    digitalWrite(_dcPin, HIGH);
    SPI.transfer(d);
}
void AmebaST7789::writedata(uint8_t *d, int len) {
    digitalWrite(_dcPin, HIGH);
    SPI.transfer(d, len, SPI_LAST);
}

/* 窗口设置 */
void AmebaST7789::setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    writecommand(ST7789_CASET);
    writedata(x0 >> 8); writedata(x0 & 0xFF);
    writedata(x1 >> 8); writedata(x1 & 0xFF);

    writecommand(ST7789_PASET);
    writedata(y0 >> 8); writedata(y0 & 0xFF);
    writedata(y1 >> 8); writedata(y1 & 0xFF);

    writecommand(ST7789_RAMWR);
}

/* 旋转 - 根据ST7789的MADCTL寄存器设置 */
/* 旋转 - 根据ST7789的MADCTL寄存器设置 */
void AmebaST7789::setRotation(uint8_t m) {
    rotation = m % 4;
    
    // 先设置旋转方向
    uint8_t madctl = 0;
    switch (rotation) {
        case 0: // 0度旋转
            madctl = 0x00;
            _width = ST7789_TFTWIDTH; 
            _height = ST7789_TFTHEIGHT;
            break;
        case 1: // 90度旋转
            madctl = 0x60;
            _width = ST7789_TFTHEIGHT;
            _height = ST7789_TFTWIDTH;
            break;
        case 2: // 180度旋转
            madctl = 0xC0;
            _width = ST7789_TFTWIDTH;
            _height = ST7789_TFTHEIGHT;
            break;
        case 3: // 270度旋转
            madctl = 0xA0;
            _width = ST7789_TFTHEIGHT;
            _height = ST7789_TFTWIDTH;
            break;
    }
    
    writecommand(ST7789_MADCTL);
    writedata(madctl);
    
    // 注意：这里不设置RGB/BGR位，由setColorOrder单独控制
}

/* 基础图形函数 - 保持原有实现，但使用ST7789命令 */
void AmebaST7789::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height)) return;
    setAddress(x, y, x, y);
    // ST7789通常需要高字节在前
    writedata(color >> 8);  // 先发送高字节
    writedata(color & 0xFF); // 再发送低字节
}

void AmebaST7789::fillScreen(uint16_t color) {
    fillRectangle(0, 0, _width, _height, color);
}

void AmebaST7789::clr(void) {
    fillScreen(background);
}

void AmebaST7789::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (y < 0 || y >= _height || x >= _width) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > _width) w = _width - x;
    if (w <= 0) return;
    setAddress(x, y, x + w - 1, y);
    uint8_t hi = color >> 8, lo = color & 0xFF;  // 注意字节顺序
    digitalWrite(_dcPin, HIGH);
    for (int i = 0; i < w; ++i) { SPI.transfer(hi); SPI.transfer(lo); }
}

void AmebaST7789::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (x < 0 || x >= _width || y >= _height) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > _height) h = _height - y;
    if (h <= 0) return;
    setAddress(x, y, x, y + h - 1);
    uint8_t hi = color >> 8, lo = color & 0xFF;  // 注意字节顺序
    digitalWrite(_dcPin, HIGH);
    for (int i = 0; i < h; ++i) { SPI.transfer(hi); SPI.transfer(lo); }
}

void AmebaST7789::fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // ... 边界检查 ...
    setAddress(x, y, x + w - 1, y + h - 1);
    uint32_t n = (uint32_t)w * h;
    uint8_t hi = color >> 8;   // 高字节
    uint8_t lo = color & 0xFF; // 低字节
    digitalWrite(_dcPin, HIGH);
    for (uint32_t i = 0; i < n; ++i) { 
        SPI.transfer(hi);  // 先发送高字节
        SPI.transfer(lo);  // 再发送低字节
    }
}

void AmebaST7789::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color) {
    if ((x >= _width) || (y >= _height)) return;
    if (x + w > _width)  w = _width  - x;
    if (y + h > _height) h = _height - y;
    setAddress(x, y, x + w - 1, y + h - 1);
    digitalWrite(_dcPin, HIGH);
    
    uint16_t *temp_buffer = (uint16_t *)malloc(w * h * sizeof(uint16_t));
    if (temp_buffer) {
        for (int i = 0; i < w * h; i++) {
            uint16_t pixel = color[i];
            temp_buffer[i] = ((pixel & 0xFF) << 8) | (pixel >> 8);  // 交换字节顺序
        }
        SPI.transfer((uint8_t *)temp_buffer, w * h * 2, SPI_LAST);
        free(temp_buffer);
    } else {
        // 如果内存分配失败，使用逐像素方法
        for (int i = 0; i < w * h; i++) {
            uint16_t pixel = color[i];
            SPI.transfer(pixel >> 8);
            SPI.transfer(pixel & 0xFF);
        }
    }
}

// 其他图形函数保持不变...
void AmebaST7789::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = (dx > dy ? dx : -dy) / 2, e2;
    for (;;) {
        drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}

void AmebaST7789::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    drawLine(x0, y0, x1, y1, foreground);
}

void AmebaST7789::drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
}

void AmebaST7789::drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h) {
    drawRectangle(x, y, w, h, foreground);
}

void AmebaST7789::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
    drawPixel(x0, y0 + r, color); drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color); drawPixel(x0 - r, y0, color);
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        drawPixel(x0 + x, y0 + y, color); drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color); drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color); drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color); drawPixel(x0 - y, y0 - x, color);
    }
}

void AmebaST7789::drawCircle(int16_t x0, int16_t y0, int16_t r) {
    drawCircle(x0, y0, r, foreground);
}

/* 字符 / Print */
void AmebaST7789::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
    if ((x >= _width) || (y >= _height) || (x + 6 * size - 1) < 0 || (y + 8 * size - 1) < 0) return;
    for (int i = 0; i < 6; i++) {
        uint8_t line = (i < 5) ? font5x7[c * 5 + i] : 0x00;
        for (int j = 0; j < 8; j++, line >>= 1) {
            uint16_t col = (line & 0x01) ? color : bg;
            if (size == 1) drawPixel(x + i, y + j, col);
            else fillRectangle(x + i * size, y + j * size, size, size, col);
        }
    }
}

void AmebaST7789::drawChar(unsigned char c) {
    drawChar(cursor_x, cursor_y, c, foreground, background, fontsize);
    cursor_x += fontsize * 6;
}

size_t AmebaST7789::write(uint8_t c) {
    if (c == '\n') { cursor_x = 0; cursor_y += fontsize * 8; }
    else if (c != '\r') drawChar(c);
    return 1;
}

/* 光标 / 颜色 */
void AmebaST7789::setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }

/* 设置RGB/BGR颜色顺序 */
void AmebaST7789::setColorOrder(bool rgbOrder) {
    uint8_t madctl = 0;
    switch (rotation) {
        case 0: // 0度旋转
            madctl = 0x00;  // MY=0, MX=0, MV=0, ML=0, MH=0
            break;
        case 1: // 90度旋转
            madctl = 0x60;  // MY=0, MX=0, MV=1, ML=1, MH=0
            break;
        case 2: // 180度旋转
            madctl = 0xC0;  // MY=1, MX=1, MV=0, ML=0, MH=0
            break;
        case 3: // 270度旋转
            madctl = 0xA0;  // MY=1, MX=0, MV=1, ML=0, MH=0
            break;
    }
    
    // 设置RGB/BGR位 (第3位)
    if (rgbOrder) {
        madctl &= ~0x08;  // 清除第3位，设置为RGB模式
    } else {
        madctl |= 0x08;   // 设置第3位，设置为BGR模式
    }
    
    writecommand(ST7789_MADCTL);
    writedata(madctl);
}