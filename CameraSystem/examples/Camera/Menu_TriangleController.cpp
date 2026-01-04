/*
 * Menu_TriangleController.cpp - 三角形控制器实现
 * 封装三角形位置控制和显示功能
 * 阶段六：菜单模块优化 - 三角形控制器开发
 */

#include "Menu_TriangleController.h"

// 初始化三角形控制器
bool TriangleController::init()
{
    // 初始化当前位置为A
    currentPosition = POSITION_A;
    
    // 绘制初始三角形
    drawTriangle(currentPosition);
    
    Utils_Logger::info("[TriangleController] 初始化完成");
    return true;
}

// 清除指定位置的三角形
void TriangleController::clearTriangle(MenuPosition position)
{
    if (position < POSITION_A || position > POSITION_F) {
        return;
    }
    
    // 使用固定数值定位方式
    int centerY = fixedYPositions[position];
    
    // 三角形顶点坐标
    int x1 = xOffset + triangleSize * 2;  // 右侧顶点
    int y1 = centerY;
    int x2 = xOffset;                      // 左侧顶点1
    int y2 = centerY - triangleSize;
    int x3 = xOffset;                      // 左侧顶点2
    int y3 = centerY + triangleSize;
    
    // 使用黑色线条重新绘制三角形边框来清除
    tftManager.drawLine(x1, y1, x2, y2, ST7789_BLACK);
    tftManager.drawLine(x1, y1, x3, y3, ST7789_BLACK);
    tftManager.drawLine(x2, y2, x3, y3, ST7789_BLACK);
    
    // 额外清除三角形内部可能残留的像素
    int x = xOffset;
    int y = centerY - triangleSize;
    int width = triangleSize * 3;
    int height = triangleSize * 2;
    tftManager.fillRectangle(x, y, width, height, ST7789_BLACK);
}

// 在指定位置绘制三角形
void TriangleController::drawTriangle(MenuPosition position)
{
    if (position < POSITION_A || position > POSITION_F) {
        return;
    }
    
    // 使用固定数值定位方式
    int centerY = fixedYPositions[position];
    
    // 更新三角形顶点坐标
    int x1 = xOffset + triangleSize * 2;  // 右侧顶点
    int y1 = centerY;
    int x2 = xOffset;                      // 左侧顶点1
    int y2 = centerY - triangleSize;
    int x3 = xOffset;                      // 左侧顶点2
    int y3 = centerY + triangleSize;
    
    // 绘制新位置的三角形（白色）
    tftManager.drawLine(x1, y1, x2, y2, ST7789_WHITE);
    tftManager.drawLine(x1, y1, x3, y3, ST7789_WHITE);
    tftManager.drawLine(x2, y2, x3, y3, ST7789_WHITE);
}

// 向上移动三角形
void TriangleController::moveUp()
{
    MenuPosition newPosition = static_cast<MenuPosition>((currentPosition - 1 + 6) % 6);
    Utils_Logger::info("三角形从位置 %c 向上移动到位置: %c", 
                      (char)('A' + currentPosition), 
                      (char)('A' + newPosition));
    
    moveToPosition(newPosition);
}

// 向下移动三角形
void TriangleController::moveDown()
{
    MenuPosition newPosition = static_cast<MenuPosition>((currentPosition + 1) % 6);
    Utils_Logger::info("三角形从位置 %c 向下移动到位置: %c", 
                      (char)('A' + currentPosition), 
                      (char)('A' + newPosition));
    
    moveToPosition(newPosition);
}

// 移动三角形到指定位置
void TriangleController::moveToPosition(MenuPosition position)
{
    if (position >= POSITION_A && position <= POSITION_F && position != currentPosition) {
        // 清除当前位置的三角形
        clearTriangle(currentPosition);
        
        // 更新位置
        currentPosition = position;
        
        // 绘制新位置的三角形
        drawTriangle(currentPosition);
        
        Utils_Logger::info("三角形移动到位置: %c", (char)('A' + position));
    }
}

// 更新三角形显示
void TriangleController::updateDisplay()
{
    // 清除所有可能的位置
    for (int i = POSITION_A; i <= POSITION_F; i++) {
        clearTriangle(static_cast<MenuPosition>(i));
    }
    
    // 绘制当前位置的三角形
    drawTriangle(currentPosition);
}

// 计算指定位置的三角形顶点坐标
void TriangleController::calculateTriangleVertices(MenuPosition position, int& x1, int& y1, int& x2, int& y2, int& x3, int& y3)
{
    if (position < POSITION_A || position > POSITION_F) {
        return;
    }
    
    // 使用固定数值定位方式
    int centerY = fixedYPositions[position];
    
    // 计算三角形顶点坐标
    x1 = xOffset + triangleSize * 2;  // 右侧顶点
    y1 = centerY;
    x2 = xOffset;                      // 左侧顶点1
    y2 = centerY - triangleSize;
    x3 = xOffset;                      // 左侧顶点2
    y3 = centerY + triangleSize;
}

// 执行动画移动
void TriangleController::animateMove(MenuPosition from, MenuPosition to)
{
    if (!animationEnabled || from == to) {
        return;
    }
    
    Utils_Logger::info("执行动画移动: 从位置 %c 到位置 %c", 
                      (char)('A' + from), 
                      (char)('A' + to));
    
    // 简单的动画实现：逐帧移动
    int steps = 10; // 动画步数
    int fromY = fixedYPositions[from];
    int toY = fixedYPositions[to];
    
    for (int i = 0; i <= steps; i++) {
        // 计算中间位置
        int currentY = fromY + (toY - fromY) * i / steps;
        
        // 清除当前位置
        clearTriangle(currentPosition);
        
        // 在中间位置绘制三角形
        int x1 = xOffset + triangleSize * 2;
        int y1 = currentY;
        int x2 = xOffset;
        int y2 = currentY - triangleSize;
        int x3 = xOffset;
        int y3 = currentY + triangleSize;
        
        tftManager.drawLine(x1, y1, x2, y2, ST7789_WHITE);
        tftManager.drawLine(x1, y1, x3, y3, ST7789_WHITE);
        tftManager.drawLine(x2, y2, x3, y3, ST7789_WHITE);
        
        // 延迟以实现动画效果
        delay(animationSpeed / steps);
    }
    
    // 更新最终位置
    currentPosition = to;
}

// 显示三角形
void TriangleController::show()
{
    if (!isVisible) {
        isVisible = true;
        drawTriangle(currentPosition);
        Utils_Logger::info("三角形显示");
    }
}

// 隐藏三角形
void TriangleController::hide()
{
    if (isVisible) {
        isVisible = false;
        clearTriangle(currentPosition);
        Utils_Logger::info("三角形隐藏");
    }
}

// 获取指定位置的X坐标
uint16_t TriangleController::getPositionX(MenuPosition position) const
{
    if (position < POSITION_A || position > POSITION_F) {
        return 0;
    }
    return xOffset + triangleSize * 2; // 返回右侧顶点X坐标
}

// 获取指定位置的Y坐标
uint16_t TriangleController::getPositionY(MenuPosition position) const
{
    if (position < POSITION_A || position > POSITION_F) {
        return 0;
    }
    return fixedYPositions[position];
}

// 清理资源
void TriangleController::cleanup()
{
    // 清除当前显示的三角形
    clearTriangle(currentPosition);
    
    // 重置状态
    currentPosition = POSITION_A;
    isVisible = false;
    animationEnabled = false;
    
    Utils_Logger::info("[TriangleController] 资源清理完成");
}
