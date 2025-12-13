/*
 * FT6336U触摸屏驱动库 - 实现文件
 */

#include "FT6336U_SoftI2C.h"
#include <Arduino.h>

// ===== 静态成员初始化 =====
FT6336U_SoftI2C* FT6336U_SoftI2C::instance = nullptr;

// ===== FT6336U寄存器定义 =====
#define FT6336_ADDR          0x38    // 7位I2C地址
#define FT_CMD_WR            0x70    // 写命令 (0x38 << 1)
#define FT_CMD_RD            0x71    // 读命令 (0x38 << 1 | 1)

// 触摸状态寄存器
#define FT_REG_TD_STATUS     0x02    // 触摸状态（低4位为触摸点数）

// 触摸点数据寄存器
#define FT_TP1_REG           0x03    // 触摸点1起始地址
#define FT_TP2_REG           0x09    // 触摸点2起始地址

// 芯片信息寄存器
#define FT_REG_CHIP_ID_H     0xA3    // 芯片ID高字节
#define FT_REG_CHIP_ID_L     0xA4    // 芯片ID低字节
#define FT_REG_FIRMWARE_ID   0xA6    // 固件版本
#define FT_REG_VENDOR_ID     0xA8    // 厂商ID

// 配置寄存器
#define FT_REG_THGROUP       0x80    // 触摸阈值

// ===== 构造函数 =====
FT6336U_SoftI2C::FT6336U_SoftI2C() 
    : pin_sda(12), pin_scl(13), pin_rst(15), pin_int(16), 
      interrupt_occurred(false), debug_mode(false), user_callback(nullptr) {
    memset(&touch_state, 0, sizeof(TouchState));
    memset(&touch_tracker, 0, sizeof(TouchTracker));
    instance = this;
}

FT6336U_SoftI2C::FT6336U_SoftI2C(uint8_t sda_pin, uint8_t scl_pin, uint8_t rst_pin, uint8_t int_pin)
    : pin_sda(sda_pin), pin_scl(scl_pin), pin_rst(rst_pin), pin_int(int_pin),
      interrupt_occurred(false), debug_mode(false), user_callback(nullptr) {
    memset(&touch_state, 0, sizeof(TouchState));
    memset(&touch_tracker, 0, sizeof(TouchTracker));
    instance = this;
}

// ===== 初始化函数 =====
void FT6336U_SoftI2C::begin(bool debug) {
    debug_mode = debug;
    
    if (debug_mode) {
        Serial.println("初始化FT6336触摸屏...");
    }
    
    // 硬件复位
    pinMode(pin_rst, OUTPUT);
    digitalWrite(pin_rst, LOW);
    delay(50);
    digitalWrite(pin_rst, HIGH);
    delay(500);
    
    // 初始化I2C
    i2c_init();
    
    // 配置中断引脚（如果使用中断模式）
    #if USE_INTERRUPT_MODE
    if (pin_int > 0) {
        pinMode(pin_int, INPUT_IRQ_FALL);
        if (debug_mode) {
            Serial.print("配置中断引脚: GPIO");
            Serial.println(pin_int);
        }
    }
    #endif
    
    if (debug_mode) {
        Serial.println("FT6336初始化完成");
    }
}

// ===== GPIO模拟I2C函数 =====
void FT6336U_SoftI2C::i2c_init() {
    pinMode(pin_sda, OUTPUT);
    pinMode(pin_scl, OUTPUT);
    
    sda_high();
    scl_high();
}

void FT6336U_SoftI2C::i2c_start() {
    sda_output();
    sda_high();
    scl_high();
    i2c_delay();
    sda_low();
    i2c_delay();
    scl_low();
    i2c_delay();
}

void FT6336U_SoftI2C::i2c_stop() {
    sda_output();
    scl_low();
    i2c_delay();
    sda_low();
    i2c_delay();
    scl_high();
    i2c_delay();
    sda_high();
    i2c_delay();
}

uint8_t FT6336U_SoftI2C::i2c_wait_ack() {
    uint8_t timeout = 0;
    
    sda_input();
    sda_high();
    i2c_delay();
    scl_high();
    i2c_delay();
    
    while(sda_read()) {
        timeout++;
        if(timeout > 200) {
            i2c_stop();
            return 1;
        }
        delayMicroseconds(1);
    }
    
    scl_low();
    return 0;
}

