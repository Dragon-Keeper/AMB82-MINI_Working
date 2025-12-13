/**
 * @file FT6336U_HardI2C.cpp
 * @brief FT6336U硬件I2C驱动库实现
 * @version 1.0.0
 * @date 2024-03
 */

#include "FT6336U_HardI2C.h"
#include <Wire.h>

// Initialize static instance pointer
FT6336U_HardI2C* FT6336U_HardI2C::_instance = nullptr;

// Constructor
FT6336U_HardI2C::FT6336U_HardI2C(uint8_t sdaPin, uint8_t sclPin, uint8_t rstPin, uint8_t intPin) :
    _sdaPin(sdaPin),
    _sclPin(sclPin),
    _rstPin(rstPin),
    _intPin(intPin),
    _i2cAddress(FT6336U_DEFAULT_I2C_ADDRESS),
    _i2cClock(FT6336U_DEFAULT_I2C_CLOCK),
    _interruptMode(false),
    _debugEnabled(false),
    _pollingInterval(FT6336U_DEFAULT_POLL_INTERVAL),
    _interruptOccurred(false),
    _lastPrintTime(0),
    _initialized(false)
{
    _initTouchState();
    _initTouchTracker();
    _instance = this;
}

// Initialize the touch controller
bool FT6336U_HardI2C::begin(uint8_t i2cAddress, uint32_t i2cClock) {
    _i2cAddress = i2cAddress;
    _i2cClock = i2cClock;
    
    if (_debugEnabled) {
        Serial.println("FT6336U: Initializing touch controller...");
    }
    
    // Initialize I2C
    Wire.begin();
    Wire.setClock(_i2cClock);
    
    // Configure reset pin
    pinMode(_rstPin, OUTPUT);
    
    // Perform hardware reset
    reset();
    
    // Initialize touch state
    _initTouchState();
    _initTouchTracker();
    
    // Configure chip
    if (!_configure()) {
        if (_debugEnabled) {
            Serial.println("FT6336U: Configuration failed");
        }
        return false;
    }
    
    // Configure interrupt pin if in interrupt mode
    if (_interruptMode && _intPin != 0xFF) {
        pinMode(_intPin, INPUT_IRQ_FALL);
        digitalSetIrqHandler(_intPin, _interruptHandler);
        
        if (_debugEnabled) {
            Serial.println("FT6336U: Interrupt mode enabled");
        }
    }
    
    _initialized = true;
    
    if (_debugEnabled) {
        Serial.print("FT6336U: Initialized with I2C address 0x");
        Serial.print(_i2cAddress, HEX);
        Serial.print(", clock ");
        Serial.print(_i2cClock / 1000);
        Serial.println(" kHz");
    }
    
    return true;
}

// Reset the touch controller
void FT6336U_HardI2C::reset() {
    digitalWrite(_rstPin, LOW);
    delay(50);
    digitalWrite(_rstPin, HIGH);
    delay(500);
}

// Configure chip parameters
bool FT6336U_HardI2C::_configure() {
    uint8_t configValue;
    
    // Set touch threshold (lower values = more sensitive, range 1-255)
    configValue = 0x12;  // Default sensitivity
    if (!_writeRegister(FT_REG_THGROUP, &configValue, 1)) {
        if (_debugEnabled) {
            Serial.println("FT6336U: Failed to set touch threshold");
        }
        return false;
    }
    
    // Set report rate: 80ms
    configValue = 0x08;
    if (!_writeRegister(0x88, &configValue, 1)) {
        if (_debugEnabled) {
            Serial.println("FT6336U: Failed to set report rate");
        }
        return false;
    }
    
    if (_debugEnabled) {
        Serial.println("FT6336U: Configuration complete");
    }
    
    return true;
}

