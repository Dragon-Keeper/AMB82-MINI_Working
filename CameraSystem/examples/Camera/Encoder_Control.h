/*
 * Encoder_Control.h - 编码器控制头文件
 * 封装旋转编码器的初始化和旋转检测功能
 * 阶段六：菜单模块优化 - 编码器输入处理
 */

#ifndef ENCODER_CONTROL_H
#define ENCODER_CONTROL_H

#include <Arduino.h>
#include "Shared_GlobalDefines.h"
#include "Utils_Timer.h"
#include "Utils_Logger.h"

// 编码器旋转方向枚举
typedef enum {
    ROTATION_NONE = 0,
    ROTATION_CW = 1,      // 顺时针
    ROTATION_CCW = -1     // 逆时针
} RotationDirection;

// 编码器事件回调函数类型定义
typedef void (*RotationCallback)(RotationDirection direction);
typedef void (*ButtonCallback)(void);

class EncoderControl {
public:
    // 构造函数和析构函数
    EncoderControl();
    ~EncoderControl();
    
    // 初始化函数
    bool init(uint8_t clkPin = ENCODER_CLK, uint8_t dtPin = ENCODER_DT, uint8_t swPin = ENCODER_SW);
    
    // 事件回调注册
    void setRotationCallback(RotationCallback cb);
    void setButtonCallback(ButtonCallback cb);
    
    // 状态查询函数
    RotationDirection getLastRotation();
    bool isButtonPressed();
    int getEncoderCount();
    
    // 消抖参数配置
    void setDebounceTime(unsigned long ms);
    
    // 按钮检测函数（需要在主循环中调用）
    void checkButton();
    
    // 清理资源
    void cleanup();
    
    // 中断处理函数（静态成员函数）
    static void encoderRotation_handler(uint32_t id, uint32_t event);
    
private:
    // 引脚配置
    uint8_t m_clkPin;
    uint8_t m_dtPin;
    uint8_t m_swPin;
    
    // 编码器状态变量
    volatile int m_encoderCount;
    volatile bool m_rotationDetected;
    volatile RotationDirection m_rotationDirection;
    
    // 按钮状态变量
    bool m_lastButtonState;
    unsigned long m_lastButtonTime;
    bool m_buttonPressDetected;
    
    // 消抖参数
    unsigned long m_debounceDelay;
    
    // 事件回调函数
    RotationCallback m_rotationCallback;
    ButtonCallback m_buttonCallback;
    
    // 实例指针（用于中断处理）
    static EncoderControl* s_instance;
    
    // 私有方法
    void handleRotation();
    void handleButtonPress();
};

#endif // ENCODER_CONTROL_H