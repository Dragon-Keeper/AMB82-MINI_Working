/*
 * FT6336U触摸屏驱动库 - 头文件
 * 适用于AMB82-MINI开发板
 * 支持GPIO模拟I2C、多点触摸、事件检测
 */

#ifndef FT6336U_SOFTI2C_H
#define FT6336U_SOFTI2C_H

#include <Arduino.h>

// ===== 配置选项 =====
#ifndef USE_INTERRUPT_MODE
#define USE_INTERRUPT_MODE   1     // 0=轮询模式，1=中断模式
#endif

#ifndef ENABLE_DEBUG_INFO
#define ENABLE_DEBUG_INFO    1     // 1=启用调试信息，0=禁用
#endif

#ifndef MAX_TOUCH_POINTS
#define MAX_TOUCH_POINTS     2     // 硬件支持的最大触摸点数
#endif

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

// ===== 事件数据结构 =====
typedef struct {
    uint8_t id;           // 触摸点ID (0或1)
    uint16_t x;           // X坐标
    uint16_t y;           // Y坐标
    bool pressed;         // 是否刚按下
    bool released;        // 是否刚抬起
} TouchEvent;

// ===== FT6336U_SoftI2C类定义 =====
class FT6336U_SoftI2C {
public:
    // 构造函数（使用默认引脚）
    FT6336U_SoftI2C();
    
    // 构造函数（自定义引脚）
    FT6336U_SoftI2C(uint8_t sda_pin, uint8_t scl_pin, uint8_t rst_pin, uint8_t int_pin = 0);
    
    // 初始化函数
    void begin(bool debug = false);
    
    // 更新触摸数据（轮询模式下调用）
    void update();
    
    // 获取当前触摸点数
    uint8_t getTouchCount();
    
    // 获取指定触摸点的坐标
    bool getTouchPoint(uint8_t index, uint16_t *x, uint16_t *y);
    
    // 获取完整的触摸点信息
    bool getTouchPointFull(uint8_t index, TouchPoint *point);
    
    // 检查是否有触摸
    bool isTouched();
    
    // 检查特定ID的触摸点是否活跃
    bool isTouchActive(uint8_t id);
    
    // 检查是否有新事件（按下或抬起）
    bool hasNewEvent(TouchEvent *event);
    
    // 读取芯片信息
    void readChipInfo();
    
    // 配置触摸参数
    void configure(uint8_t threshold = 0x12);
    
    // 打印触摸状态（调试用）
    void printStatus();
    
    // 测试函数
    void testEvents();
    
    // 中断处理函数（中断模式下自动调用）
    void handleInterrupt();
    
    // 设置回调函数
    typedef void (*TouchCallback)(TouchEvent event);
    void setTouchCallback(TouchCallback callback);
    
    // 设置调试输出
    void setDebug(bool enable);
    
    // AMB82-MINI专用中断处理函数（适配digitalSetIrqHandler）
    static void amebaInterruptHandler(uint32_t id, uint32_t event);
    
private:
    // 引脚定义
    uint8_t pin_sda;
    uint8_t pin_scl;
    uint8_t pin_rst;
    uint8_t pin_int;
    
    // 状态变量
    TouchState touch_state;
    volatile bool interrupt_occurred;
    bool debug_mode;
    TouchCallback user_callback;
    
    // 触摸点状态跟踪器
    typedef struct {
        bool active[2];
        uint32_t last_active[2];
        uint8_t last_count;
        uint32_t release_time[2];
    } TouchTracker;
    
    TouchTracker touch_tracker;
    
    // ===== GPIO模拟I2C函数 =====
    void i2c_init();
    void i2c_start();
    void i2c_stop();
    uint8_t i2c_wait_ack();
    void i2c_send_ack();
    void i2c_send_nack();
    void i2c_write_byte(uint8_t data);
    uint8_t i2c_read_byte(bool send_ack);
    
    // ===== 设备操作函数 =====
    uint8_t writeRegister(uint8_t reg, uint8_t *data, uint8_t len);
    uint8_t readRegister(uint8_t reg, uint8_t *data, uint8_t len);
    
    // ===== 内部处理函数 =====
    void readTouchData();
    void updateTouchEvents();
    void resetDevice();
    
    // GPIO辅助函数
    inline void sda_input() { pinMode(pin_sda, INPUT); }
    inline void sda_output() { pinMode(pin_sda, OUTPUT); }
    inline void scl_high() { digitalWrite(pin_scl, HIGH); }
    inline void scl_low() { digitalWrite(pin_scl, LOW); }
    inline void sda_high() { digitalWrite(pin_sda, HIGH); }
    inline void sda_low() { digitalWrite(pin_sda, LOW); }
    inline bool sda_read() { return digitalRead(pin_sda); }
    inline void i2c_delay() { delayMicroseconds(3); }
    
    // 中断处理函数（静态，供Arduino中断使用）
    static void interruptHandler();
    static FT6336U_SoftI2C* instance;  // 静态实例指针
};

#endif // FT6336U_SOFTI2C_H