void FT6336U_SoftI2C::i2c_send_ack() {
    scl_low();
    sda_output();
    i2c_delay();
    sda_low();
    i2c_delay();
    scl_high();
    i2c_delay();
    scl_low();
}

void FT6336U_SoftI2C::i2c_send_nack() {
    scl_low();
    sda_output();
    i2c_delay();
    sda_high();
    i2c_delay();
    scl_high();
    i2c_delay();
    scl_low();
}

void FT6336U_SoftI2C::i2c_write_byte(uint8_t data) {
    uint8_t i;
    
    sda_output();
    scl_low();
    
    for(i = 0; i < 8; i++) {
        if(data & 0x80) {
            sda_high();
        } else {
            sda_low();
        }
        data <<= 1;
        
        i2c_delay();
        scl_high();
        i2c_delay();
        scl_low();
        i2c_delay();
    }
}

uint8_t FT6336U_SoftI2C::i2c_read_byte(bool send_ack) {
    uint8_t i, data = 0;
    
    sda_input();
    
    for(i = 0; i < 8; i++) {
        scl_low();
        i2c_delay();
        scl_high();
        data <<= 1;
        
        if(sda_read()) {
            data |= 0x01;
        }
        
        i2c_delay();
    }
    
    if(send_ack) {
        i2c_send_ack();
    } else {
        i2c_send_nack();
    }
    
    return data;
}

// ===== 设备操作函数 =====
uint8_t FT6336U_SoftI2C::writeRegister(uint8_t reg, uint8_t *data, uint8_t len) {
    uint8_t i, ret = 0;
    
    i2c_start();
    i2c_write_byte(FT_CMD_WR);
    if(i2c_wait_ack()) {
        i2c_stop();
        return 1;
    }
    
    i2c_write_byte(reg);
    if(i2c_wait_ack()) {
        i2c_stop();
        return 1;
    }
    
    for(i = 0; i < len; i++) {
        i2c_write_byte(data[i]);
        ret = i2c_wait_ack();
        if(ret) {
            break;
        }
    }
    
    i2c_stop();
    return ret;
}

uint8_t FT6336U_SoftI2C::readRegister(uint8_t reg, uint8_t *data, uint8_t len) {
    uint8_t i;
    
    i2c_start();
    i2c_write_byte(FT_CMD_WR);
    if(i2c_wait_ack()) {
        i2c_stop();
        return 1;
    }
    
    i2c_write_byte(reg);
    if(i2c_wait_ack()) {
        i2c_stop();
        return 1;
    }
    
    i2c_start();
    i2c_write_byte(FT_CMD_RD);
    if(i2c_wait_ack()) {
        i2c_stop();
        return 1;
    }
    
    for(i = 0; i < len; i++) {
        data[i] = i2c_read_byte(i < (len - 1));
    }
    
    i2c_stop();
    return 0;
}

// ===== 内部处理函数 =====
void FT6336U_SoftI2C::readTouchData() {
    uint8_t buffer[12];
    uint8_t touch_count;
    
    // 读取触摸状态
    if(readRegister(FT_REG_TD_STATUS, buffer, 1)) {
        return;
    }
    
    touch_count = buffer[0] & 0x0F;
    if(touch_count > 2) touch_count = 2;
    
    touch_state.count = touch_count;
    
    // 重置所有触摸点状态
    for(uint8_t i = 0; i < 2; i++) {
        touch_state.points[i].valid = false;
    }
    
    // 读取触摸点数据
    if(touch_count > 0) {
        readRegister(FT_TP1_REG, buffer, 6);
        
        touch_state.points[0].id = (buffer[2] >> 4) & 0x0F;
        touch_state.points[0].x = ((buffer[0] & 0x0F) << 8) | buffer[1];
        touch_state.points[0].y = ((buffer[2] & 0x0F) << 8) | buffer[3];
        touch_state.points[0].area = buffer[4];
        touch_state.points[0].event = 2;  // 默认设为"接触"，后续由软件判断
        touch_state.points[0].valid = true;
    }
    
    if(touch_count > 1) {
        readRegister(FT_TP2_REG, buffer, 6);
        
        touch_state.points[1].id = (buffer[2] >> 4) & 0x0F;
        touch_state.points[1].x = ((buffer[0] & 0x0F) << 8) | buffer[1];
        touch_state.points[1].y = ((buffer[2] & 0x0F) << 8) | buffer[3];
        touch_state.points[1].area = buffer[4];
        touch_state.points[1].event = 2;  // 默认设为"接触"，后续由软件判断
        touch_state.points[1].valid = true;
    }
    
    touch_state.timestamp = millis();
}

