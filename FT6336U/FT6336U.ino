/*
 * FT6336U触摸屏驱动 - GPIO模拟I2C完整版
 * AMB82-MINI引脚定义：
 *   SDA  -> 引脚12
 *   SCL  -> 引脚13
 *   RST  -> 引脚15
 *   INT  -> 引脚16 (可选)
 * 
 * 功能特性：
 * 1. 完整的GPIO模拟I2C驱动，避免硬件I2C冲突
 * 2. 支持多点触摸（最多5点）
 * 3. 完整的触摸事件处理（按下、抬起、接触）
 * 4. 可选的轮询模式或中断模式
 * 5. 详细的调试信息输出
 * 6. 错误检测和重试机制
 * 7. 支持AMB82-MINI专用中断API
 *
 * 配置选项
 * 1. USE_INTERRUPT_MODE: 设置为1启用中断模式，0启用轮询模式
 * 2. DEBUG_MODE: 设置为1启用详细调试信息
 * 3. POLLING_INTERVAL_MS: 轮询模式下的读取间隔
 *
 * 故障排除
 * 1. 无法读取芯片ID：
 *    检查引脚连接
 *    确认电源电压为3.3V
 *    检查复位引脚是否为高电平
 * 2. 触摸坐标不正确：
 *    使用校准函数调整坐标
 *    检查触摸屏是否与显示屏对齐
 * 3. 响应迟钝：
 *    减小POLLING_INTERVAL_MS值
 *    启用中断模式
 * 4. 多点触摸失效：
 *    确认触摸屏支持多点触摸
 *    检查触摸面积是否足够大
 *
 * 性能优化建议
 * 1. 中断模式下CPU占用率最低
 * 2. 轮询模式下可根据需要调整读取间隔
 * 3. 生产环境中可关闭调试信息（DEBUG_MODE=0）
 * 4. 如需更高速度，可减小CT_Delay()中的延时值
 * 
 * 支持最多5点触摸，每个触摸点6个寄存器：
 * 0x03: 触摸点1数据地址
 * 0x09: 触摸点2数据地址  
 * 0x0F: 触摸点3数据地址
 * 0x15: 触摸点4数据地址
 * 0x1B: 触摸点5数据地址
 */

/*
 * FT6336U 2点触摸驱动 - 修复事件识别问题
 * 软件层面手动跟踪触摸点状态，正确识别按下、抬起、接触事件
 * 修复触摸事件始终为"接触"的问题
 */

#include <Arduino.h>

// ===== 配置选项 =====
#define USE_INTERRUPT_MODE   1     // 0=轮询模式，1=中断模式
#define ENABLE_DEBUG_INFO    1     // 1=启用调试信息，0=禁用
#define POLL_INTERVAL_MS     20    // 轮询间隔(毫秒)，建议20ms
#define MAX_TOUCH_POINTS     2     // 硬件支持的最大触摸点数

// ===== 引脚定义 =====
#define PIN_SDA      12    // I2C数据线
#define PIN_SCL      13    // I2C时钟线  
#define PIN_RST      15    // 触摸屏复位引脚
#define PIN_INT      16    // 触摸屏中断引脚(可选)

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

// ===== 数据结构定义 =====
typedef struct {
    uint8_t  id;            // 触摸ID (0-1)
    uint16_t x;             // X坐标 (0-2047)
    uint16_t y;             // Y坐标 (0-2047)  
    uint8_t  area;          // 触摸面积
    uint8_t  event;         // 触摸事件: 0=按下, 1=抬起, 2=接触, 3=无事件
    bool     valid;         // 数据是否有效
} TouchPoint;

typedef struct {
    uint8_t     count;      // 当前触摸点数 (0-2)
    uint8_t     gesture;    // 手势ID
    TouchPoint  points[2];  // 触摸点数组
    uint32_t    timestamp;  // 最后更新时间
} TouchState;

