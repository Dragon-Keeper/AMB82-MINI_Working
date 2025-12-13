/**
 * @file InterruptExample.ino
 * @brief FT6336U_HardI2C 库中断模式示例
 * 
 * 本示例展示FT6336U_HardI2C库的中断模式使用方法。
 * 使用中断引脚检测触摸事件，提高响应速度并降低CPU使用率。
 *
 * 硬件连接：
 * - SDA: 引脚12 (AMB82-MINI硬件I2C0_SDA)
 * - SCL: 引脚13 (AMB82-MINI硬件I2C0_SCL)
 * - RST: 引脚15 (复位引脚)
 * - INT: 引脚16 (中断引脚，必需)
 * 
 * 特性：
 * 1. 中断模式实现即时响应
 * 2. 低功耗设计（CPU在无触摸时休眠）
 * 3. 清晰的触摸事件分类
 * 4. 两点触摸状态跟踪
 */

#include <FT6336U_HardI2C.h>

// 创建触摸控制器实例
FT6336U_HardI2C touch(12, 13, 15, 16); // 指定中断引脚

// 触摸事件结构体
TouchEvent touchEvent;

// 系统状态
enum SystemState {
    STATE_IDLE,      // 空闲状态
    STATE_TOUCHING,  // 触摸中
    STATE_MULTI_TOUCH // 多点触摸
};

SystemState currentState = STATE_IDLE;
uint32_t lastTouchTime = 0;
uint32_t idleStartTime = 0;

// 触摸点历史记录
struct TouchHistory {
    uint16_t x;
    uint16_t y;
    uint32_t timestamp;
} touchHistory[2][5]; // 每个触摸点存储最近5次位置

uint8_t historyIndex[2] = {0, 0};

void setup() {
    // 初始化串口通信
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n╔══════════════════════════════════════╗");
    Serial.println(  "║    FT6336U 中断模式示例             ║");
    Serial.println(  "╚══════════════════════════════════════╝");
    Serial.println("🔧 正在初始化触摸控制器（中断模式）...");
    
    // 初始化触摸历史记录
    memset(touchHistory, 0, sizeof(touchHistory));
    
    // 初始化触摸控制器
    touch.enableDebug(true);
    if (!touch.begin()) {
        Serial.println("❌ 触摸控制器初始化失败!");
        while (1) {
            delay(1000);
        }
    }
    
    // 启用中断模式（重要！）
    touch.setInterruptMode(true);
    Serial.println("🔔 中断模式已启用");
    
    // 设置较低的轮询间隔（中断模式下仍用于更新数据）
    touch.setPollingInterval(10);
    
    // 读取芯片信息
    if (touch.readChipInfo()) {
        Serial.println("✅ 芯片信息读取成功");
    }
    
    idleStartTime = millis();
    Serial.println("\n📱 系统已就绪，等待触摸中断...");
    Serial.println("────────────────────────");
}

void loop() {
    // 更新触摸数据（中断模式下由中断触发）
    touch.update();
    
    // 检查触摸事件
    bool eventProcessed = false;
    
    while (touch.hasNewEvent(&touchEvent)) {
        eventProcessed = true;
        lastTouchTime = millis();
        
        if (touchEvent.pressed) {
            handleTouchPress(touchEvent);
        } else if (touchEvent.released) {
            handleTouchRelease(touchEvent);
        }
    }
    
    // 检查触摸状态
    checkTouchStatus();
    
    // 在空闲状态下可以添加低功耗处理
    if (!eventProcessed && currentState == STATE_IDLE) {
        uint32_t idleDuration = millis() - idleStartTime;
        if (idleDuration > 5000) {
            // 每5秒显示一次空闲状态
            static uint32_t lastIdlePrint = 0;
            if (millis() - lastIdlePrint > 5000) {
                Serial.print("💤 系统空闲中... (已空闲 ");
                Serial.print(idleDuration / 1000);
                Serial.println(" 秒)");
                lastIdlePrint = millis();
            }
        }
        
        // 轻微延迟以降低CPU使用率
        delay(50);
    } else {
        // 有触摸活动时稍微快一些
        delay(10);
    }
}

/**
 * 处理触摸按下事件
 */