void FT6336U_SoftI2C::updateTouchEvents() {
    static uint32_t last_update = 0;
    uint32_t current_time = millis();
    
    // 防抖处理，避免频繁更新
    if(current_time - last_update < 10) {
        return;
    }
    last_update = current_time;
    
    // 读取触摸数据
    readTouchData();
    
    // ===== 软件层面的事件跟踪 =====
    uint8_t current_active[2] = {0};
    
    // 标记当前活跃的触摸点（按ID）
    for(uint8_t i = 0; i < touch_state.count; i++) {
        if(touch_state.points[i].valid) {
            uint8_t id = touch_state.points[i].id;
            if(id < 2) {
                current_active[id] = 1;
            }
        }
    }
    
    // 检测新按下的点
    for(uint8_t id = 0; id < 2; id++) {
        if(current_active[id] && !touch_tracker.active[id]) {
            // 新触摸点按下
            touch_tracker.active[id] = true;
            touch_tracker.last_active[id] = current_time;
            
            // 找到对应的触摸点，设置事件为"按下"
            for(uint8_t i = 0; i < touch_state.count; i++) {
                if(touch_state.points[i].valid && touch_state.points[i].id == id) {
                    touch_state.points[i].event = 0; // 按下
                    break;
                }
            }
        }
    }
    
    // 检测持续接触的点
    for(uint8_t id = 0; id < 2; id++) {
        if(current_active[id] && touch_tracker.active[id]) {
            // 触摸点持续接触
            touch_tracker.last_active[id] = current_time;
            
            // 找到对应的触摸点，设置事件为"接触"
            for(uint8_t i = 0; i < touch_state.count; i++) {
                if(touch_state.points[i].valid && touch_state.points[i].id == id) {
                    // 如果之前已经是接触状态，保持为接触
                    // 如果之前是按下状态，现在改为接触
                    if(touch_state.points[i].event == 0) {
                        touch_state.points[i].event = 2; // 按下后变为接触
                    }
                    break;
                }
            }
        }
    }
    
    // 检测抬起的点（防抖处理）
    for(uint8_t id = 0; id < 2; id++) {
        if(!current_active[id] && touch_tracker.active[id]) {
            // 触摸点消失，检查是否需要标记为抬起
            if(current_time - touch_tracker.last_active[id] > 50) { // 50ms防抖
                // 触摸点抬起
                touch_tracker.active[id] = false;
                touch_tracker.release_time[id] = current_time;
            }
        }
    }
    
    touch_tracker.last_count = touch_state.count;
}

void FT6336U_SoftI2C::update() {
    #if USE_INTERRUPT_MODE
    // 中断模式下，检查中断标志
    if(interrupt_occurred) {
        interrupt_occurred = false;
        handleInterrupt();
    }
    #else
    // 轮询模式下，直接更新触摸数据
    readTouchData();
    updateTouchEvents();
    
    // 如果有用户回调函数，立即处理事件
    if(user_callback) {
        TouchEvent event;
        if(hasNewEvent(&event)) {
            user_callback(event);
        }
    }
    #endif
}

uint8_t FT6336U_SoftI2C::getTouchCount() {
    return touch_state.count;
}

bool FT6336U_SoftI2C::getTouchPoint(uint8_t index, uint16_t *x, uint16_t *y) {
    if(index >= 2 || !touch_state.points[index].valid) {
        return false;
    }
    
    *x = touch_state.points[index].x;
    *y = touch_state.points[index].y;
    return true;
}

