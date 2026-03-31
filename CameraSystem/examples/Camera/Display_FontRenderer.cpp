/*
 * Display_FontRenderer.cpp - 字体渲染器模块实现
 * 封装中文字符和字符串渲染功能，提供统一的字体显示接口
 * 阶段三：显示模块开发
 */

#include "Display_FontRenderer.h"
#include "Utils_Logger.h"

// 全局字体渲染器实例定义
Display_FontRenderer fontRenderer;

Display_FontRenderer::Display_FontRenderer() 
    : m_tftManager(nullptr), m_initialized(false) {
    // 初始化字体缓冲区
    memset(m_fontBuffer, 0, sizeof(m_fontBuffer));
}

bool Display_FontRenderer::begin(Display_TFTManager* tftManager) {
    if (!tftManager || !tftManager->isInitialized()) {
        Utils_Logger::error("字体渲染器初始化失败：TFT管理器未初始化!");
        return false;
    }
    
    m_tftManager = tftManager;
    m_initialized = true;
    
    Utils_Logger::info("字体渲染器初始化成功");
    return true;
}

void Display_FontRenderer::drawChineseChar(int16_t x, int16_t y, uint8_t charIndex, uint16_t color, uint16_t bgColor) {
    if (!m_initialized || !m_tftManager) {
        Utils_Logger::error("字体渲染器未初始化，无法绘制字符!");
        return;
    }
    
    // 检查字库索引是否有效，防止数组越界访问
    if (charIndex >= sizeof(font16x16)/sizeof(font16x16[0])) {
        Utils_Logger::error("无效的字库索引: %d", charIndex);
        return;
    }
    
    // 获取字库数据，font16x16数组中存储了预定义的字符点阵数据
    const uint8_t* fontData = font16x16[charIndex];
    
    // 处理字体数据并填充缓冲区
    processFontData(fontData, color, bgColor);
    
    // 将缓冲区数据绘制到屏幕指定位置
    m_tftManager->drawBitmap(x, y, 16, 16, m_fontBuffer);
}

void Display_FontRenderer::processFontData(const uint8_t* fontData, uint16_t color, uint16_t bgColor) {
    // 按字库原始顺序填充缓冲区，行优先存储
    for (int row = 0; row < 16; row++) {
        // 计算当前行在字库数据中的起始位置
        int dataOffset = row * 2;
        // 合并为16位数据，获取该行的点阵信息
        uint16_t rowBits = (fontData[dataOffset] << 8) | fontData[dataOffset + 1];
            
        // 计算当前行在缓冲区中的起始位置
        int bufferRowStart = row * 16;
        for (int col = 0; col < 16; col++) {
            // 检查对应位是否为字符像素（高位在前）
            if (rowBits & (0x8000 >> col)) {
                m_fontBuffer[bufferRowStart + col] = color;  // 字符像素
            } else {
                m_fontBuffer[bufferRowStart + col] = bgColor;  // 背景像素
            }
        }
    }
}

void Display_FontRenderer::drawChineseString(int16_t x, int16_t y, const uint8_t* charIndices, 
                                            uint16_t color, uint16_t bgColor) {
    if (!m_initialized || !m_tftManager) {
        Utils_Logger::error("字体渲染器未初始化，无法绘制字符串!");
        return;
    }
    
    int i = 0;
    // 遍历字符索引数组直到遇到终止符（索引值为0）
    while (charIndices[i] != 0) {
        // 绘制单个字符，每个字符之间间隔16像素（与字符宽度一致）
        // x + i * 16 计算第i个字符的X坐标位置
        drawChineseChar(x + i * 16, y, charIndices[i], color, bgColor);
        i++;
    }
}

void Display_FontRenderer::showCameraStatus(const uint8_t* message, int x, int y) {
    if (!m_initialized || !m_tftManager) {
        Utils_Logger::error("字体渲染器未初始化，无法显示状态!");
        return;
    }
    
    // 绘制状态提示信息，使用白色字符和黑色背景的标准配色
    drawChineseString(x, y, message, ST7789_WHITE, ST7789_BLACK);
}

const uint8_t* Display_FontRenderer::getRealTimePreviewString() {
    return m_strRealTimePreview;
}

const uint8_t* Display_FontRenderer::getPhotoSuccessString() {
    return m_strPhotoSuccess;
}

const uint8_t* Display_FontRenderer::getSaveFailedString() {
    return m_strSaveFailed;
}

const uint8_t* Display_FontRenderer::getCaptureFailedString() {
    return m_strCaptureFailed;
}

uint8_t Display_FontRenderer::getStringLength(const uint8_t* charIndices) {
    uint8_t length = 0;
    while (charIndices[length] != 0) {
        length++;
    }
    return length;
}

int16_t Display_FontRenderer::calculateCenterPosition(int16_t screenWidth, const uint8_t* charIndices) {
    uint8_t length = getStringLength(charIndices);
    int16_t stringWidth = length * 16;  // 每个字符16像素宽
    return (screenWidth - stringWidth) / 2;
}

bool Display_FontRenderer::isInitialized() const {
    return m_initialized;
}