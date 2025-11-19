#include "AmebaILI9341.h"
#include "font5x7.h"
#include <SPI.h>
#include <stdlib.h>

/* 构造函数 */
AmebaILI9341::AmebaILI9341(int csPin, int dcPin, int resetPin) {
    _csPin    = csPin;
    _dcPin    = dcPin;
    _resetPin = resetPin;
    _width    = ILI9341_TFTWIDTH;
    _height   = ILI9341_TFTHEIGHT;
    cursor_x  = 0;
    cursor_y  = 0;
    foreground = ILI9341_WHITE;
    background = ILI9341_BLACK;
    fontsize   = 1;
    rotation   = 0;
}

/* 初始化序列 */
void AmebaILI9341::begin(void) {
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, LOW);
    delay(10);
    digitalWrite(_resetPin, HIGH);
    delay(120);

    pinMode(_dcPin, OUTPUT);
    SPI.begin();          // Arduino SPI0
    reset();

    const uint8_t *seq = (const uint8_t[]){
        0xEF,3,0x03,0x80,0x02,
        0xCF,3,0x00,0xC1,0x30,
        0xED,4,0x64,0x03,0x12,0x81,
        0xE8,3,0x85,0x00,0x78,
        0xCB,5,0x39,0x2C,0x00,0x34,0x02,
        0xF7,1,0x20,
        0xEA,2,0x00,0x00,
        0xC0,1,0x21,               // PWCTR1  电源控制（降低红色通道增益）
        0xC1,1,0x0C,               // PWCTR2  电源控制（调整绿色通道）
        0xC5,2,0x3E,0x28,          // VMCTR1  VCOM电压控制
        0xC7,1,0xC0,               // VMCTR2  VCOM偏移调整（修正颜色偏差）
        // 不设置MADCTL，由setRotation()控制颜色顺序
        0x3A,1,0x55,               // PIXFMT  RGB565
        0xB1,2,0x00,0x18,          // FRMCTR1
        0xB6,3,0x08,0x82,0x27,     // DFUNCTR
        0xF2,1,0x00,               // 3Gamma disable
        0x26,1,0x01,               // Gamma set
        // 30 bytes Gamma +/-  - 使用HSD028屏幕的正确伽马值
        0xE0,15,0x0F,0x27,0x24,0x0C,0x10,0x08,0x55,0x87,0x45,0x08,0x14,0x07,0x13,0x08,0x00,  // 正伽马（HSD028屏幕）
        0xE1,15,0x00,0x0F,0x12,0x05,0x11,0x06,0x25,0x34,0x37,0x01,0x08,0x07,0x2B,0x34,0x0F,  // 负伽马（HSD028屏幕）
        0x11,0,                      // Exit Sleep
        0x29,0                       // Display ON
    };
    for (const uint8_t *p = seq; *p; ) {
        uint8_t cmd = *p++;
        uint8_t len = *p++;
        writecommand(cmd);
        for (uint8_t i = 0; i < len; i++) writedata(*p++);
    }
    delay(120);
}

void AmebaILI9341::reset(void) {
    digitalWrite(_resetPin, HIGH);
    delay(5);
    digitalWrite(_resetPin, LOW);
    delay(20);
    digitalWrite(_resetPin, HIGH);
    delay(150);
}

/* 底层命令/数据 */
inline void AmebaILI9341::writecommand(uint8_t c) {
    digitalWrite(_dcPin, LOW);
    SPI.transfer(c);
}
inline void AmebaILI9341::writedata(uint8_t d) {
    digitalWrite(_dcPin, HIGH);
    SPI.transfer(d);
}
void AmebaILI9341::writedata(uint8_t *d, int len) {
    digitalWrite(_dcPin, HIGH);
    SPI.transfer(d, len, SPI_LAST);
}

/* 窗口设置 */
void AmebaILI9341::setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    writecommand(ILI9341_CASET);
    writedata(x0 >> 8); writedata(x0 & 0xFF);
    writedata(x1 >> 8); writedata(x1 & 0xFF);

    writecommand(ILI9341_PASET);
    writedata(y0 >> 8); writedata(y0 & 0xFF);
    writedata(y1 >> 8); writedata(y1 & 0xFF);

    writecommand(ILI9341_RAMWR);
}

/* 旋转 */
void AmebaILI9341::setRotation(uint8_t m) {
    rotation = m % 4;
    uint8_t madctl = 0;
    switch (rotation) {
        case 0: madctl = ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR;  // 默认BGR模式，与硬件匹配
                _width = ILI9341_TFTWIDTH; _height = ILI9341_TFTHEIGHT; break;
        case 1: madctl = ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR;  // 默认BGR模式，与硬件匹配
                _width = ILI9341_TFTHEIGHT; _height = ILI9341_TFTWIDTH; break;
        case 2: madctl = ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR;  // 默认BGR模式，与硬件匹配
                _width = ILI9341_TFTWIDTH; _height = ILI9341_TFTHEIGHT; break;
        case 3: madctl = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR;  // 默认BGR模式，与硬件匹配
                _width = ILI9341_TFTHEIGHT; _height = ILI9341_TFTWIDTH; break;
    }
    writecommand(ILI9341_MADCTL);
    writedata(madctl);
}

/* 基础图形 */
void AmebaILI9341::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height)) return;
    setAddress(x, y, x, y);
    // 交换字节顺序以匹配ILI9341的RGB565格式
    writedata(color & 0xFF); writedata(color >> 8);
}
void AmebaILI9341::fillScreen(uint16_t color) {
    fillRectangle(0, 0, _width, _height, color);
}
void AmebaILI9341::clr(void) {
    fillScreen(background);
}

