/*
 * Display_GraphicsPrimitives.h - 图形绘制原语模块头文件
 * 封装高级图形绘制功能，如三角形、圆形等
 * 阶段三：显示模块开发
 */

#ifndef DISPLAY_GRAPHICS_PRIMITIVES_H
#define DISPLAY_GRAPHICS_PRIMITIVES_H

#include <Arduino.h>
#include "Display_TFTManager.h"

class Display_GraphicsPrimitives
{
private:
    Display_TFTManager* m_tftManager;  // TFT管理器指针
    bool m_initialized;                // 初始化状态标志

public:
    // 构造函数
    Display_GraphicsPrimitives();
    
    // 初始化函数
    bool begin(Display_TFTManager* tftManager);
    
    // 基本图形绘制函数
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color);
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color);
    
    // 三角形移动相关功能（针对Camera.ino的特殊需求）
    void drawTriangleAtPosition(int position, int xOffset, uint16_t color);
    void clearTriangleAtPosition(int position, int xOffset);
    
    // 状态检查
    bool isInitialized() const;
    
private:
    // 内部辅助函数
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    
    // 三角形位置计算
    void calculateTriangleVertices(int position, int xOffset, int& x1, int& y1, int& x2, int& y2, int& x3, int& y3);
};

#endif // DISPLAY_GRAPHICS_PRIMITIVES_H