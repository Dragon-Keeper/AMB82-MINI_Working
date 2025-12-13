/**
 * @file FT6336U_HardI2C.h
 * @brief FT6336U 2点触摸屏硬件I2C驱动库
 * @version 1.0.0
 * @date 2024-03
 * 
 * 本库提供FT6336U触摸屏的硬件I2C驱动，支持2点触摸、中断和轮询两种工作模式。
 * 适用于AMB82-MINI平台，已适配硬件I2C接口。
 * 
 * 特性：
 * 1. 完整的硬件I2C驱动，稳定可靠
 * 2. 支持最多2点触摸检测
 * 3. 完整的触摸事件识别（按下、抬起、接触）
 * 4. 支持中断和轮询两种工作模式
 * 5. 软件层面的触摸点状态跟踪
 * 6. 防抖处理和事件去重
 * 7. 详细的调试信息输出
 * 
 * 使用示例：
 * @code
 * #include <FT6336U_HardI2C.h>
 * 
 * #define PIN_RST 15
 * #define PIN_INT 16
 * 
 * FT6336U_HardI2C touch(PIN_RST, PIN_INT);
 * 
 * void setup() {
 *   Serial.begin(115200);
 *   touch.begin();
 * }
 * 
 * void loop() {
 *   if (touch.isTouched()) {
 *     uint16_t x, y;
 *     if (touch.getTouchPoint(0, &x, &y)) {
 *       Serial.printf("Touch: X=%d, Y=%d\n", x, y);
 *     }
 *   }
 *   delay(20);
 * }
 * @endcode
 */

#ifndef FT6336U_HARDI2C_H
#define FT6336U_HARDI2C_H

#include <Arduino.h>
#include <Wire.h>

// Default configuration
#ifndef FT6336U_DEFAULT_I2C_ADDRESS
#define FT6336U_DEFAULT_I2C_ADDRESS 0x38
#endif

#ifndef FT6336U_DEFAULT_I2C_CLOCK
#define FT6336U_DEFAULT_I2C_CLOCK 400000
#endif

#ifndef FT6336U_MAX_TOUCH_POINTS
#define FT6336U_MAX_TOUCH_POINTS 2
#endif

#ifndef FT6336U_DEFAULT_POLL_INTERVAL
#define FT6336U_DEFAULT_POLL_INTERVAL 20
#endif

// Touch event types
enum TouchEventType {
    TOUCH_EVENT_NONE = 0x03,
    TOUCH_EVENT_PRESS = 0x00,
    TOUCH_EVENT_RELEASE = 0x01,
    TOUCH_EVENT_CONTACT = 0x02
};

// Touch point structure
struct TouchPoint {
    uint8_t id;             // Touch ID (0-1)
    uint16_t x;             // X coordinate (0-2047)
    uint16_t y;             // Y coordinate (0-2047)
    uint8_t area;           // Touch area
    TouchEventType event;   // Touch event type
    bool valid;             // Data validity flag
};

// Touch event structure for event detection
struct TouchEvent {
    uint8_t id;             // Touch point ID (0 or 1)
    uint16_t x;             // X coordinate
    uint16_t y;             // Y coordinate
    bool pressed;           // True if just pressed
    bool released;          // True if just released
};

// Main FT6336U class
class FT6336U_HardI2C {
public:
    // Constructor and initialization
    FT6336U_HardI2C(uint8_t sdaPin = 12, uint8_t sclPin = 13, uint8_t rstPin = 15, uint8_t intPin = 16);
    bool begin(uint8_t i2cAddress = FT6336U_DEFAULT_I2C_ADDRESS, uint32_t i2cClock = FT6336U_DEFAULT_I2C_CLOCK);
    
    // Configuration methods
    void setInterruptMode(bool enabled);
    void setPollingInterval(uint16_t interval);
    void enableDebug(bool enabled);
    void reset();
    
    // Core touch functions
    uint8_t getTouchCount();
    bool isTouched();
    bool getTouchPoint(uint8_t index, uint16_t *x, uint16_t *y);
    bool getTouchPointFull(uint8_t index, TouchPoint *point);
    
    // Event detection functions
    bool isTouchActive(uint8_t id);
    bool hasNewEvent(TouchEvent *event = nullptr);
    
    // Status and debug
    void update();
    void printStatus();
    
    // Chip information
    bool readChipInfo();

private:
    // Debouncing functions
    uint8_t _getStableTouchCount();
    // Pin definitions
    uint8_t _sdaPin;
    uint8_t _sclPin;
    uint8_t _rstPin;
    uint8_t _intPin;
    
    // I2C configuration
    uint8_t _i2cAddress;
    uint32_t _i2cClock;
    
    // Operational modes
    bool _interruptMode;
    bool _debugEnabled;
    uint16_t _pollingInterval;
    
    // Touch state
    struct TouchState {
        uint8_t count;
        TouchPoint points[FT6336U_MAX_TOUCH_POINTS];
        uint32_t timestamp;
    } _touchState;
    
    // Touch tracker for event detection
    struct TouchTracker {
        bool active[FT6336U_MAX_TOUCH_POINTS];
        uint32_t lastActive[FT6336U_MAX_TOUCH_POINTS];
        uint32_t releaseTime[FT6336U_MAX_TOUCH_POINTS];
        uint8_t lastCount;
        uint32_t lastValidTouchTime;
        uint8_t consecutiveNoTouchCount;
        uint8_t touchHistory[4];
        uint8_t historyIndex;
    } _touchTracker;
    
    // Interrupt handling
    volatile bool _interruptOccurred;
    
    // Internal methods
    void _initTouchState();
    void _initTouchTracker();
    void _updateTouchEvents();
    void _readTouchData();
    
    // I2C communication
    bool _writeRegister(uint8_t reg, uint8_t *data, uint8_t len);
    bool _readRegister(uint8_t reg, uint8_t *data, uint8_t len);
    
    // Interrupt handler (static for C callback)
    static void _interruptHandler(uint32_t id, uint32_t event);
    static FT6336U_HardI2C* _instance;
    
    // Chip configuration
    bool _configure();
    
    // Debounce helper
    uint32_t _lastPrintTime;
    
    // Register addresses
    static const uint8_t FT_REG_TD_STATUS = 0x02;
    static const uint8_t FT_TP1_REG = 0x03;
    static const uint8_t FT_TP2_REG = 0x09;
    static const uint8_t FT_REG_CHIP_ID_H = 0xA3;
    static const uint8_t FT_REG_CHIP_ID_L = 0xA4;
    static const uint8_t FT_REG_FIRMWARE_ID = 0xA6;
    static const uint8_t FT_REG_VENDOR_ID = 0xA8;
    static const uint8_t FT_REG_THGROUP = 0x80;
    
    // Internal state flags
    bool _initialized;
};

#endif // FT6336U_HARDI2C_H