bool FT6336U_SoftI2C::getTouchPointFull(uint8_t index, TouchPoint *point) {
    if(index >= 2 || !touch_state.points[index].valid) {
        return false;
    }
    
    *point = touch_state.points[index];
    return true;
}

bool FT6336U_SoftI2C::isTouched() {
    return touch_state.count > 0;
}

bool FT6336U_SoftI2C::isTouchActive(uint8_t id) {
    for(uint8_t i = 0; i < 2; i++) {
        if(touch_state.points[i].valid && touch_state.points[i].id == id) {
            return true;
        }
    }
    return false;
}

bool FT6336U_SoftI2C::hasNewEvent(TouchEvent *event) {
    static bool last_active[2] = {false, false};
    static uint32_t last_event_time[2] = {0, 0};
    uint32_t current_time = millis();
    bool found_event = false;
    
    // 检查按下事件
    for(uint8_t id = 0; id < 2; id++) {
        if(touch_tracker.active[id] && !last_active[id]) {
            // 新按下事件
            if(event) {
                // 找到对应的触摸点坐标
                event->id = id;
                event->pressed = true;
                event->released = false;
                
                for(uint8_t i = 0; i < touch_state.count; i++) {
                    if(touch_state.points[i].valid && touch_state.points[i].id == id) {
                        event->x = touch_state.points[i].x;
                        event->y = touch_state.points[i].y;
                        break;
                    }
                }
            }
            
            last_active[id] = true;
            last_event_time[id] = current_time;
            found_event = true;
            break; // 每次只报告一个事件
        }
    }
    
    // 检查抬起事件
    if(!found_event) {
        for(uint8_t id = 0; id < 2; id++) {
            if(!touch_tracker.active[id] && last_active[id]) {
                // 防抖：确保抬起事件之间有足够的时间间隔
                if(current_time - last_event_time[id] > 50) {
                    // 抬起事件
                    if(event) {
                        event->id = id;
                        event->x = 0;
                        event->y = 0;
                        event->pressed = false;
                        event->released = true;
                    }
                    
                    last_active[id] = false;
                    last_event_time[id] = current_time;
                    found_event = true;
                    break; // 每次只报告一个事件
                }
            }
        }
    }
    
    return found_event;
}

void FT6336U_SoftI2C::readChipInfo() {
    uint8_t buffer[4];
    
    if(readRegister(FT_REG_CHIP_ID_H, buffer, 2)) {
        return;
    }
    
    uint16_t chip_id = (buffer[0] << 8) | buffer[1];
    
    if(readRegister(FT_REG_FIRMWARE_ID, buffer, 1)) {
        return;
    }
    uint8_t firmware_id = buffer[0];
    
    if(readRegister(FT_REG_VENDOR_ID, buffer, 1)) {
        return;
    }
    uint8_t vendor_id = buffer[0];
    
    if(debug_mode) {
        Serial.print("芯片ID: 0x");
        Serial.println(chip_id, HEX);
        Serial.print("固件版本: 0x");
        Serial.println(firmware_id, HEX);
        Serial.print("厂商ID: 0x");
        Serial.println(vendor_id, HEX);
    }
}

void FT6336U_SoftI2C::configure(uint8_t threshold) {
    uint8_t data = threshold;
    
    if(writeRegister(FT_REG_THGROUP, &data, 1)) {
        if(debug_mode) {
            Serial.println("配置触摸阈值失败");
        }
    } else {
        if(debug_mode) {
            Serial.print("触摸阈值配置为: 0x");
            Serial.println(threshold, HEX);
        }
    }
}

void FT6336U_SoftI2C::printStatus() {
    static uint32_t last_print = 0;
    uint32_t current_time = millis();
    
    // 限制打印频率
    if(current_time - last_print < 500) {
        return;
    }
    last_print = current_time;
    
    Serial.print("触摸点数: ");
    Serial.println(touch_state.count);
    
    for(uint8_t i = 0; i < touch_state.count; i++) {
        if(touch_state.points[i].valid) {
            Serial.print("  触摸点");
            Serial.print(i);
            Serial.print(": ID=");
            Serial.print(touch_state.points[i].id);
            Serial.print(", X=");
            Serial.print(touch_state.points[i].x);
            Serial.print(", Y=");
            Serial.print(touch_state.points[i].y);
            Serial.print(", 事件=");
            
            switch(touch_state.points[i].event) {
                case 0: Serial.print("按下"); break;
                case 1: Serial.print("抬起"); break;
                case 2: Serial.print("接触"); break;
                case 3: Serial.print("无事件"); break;
                default: Serial.print("未知"); break;
            }
            Serial.println();
        }
    }
}

