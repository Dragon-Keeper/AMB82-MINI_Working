#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <Arduino.h>
#include "AmebaST7789_DMA_SPI1.h"

class MenuManager
{
private:
    AmebaST7789_DMA_SPI1 &tft; // 使用外部显示屏实例的引用
    
    // 菜单页面数据
    const uint16_t *currentMenuImage = nullptr;
    size_t currentMenuSize = 0;
    int menuWidth = 0;
    int menuHeight = 0;
    
    // 菜单页面数组
    static const int MAX_MENU_PAGES = 10;
    const uint16_t *menuPages[MAX_MENU_PAGES];
    size_t menuSizes[MAX_MENU_PAGES];
    int menuWidths[MAX_MENU_PAGES];
    int menuHeights[MAX_MENU_PAGES];
    int menuCount = 0;
    int currentPageIndex = 0;
    
    // 旋转编码器状态
    int encoderPosition = 0;
    int lastEncoderPosition = 0;
    bool encoderButtonPressed = false;
    bool lastEncoderButtonState = HIGH;
    
    // 引脚定义
    int encoderCLKPin;
    int encoderDTPin;
    int encoderSWPin;
    
public:
    // 通过引用接收外部TFT实例和编码器引脚
    MenuManager(AmebaST7789_DMA_SPI1 &display, int clkPin, int dtPin, int swPin) 
        : tft(display), encoderCLKPin(clkPin), encoderDTPin(dtPin), encoderSWPin(swPin) {}

    // 初始化菜单管理器
    bool init() {
        // 初始化编码器引脚
        pinMode(encoderCLKPin, INPUT_PULLUP);
        pinMode(encoderDTPin, INPUT_PULLUP);
        pinMode(encoderSWPin, INPUT_PULLUP);
        
        // 读取初始状态
        lastEncoderButtonState = digitalRead(encoderSWPin);
        
        Serial.println("[MenuManager] 初始化完成");
        return true;
    }

    // 添加菜单页面
    void addMenuPage(const uint16_t *imageData, size_t dataSize, int width, int height) {
        if (menuCount < MAX_MENU_PAGES) {
            menuPages[menuCount] = imageData;
            menuSizes[menuCount] = dataSize;
            menuWidths[menuCount] = width;
            menuHeights[menuCount] = height;
            menuCount++;
            
            Serial.print("[MenuManager] 添加菜单页面 ");
            Serial.print(menuCount);
            Serial.print(": ");
            Serial.print(width);
            Serial.print("x");
            Serial.print(height);
            Serial.print(" (");
            Serial.print(dataSize);
            Serial.println(" 字节)");
        } else {
            Serial.println("[MenuManager] 错误: 菜单页面数量已达上限");
        }
    }

    // 显示当前菜单页面
    void showCurrentMenu() {
        if (menuCount == 0) {
            Serial.println("[MenuManager] 错误: 没有可用的菜单页面");
            return;
        }
        
        if (currentPageIndex < 0 || currentPageIndex >= menuCount) {
            currentPageIndex = 0;
        }
        
        currentMenuImage = menuPages[currentPageIndex];
        currentMenuSize = menuSizes[currentPageIndex];
        menuWidth = menuWidths[currentPageIndex];
        menuHeight = menuHeights[currentPageIndex];
        
        if (!currentMenuImage || currentMenuSize == 0) {
            Serial.println("[MenuManager] 错误: 当前菜单页面数据无效");
            return;
        }
        
        if (menuWidth != tft.getWidth() || menuHeight != tft.getHeight()) {
            Serial.print("[MenuManager] 警告: 菜单尺寸不匹配 ");
            Serial.print(menuWidth);
            Serial.print("x");
            Serial.print(menuHeight);
            Serial.print(" vs 屏幕 ");
            Serial.print(tft.getWidth());
            Serial.print("x");
            Serial.println(tft.getHeight());
        }
        
        Serial.print("[MenuManager] 显示菜单页面 ");
        Serial.print(currentPageIndex + 1);
        Serial.print("/");
        Serial.println(menuCount);
        
        // 使用DMA方式显示图像
        tft.drawBitmap(0, 0, menuWidth, menuHeight, currentMenuImage);
    }

    // 切换到下一个菜单页面
    void nextMenu() {
        if (menuCount > 1) {
            currentPageIndex = (currentPageIndex + 1) % menuCount;
            Serial.print("[MenuManager] 切换到下一个页面: ");
            Serial.print(currentPageIndex + 1);
            Serial.print("/");
            Serial.println(menuCount);
            showCurrentMenu();
        }
    }

    // 切换到上一个菜单页面
    void previousMenu() {
        if (menuCount > 1) {
            currentPageIndex = (currentPageIndex - 1 + menuCount) % menuCount;
            Serial.print("[MenuManager] 切换到上一个页面: ");
            Serial.print(currentPageIndex + 1);
            Serial.print("/");
            Serial.println(menuCount);
            showCurrentMenu();
        }
    }

    // 直接切换到指定页面
    void switchToPage(int pageIndex) {
        if (pageIndex >= 0 && pageIndex < menuCount) {
            currentPageIndex = pageIndex;
            Serial.print("[MenuManager] 切换到指定页面: ");
            Serial.print(currentPageIndex + 1);
            Serial.print("/");
            Serial.println(menuCount);
            showCurrentMenu();
        } else {
            Serial.println("[MenuManager] 错误: 页面索引超出范围");
        }
    }

    // 处理编码器旋转
    void handleEncoderRotation() {
        int clkState = digitalRead(encoderCLKPin);
        int dtState = digitalRead(encoderDTPin);
        
        // 检测旋转方向
        if (clkState != lastEncoderPosition) {
            if (dtState != clkState) {
                // 顺时针旋转
                nextMenu();
            } else {
                // 逆时针旋转
                previousMenu();
            }
        }
        
        lastEncoderPosition = clkState;
    }

    // 处理编码器按钮
    void handleEncoderButton() {
        bool currentButtonState = digitalRead(encoderSWPin);
        
        // 检测按钮按下（下降沿）
        if (currentButtonState == LOW && lastEncoderButtonState == HIGH) {
            encoderButtonPressed = true;
            Serial.println("[MenuManager] 编码器按钮按下");
        } else if (currentButtonState == HIGH && lastEncoderButtonState == LOW) {
            encoderButtonPressed = false;
            Serial.println("[MenuManager] 编码器按钮释放");
        }
        
        lastEncoderButtonState = currentButtonState;
    }

    // 更新菜单管理器状态
    void update() {
        handleEncoderRotation();
        handleEncoderButton();
    }

    // 获取当前页面索引
    int getCurrentPageIndex() const {
        return currentPageIndex;
    }

    // 获取总页面数
    int getTotalPages() const {
        return menuCount;
    }

    // 获取按钮状态
    bool isButtonPressed() const {
        return encoderButtonPressed;
    }
};

#endif // MENU_MANAGER_H