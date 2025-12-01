#include "AmebaST7789_DMA.h"
#include <stdlib.h>
#include <string.h>

/* 静态定义 */
uint16_t AmebaST7789_DMA::dmaBuf[2][DMA_BUF_SIZE] = {0};
uint8_t  AmebaST7789_DMA::bufIdx = 0;

/* 底层 DMA 发送 RGB565 块 */
inline void AmebaST7789_DMA::startDMAtransfer(uint16_t *pixels, uint32_t pixels_cnt) {
    digitalWrite(_dcPin, HIGH);                       // 数据模式
    SPI.transfer((uint8_t *)pixels, pixels_cnt * 2, SPI_LAST); // DMA 传输
}

/* 重写 drawBitmap → DMA 整块送出 */
void AmebaST7789_DMA::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color) {
    // ... 边界检查 ...
    setAddress(x, y, x + w - 1, y + h - 1);

    /* 拷贝数据到DMA缓冲区 - 转换为ST7789格式（高字节在前）*/
    uint16_t *dst = dmaBuf[bufIdx];
    const uint16_t *src = (const uint16_t *)color;
    uint32_t pixel_count = w * h;
    
    // 转换字节顺序：从RGB565小端格式转换为ST7789大端格式
    for (uint32_t i = 0; i < pixel_count; i++) {
        uint16_t pixel = src[i];
        dst[i] = ((pixel & 0xFF) << 8) | (pixel >> 8);  // 交换字节
    }
    
    startDMAtransfer(dst, pixel_count);
}

/* 纯色矩形：单颜色→整块 DMA */
void AmebaST7789_DMA::fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x >= _width || y >= _height) return;
    if (x + w > _width)  w = _width  - x;
    if (y + h > _height) h = _height - y;

    setAddress(x, y, x + w - 1, y + h - 1);

    uint16_t *dst = dmaBuf[bufIdx];
    // 交换颜色值的字节顺序
    uint16_t swapped_color = ((color & 0xFF) << 8) | (color >> 8);
    for (uint32_t i = 0; i < (uint32_t)w * h; ++i) dst[i] = swapped_color;
    startDMAtransfer(dst, w * h);
}

/* 单点像素：仍走 DMA（开销可忽略） */
void AmebaST7789_DMA::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    setAddress(x, y, x, y);
    dmaBuf[bufIdx][0] = ((color & 0xFF) << 8) | (color >> 8);  // 交换字节顺序
    startDMAtransfer(dmaBuf[bufIdx], 1);
}