void AmebaILI9341::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (y < 0 || y >= _height || x >= _width) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > _width) w = _width - x;
    if (w <= 0) return;
    setAddress(x, y, x + w - 1, y);
    uint8_t lo = color & 0xFF, hi = color >> 8;  // 交换字节顺序
    digitalWrite(_dcPin, HIGH);
    for (int i = 0; i < w; ++i) { SPI.transfer(lo); SPI.transfer(hi); }
}
void AmebaILI9341::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (x < 0 || x >= _width || y >= _height) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > _height) h = _height - y;
    if (h <= 0) return;
    setAddress(x, y, x, y + h - 1);
    uint8_t lo = color & 0xFF, hi = color >> 8;  // 交换字节顺序
    digitalWrite(_dcPin, HIGH);
    for (int i = 0; i < h; ++i) { SPI.transfer(lo); SPI.transfer(hi); }
}
void AmebaILI9341::fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if ((x >= _width) || (y >= _height)) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > _width)  w = _width  - x;
    if (y + h > _height) h = _height - y;
    setAddress(x, y, x + w - 1, y + h - 1);
    uint32_t n = (uint32_t)w * h;
    uint8_t  lo = color & 0xFF, hi = color >> 8;  // 交换字节顺序
    digitalWrite(_dcPin, HIGH);
    for (uint32_t i = 0; i < n; ++i) { SPI.transfer(lo); SPI.transfer(hi); }
}
void AmebaILI9341::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color) {
    if ((x >= _width) || (y >= _height)) return;
    if (x + w > _width)  w = _width  - x;
    if (y + h > _height) h = _height - y;
    setAddress(x, y, x + w - 1, y + h - 1);
    digitalWrite(_dcPin, HIGH);
    // 使用DMA传输，但需要在传输前交换字节顺序并调整颜色平衡
    uint16_t *temp_buffer = (uint16_t *)malloc(w * h * sizeof(uint16_t));
    if (temp_buffer) {
        for (int i = 0; i < w * h; i++) {
            uint16_t pixel = color[i];
            // 调整颜色通道：降低红色，增加绿色，保持蓝色
            uint8_t r = (pixel >> 11) & 0x1F;
            uint8_t g = (pixel >> 5) & 0x3F;
            uint8_t b = pixel & 0x1F;
            
            // 调整颜色平衡：R*0.9, G*1.1, B*1.0
            r = (r * 9) / 10;  // 降低红色强度
            if (r > 31) r = 31;
            g = (g * 11) / 10;  // 增加绿色强度
            if (g > 63) g = 63;
            
            pixel = (r << 11) | (g << 5) | b;
            temp_buffer[i] = ((pixel & 0xFF) << 8) | (pixel >> 8);
        }
        SPI.transfer((uint8_t *)temp_buffer, w * h * 2, SPI_LAST);
        free(temp_buffer);
    } else {
        // 如果内存分配失败，使用逐像素方法并调整颜色
        for (int i = 0; i < w * h; i++) {
            uint16_t pixel = color[i];
            // 调整颜色通道
            uint8_t r = (pixel >> 11) & 0x1F;
            uint8_t g = (pixel >> 5) & 0x3F;
            uint8_t b = pixel & 0x1F;
            
            r = (r * 9) / 10;
            if (r > 31) r = 31;
            g = (g * 11) / 10;
            if (g > 63) g = 63;
            
            pixel = (r << 11) | (g << 5) | b;
            SPI.transfer(pixel & 0xFF);
            SPI.transfer(pixel >> 8);
        }
    }
}

/* 画线/矩形/圆 实现 */
void AmebaILI9341::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
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
void AmebaILI9341::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    drawLine(x0, y0, x1, y1, foreground);
}
void AmebaILI9341::drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
}
void AmebaILI9341::drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h) {
    drawRectangle(x, y, w, h, foreground);
}
void AmebaILI9341::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
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
void AmebaILI9341::drawCircle(int16_t x0, int16_t y0, int16_t r) {
    drawCircle(x0, y0, r, foreground);
}

/* 字符 / Print */
void AmebaILI9341::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
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
void AmebaILI9341::drawChar(unsigned char c) {
    drawChar(cursor_x, cursor_y, c, foreground, background, fontsize);
    cursor_x += fontsize * 6;
}
size_t AmebaILI9341::write(uint8_t c) {
    if (c == '\n') { cursor_x = 0; cursor_y += fontsize * 8; }
    else if (c != '\r') drawChar(c);
    return 1;
}

/* 光标 / 颜色 */
void AmebaILI9341::setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }

/* 设置RGB/BGR颜色顺序 */
void AmebaILI9341::setColorOrder(bool rgbOrder) {
    uint8_t madctl = 0;
    switch (rotation) {
        case 0: madctl = ILI9341_MADCTL_MX | (rgbOrder ? ILI9341_MADCTL_RGB : ILI9341_MADCTL_BGR); break;
        case 1: madctl = ILI9341_MADCTL_MV | (rgbOrder ? ILI9341_MADCTL_RGB : ILI9341_MADCTL_BGR); break;
        case 2: madctl = ILI9341_MADCTL_MY | (rgbOrder ? ILI9341_MADCTL_RGB : ILI9341_MADCTL_BGR); break;
        case 3: madctl = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | (rgbOrder ? ILI9341_MADCTL_RGB : ILI9341_MADCTL_BGR); break;
    }
    writecommand(ILI9341_MADCTL);
    writedata(madctl);
}