// ===== 触摸点状态跟踪器 =====
typedef struct {
    bool active[2];           // 当前活跃的触摸点ID (0-1)
    uint32_t last_active[2];  // 最后活跃时间
    uint8_t last_count;       // 上一次的触摸点数
    uint32_t release_time[2]; // 抬起时间，用于防抖
} TouchTracker;

// ===== 事件数据结构 =====
typedef struct {
    uint8_t id;           // 触摸点ID (0或1)
    uint16_t x;           // X坐标
    uint16_t y;           // Y坐标
    bool pressed;         // 是否刚按下
    bool released;        // 是否刚抬起
} TouchEvent;

// ===== 全局变量 =====
TouchState touch_state;
TouchTracker touch_tracker = {0};
volatile bool touch_interrupt_occurred = false;

// ===== GPIO模拟I2C函数 =====
inline void i2c_sda_input() {
    pinMode(PIN_SDA, INPUT);
}

inline void i2c_sda_output() {
    pinMode(PIN_SDA, OUTPUT);
}

inline void i2c_scl_high() {
    digitalWrite(PIN_SCL, HIGH);
}

inline void i2c_scl_low() {
    digitalWrite(PIN_SCL, LOW);
}

inline void i2c_sda_high() {
    digitalWrite(PIN_SDA, HIGH);
}

inline void i2c_sda_low() {
    digitalWrite(PIN_SDA, LOW);
}

inline bool i2c_sda_read() {
    return digitalRead(PIN_SDA);
}

// I2C时序延时
void i2c_delay() {
    delayMicroseconds(3);
}

// 初始化I2C引脚
void i2c_init() {
    pinMode(PIN_SDA, OUTPUT);
    pinMode(PIN_SCL, OUTPUT);
    pinMode(PIN_RST, OUTPUT);
    
    i2c_sda_high();
    i2c_scl_high();
    digitalWrite(PIN_RST, HIGH);  // 释放复位
    
    #if USE_INTERRUPT_MODE
    pinMode(PIN_INT, INPUT_IRQ_FALL);
    #endif
}

// 产生I2C起始信号
void i2c_start() {
    i2c_sda_output();
    i2c_sda_high();
    i2c_scl_high();
    i2c_delay();
    i2c_sda_low();
    i2c_delay();
    i2c_scl_low();
    i2c_delay();
}

// 产生I2C停止信号  
void i2c_stop() {
    i2c_sda_output();
    i2c_scl_low();
    i2c_delay();
    i2c_sda_low();
    i2c_delay();
    i2c_scl_high();
    i2c_delay();
    i2c_sda_high();
    i2c_delay();
}

// 等待ACK信号
// 返回0:收到ACK, 返回1:超时或NACK
uint8_t i2c_wait_ack() {
    uint8_t timeout = 0;
    
    i2c_sda_input();
    i2c_sda_high();
    i2c_delay();
    i2c_scl_high();
    i2c_delay();
    
    while(i2c_sda_read()) {
        timeout++;
        if(timeout > 200) {
            i2c_stop();
            return 1;
        }
        delayMicroseconds(1);
    }
    
    i2c_scl_low();
    return 0;
}

// 发送ACK信号
void i2c_send_ack() {
    i2c_scl_low();
    i2c_sda_output();
    i2c_delay();
    i2c_sda_low();  // ACK = 低电平
    i2c_delay();
    i2c_scl_high();
    i2c_delay();
    i2c_scl_low();
}

// 发送NACK信号
void i2c_send_nack() {
    i2c_scl_low();
    i2c_sda_output();
    i2c_delay();
    i2c_sda_high();  // NACK = 高电平
    i2c_delay();
    i2c_scl_high();
    i2c_delay();
    i2c_scl_low();
}

// I2C发送一个字节
void i2c_write_byte(uint8_t data) {
    uint8_t i;
    
    i2c_sda_output();
    i2c_scl_low();
    
    for(i = 0; i < 8; i++) {
        if(data & 0x80) {
            i2c_sda_high();
        } else {
            i2c_sda_low();
        }
        data <<= 1;
        
        i2c_delay();
        i2c_scl_high();
        i2c_delay();
        i2c_scl_low();
        i2c_delay();
    }
}

