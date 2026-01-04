/*
 * Utils_Timer.h - 定时器工具头文件
 * 提供时间计时和超时检查功能
 * 阶段五：相机模块开发 - 工具类开发
 */

#ifndef UTILS_TIMER_H
#define UTILS_TIMER_H

#include <Arduino.h>

class Utils_Timer {
public:
    // 构造函数，可选设置超时时间
    Utils_Timer(unsigned long timeoutMs = 0);
    
    // 启动定时器
    void start();
    
    // 重启定时器（重新开始计时）
    void restart();
    
    // 检查是否超时
    bool isExpired() const;
    
    // 获取经过的时间（毫秒）
    unsigned long elapsed() const;
    
    // 获取剩余时间（毫秒）
    unsigned long remaining() const;
    
    // 设置超时时间
    void setTimeout(unsigned long timeoutMs);
    
    // 获取超时时间
    unsigned long getTimeout() const;
    
    // 停止定时器
    void stop();
    
    // 检查定时器是否正在运行
    bool isRunning() const;
    
    // 静态方法：获取当前时间（毫秒）
    static unsigned long getCurrentTime();
    
    // 静态方法：延迟（毫秒），替代delay()
    static void delayMs(unsigned long ms);
    
    // 静态方法：检查时间是否到达（用于周期性任务）
    static bool isTimeReached(unsigned long lastTime, unsigned long interval);
    
private:
    unsigned long startTime;     // 开始时间
    unsigned long timeoutMs;     // 超时时间（毫秒）
    bool running;                // 是否正在运行
};

#endif // UTILS_TIMER_H