void FT6336U_SoftI2C::testEvents() {
    TouchEvent event;
    
    if(hasNewEvent(&event)) {
        if(event.pressed) {
            Serial.print("[测试] 按下事件 - ID:");
            Serial.print(event.id);
            Serial.print(" (");
            Serial.print(event.x);
            Serial.print(",");
            Serial.print(event.y);
            Serial.println(")");
        } else if(event.released) {
            Serial.print("[测试] 抬起事件 - ID:");
            Serial.println(event.id);
        }
    }
}

void FT6336U_SoftI2C::handleInterrupt() {
    // 中断发生时立即读取触摸数据，避免防抖延迟
    readTouchData();
    
    // 快速更新触摸事件，跳过防抖检查
    uint32_t current_time = millis();
    
    // ===== 软件层面的事件跟踪 =====
    uint8_t current_active[2] = {0};
    
    // 标记当前活跃的触摸点（按ID）
    for(uint8_t i = 0; i < touch_state.count; i++) {
        if(touch_state.points[i].valid) {
            uint8_t id = touch_state.points[i].id;
            if(id < 2) {
                current_active[id] = 1;
            }
        }
    }
    
    // 检测新按下的点
    for(uint8_t id = 0; id < 2; id++) {
        if(current_active[id] && !touch_tracker.active[id]) {
            // 新触摸点按下
            touch_tracker.active[id] = true;
            touch_tracker.last_active[id] = current_time;
            
            // 找到对应的触摸点，设置事件为"按下"
            for(uint8_t i = 0; i < touch_state.count; i++) {
                if(touch_state.points[i].valid && touch_state.points[i].id == id) {
                    touch_state.points[i].event = 0; // 按下
                    break;
                }
            }
        }
    }
    
    // 检测持续接触的点
    for(uint8_t id = 0; id < 2; id++) {
        if(current_active[id] && touch_tracker.active[id]) {
            // 触摸点持续接触
            touch_tracker.last_active[id] = current_time;
            
            // 找到对应的触摸点，设置事件为"接触"
            for(uint8_t i = 0; i < touch_state.count; i++) {
                if(touch_state.points[i].valid && touch_state.points[i].id == id) {
                    // 如果之前已经是接触状态，保持为接触
                    // 如果之前是按下状态，现在改为接触
                    if(touch_state.points[i].event == 0) {
                        touch_state.points[i].event = 2; // 按下后变为接触
                    }
                    break;
                }
            }
        }
    }
    
    // 检测抬起的点（防抖处理）
    for(uint8_t id = 0; id < 2; id++) {
        if(!current_active[id] && touch_tracker.active[id]) {
            // 触摸点消失，检查是否需要标记为抬起
            if(current_time - touch_tracker.last_active[id] > 50) { // 50ms防抖
                // 触摸点抬起
                touch_tracker.active[id] = false;
                touch_tracker.release_time[id] = current_time;
            }
        }
    }
    
    touch_tracker.last_count = touch_state.count;
    
    // 如果有用户回调函数，立即调用它
    if(user_callback) {
        TouchEvent event;
        if(hasNewEvent(&event)) {
            user_callback(event);
        }
    }
}

void FT6336U_SoftI2C::setTouchCallback(TouchCallback callback) {
    user_callback = callback;
}

void FT6336U_SoftI2C::setDebug(bool enable) {
    debug_mode = enable;
}

void FT6336U_SoftI2C::interruptHandler() {
    if(instance) {
        instance->interrupt_occurred = true;
    }
}

void FT6336U_SoftI2C::amebaInterruptHandler(uint32_t id, uint32_t event) {
    // 忽略参数，直接调用中断处理
    if(instance) {
        instance->interrupt_occurred = true;
    }
}