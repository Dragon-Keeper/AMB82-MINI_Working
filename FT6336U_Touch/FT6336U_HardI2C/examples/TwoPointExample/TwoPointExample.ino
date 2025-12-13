
/**
 * @file TwoPointExample.ino
 * @brief FT6336U_HardI2C 库两点触摸示例 - 优化版
 * 
 * 本示例展示FT6336U_HardI2C库的两点触摸功能。
 * 支持同时检测和跟踪两个触摸点。
 * 可选轮询模式或中断模式，默认轮询模式。
 *
 * 硬件连接：
 * - SDA: 引脚12 (AMB82-MINI硬件I2C0_SDA)
 * - SCL: 引脚13 (AMB82-MINI硬件I2C0_SCL)
 * - RST: 引脚15 (复位引脚)
 * - INT: 引脚16 (中断引脚，可选)
 * 
 * 优化特性：
 * 1. 更清晰的触摸事件输出（带图标和状态标识）
 * 2. 明确区分单点/多点触摸
 * 3. 详细的触摸点跟踪信息
 */

#include <FT6336U_HardI2C.h>

// 创建触摸控制器实例
FT6336U_HardI2C touch;

// 触摸事件结构体
TouchEvent currentEvent;

// 触摸点状态跟踪
struct TouchPointState {
    bool active;
    uint16_t x;
    uint16_t y;
    uint32_t touchStartTime;
} touchPoints[2];

void setup() {
    // 初始化串口通信
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n╔══════════════════════════════════════╗");
    Serial.println(  "║    FT6336U 两点触摸示例 - 优化版    ║");
    Serial.println(  "╚══════════════════════════════════════╝");
    Serial.println("正在初始化触摸控制器...");
    
    // 初始化触摸点状态
    memset(touchPoints, 0, sizeof(touchPoints));
    
    // 初始化触摸控制器
    touch.enableDebug(true);  // 启用详细调试输出
    if (!touch.begin()) {
        Serial.println("❌ 触摸控制器初始化失败!");
        while (1) {
            delay(1000);
        }
    }
    
    // 默认使用轮询模式
    // touch.setInterruptMode(true);  // 取消注释以使用中断模式
    
    // 设置轮询间隔为20ms
    touch.setPollingInterval(20);
    
    Serial.println("✅ 触摸控制器初始化成功");
    Serial.println("📱 尝试以下操作：");
    Serial.println("  1. 单点触摸");
    Serial.println("  2. 两点同时触摸");
    Serial.println("  3. 拖动手指");
    Serial.println("  4. 快速点击");
    Serial.println();
    
    // 读取芯片信息
    touch.readChipInfo();
    Serial.println("────────────────────────");
}

void loop() {
    // 更新触摸数据
    touch.update();
    
    // 检查新的触摸事件（按下/抬起）
    if (touch.hasNewEvent(&currentEvent)) {
        if (currentEvent.pressed) {
            Serial.print("🎯 [事件] 触摸点 ");
            Serial.print(currentEvent.id);
            Serial.print(" 按下 - 位置(");
            Serial.print(currentEvent.x);
            Serial.print(", ");
            Serial.print(currentEvent.y);
            Serial.println(")");
            
            // 更新触摸点状态
            if (currentEvent.id < 2) {
                touchPoints[currentEvent.id].active = true;
                touchPoints[currentEvent.id].x = currentEvent.x;
                touchPoints[currentEvent.id].y = currentEvent.y;
                touchPoints[currentEvent.id].touchStartTime = millis();
            }
            
        } else if (currentEvent.released) {
            Serial.print("✅ [事件] 触摸点 ");
            Serial.print(currentEvent.id);
            Serial.print(" 抬起");
            
            // 计算触摸持续时间
            if (currentEvent.id < 2 && touchPoints[currentEvent.id].active) {
                uint32_t duration = millis() - touchPoints[currentEvent.id].touchStartTime;
                Serial.print(" (持续时间: ");
                Serial.print(duration);
                Serial.print("ms)");
                
                // 判断是否为点击
                if (duration < 300) {
                    Serial.print(" - 点击");
                }
                
                touchPoints[currentEvent.id].active = false;
            }
            Serial.println();
        }
    }
    
    // 显示当前触摸状态（每300ms更新一次）
    static uint32_t lastDisplayTime = 0;
    uint32_t currentTime = millis();
    
    if (currentTime - lastDisplayTime > 300) {
        lastDisplayTime = currentTime;
        
        uint8_t touchCount = touch.getTouchCount();
        
        if (touchCount > 0) {
            // 显示触摸点数量统计
            if (touchCount == 1) {
                Serial.print("👆 单点触摸");
            } else if (touchCount == 2) {
                Serial.print("👆👆 两点触摸");
            }
            Serial.print(" - ");
            Serial.print(touchCount);
            Serial.println("个活动点");
            
            // 显示所有活动触摸点的详细信息
            for (uint8_t i = 0; i < touchCount; i++) {
                uint16_t x, y;
                if (touch.getTouchPoint(i, &x, &y)) {
                    Serial.print("  📍 点 ");
                    Serial.print(i + 1);
                    Serial.print(": X=");
                    Serial.print(x);
                    Serial.print(", Y=");
                    Serial.print(y);
                    
                    // 显示触摸ID
                    uint8_t activeId = 0xFF;
                    for (uint8_t id = 0; id < 2; id++) {
                        if (touch.isTouchActive(id)) {
                            activeId = id;
                            break;
                        }
                    }
                    if (activeId != 0xFF) {
                        Serial.print(", ID=");
                        Serial.print(activeId);
                    }
                    
                    Serial.println();
                }
            }
            
            // 显示活动ID
            Serial.print("  🔄 活动ID: ");
            bool hasActive = false;
            for (uint8_t id = 0; id < 2; id++) {
                if (touch.isTouchActive(id)) {
                    if (hasActive) Serial.print(", ");
                    Serial.print(id);
                    hasActive = true;
                }
            }
            if (!hasActive) {
                Serial.print("无");
            }
            Serial.println();
            
            Serial.println("────────────────────────");
        } else {
            // 没有触摸时显示状态
            static uint32_t lastIdleTime = 0;
            if (currentTime - lastIdleTime > 2000) {
                lastIdleTime = currentTime;
                Serial.println("📱 等待触摸...");
            }
        }
    }
    
    // 轮询模式下的短暂延迟
    if (!touch.hasNewEvent(nullptr)) {
        delay(10);
    }
}
