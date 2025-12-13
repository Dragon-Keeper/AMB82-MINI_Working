/*
 * FT6336U触摸屏驱动 - 使用库版本
 * 简化主程序，所有功能都封装在库中
 */

#include <FT6336U_SoftI2C.h>

// 创建触摸屏对象
FT6336U_SoftI2C touch;

// 触摸事件回调函数
void handleTouchEvent(TouchEvent event) {
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

void setup() {
    // 初始化串口
    Serial.begin(115200);
    while(!Serial && millis() < 3000);
    
    Serial.println("\n=== FT6336U触摸屏驱动 - 库版本 ===");
    Serial.println("引脚: SDA=12, SCL=13, RST=15, INT=16");
    
    // 初始化触摸屏（启用调试信息）
    touch.begin(true);
    
    // 设置触摸事件回调
    touch.setTouchCallback(handleTouchEvent);
    
    // 读取芯片信息
    touch.readChipInfo();
    delay(100);
    
    // 配置触摸参数
    touch.configure(0x12);
    delay(100);
    
    #if USE_INTERRUPT_MODE
    Serial.println("工作模式: 中断驱动");
    // 配置AMB82-MINI专用中断引脚
    pinMode(16, INPUT_IRQ_FALL);
    // 使用AMB82-MINI专用中断API（适配版本）
    digitalSetIrqHandler(16, FT6336U_SoftI2C::amebaInterruptHandler);
    #else
    Serial.println("工作模式: 轮询驱动");
    #endif
    
    Serial.println("\n触摸屏已就绪，开始检测...");
    Serial.println("======================================");
}

void loop() {
    // 更新触摸数据
    // 在轮询模式下，这个函数会读取数据并处理事件
    // 在中断模式下，这个函数会检查中断标志并处理
    touch.update();
    
    // 可选：打印状态信息
    touch.printStatus();
    
    // 轮询间隔（仅在轮询模式下需要）
    #if !USE_INTERRUPT_MODE
    delay(20);
    #endif
}