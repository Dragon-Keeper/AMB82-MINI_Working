/*
 * Display_FontRenderer.h - 字体渲染器模块
 * 封装中文字符和字符串渲染功能，提供统一的字体显示接口
 * 阶段三：显示模块开发
 */

#ifndef DISPLAY_FONTRENDERER_H
#define DISPLAY_FONTRENDERER_H

#include <Arduino.h>
#include "Display_TFTManager.h"
#include "Display_font16x16.h"

// 预定义字符串常量
#define STR_REAL_TIME_PREVIEW_SIZE 5
#define STR_PHOTO_SUCCESS_SIZE 5
#define STR_SAVE_FAILED_CN_SIZE 5
#define STR_CAPTURE_FAILED_CN_SIZE 5

class Display_FontRenderer {
public:
    // 构造函数
    Display_FontRenderer();
    
    // 初始化函数
    bool begin(Display_TFTManager* tftManager);
    
    // 字符渲染函数
    void drawChineseChar(int16_t x, int16_t y, uint8_t charIndex, uint16_t color, uint16_t bgColor);
    void drawChineseString(int16_t x, int16_t y, const uint8_t* charIndices, uint16_t color, uint16_t bgColor);
    
    // 状态显示函数
    void showCameraStatus(const uint8_t* message, int x, int y);
    
    // 预定义字符串获取函数
    const uint8_t* getRealTimePreviewString();
    const uint8_t* getPhotoSuccessString();
    const uint8_t* getSaveFailedString();
    const uint8_t* getCaptureFailedString();
    
    // 获取字符串长度
    uint8_t getStringLength(const uint8_t* charIndices);
    
    // 计算字符串居中位置
    int16_t calculateCenterPosition(int16_t screenWidth, const uint8_t* charIndices);
    
    // 状态检查
    bool isInitialized() const;
    
private:
    Display_TFTManager* m_tftManager;  // TFT管理器指针
    uint16_t m_fontBuffer[256];        // 字体渲染缓冲区（16x16像素）
    bool m_initialized;                // 初始化状态标志
    
    // 预定义字符串常量
    const uint8_t m_strRealTimePreview[STR_REAL_TIME_PREVIEW_SIZE] = {FONT16_IDX_SHI6, FONT16_IDX_SHI5, FONT16_IDX_YU2, FONT16_IDX_LAN2, 0};
    const uint8_t m_strPhotoSuccess[STR_PHOTO_SUCCESS_SIZE] = {FONT16_IDX_PAI2, FONT16_IDX_ZHAO2, FONT16_IDX_CHENG, FONT16_IDX_GONG, 0};
    const uint8_t m_strSaveFailed[STR_SAVE_FAILED_CN_SIZE] = {FONT16_IDX_BAO, FONT16_IDX_CUN, FONT16_IDX_SHI4, FONT16_IDX_BAI, 0};
    const uint8_t m_strCaptureFailed[STR_CAPTURE_FAILED_CN_SIZE] = {FONT16_IDX_PAI2, FONT16_IDX_ZHAO2, FONT16_IDX_SHI4, FONT16_IDX_BAI, 0};
    
    // 字体数据处理
    void processFontData(const uint8_t* fontData, uint16_t color, uint16_t bgColor);
};

// 全局字体渲染器实例声明
extern Display_FontRenderer fontRenderer;

#endif