/*
 * Display_TFTManager.cpp - TFT显示管理器模块实现
 * 封装AmebaST7789_DMA_SPI1显示操作，提供统一的TFT管理接口
 * 阶段三：显示模块开发
 */

#include "Display_TFTManager.h"
#include "Utils_Logger.h"

Display_TFTManager::Display_TFTManager() 
    : m_tft(TFT_CS, TFT_DC, TFT_RST), m_initialized(false) {
}

bool Display_TFTManager::begin() {
    Utils_Logger::info("初始化TFT屏幕...");
    
    // 初始化SPI配置
    initSPI();
    
    // 初始化TFT显示屏 (begin()返回void，直接调用)
    m_tft.begin();
    
    // 配置TFT显示参数
    m_tft.setRotation(1);
    m_tft.fillScreen(ST7789_BLACK);
    
    // 设置默认文本属性
    m_tft.setForeground(ST7789_WHITE);
    m_tft.setBackground(ST7789_BLACK);
    m_tft.setFontSize(1);
    
    m_initialized = true;
    Utils_Logger::info("TFT显示屏初始化成功");
    return true;
}

void Display_TFTManager::initSPI() {
    // 配置SPI通信参数
    SPI1.setDataMode(TFT_CS, SPI_MODE0, SPI_MODE_MASTER);
    SPI1.setDefaultFrequency(25000000);
    SPI1.begin();
}

void Display_TFTManager::fillScreen(uint16_t color) {
    if (!m_initialized) return;
    m_tft.fillScreen(color);
}

void Display_TFTManager::setRotation(uint8_t rotation) {
    if (!m_initialized) return;
    m_tft.setRotation(rotation);
}

void Display_TFTManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (!m_initialized) return;
    m_tft.drawLine(x0, y0, x1, y1, color);
}

void Display_TFTManager::drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!m_initialized) return;
    m_tft.drawRectangle(x, y, w, h, color);
}

void Display_TFTManager::fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!m_initialized) return;
    m_tft.fillRectangle(x, y, w, h, color);
}

void Display_TFTManager::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *bitmap) {
    if (!m_initialized) return;
    m_tft.drawBitmap(x, y, w, h, bitmap);
}

void Display_TFTManager::setTextSize(uint8_t size) {
    if (!m_initialized) return;
    m_tft.setFontSize(size);
}

void Display_TFTManager::setCursor(int16_t x, int16_t y) {
    if (!m_initialized) return;
    m_tft.setCursor(x, y);
}

void Display_TFTManager::setTextColor(uint16_t color) {
    if (!m_initialized) return;
    m_tft.setForeground(color);
}

void Display_TFTManager::setTextColor(uint16_t color, uint16_t bgColor) {
    if (!m_initialized) return;
    m_tft.setForeground(color);
    m_tft.setBackground(bgColor);
}

void Display_TFTManager::print(const char* text) {
    if (!m_initialized) return;
    m_tft.print(text);
}

void Display_TFTManager::println(const char* text) {
    if (!m_initialized) return;
    m_tft.println(text);
}

AmebaST7789_DMA_SPI1& Display_TFTManager::getTFT() {
    return m_tft;
}

bool Display_TFTManager::isInitialized() const {
    return m_initialized;
}

void Display_TFTManager::setBacklight(bool enabled) {
    // 初始化TFT背光引脚
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, enabled ? HIGH : LOW);
}