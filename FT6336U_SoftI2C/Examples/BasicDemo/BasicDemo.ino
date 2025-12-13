/*
 * FT6336U触摸屏库 - 基础演示
 * 展示如何使用触摸屏库的基本功能
 */

#include <FT6336U_SoftI2C.h>

// 创建触摸屏对象（使用默认引脚）
FT6336U_SoftI2C touch;

// 或者使用自定义引脚：
// FT6336U_SoftI2C touch(12, 13, 15, 16);

// 触摸事件回调函数
void onTouchEvent(TouchEvent event) {
    if (event.pressed) {
        Serial.print("触摸按下 - ID:");
        Serial.print(event.id);
        Serial.print(" X:");
        Serial.print(event.x);
        Serial.print(" Y:");
        Serial.println(event.y);
    } else if (event.released) {
        Serial.print("触摸抬起 - ID:");
        Serial.println(event.id);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== FT6336U触摸屏演示 ===");
    
    // 初始化触摸屏（启用调试信息）
    touch.begin(true);
    
    // 设置触摸事件回调
    touch.setTouchCallback(onTouchEvent);
    
    // 读取芯片信息
    touch.readChipInfo();
    
    // 配置触摸参数
    touch.configure(0x12);
    
    Serial.println("触摸屏初始化完成，开始检测...");
    Serial.println("======================================");
}

void loop() {
    // 更新触摸数据（轮询模式）
    // 在中断模式下，这个函数会检查中断标志
    touch.update();
    
    // 可选：手动检查触摸状态
    if (touch.isTouched()) {
        uint8_t count = touch.getTouchCount();
        for (uint8_t i = 0; i < count; i++) {
            uint16_t x, y;
            if (touch.getTouchPoint(i, &x, &y)) {
                // 处理触摸点坐标
            }
        }
    }
    
    // 可选：打印状态（调试用）
    touch.printStatus();
    
    // 轮询间隔
    delay(20);
}