void handleTouchPress(TouchEvent &event) {
    Serial.print("🔔 [中断] 触摸点 ");
    Serial.print(event.id);
    Serial.print(" 按下 - 位置(");
    Serial.print(event.x);
    Serial.print(", ");
    Serial.print(event.y);
    Serial.println(")");
    
    // 更新触摸历史
    if (event.id < 2) {
        touchHistory[event.id][historyIndex[event.id]].x = event.x;
        touchHistory[event.id][historyIndex[event.id]].y = event.y;
        touchHistory[event.id][historyIndex[event.id]].timestamp = millis();
        historyIndex[event.id] = (historyIndex[event.id] + 1) % 5;
    }
    
    // 更新系统状态
    uint8_t touchCount = touch.getTouchCount();
    if (touchCount == 1) {
        currentState = STATE_TOUCHING;
        Serial.println("👆 进入单点触摸状态");
    } else if (touchCount >= 2) {
        currentState = STATE_MULTI_TOUCH;
        Serial.println("👆👆 进入多点触摸状态");
    }
}

/**
 * 处理触摸抬起事件
 */
void handleTouchRelease(TouchEvent &event) {
    Serial.print("🔔 [中断] 触摸点 ");
    Serial.print(event.id);
    Serial.print(" 抬起");
    
    // 计算触摸持续时间
    uint32_t pressTime = 0;
    for (int i = 0; i < 5; i++) {
        if (touchHistory[event.id][i].timestamp > 0) {
            pressTime = touchHistory[event.id][i].timestamp;
            break;
        }
    }
    
    if (pressTime > 0) {
        uint32_t duration = millis() - pressTime;
        Serial.print(" (持续时间: ");
        Serial.print(duration);
        Serial.print("ms)");
        
        // 判断触摸类型
        if (duration < 200) {
            Serial.print(" - 轻触点击");
        } else if (duration < 1000) {
            Serial.print(" - 普通触摸");
        } else {
            Serial.print(" - 长按");
        }
        
        // 清除历史记录
        for (int i = 0; i < 5; i++) {
            touchHistory[event.id][i].timestamp = 0;
        }
    }
    Serial.println();
}

/**
 * 检查当前触摸状态
 */
void checkTouchStatus() {
    static uint32_t lastStatusTime = 0;
    uint32_t currentTime = millis();
    
    // 每500ms更新一次状态显示
    if (currentTime - lastStatusTime > 500) {
        lastStatusTime = currentTime;
        
        uint8_t touchCount = touch.getTouchCount();
        
        // 状态转换处理
        if (touchCount == 0 && currentState != STATE_IDLE) {
            currentState = STATE_IDLE;
            idleStartTime = millis();
            Serial.println("💤 返回空闲状态");
            Serial.println("────────────────────────");
        } else if (touchCount == 1 && currentState != STATE_TOUCHING) {
            currentState = STATE_TOUCHING;
        } else if (touchCount >= 2 && currentState != STATE_MULTI_TOUCH) {
            currentState = STATE_MULTI_TOUCH;
        }
        
        // 显示当前触摸状态
        switch (currentState) {
            case STATE_IDLE:
                // 空闲状态不频繁显示
                break;
                
            case STATE_TOUCHING:
                {
                    uint16_t x, y;
                    if (touch.getTouchPoint(0, &x, &y)) {
                        Serial.print("📍 单点触摸中 - 位置(");
                        Serial.print(x);
                        Serial.print(", ");
                        Serial.print(y);
                        Serial.println(")");
                    }
                }
                break;
                
            case STATE_MULTI_TOUCH:
                Serial.print("📍📍 多点触摸中 - ");
                Serial.print(touchCount);
                Serial.println("个点");
                
                // 显示所有触摸点
                for (uint8_t i = 0; i < touchCount; i++) {
                    uint16_t x, y;
                    if (touch.getTouchPoint(i, &x, &y)) {
                        Serial.print("  点");
                        Serial.print(i + 1);
                        Serial.print(": (");
                        Serial.print(x);
                        Serial.print(", ");
                        Serial.print(y);
                        Serial.print(")");
                        
                        // 显示触摸ID
                        for (uint8_t id = 0; id < 2; id++) {
                            if (touch.isTouchActive(id)) {
                                Serial.print(" [ID:");
                                Serial.print(id);
                                Serial.print("]");
                                break;
                            }
                        }
                        Serial.println();
                    }
                }
                break;
        }
    }
}

/**
 * 中断处理函数（如果需要自定义中断处理）
 * 注意：库中已经有默认的中断处理，此函数仅作示例
 */
void onInterrupt() {
    // 自定义中断处理代码
    // 注意：在中断服务程序中不要做耗时操作
    // digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}