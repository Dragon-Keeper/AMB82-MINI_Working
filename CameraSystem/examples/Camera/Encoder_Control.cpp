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
    m_lastButtonInterruptTime(0),
    m_buttonEverReleased(false),
    m_initCompleteTime(0),
    m_debounceDelay(BUTTON_DEBOUNCE_DELAY),
    m_lastInterruptTime(0),
    m_lastValidRotationTime(0),
    m_rotationDebounceUs(300000),
    m_rotationCallback(nullptr),
    m_buttonCallback(nullptr)
{
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
    
    m_buttonEverReleased = false;
    m_buttonPressDetected = false;
    m_rotationDetected = false;
    m_rotationDirection = ROTATION_NONE;
    m_encoderCount = 0;
    m_lastButtonState = HIGH;
    
    pinMode(m_clkPin, INPUT_IRQ_FALL);
    pinMode(m_dtPin, INPUT_PULLUP);
    pinMode(m_swPin, INPUT_PULLUP);
    
    delay(100);
    
    for (int i = 0; i < 10; i++) {
        if (digitalRead(m_swPin) == HIGH) {
            m_buttonEverReleased = true;
            break;
        }
        delay(10);
    }
    
    m_initCompleteTime = millis() + 3000;
    
    digitalSetIrqHandler(m_clkPin, encoderRotation_handler);
    
    delay(50);
    
    digitalSetIrqHandler(m_swPin, encoderButton_handler);
    
    m_lastInterruptTime = micros();
    m_lastButtonInterruptTime = micros();
    
    Utils_Logger::info("编码器引脚配置完成（旋转+按钮中断模式），3秒稳定期");
    Utils_Logger::info("CLK引脚: %d, DT引脚: %d, SW引脚: %d", m_clkPin, m_dtPin, m_swPin);
    Utils_Logger::info("按钮初始释放状态: %s", m_buttonEverReleased ? "已释放" : "未释放(等待)");
    
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

void EncoderControl::setRotationDebounceTime(unsigned long us)
{
    m_rotationDebounceUs = us;
}

void EncoderControl::checkButton()
{
    if (millis() < m_initCompleteTime) return;

    bool currentButtonState = digitalRead(m_swPin);

    if (currentButtonState == HIGH) {
        m_buttonEverReleased = true;
    }

    if (!m_buttonEverReleased) {
        m_lastButtonState = currentButtonState;
        m_buttonPressDetected = false;
        return;
    }

    if (m_buttonPressDetected) {
        m_buttonPressDetected = false;
        handleButtonPress();
        return;
    }
    
    unsigned long currentTime = Utils_Timer::getCurrentTime();
    
    if (currentButtonState == LOW && m_lastButtonState == HIGH) {
        if (currentTime - m_lastButtonTime >= m_debounceDelay) {
            m_lastButtonTime = currentTime;
            handleButtonPress();
        }
    }
    
    if (currentButtonState == HIGH && m_lastButtonState == LOW) {
        if (currentTime - m_lastButtonTime >= m_debounceDelay) {
            m_lastButtonTime = currentTime;
        }
    }
    
    m_lastButtonState = currentButtonState;
}

void EncoderControl::cleanup()
{
    if (m_clkPin != 0) {
        digitalSetIrqHandler(m_clkPin, nullptr);
    }
    if (m_swPin != 0) {
        digitalSetIrqHandler(m_swPin, nullptr);
    }
    
    m_encoderCount = 0;
    m_rotationDetected = false;
    m_rotationDirection = ROTATION_NONE;
    m_buttonPressDetected = false;
    
    m_rotationCallback = nullptr;
    m_buttonCallback = nullptr;
    
    Utils_Logger::info("编码器资源清理完成");
}

void EncoderControl::clearPendingEvents()
{
    m_rotationDetected = false;
    m_rotationDirection = ROTATION_NONE;
    m_buttonPressDetected = false;
    m_encoderCount = 0;
}

void EncoderControl::encoderRotation_handler(uint32_t id, uint32_t event)
{
    if (s_instance != nullptr) {
        s_instance->handleRotation();
    }
}

void EncoderControl::encoderButton_handler(uint32_t id, uint32_t event)
{
    if (s_instance != nullptr) {
        s_instance->handleButtonISR();
    }
}

void EncoderControl::handleRotation()
{
    if (millis() < m_initCompleteTime) return;
    
    unsigned long currentTime = micros();
    
    if (currentTime - m_lastInterruptTime < 100000) {
        return;
    }
    
    m_lastInterruptTime = currentTime;
    
    bool dtState = digitalRead(m_dtPin);
    
    if (dtState == HIGH) {
        m_rotationDirection = ROTATION_CW;
    } else {
        m_rotationDirection = ROTATION_CCW;
    }
    
    m_rotationDetected = true;
}

void EncoderControl::handleButtonISR()
{
    if (millis() < m_initCompleteTime) return;
    
    if (!m_buttonEverReleased) return;
    
    if (digitalRead(m_swPin) != LOW) return;
    
    unsigned long currentTime = micros();
    
    if (currentTime - m_lastButtonInterruptTime < 200000) {
        return;
    }
    
    m_lastButtonInterruptTime = currentTime;
    m_buttonPressDetected = true;
}

void EncoderControl::checkRotation()
{
    if (m_rotationDetected && m_rotationCallback != nullptr) {
        unsigned long currentTime = micros();
        
        if (currentTime - m_lastValidRotationTime >= m_rotationDebounceUs) {
            m_lastValidRotationTime = currentTime;
            m_rotationCallback(m_rotationDirection);
            m_rotationDetected = false;
            m_rotationDirection = ROTATION_NONE;
        } else {
            m_rotationDetected = false;
            m_rotationDirection = ROTATION_NONE;
        }
    }
}

void EncoderControl::handleButtonPress()
{
    Utils_Logger::info("编码器按钮按下");
    
    if (m_buttonCallback != nullptr) {
        m_buttonCallback();
    }
}