// Read chip information
bool FT6336U_HardI2C::readChipInfo() {
    uint8_t data[4];
    
    if (_debugEnabled) {
        Serial.println("FT6336U: Reading chip information...");
    }
    
    // Read chip ID
    if (_readRegister(FT_REG_CHIP_ID_H, data, 2)) {
        Serial.print("FT6336U: Chip ID: 0x");
        if (data[0] < 0x10) Serial.print("0");
        Serial.print(data[0], HEX);
        if (data[1] < 0x10) Serial.print("0");
        Serial.println(data[1], HEX);
    } else {
        Serial.println("FT6336U: Failed to read chip ID");
        return false;
    }
    
    // Read firmware version
    if (_readRegister(FT_REG_FIRMWARE_ID, data, 1)) {
        Serial.print("FT6336U: Firmware version: 0x");
        if (data[0] < 0x10) Serial.print("0");
        Serial.println(data[0], HEX);
    } else {
        Serial.println("FT6336U: Failed to read firmware version");
        return false;
    }
    
    // Read vendor ID
    if (_readRegister(FT_REG_VENDOR_ID, data, 1)) {
        Serial.print("FT6336U: Vendor ID: 0x");
        if (data[0] < 0x10) Serial.print("0");
        Serial.println(data[0], HEX);
    } else {
        Serial.println("FT6336U: Failed to read vendor ID");
        return false;
    }
    
    return true;
}

// Set interrupt mode
void FT6336U_HardI2C::setInterruptMode(bool enabled) {
    _interruptMode = enabled;
    
    if (_initialized && _intPin != 0xFF) {
        if (_interruptMode) {
            pinMode(_intPin, INPUT_IRQ_FALL);
            digitalSetIrqHandler(_intPin, _interruptHandler);
            
            if (_debugEnabled) {
                Serial.println("FT6336U: Interrupt mode enabled");
            }
        } else {
            // Note: AMB82-MINI doesn't have a standard way to disable interrupt
            // This would depend on the platform's implementation
            if (_debugEnabled) {
                Serial.println("FT6336U: Interrupt mode disabled (polling)");
            }
        }
    }
}

// Set polling interval
void FT6336U_HardI2C::setPollingInterval(uint16_t interval) {
    _pollingInterval = interval;
    
    if (_debugEnabled) {
        Serial.print("FT6336U: Polling interval set to ");
        Serial.print(interval);
        Serial.println(" ms");
    }
}

// Enable/disable debug output
void FT6336U_HardI2C::enableDebug(bool enabled) {
    _debugEnabled = enabled;
}

// Get number of active touch points
uint8_t FT6336U_HardI2C::getTouchCount() {
    return _touchState.count;
}

// Check if screen is touched
bool FT6336U_HardI2C::isTouched() {
    return _touchState.count > 0;
}

// Get coordinates of specific touch point
bool FT6336U_HardI2C::getTouchPoint(uint8_t index, uint16_t *x, uint16_t *y) {
    if (index >= FT6336U_MAX_TOUCH_POINTS || !_touchState.points[index].valid) {
        return false;
    }
    
    *x = _touchState.points[index].x;
    *y = _touchState.points[index].y;
    return true;
}

// Get complete touch point information
bool FT6336U_HardI2C::getTouchPointFull(uint8_t index, TouchPoint *point) {
    if (index >= FT6336U_MAX_TOUCH_POINTS || !_touchState.points[index].valid) {
        return false;
    }
    
    *point = _touchState.points[index];
    return true;
}

// Check if specific touch ID is active
bool FT6336U_HardI2C::isTouchActive(uint8_t id) {
    if (id >= FT6336U_MAX_TOUCH_POINTS) {
        return false;
    }
    return _touchTracker.active[id];
}

// Update touch data (call in loop for polling mode)
void FT6336U_HardI2C::update() {
    if (!_initialized) {
        return;
    }
    
    if (_interruptMode) {
        // In interrupt mode, only read when interrupt occurs
        if (_interruptOccurred) {
            _interruptOccurred = false;
            _readTouchData();
        }
    } else {
        // In polling mode, read data periodically
        static uint32_t lastPollTime = 0;
        uint32_t currentTime = millis();
        
        if (currentTime - lastPollTime >= _pollingInterval) {
            lastPollTime = currentTime;
            _readTouchData();
        }
    }
}

