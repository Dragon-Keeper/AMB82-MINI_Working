/*
 * Display_TFTManager.h - TFT显示管理器模块
 * 封装AmebaST7789_DMA_SPI1显示操作，提供统一的TFT管理接口
 * 阶段三：显示模块开发
 */

#ifndef DISPLAY_TFTMANAGER_H
#define DISPLAY_TFTMANAGER_H

#include <Arduino.h>
#include "Display_AmebaST7789_DMA_SPI1.h"
#include "Shared_GlobalDefines.h"

// TFT颜色定义
#define ST7789_BLACK       0x0000
#define ST7789_BLUE        0x001F
#define ST7789_RED         0xF800
#define ST7789_GREEN       0x07E0
#define ST7789_CYAN        0x07FF
#define ST7789_MAGENTA     0xF81F
#define ST7789_YELLOW      0xFFE0
#define ST7789_WHITE       0xFFFF

class Display_TFTManager {
public:
    // 构造函数
    Display_TFTManager();
    
    // 初始化函数
    bool begin();
    
    // 基本显示操作
    void fillScreen(uint16_t color);
    void setRotation(uint8_t rotation);
    
    // 图形绘制操作
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *bitmap);
    
    // 文本操作
    void setTextSize(uint8_t size);
    void setCursor(int16_t x, int16_t y);
    void setTextColor(uint16_t color);
    void setTextColor(uint16_t color, uint16_t bgColor);
    void print(const char* text);
    void println(const char* text);
    
    // 获取TFT对象引用（用于特殊操作）
    AmebaST7789_DMA_SPI1& getTFT();
    
    // 状态检查
    bool isInitialized() const;
    
    // 颜色顺序控制
    void setColorOrder(bool rgbOrder);
    
    // 背光控制
    void setBacklight(bool enabled);
    
    // 资源清理
    void cleanup();
    
private:
    AmebaST7789_DMA_SPI1 m_tft;  // TFT显示对象
    bool m_initialized;          // 初始化状态标志
    
    // 初始化SPI配置
    void initSPI();
};

// 全局TFT管理器实例声明
extern Display_TFTManager tftManager;

#endif