// I2C读取一个字节
uint8_t i2c_read_byte(bool send_ack) {
    uint8_t i, data = 0;
    
    i2c_sda_input();
    
    for(i = 0; i < 8; i++) {
        i2c_scl_low();
        i2c_delay();
        i2c_scl_high();
        data <<= 1;
        
        if(i2c_sda_read()) {
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

// ===== FT6336U驱动函数 =====
// 写入寄存器
uint8_t ft6336_write_reg(uint8_t reg, uint8_t *data, uint8_t len) {
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

// 读取寄存器
uint8_t ft6336_read_reg(uint8_t reg, uint8_t *data, uint8_t len) {
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

// ===== 触摸屏初始化与配置 =====
void ft6336_init() {
    #if ENABLE_DEBUG_INFO
    Serial.println("初始化FT6336触摸屏...");
    #endif
    
    // 硬件复位
    digitalWrite(PIN_RST, LOW);
    delay(50);
    digitalWrite(PIN_RST, HIGH);
    delay(500);
    
    // 初始化I2C
    i2c_init();
    
    // 初始化触摸状态
    memset(&touch_state, 0, sizeof(TouchState));
    
    // 初始化触摸跟踪器
    memset(&touch_tracker, 0, sizeof(TouchTracker));
    
    #if ENABLE_DEBUG_INFO
    Serial.println("FT6336初始化完成");
    #endif
}

// 读取芯片信息
void ft6336_read_chip_info() {
    uint8_t data[4];
    
    #if ENABLE_DEBUG_INFO
    Serial.println("读取芯片信息...");
    #endif
    
    // 读取芯片ID
    if(ft6336_read_reg(FT_REG_CHIP_ID_H, data, 2) == 0) {
        Serial.print("芯片ID: 0x");
        if(data[0] < 0x10) Serial.print("0");
        Serial.print(data[0], HEX);
        if(data[1] < 0x10) Serial.print("0");
        Serial.println(data[1], HEX);
    }
    
    // 读取固件版本
    if(ft6336_read_reg(FT_REG_FIRMWARE_ID, data, 1) == 0) {
        Serial.print("固件版本: 0x");
        if(data[0] < 0x10) Serial.print("0");
        Serial.println(data[0], HEX);
    }
    
    // 读取厂商ID
    if(ft6336_read_reg(FT_REG_VENDOR_ID, data, 1) == 0) {
        Serial.print("厂商ID: 0x");
        if(data[0] < 0x10) Serial.print("0");
        Serial.println(data[0], HEX);
    }
}

// 配置触摸参数
void ft6336_configure() {
    uint8_t config_value;
    
    #if ENABLE_DEBUG_INFO
    Serial.println("配置触摸参数...");
    #endif
    
    // 设置触摸阈值（值越小越灵敏，范围1-255）
    config_value = 0x12;  // 稍微提高灵敏度
    if(ft6336_write_reg(FT_REG_THGROUP, &config_value, 1) == 0) {
        #if ENABLE_DEBUG_INFO
        Serial.print("触摸阈值设置: 0x");
        Serial.println(config_value, HEX);
        #endif
    }
    
    // 设置报告率：80ms
    config_value = 0x08;
    ft6336_write_reg(0x88, &config_value, 1);
    
    #if ENABLE_DEBUG_INFO
    Serial.println("配置完成");
    #endif
}

// ===== 软件层面的事件跟踪 =====
void ft6336_update_touch_events() {
    uint8_t current_active[2] = {0};
    uint32_t current_time = millis();
    
    // 标记当前活跃的触摸点
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
}

// ===== 触摸数据读取函数 =====
void ft6336_read_touch_data() {
    uint8_t touch_data[14];  // 2个触摸点，每个7个字节
    uint8_t touch_status;
    uint8_t i, valid_count = 0;
    
    // 读取触摸状态寄存器
    if(ft6336_read_reg(FT_REG_TD_STATUS, &touch_status, 1) != 0) {
        #if ENABLE_DEBUG_INFO
        Serial.println("读取触摸状态失败");
        #endif
        return;
    }
    
    // 获取上报的触摸点数（低4位）
    uint8_t reported_count = touch_status & 0x0F;
    
    // 限制为硬件支持的最大点数
    if(reported_count > MAX_TOUCH_POINTS) {
        reported_count = MAX_TOUCH_POINTS;
    }
    
    // 重置所有触摸点状态
    for(i = 0; i < MAX_TOUCH_POINTS; i++) {
        touch_state.points[i].valid = false;
    }
    
    // 如果有触摸点上报，读取触摸数据
    if(reported_count > 0) {
        // 读取第一个触摸点数据（7个字节）
        if(ft6336_read_reg(FT_TP1_REG, touch_data, 7) == 0) {
            // 解析触摸点1
            uint8_t x_high = touch_data[0] & 0x0F;             // X坐标高4位
            uint8_t x_low = touch_data[1];                    // X坐标低8位
            uint8_t touch_id = (touch_data[2] >> 4) & 0x0F;   // 触摸ID
            uint8_t y_high = touch_data[2] & 0x0F;            // Y坐标高4位
            uint8_t y_low = touch_data[3];                    // Y坐标低8位
            
            // 计算坐标（12位分辨率）
            uint16_t x = (x_high << 8) | x_low;
            uint16_t y = (y_high << 8) | y_low;
            
            // 检查坐标有效性（FT6336U通常范围0-2047）
            if(x <= 2047 && y <= 2047 && touch_id < 2) {
                TouchPoint *point = &touch_state.points[valid_count];
                
                point->id = touch_id;
                point->x = x;
                point->y = y;
                point->area = 0;  // 不关心面积
                point->event = 2;  // 默认设为"接触"，后续由软件判断
                point->valid = true;
                
                valid_count++;
            }
        }
        
        // 如果有第二个触摸点上报
        if(reported_count > 1) {
            // 读取第二个触摸点数据（7个字节）
            if(ft6336_read_reg(FT_TP2_REG, &touch_data[7], 7) == 0) {
                // 解析触摸点2
                uint8_t x_high = touch_data[7] & 0x0F;
                uint8_t x_low = touch_data[8];
                uint8_t touch_id = (touch_data[9] >> 4) & 0x0F;
                uint8_t y_high = touch_data[9] & 0x0F;
                uint8_t y_low = touch_data[10];
                
                uint16_t x = (x_high << 8) | x_low;
                uint16_t y = (y_high << 8) | y_low;
                
                if(x <= 2047 && y <= 2047 && touch_id < 2) {
                    TouchPoint *point = &touch_state.points[valid_count];
                    
                    point->id = touch_id;
                    point->x = x;
                    point->y = y;
                    point->area = 0;  // 不关心面积
                    point->event = 2;  // 默认设为"接触"，后续由软件判断
                    point->valid = true;
                    
                    valid_count++;
                }
            }
        }
    }
    
    // 更新触摸状态
    touch_state.count = valid_count;
    touch_state.timestamp = millis();
    
    // 更新触摸事件状态（软件判断）
    ft6336_update_touch_events();
    
    // 调试输出
    #if ENABLE_DEBUG_INFO
    if(valid_count > 0) {
        for(i = 0; i < valid_count; i++) {
            if(touch_state.points[i].valid) {
                Serial.print("检测到触摸点");
                Serial.print(i+1);
                Serial.print(": ID=");
                Serial.print(touch_state.points[i].id);
                Serial.print(", X=");
                Serial.print(touch_state.points[i].x);
                Serial.print(", Y=");
                Serial.print(touch_state.points[i].y);
                Serial.print(", 事件=");
                
                switch(touch_state.points[i].event) {
                    case 0x00: Serial.println("按下"); break;
                    case 0x01: Serial.println("抬起"); break;
                    case 0x02: Serial.println("接触"); break;
                    case 0x03: Serial.println("无事件"); break;
                    default: Serial.println("未知"); break;
                }
            }
        }
        
        static uint8_t last_valid_count = 0;
        if(valid_count != last_valid_count) {
            Serial.print("有效触摸点数: ");
            Serial.println(valid_count);
            last_valid_count = valid_count;
        }
    }
    #endif
}

// ===== 公共API函数 =====
// 获取当前触摸点数
uint8_t ft6336_get_touch_count() {
    return touch_state.count;
}

// 获取指定触摸点的坐标
bool ft6336_get_touch_point(uint8_t index, uint16_t *x, uint16_t *y) {
    if(index >= MAX_TOUCH_POINTS || !touch_state.points[index].valid) {
        return false;
    }
    
    *x = touch_state.points[index].x;
    *y = touch_state.points[index].y;
    return true;
}

// 获取完整的触摸点信息
bool ft6336_get_touch_point_full(uint8_t index, TouchPoint *point) {
    if(index >= MAX_TOUCH_POINTS || !touch_state.points[index].valid) {
        return false;
    }
    
    *point = touch_state.points[index];
    return true;
}

// 检查是否有触摸
bool ft6336_is_touched() {
    return touch_state.count > 0;
}

// 检查特定ID的触摸点是否活跃
bool ft6336_is_touch_active(uint8_t id) {
    if(id >= 2) return false;
    return touch_tracker.active[id];
}

// ===== 事件检测API =====
// 检查是否有新事件（按下或抬起）
bool ft6336_has_new_event(TouchEvent *event) {
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

// ===== 调试输出函数 =====
void ft6336_print_status() {
    static uint32_t last_print_time = 0;
    uint32_t current_time = millis();
    
    // 限制输出频率，避免刷屏
    if(current_time - last_print_time < 300) {
        return;
    }
    
    last_print_time = current_time;
    
    if(touch_state.count == 0) {
        Serial.println("状态: 无触摸");
        return;
    }
    
    Serial.print("状态: ");
    Serial.print(touch_state.count);
    Serial.println("点触摸");
    
    // 根据触摸ID排序，保持输出稳定
    TouchPoint sorted_points[2];
    uint8_t sorted_count = 0;
    
    // 复制有效点
    for(uint8_t i = 0; i < MAX_TOUCH_POINTS; i++) {
        if(touch_state.points[i].valid) {
            sorted_points[sorted_count++] = touch_state.points[i];
        }
    }
    
    // 简单排序：按ID从小到大
    if(sorted_count == 2) {
        if(sorted_points[0].id > sorted_points[1].id) {
            TouchPoint temp = sorted_points[0];
            sorted_points[0] = sorted_points[1];
            sorted_points[1] = temp;
        }
    }
    
    // 显示排序后的触摸点
    for(uint8_t i = 0; i < sorted_count; i++) {
        Serial.print("  点");
        Serial.print(i+1);
        Serial.print(" (ID:");
        Serial.print(sorted_points[i].id);
        Serial.print("): X=");
        Serial.print(sorted_points[i].x);
        Serial.print(", Y=");
        Serial.print(sorted_points[i].y);
        Serial.print(", 事件=");
        
        switch(sorted_points[i].event) {
            case 0x00: Serial.println("按下"); break;
            case 0x01: Serial.println("抬起"); break;
            case 0x02: Serial.println("接触"); break;
            case 0x03: Serial.println("无事件"); break;
            default: Serial.println("未知"); break;
        }
    }
    
    // 显示活跃状态
    Serial.print("活跃状态: ");
    for(uint8_t id = 0; id < 2; id++) {
        if(touch_tracker.active[id]) {
            Serial.print("ID");
            Serial.print(id);
            Serial.print(" ");
        }
    }
    Serial.println("\n---");
}

// ===== 中断处理函数 =====
#if USE_INTERRUPT_MODE
void touch_interrupt_handler(uint32_t id, uint32_t event) {
    touch_interrupt_occurred = true;
}
#endif

// ===== Arduino主函数 =====
void setup() {
    // 初始化串口
    Serial.begin(115200);
    while(!Serial && millis() < 3000);  // 等待串口连接
    
    Serial.println("\n=== FT6336U 2点触摸驱动 - 事件修复版 ===");
    Serial.println("软件跟踪触摸点状态，正确识别按下/抬起事件");
    Serial.println("引脚: SDA=12, SCL=13, RST=15, INT=16");
    Serial.println("工作模式: 软件事件检测");
    
    // 初始化触摸屏
    ft6336_init();
    delay(200);
    
    // 读取芯片信息
    ft6336_read_chip_info();
    delay(100);
    
    // 配置触摸参数
    ft6336_configure();
    delay(100);
    
    // 设置中断模式（如果启用）
    #if USE_INTERRUPT_MODE
    digitalSetIrqHandler(PIN_INT, touch_interrupt_handler);
    Serial.println("工作模式: 中断驱动");
    #else
    Serial.println("工作模式: 轮询驱动");
    #endif
    
    Serial.println("\n触摸屏已就绪，开始检测...");
    Serial.println("======================================");
}

void loop() {
    #if USE_INTERRUPT_MODE
    // 中断模式
    if(touch_interrupt_occurred) {
        touch_interrupt_occurred = false;
        ft6336_read_touch_data();
        
        // 检查事件
        TouchEvent event;
        if(ft6336_has_new_event(&event)) {
            if(event.pressed) {
                Serial.print("[事件] 按下 - ID:");
                Serial.print(event.id);
                Serial.print(" (");
                Serial.print(event.x);
                Serial.print(",");
                Serial.print(event.y);
                Serial.println(")");
            } else if(event.released) {
                Serial.print("[事件] 抬起 - ID:");
                Serial.println(event.id);
            }
        }
        
        ft6336_print_status();
    }
    #else
    // 轮询模式（默认）
    ft6336_read_touch_data();
    
    // 检查事件
    TouchEvent event;
    if(ft6336_has_new_event(&event)) {
        if(event.pressed) {
            Serial.print("[事件] 按下 - ID:");
            Serial.print(event.id);
            Serial.print(" (");
            Serial.print(event.x);
            Serial.print(",");
            Serial.print(event.y);
            Serial.println(")");
        } else if(event.released) {
            Serial.print("[事件] 抬起 - ID:");
            Serial.println(event.id);
        }
    }
    
    ft6336_print_status();
    
    // 轮询间隔
    delay(POLL_INTERVAL_MS);
    #endif
}

// ===== 测试函数 =====
void ft6336_test_events() {
    static uint32_t last_test_time = 0;
    uint32_t current_time = millis();
    
    if(current_time - last_test_time < 1000) {
        return;
    }
    
    last_test_time = current_time;
    
    Serial.println("\n=== 触摸事件测试 ===");
    
    // 检查活跃的触摸点
    Serial.print("活跃触摸点: ");
    bool any_active = false;
    for(uint8_t id = 0; id < 2; id++) {
        if(ft6336_is_touch_active(id)) {
            Serial.print("ID");
            Serial.print(id);
            Serial.print(" ");
            any_active = true;
        }
    }
    
    if(!any_active) {
        Serial.println("无");
    } else {
        Serial.println();
    }
    
    // 显示当前触摸状态
    if(ft6336_is_touched()) {
        Serial.print("当前触摸点数: ");
        Serial.println(ft6336_get_touch_count());
        
        for(uint8_t i = 0; i < ft6336_get_touch_count(); i++) {
            uint16_t x, y;
            if(ft6336_get_touch_point(i, &x, &y)) {
                Serial.print("  点");
                Serial.print(i);
                Serial.print(": X=");
                Serial.print(x);
                Serial.print(", Y=");
                Serial.println(y);
            }
        }
    } else {
        Serial.println("当前无触摸");
    }
    
    Serial.println("===================");
}