// Check for new touch events
bool FT6336U_HardI2C::hasNewEvent(TouchEvent *event) {
    static bool lastActive[FT6336U_MAX_TOUCH_POINTS] = {false, false};
    static uint32_t lastEventTime[FT6336U_MAX_TOUCH_POINTS] = {0, 0};
    uint32_t currentTime = millis();
    bool foundEvent = false;
    
    // Check for press events
    for (uint8_t id = 0; id < FT6336U_MAX_TOUCH_POINTS; id++) {
        if (_touchTracker.active[id] && !lastActive[id]) {
            // New press event
            if (event != nullptr) {
                event->id = id;
                event->pressed = true;
                event->released = false;
                
                // Find coordinates for this touch point
                for (uint8_t i = 0; i < _touchState.count; i++) {
                    if (_touchState.points[i].valid && _touchState.points[i].id == id) {
                        event->x = _touchState.points[i].x;
                        event->y = _touchState.points[i].y;
                        break;
                    }
                }
            }
            
            lastActive[id] = true;
            lastEventTime[id] = currentTime;
            foundEvent = true;
            break;  // Report one event at a time
        }
    }
    
    // Check for release events
    if (!foundEvent) {
        for (uint8_t id = 0; id < FT6336U_MAX_TOUCH_POINTS; id++) {
            if (!_touchTracker.active[id] && lastActive[id]) {
                // Debounce: ensure enough time between events
                if (currentTime - lastEventTime[id] > 50) {
                    // Release event
                    if (event != nullptr) {
                        event->id = id;
                        event->x = 0;
                        event->y = 0;
                        event->pressed = false;
                        event->released = true;
                    }
                    
                    lastActive[id] = false;
                    lastEventTime[id] = currentTime;
                    foundEvent = true;
                    break;  // Report one event at a time
                }
            }
        }
    }
    
    return foundEvent;
}

// 优化 printStatus() 函数
void FT6336U_HardI2C::printStatus() {
    uint32_t currentTime = millis();
    
    // 限制输出频率
    if (currentTime - _lastPrintTime < 300) {
        return;
    }
    _lastPrintTime = currentTime;
    
    // 检查是否有活动的触摸点（增强防抖机制）
    bool hasActiveTouch = false;
    
    // 检查触摸点活动状态（延长防抖时间到500ms）
    for (uint8_t id = 0; id < FT6336U_MAX_TOUCH_POINTS; id++) {
        if (_touchTracker.active[id] && (currentTime - _touchTracker.lastActive[id] < 500)) {
            hasActiveTouch = true;
            break;
        }
    }
    
    // 检查最近是否有有效触摸（延长到1秒）
    bool recentTouch = (currentTime - _touchTracker.lastValidTouchTime < 1000);
    
    // 只有当连续3次读取无触摸且最近1秒内无触摸时才显示"等待触摸..."
    if (!hasActiveTouch && _touchState.count == 0 && 
        _touchTracker.consecutiveNoTouchCount >= 3 && !recentTouch) {
        Serial.println("📱 等待触摸...");
        return;
    }
    
    // 如果触摸状态不稳定，显示稳定状态
    if (_touchState.count == 0 && hasActiveTouch) {
        Serial.println("📱 触摸状态不稳定，正在稳定中...");
        return;
    }
    
    // 显示触摸点数量（单点/多点）
    Serial.print("📱 触摸状态: ");
    if (_touchState.count == 1) {
        Serial.print("单点触摸");
    } else if (_touchState.count == 2) {
        Serial.print("两点触摸");
    }
    Serial.print(" (");
    Serial.print(_touchState.count);
    Serial.println("个活动点)");
    
    // 按ID排序输出
    TouchPoint sortedPoints[FT6336U_MAX_TOUCH_POINTS];
    uint8_t sortedCount = 0;
    
    for (uint8_t i = 0; i < FT6336U_MAX_TOUCH_POINTS; i++) {
        if (_touchState.points[i].valid) {
            sortedPoints[sortedCount++] = _touchState.points[i];
        }
    }
    
    // 按ID排序
    if (sortedCount == 2 && sortedPoints[0].id > sortedPoints[1].id) {
        TouchPoint temp = sortedPoints[0];
        sortedPoints[0] = sortedPoints[1];
        sortedPoints[1] = temp;
    }
    
    // 显示每个触摸点的详细信息
    for (uint8_t i = 0; i < sortedCount; i++) {
        Serial.print("  🔹 触摸点 ");
        Serial.print(i + 1);
        Serial.print(" (ID:");
        Serial.print(sortedPoints[i].id);
        Serial.print(") - X=");
        Serial.print(sortedPoints[i].x);
        Serial.print(", Y=");
        Serial.print(sortedPoints[i].y);
        Serial.print(", 状态:");
        
        switch (sortedPoints[i].event) {
            case TOUCH_EVENT_PRESS:
                Serial.println("按下");
                break;
            case TOUCH_EVENT_RELEASE:
                Serial.println("抬起");
                break;
            case TOUCH_EVENT_CONTACT:
                Serial.println("接触中");
                break;
            case TOUCH_EVENT_NONE:
                Serial.println("无事件");
                break;
            default:
                Serial.println("未知");
                break;
        }
    }
    
    Serial.println("────────────────────────");
}

