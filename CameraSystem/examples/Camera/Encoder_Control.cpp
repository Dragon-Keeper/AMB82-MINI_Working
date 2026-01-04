/*
 * Encoder_Control.cpp - 编码器控制实现
 * 封装旋转编码器的初始化和旋转检测功能
 * 阶段六：菜单模块优化 - 编码器输入处理
 */

#include "Encoder_Control.h"

// 静态实例指针初始化
EncoderControl* EncoderControl::s_instance = nullptr;

EncoderControl::EncoderControl() :
    m_clkPin(ENCODER_CLK),
    m_dtPin(ENCODER_DT),
    m_swPin(ENCODER_SW),
    m_encoderCount(0),
    m_rotationDetected(false),
    m_rotationDirection(ROTATION_NONE),
    m_lastButtonState(HIGH),
    m_lastButtonTime(0),
    m_buttonPressDetected(false),
    m_debounceDelay(BUTTON_DEBOUNCE_DELAY),
    m_rotationCallback(nullptr),
    m_buttonCallback(nullptr)
{
    // 设置静态实例指针
    s_instance = this;
}

EncoderControl::~EncoderControl()
{
    cleanup();
}

bool EncoderControl::init(uint8_t clkPin, uint8_t dtPin, uint8_t swPin)
{
    m_clkPin = clkPin;
    m_dtPin = dtPin;
    m_swPin = swPin;
    
    // 配置编码器引脚
    pinMode(m_clkPin, INPUT_IRQ_FALL);
    pinMode(m_dtPin, INPUT_PULLUP);
    pinMode(m_swPin, INPUT_PULLUP);
    
    // 注册中断处理函数
    digitalSetIrqHandler(m_clkPin, encoderRotation_handler);
    
    Utils_Logger::info("编码器引脚配置完成");
    Utils_Logger::info("CLK引脚: %d, DT引脚: %d, SW引脚: %d", m_clkPin, m_dtPin, m_swPin);
    
    return true;
}

void EncoderControl::setRotationCallback(RotationCallback cb)
{
    m_rotationCallback = cb;
}

void EncoderControl::setButtonCallback(ButtonCallback cb)
{
    m_buttonCallback = cb;
}

RotationDirection EncoderControl::getLastRotation()
{
    return m_rotationDirection;
}

bool EncoderControl::isButtonPressed()
{
    return m_buttonPressDetected;
}

int EncoderControl::getEncoderCount()
{
    return m_encoderCount;
}

void EncoderControl::setDebounceTime(unsigned long ms)
{
    m_debounceDelay = ms;
}

void EncoderControl::checkButton()
{
    bool currentButtonState = digitalRead(m_swPin);
    unsigned long currentTime = Utils_Timer::getCurrentTime();
    
    // 检测按钮按下（下降沿）
    if (currentButtonState == LOW && m_lastButtonState == HIGH) {
        // 消抖处理
        if (currentTime - m_lastButtonTime >= m_debounceDelay) {
            m_lastButtonTime = currentTime;
            handleButtonPress();
        }
    }
    
    // 检测按钮释放（上升沿）
    if (currentButtonState == HIGH && m_lastButtonState == LOW) {
        // 消抖处理
        if (currentTime - m_lastButtonTime >= m_debounceDelay) {
            m_lastButtonTime = currentTime;
            // 按钮释放处理（如果需要）
        }
    }
    
    m_lastButtonState = currentButtonState;
}

void EncoderControl::cleanup()
{
    // 移除中断处理
    if (m_clkPin != 0) {
        digitalSetIrqHandler(m_clkPin, nullptr);
    }
    
    // 重置状态变量
    m_encoderCount = 0;
    m_rotationDetected = false;
    m_rotationDirection = ROTATION_NONE;
    m_buttonPressDetected = false;
    
    // 清空回调函数
    m_rotationCallback = nullptr;
    m_buttonCallback = nullptr;
    
    Utils_Logger::info("编码器资源清理完成");
}

// 静态中断处理函数
void EncoderControl::encoderRotation_handler(uint32_t id, uint32_t event)
{
    // 通过静态实例指针调用实例方法
    if (s_instance != nullptr) {
        s_instance->handleRotation();
    }
}

void EncoderControl::handleRotation()
{
    // 读取DT引脚状态
    bool dtState = digitalRead(m_dtPin);
    
    // 检测旋转方向（在下降沿时检测DT引脚状态）
    if (dtState == HIGH) {
        // 顺时针旋转：向下移动三角形
        m_rotationDirection = ROTATION_CW;
        m_rotationDetected = true;
    } else if (dtState == LOW) {
        // 逆时针旋转：向上移动三角形
        m_rotationDirection = ROTATION_CCW;
        m_rotationDetected = true;
    }
    
    // 如果有旋转回调函数，立即调用
    if (m_rotationCallback != nullptr && m_rotationDetected) {
        m_rotationCallback(m_rotationDirection);
    }
    
    Utils_Logger::debug("编码器旋转检测: 方向=%d, 检测=%s", 
                       m_rotationDirection, 
                       m_rotationDetected ? "true" : "false");
}

void EncoderControl::handleButtonPress()
{
    m_buttonPressDetected = true;
    
    // 如果有按钮回调函数，立即调用
    if (m_buttonCallback != nullptr) {
        m_buttonCallback();
    }
    
    Utils_Logger::debug("按钮按下检测");
}