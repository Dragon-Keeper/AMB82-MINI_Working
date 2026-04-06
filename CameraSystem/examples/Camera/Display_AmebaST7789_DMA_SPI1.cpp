/*
 * Display_AmebaST7789_DMA_SPI1.cpp - ST7789 DMA SPI1显示驱动实现
 * 封装ST7789显示屏的SPI1 DMA接口驱动功能
 * 阶段三：显示模块开发 - ST7789 DMA驱动实现
 */

#include "Display_AmebaST7789_DMA_SPI1.h"
#include <stdlib.h>
#include <string.h>

/* 静态定义 */
uint16_t AmebaST7789_DMA_SPI1::dmaBuf[2][DMA_BUF_SIZE] = {0};
uint8_t  AmebaST7789_DMA_SPI1::bufIdx = 0;

/* 底层 DMA 发送 RGB565 块 - 完全同步版本 */
inline void AmebaST7789_DMA_SPI1::startDMAtransfer(uint16_t *pixels, uint32_t pixels_cnt) {
    digitalWrite(_dcPin, HIGH);                       // 数据模式
    SPI1.transfer((uint8_t *)pixels, pixels_cnt * 2, SPI_LAST); // DMA 传输
    
    // 使用足够长的固定等待时间，确保任何大小的数据都能传输完成
    // SPI频率40MHz，最大数据量320*240*2=153600字节，需要约3.84ms
    // 使用5ms可以覆盖所有情况，包括系统开销
    delayMicroseconds(5000);
}

/* 重写 drawBitmap → DMA 整块送出 */
void AmebaST7789_DMA_SPI1::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color) {
    // 边界检查
    if ((x >= _width) || (y >= _height)) return;
    if (x + w > _width)  w = _width  - x;
    if (y + h > _height) h = _height - y;
    
    setAddress(x, y, x + w - 1, y + h - 1);

    /* 拷贝数据到DMA缓冲区，并转换字节顺序 */
    /* JPEGDEC库输出小端RGB565，ST7789需要大端RGB565 */
    uint16_t *dst = dmaBuf[0];
    const uint16_t *src = (const uint16_t *)color;
    uint32_t pixel_count = (uint32_t)w * h;
    
    // 转换字节顺序：从RGB565小端格式转换为ST7789大端格式
    for (uint32_t i = 0; i < pixel_count; i++) {
        uint16_t pixel = src[i];
        dst[i] = ((pixel & 0xFF) << 8) | (pixel >> 8);  // 交换高低字节
    }
    
    startDMAtransfer(dst, pixel_count);
}

/* 纯色矩形：单颜色→整块 DMA */
void AmebaST7789_DMA_SPI1::fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x >= _width || y >= _height) return;
    if (x + w > _width)  w = _width  - x;
    if (y + h > _height) h = _height - y;

    setAddress(x, y, x + w - 1, y + h - 1);

    /* 始终使用缓冲区0 */
    uint16_t *dst = dmaBuf[0];
    uint32_t pixel_count = (uint32_t)w * h;
    for (uint32_t i = 0; i < pixel_count; ++i) dst[i] = color;
    
    startDMAtransfer(dst, pixel_count);
}

/* 单点像素：仍走 DMA（开销可忽略）*/
void AmebaST7789_DMA_SPI1::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    setAddress(x, y, x, y);
    
    dmaBuf[0][0] = color;
    startDMAtransfer(dmaBuf[0], 1);
}
