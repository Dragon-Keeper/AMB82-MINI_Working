#ifndef _AMEBA_ST7789_DMA_SPI1_H_
#define _AMEBA_ST7789_DMA_SPI1_H_

#include "AmebaST7789_SPI1.h"

class AmebaST7789_DMA_SPI1 : public AmebaST7789_SPI1 {
public:
    AmebaST7789_DMA_SPI1(int csPin, int dcPin, int resetPin)
        : AmebaST7789_SPI1(csPin, dcPin, resetPin) {}

    /* 重写送显函数 → 启用 DMA */
    void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color);
    void fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    
    // 继承父类的setColorOrder方法
    using AmebaST7789_SPI1::setColorOrder;

private:
    void startDMAtransfer(uint16_t *pixels, uint32_t pixels_cnt);

    static constexpr size_t DMA_BUF_SIZE = 320UL * 240UL;
    static uint16_t dmaBuf[2][DMA_BUF_SIZE];
    static uint8_t  bufIdx;
};

#endif