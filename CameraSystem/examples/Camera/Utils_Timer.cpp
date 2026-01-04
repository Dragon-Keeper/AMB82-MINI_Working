/*
 * Utils_Timer.cpp - 定时器工具实现
 * 提供时间计时和超时检查功能
 * 阶段五：相机模块开发 - 工具类开发
 */

#include "Utils_Timer.h"

Utils_Timer::Utils_Timer(unsigned long timeoutMs) 
    : startTime(0), timeoutMs(timeoutMs), running(false) {
}

void Utils_Timer::start() {
    startTime = millis();
    running = true;
}

void Utils_Timer::restart() {
    startTime = millis();
    running = true;
}

bool Utils_Timer::isExpired() const {
    if (!running) {
        return false;
    }
    return (millis() - startTime) >= timeoutMs;
}

unsigned long Utils_Timer::elapsed() const {
    if (!running) {
        return 0;
    }
    return millis() - startTime;
}

unsigned long Utils_Timer::remaining() const {
    if (!running) {
        return timeoutMs;
    }
    unsigned long elapsedTime = millis() - startTime;
    if (elapsedTime >= timeoutMs) {
        return 0;
    }
    return timeoutMs - elapsedTime;
}

void Utils_Timer::setTimeout(unsigned long timeoutMs) {
    this->timeoutMs = timeoutMs;
}

unsigned long Utils_Timer::getTimeout() const {
    return timeoutMs;
}

void Utils_Timer::stop() {
    running = false;
}

bool Utils_Timer::isRunning() const {
    return running;
}

unsigned long Utils_Timer::getCurrentTime() {
    return millis();
}

void Utils_Timer::delayMs(unsigned long ms) {
    delay(ms);
}

bool Utils_Timer::isTimeReached(unsigned long lastTime, unsigned long interval) {
    return (millis() - lastTime) >= interval;
}