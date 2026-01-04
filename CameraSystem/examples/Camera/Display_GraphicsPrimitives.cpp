/*
 * Display_GraphicsPrimitives.cpp - 图形绘制原语模块实现文件
 * 封装高级图形绘制功能，如三角形、圆形等
 * 阶段三：显示模块开发
 */

#include "Display_GraphicsPrimitives.h"
#include "Utils_Logger.h"

// 构造函数
Display_GraphicsPrimitives::Display_GraphicsPrimitives()
    : m_tftManager(nullptr), m_initialized(false)
{
    Utils_Logger::info("[GraphicsPrimitives] 图形绘制原语模块构造函数调用");
}

// 初始化函数
bool Display_GraphicsPrimitives::begin(Display_TFTManager* tftManager)
{
    if (tftManager == nullptr) {
        Utils_Logger::error("[GraphicsPrimitives] 错误：TFT管理器指针为空");
        return false;
    }
    
    if (!tftManager->isInitialized()) {
        Utils_Logger::error("[GraphicsPrimitives] 错误：TFT管理器未初始化");
        return false;
    }
    
    m_tftManager = tftManager;
    m_initialized = true;
    
    Utils_Logger::info("[GraphicsPrimitives] 图形绘制原语模块初始化成功");
    return true;
}

// 绘制三角形
void Display_GraphicsPrimitives::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    if (!m_initialized || m_tftManager == nullptr) {
        Utils_Logger::error("[GraphicsPrimitives] 错误：模块未初始化，无法绘制三角形");
        return;
    }
    
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

// 填充三角形
void Display_GraphicsPrimitives::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    if (!m_initialized || m_tftManager == nullptr) {
        Utils_Logger::error("[GraphicsPrimitives] 错误：模块未初始化，无法填充三角形");
        return;
    }
    
    // 简单实现：使用线扫描算法填充三角形
    // 这里使用一个简化的实现，实际项目中可能需要更复杂的算法
    
    // 找到三角形的边界框
    int16_t minX = min(x0, min(x1, x2));
    int16_t maxX = max(x0, max(x1, x2));
    int16_t minY = min(y0, min(y1, y2));
    int16_t maxY = max(y0, max(y1, y2));
    
    // 在边界框内逐行填充
    for (int16_t y = minY; y <= maxY; y++) {
        for (int16_t x = minX; x <= maxX; x++) {
            // 检查点是否在三角形内（使用重心坐标法）
            int32_t denominator = ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
            if (denominator == 0) continue;
            
            int32_t alpha = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / denominator;
            int32_t beta = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / denominator;
            int32_t gamma = 1 - alpha - beta;
            
            if (alpha >= 0 && beta >= 0 && gamma >= 0) {
                m_tftManager->drawLine(x, y, x, y, color);
            }
        }
    }
}

// 绘制圆形
void Display_GraphicsPrimitives::drawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color)
{
    if (!m_initialized || m_tftManager == nullptr) {
        Utils_Logger::error("[GraphicsPrimitives] 错误：模块未初始化，无法绘制圆形");
        return;
    }
    
    int16_t x = radius;
    int16_t y = 0;
    int16_t decision = 1 - radius;
    
    while (x >= y) {
        m_tftManager->drawLine(x0 + x, y0 + y, x0 + x, y0 + y, color);
        m_tftManager->drawLine(x0 - x, y0 + y, x0 - x, y0 + y, color);
        m_tftManager->drawLine(x0 + x, y0 - y, x0 + x, y0 - y, color);
        m_tftManager->drawLine(x0 - x, y0 - y, x0 - x, y0 - y, color);
        m_tftManager->drawLine(x0 + y, y0 + x, x0 + y, y0 + x, color);
        m_tftManager->drawLine(x0 - y, y0 + x, x0 - y, y0 + x, color);
        m_tftManager->drawLine(x0 + y, y0 - x, x0 + y, y0 - x, color);
        m_tftManager->drawLine(x0 - y, y0 - x, x0 - y, y0 - x, color);
        
        y++;
        if (decision <= 0) {
            decision += 2 * y + 1;
        } else {
            x--;
            decision += 2 * (y - x) + 1;
        }
    }
}

// 填充圆形
void Display_GraphicsPrimitives::fillCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color)
{
    if (!m_initialized || m_tftManager == nullptr) {
        Utils_Logger::error("[GraphicsPrimitives] 错误：模块未初始化，无法填充圆形");
        return;
    }
    
    for (int16_t y = -radius; y <= radius; y++) {
        for (int16_t x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                m_tftManager->drawLine(x0 + x, y0 + y, x0 + x, y0 + y, color);
            }
        }
    }
}