// Initialize touch state
void FT6336U_HardI2C::_initTouchState() {
    memset(&_touchState, 0, sizeof(_touchState));
    for (uint8_t i = 0; i < FT6336U_MAX_TOUCH_POINTS; i++) {
        _touchState.points[i].valid = false;
    }
}

// Initialize touch tracker
void FT6336U_HardI2C::_initTouchTracker() {
    memset(&_touchTracker, 0, sizeof(_touchTracker));
    for (uint8_t i = 0; i < FT6336U_MAX_TOUCH_POINTS; i++) {
        _touchTracker.active[i] = false;
        _touchTracker.lastActive[i] = 0;
        _touchTracker.releaseTime[i] = 0;
    }
    _touchTracker.lastCount = 0;
    _touchTracker.lastValidTouchTime = 0;
    _touchTracker.consecutiveNoTouchCount = 0;
    memset(_touchTracker.touchHistory, 0, sizeof(_touchTracker.touchHistory));
    _touchTracker.historyIndex = 0;
}

// Update touch events based on current state
// 在 _updateTouchEvents() 函数中添加更清晰的事件输出
void FT6336U_HardI2C::_updateTouchEvents() {
    uint8_t currentActive[FT6336U_MAX_TOUCH_POINTS] = {0};
    uint32_t currentTime = millis();
    
    // 标记当前活动的触摸点
    for (uint8_t i = 0; i < _touchState.count; i++) {
        if (_touchState.points[i].valid) {
            uint8_t id = _touchState.points[i].id;
            if (id < FT6336U_MAX_TOUCH_POINTS) {
                currentActive[id] = 1;
            }
        }
    }
    
    // 检测新按下事件
    for (uint8_t id = 0; id < FT6336U_MAX_TOUCH_POINTS; id++) {
        if (currentActive[id] && !_touchTracker.active[id]) {
            // 新的触摸点按下
            _touchTracker.active[id] = true;
            _touchTracker.lastActive[id] = currentTime;
            
            // 查找并标记为按下事件
            for (uint8_t i = 0; i < _touchState.count; i++) {
                if (_touchState.points[i].valid && _touchState.points[i].id == id) {
                    _touchState.points[i].event = TOUCH_EVENT_PRESS;
                    
                    // 输出更清晰的事件信息
                    if (_debugEnabled) {
                        Serial.print("🟢 [触摸点 ");
                        Serial.print(id);
                        Serial.print("] 按下 - 坐标(");
                        Serial.print(_touchState.points[i].x);
                        Serial.print(", ");
                        Serial.print(_touchState.points[i].y);
                        Serial.println(")");
                    }
                    break;
                }
            }
        }
    }
    
    // 检测持续接触事件
    for (uint8_t id = 0; id < FT6336U_MAX_TOUCH_POINTS; id++) {
        if (currentActive[id] && _touchTracker.active[id]) {
            _touchTracker.lastActive[id] = currentTime;
            
            // 更新事件：如果之前是按下，现在改为接触
            for (uint8_t i = 0; i < _touchState.count; i++) {
                if (_touchState.points[i].valid && _touchState.points[i].id == id) {
                    if (_touchState.points[i].event == TOUCH_EVENT_PRESS) {
                        _touchState.points[i].event = TOUCH_EVENT_CONTACT;
                        
                        // 输出接触事件（可选，避免输出太频繁）
                        if (_debugEnabled) {
                            static uint32_t lastContactPrintTime = 0;
                            if (currentTime - lastContactPrintTime > 500) { // 每500ms输出一次接触状态
                                lastContactPrintTime = currentTime;
                                Serial.print("👆 [触摸点 ");
                                Serial.print(id);
                                Serial.print("] 持续接触 - 坐标(");
                                Serial.print(_touchState.points[i].x);
                                Serial.print(", ");
                                Serial.print(_touchState.points[i].y);
                                Serial.println(")");
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    
    // 检测抬起事件（带防抖）
    for (uint8_t id = 0; id < FT6336U_MAX_TOUCH_POINTS; id++) {
        if (!currentActive[id] && _touchTracker.active[id]) {
            // 触摸点消失，检查是否为抬起
            if (currentTime - _touchTracker.lastActive[id] > 50) {  // 50ms防抖
                _touchTracker.active[id] = false;
                _touchTracker.releaseTime[id] = currentTime;
                
                // 输出抬起事件
                if (_debugEnabled) {
                    Serial.print("🔴 [触摸点 ");
                    Serial.print(id);
                    Serial.println("] 抬起");
                }
            }
        } else if (currentActive[id] && !_touchTracker.active[id]) {
            // 触摸点重新出现，可能是短暂的数据丢失，重新激活
            _touchTracker.active[id] = true;
            _touchTracker.lastActive[id] = currentTime;
        }
    }
}

// Read touch data from chip
void FT6336U_HardI2C::_readTouchData() {
    uint8_t touchData[14];  // 2 touch points × 7 bytes each
    uint8_t touchStatus;
    uint8_t validCount = 0;
    
    // Read touch status register
    if (!_readRegister(FT_REG_TD_STATUS, &touchStatus, 1)) {
        if (_debugEnabled) {
            Serial.println("FT6336U: Failed to read touch status");
        }
        return;
    }
    
    // Get reported touch count (lower 4 bits)
    uint8_t reportedCount = touchStatus & 0x0F;
    if (reportedCount > FT6336U_MAX_TOUCH_POINTS) {
        reportedCount = FT6336U_MAX_TOUCH_POINTS;
    }
    
    // Reset all touch point states
    for (uint8_t i = 0; i < FT6336U_MAX_TOUCH_POINTS; i++) {
        _touchState.points[i].valid = false;
    }
    
    // Read touch data if points are reported
    if (reportedCount > 0) {
        // Read first touch point (7 bytes)
        if (_readRegister(FT_TP1_REG, touchData, 7)) {
            // Parse touch point 1
            uint8_t xHigh = touchData[0] & 0x0F;
            uint8_t xLow = touchData[1];
            uint8_t touchId = (touchData[2] >> 4) & 0x0F;
            uint8_t yHigh = touchData[2] & 0x0F;
            uint8_t yLow = touchData[3];
            
            uint16_t x = (xHigh << 8) | xLow;
            uint16_t y = (yHigh << 8) | yLow;
            
            // Validate coordinates (FT6336U typically 0-2047)
            if (x <= 2047 && y <= 2047 && touchId < FT6336U_MAX_TOUCH_POINTS) {
                TouchPoint* point = &_touchState.points[validCount];
                
                point->id = touchId;
                point->x = x;
                point->y = y;
                point->area = 0;  // Area not used in basic mode
                point->event = TOUCH_EVENT_CONTACT;  // Default, will be updated
                point->valid = true;
                
                validCount++;
            }
        }
        
        // Read second touch point if reported
        if (reportedCount > 1) {
            if (_readRegister(FT_TP2_REG, &touchData[7], 7)) {
                // Parse touch point 2
                uint8_t xHigh = touchData[7] & 0x0F;
                uint8_t xLow = touchData[8];
                uint8_t touchId = (touchData[9] >> 4) & 0x0F;
                uint8_t yHigh = touchData[9] & 0x0F;
                uint8_t yLow = touchData[10];
                
                uint16_t x = (xHigh << 8) | xLow;
                uint16_t y = (yHigh << 8) | yLow;
                
                if (x <= 2047 && y <= 2047 && touchId < FT6336U_MAX_TOUCH_POINTS) {
                    // Check for duplicate ID
                    bool idExists = false;
                    for (uint8_t j = 0; j < validCount; j++) {
                        if (_touchState.points[j].id == touchId) {
                            idExists = true;
                            break;
                        }
                    }
                    
                    if (!idExists) {
                        TouchPoint* point = &_touchState.points[validCount];
                        
                        point->id = touchId;
                        point->x = x;
                        point->y = y;
                        point->area = 0;
                        point->event = TOUCH_EVENT_CONTACT;
                        point->valid = true;
                        
                        validCount++;
                    }
                }
            }
        }
    }
    
    // 更新触摸历史记录
    _touchTracker.touchHistory[_touchTracker.historyIndex] = validCount;
    _touchTracker.historyIndex = (_touchTracker.historyIndex + 1) % 4;
    
    // 基于历史数据进行防抖处理
    uint8_t stableCount = _getStableTouchCount();
    
    // 更新连续无触摸计数
    if (stableCount == 0) {
        _touchTracker.consecutiveNoTouchCount++;
    } else {
        _touchTracker.consecutiveNoTouchCount = 0;
        _touchTracker.lastValidTouchTime = millis();
    }
    
    // 只有当触摸状态稳定时才更新最终状态
    if (stableCount > 0 || _touchTracker.consecutiveNoTouchCount >= 3) {
        _touchState.count = stableCount;
        _touchState.timestamp = millis();
        
        // Update touch events
        _updateTouchEvents();
    }
    
    // Debug output
    if (_debugEnabled && stableCount > 0) {
        static uint8_t lastStableCount = 0;
        static uint32_t lastValidTime = 0;
        
        // 只有当触摸点数量稳定变化时才输出
        if (stableCount != lastStableCount) {
            uint32_t currentTime = millis();
            if (currentTime - lastValidTime > 100) {  // 100ms防抖
                Serial.print("FT6336U: Stable touch points: ");
                Serial.println(stableCount);
                lastStableCount = stableCount;
                lastValidTime = currentTime;
            }
        }
    }
}

// Write register via I2C
bool FT6336U_HardI2C::_writeRegister(uint8_t reg, uint8_t *data, uint8_t len) {
    Wire.beginTransmission(_i2cAddress);
    Wire.write(reg);
    
    for (uint8_t i = 0; i < len; i++) {
        Wire.write(data[i]);
    }
    
    uint8_t result = Wire.endTransmission();
    return (result == 0);
}

// Read register via I2C
bool FT6336U_HardI2C::_readRegister(uint8_t reg, uint8_t *data, uint8_t len) {
    Wire.beginTransmission(_i2cAddress);
    Wire.write(reg);
    
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(_i2cAddress, len);
    
    uint32_t startTime = millis();
    while (Wire.available() < len) {
        if (millis() - startTime > 100) {  // 100ms timeout
            return false;
        }
        delay(1);
    }
    
    for (uint8_t i = 0; i < len; i++) {
        data[i] = Wire.read();
    }
    
    return true;
}

// Get stable touch count based on history data
uint8_t FT6336U_HardI2C::_getStableTouchCount() {
    // 检查最近4次读取的触摸点数量
    uint8_t counts[4] = {0};
    uint8_t maxCount = 0;
    
    // 统计每个触摸点数量的出现次数
    for (uint8_t i = 0; i < 4; i++) {
        counts[_touchTracker.touchHistory[i]]++;
    }
    
    // 找到出现次数最多的触摸点数量
    for (uint8_t i = 0; i < 4; i++) {
        if (counts[i] > counts[maxCount]) {
            maxCount = i;
        }
    }
    
    // 只有当某个数量出现至少3次时才认为是稳定的
    if (counts[maxCount] >= 3) {
        return maxCount;
    }
    
    // 如果没有稳定的数量，返回最近一次读取的值
    return _touchTracker.touchHistory[(_touchTracker.historyIndex + 3) % 4];
}

// Static interrupt handler
void FT6336U_HardI2C::_interruptHandler(uint32_t id, uint32_t event) {
    if (_instance != nullptr) {
        _instance->_interruptOccurred = true;
    }
}