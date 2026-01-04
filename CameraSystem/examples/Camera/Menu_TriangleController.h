/*
 * Menu_TriangleController.h - 三角形控制器头文件
 * 封装三角形位置控制和显示功能
 * 阶段六：菜单模块优化 - 三角形控制器开发
 */

#ifndef MENU_TRIANGLE_CONTROLLER_H
#define MENU_TRIANGLE_CONTROLLER_H

#include <Arduino.h>
#include "Display_TFTManager.h"
#include "Utils_Logger.h"

class TriangleController
{
public:
    // 菜单位置枚举
    typedef enum {
        POSITION_A = 0,
        POSITION_B = 1,
        POSITION_C = 2,
        POSITION_D = 3,
        POSITION_E = 4,
        POSITION_F = 5
    } MenuPosition;
    
private:
    Display_TFTManager &tftManager; // 使用TFT管理器实例的引用
    
    // 三角形位置定义
    MenuPosition currentPosition = POSITION_A;
    
    // 三角形尺寸和位置参数
    const int triangleSize = 7; // 三角形大小 (原来的70%)
    const int xOffset = 40; // X轴偏移量
    
    // 固定Y轴坐标值 (像素)
    const int fixedYPositions[6] = {60, 90, 120, 150, 180, 210};
    
    // 动画控制参数
    bool animationEnabled = false;
    uint16_t animationSpeed = 100; // 毫秒
    
    // 显示状态
    bool isVisible = true;
    
    // 清除指定位置的三角形
    void clearTriangle(MenuPosition position);
    
    // 在指定位置绘制三角形
    void drawTriangle(MenuPosition position);
    
    // 计算指定位置的坐标
    void calculateTriangleVertices(MenuPosition position, int& x1, int& y1, int& x2, int& y2, int& x3, int& y3);
    
    // 执行动画移动
    void animateMove(MenuPosition from, MenuPosition to);
    
public:
    // 构造函数
    TriangleController(Display_TFTManager &tftManager) : tftManager(tftManager) {}
    
    // 初始化三角形控制器
    bool init();
    
    // 三角形移动控制
    void moveToPosition(MenuPosition position);
    void moveUp();
    void moveDown();
    
    // 显示控制
    void show();
    void hide();
    void updateDisplay();
    
    // 当前位置管理
    MenuPosition getCurrentPosition() const { return currentPosition; }
    void setCurrentPosition(MenuPosition position) { moveToPosition(position); }
    
    // 动画效果控制
    void enableAnimation(bool enable) { animationEnabled = enable; }
    void setAnimationSpeed(uint16_t ms) { animationSpeed = ms; }
    
    // 坐标计算
    uint16_t getPositionX(MenuPosition position) const;
    uint16_t getPositionY(MenuPosition position) const;
    
    // 重置三角形位置到A
    void resetPosition() { moveToPosition(POSITION_A); }
    
    // 清理资源
    void cleanup();
};

#endif // MENU_TRIANGLE_CONTROLLER_H
