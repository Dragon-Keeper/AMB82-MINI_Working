#include "AmebaILI9341_DMA.h"
#include <stdlib.h>
#include <string.h>

/* 静态定义 */
uint16_t AmebaILI9341_DMA::dmaBuf[2][DMA_BUF_SIZE] = {0};
uint8_t  AmebaILI9341_DMA::bufIdx = 0;

/* 底层 DMA 发送 RGB565 块 */
inline void AmebaILI9341_DMA::startDMAtransfer(uint16_t *pixels, uint32_t pixels_cnt) {
    digitalWrite(_dcPin, HIGH);                       // 数据模式
    SPI.transfer((uint8_t *)pixels, pixels_cnt * 2, SPI_LAST); // DMA 传输
}

/* 重写 drawBitmap → DMA 整块送出 */
void AmebaILI9341_DMA::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color) {
    if (x >= _width || y >= _height) return;
    if (x + w > _width)  w = _width  - x;
    if (y + h > _height) h = _height - y;

    setAddress(x, y, x + w - 1, y + h - 1);

    /* 拷贝数据到DMA缓冲区并交换字节顺序 */
    uint16_t *dst = dmaBuf[bufIdx];
    const uint16_t *src = (const uint16_t *)color;
    uint32_t pixel_count = w * h;
    
    // 交换每个像素的字节顺序以匹配ILI9341格式
    for (uint32_t i = 0; i < pixel_count; i++) {
        uint16_t pixel = src[i];
        // 同时调整颜色通道：降低红色，增加绿色，保持蓝色
        uint8_t r = (pixel >> 11) & 0x1F;
        uint8_t g = (pixel >> 5) & 0x3F;
        uint8_t b = pixel & 0x1F;
        
        // 调整颜色平衡：R*0.9, G*1.1, B*1.0
        r = (r * 9) / 10;  // 降低红色强度
        if (r > 31) r = 31;
        g = (g * 11) / 10;  // 增加绿色强度
        if (g > 63) g = 63;
        
        pixel = (r << 11) | (g << 5) | b;
        dst[i] = ((pixel & 0xFF) << 8) | (pixel >> 8);
    }
    
    startDMAtransfer(dst, pixel_count);
}

/* 纯色矩形：单颜色→整块 DMA */
void AmebaILI9341_DMA::fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
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
void AmebaILI9341_DMA::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    setAddress(x, y, x, y);
    dmaBuf[bufIdx][0] = color;
    startDMAtransfer(dmaBuf[bufIdx], 1);
}