// 绘制圆角矩形
void Display_GraphicsPrimitives::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
    if (!m_initialized || m_tftManager == nullptr) {
        Utils_Logger::error("[GraphicsPrimitives] 错误：模块未初始化，无法绘制圆角矩形");
        return;
    }
    
    // 绘制四条直线边
    m_tftManager->drawLine(x + radius, y, x + w - radius, y, color); // 上边
    m_tftManager->drawLine(x + radius, y + h, x + w - radius, y + h, color); // 下边
    m_tftManager->drawLine(x, y + radius, x, y + h - radius, color); // 左边
    m_tftManager->drawLine(x + w, y + radius, x + w, y + h - radius, color); // 右边
    
    // 绘制四个圆角
    drawCircle(x + radius, y + radius, radius, color); // 左上角
    drawCircle(x + w - radius, y + radius, radius, color); // 右上角
    drawCircle(x + radius, y + h - radius, radius, color); // 左下角
    drawCircle(x + w - radius, y + h - radius, radius, color); // 右下角
}

// 填充圆角矩形
void Display_GraphicsPrimitives::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
    if (!m_initialized || m_tftManager == nullptr) {
        Utils_Logger::error("[GraphicsPrimitives] 错误：模块未初始化，无法填充圆角矩形");
        return;
    }
    
    // 填充矩形主体部分
    fillRect(x + radius, y, w - 2 * radius, h, color);
    fillRect(x, y + radius, w, h - 2 * radius, color);
    
    // 填充四个圆角区域
    fillCircle(x + radius, y + radius, radius, color); // 左上角
    fillCircle(x + w - radius, y + radius, radius, color); // 右上角
    fillCircle(x + radius, y + h - radius, radius, color); // 左下角
    fillCircle(x + w - radius, y + h - radius, radius, color); // 右下角
}

// 绘制特定位置的三角形（针对Camera.ino的需求）
void Display_GraphicsPrimitives::drawTriangleAtPosition(int position, int xOffset, uint16_t color)
{
    if (!m_initialized || m_tftManager == nullptr) {
        Utils_Logger::error("[GraphicsPrimitives] 错误：模块未初始化，无法绘制位置三角形");
        return;
    }
    
    int x1, y1, x2, y2, x3, y3;
    calculateTriangleVertices(position, xOffset, x1, y1, x2, y2, x3, y3);
    drawTriangle(x1, y1, x2, y2, x3, y3, color);
}

// 清除特定位置的三角形
void Display_GraphicsPrimitives::clearTriangleAtPosition(int position, int xOffset)
{
    if (!m_initialized || m_tftManager == nullptr) {
        Utils_Logger::error("[GraphicsPrimitives] 错误：模块未初始化，无法清除位置三角形");
        return;
    }
    
    int x1, y1, x2, y2, x3, y3;
    calculateTriangleVertices(position, xOffset, x1, y1, x2, y2, x3, y3);
    
    // 使用黑色线条重新绘制三角形边框来清除（覆盖原线条）
    drawTriangle(x1, y1, x2, y2, x3, y3, ST7789_BLACK);
    
    // 额外清除三角形内部可能残留的像素（使用矩形清除）
    int x = 40;  // 从屏幕左侧开始，向左移动40像素
    int y = y2;  // 使用三角形上顶点作为Y坐标
    int width = (x1 - x2) * 3;  // 三角形宽度的3倍
    int height = (y3 - y2) * 2; // 三角形高度的2倍
    fillRect(x, y, width, height, ST7789_BLACK);
}

// 状态检查
bool Display_GraphicsPrimitives::isInitialized() const
{
    return m_initialized;
}

// ========== 私有辅助函数 ==========

// 绘制直线（内部使用）
void Display_GraphicsPrimitives::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    if (m_tftManager != nullptr) {
        m_tftManager->drawLine(x0, y0, x1, y1, color);
    }
}

// 填充矩形（内部使用）
void Display_GraphicsPrimitives::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if (m_tftManager != nullptr) {
        m_tftManager->fillRectangle(x, y, w, h, color);
    }
}

// 计算三角形顶点坐标（针对Camera.ino的特殊需求）
void Display_GraphicsPrimitives::calculateTriangleVertices(int position, int xOffset, int& x1, int& y1, int& x2, int& y2, int& x3, int& y3)
{
    // 固定的Y位置数组（与Camera.ino保持一致）
    int fixedYPositions[6] = {60, 90, 120, 150, 180, 210};
    int centerY = fixedYPositions[position];
    int triangleSizeValue = 7;
    
    x1 = xOffset + triangleSizeValue * 2;  // 右侧顶点
    y1 = centerY;
    x2 = xOffset;                          // 左侧顶点1
    y2 = centerY - triangleSizeValue;
    x3 = xOffset;                          // 左侧顶点2
    y3 = centerY + triangleSizeValue;
}