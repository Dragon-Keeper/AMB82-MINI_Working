/*
 * Utils_Logger.cpp - 日志工具实现
 * 提供系统日志记录和级别控制功能
 * 阶段五：相机模块开发 - 工具类开发
 */

#include "Utils_Logger.h"
#include <stdarg.h>

// 静态成员初始化
Utils_Logger::LogLevel Utils_Logger::currentLevel = Utils_Logger::LEVEL_INFO;
bool Utils_Logger::initialized = false;

void Utils_Logger::init(LogLevel level) {
    if (!initialized) {
        currentLevel = level;
        initialized = true;
        
        // 初始化串口（如果尚未初始化）
        if (!Serial) {
            Serial.begin(115200);
            delay(100);
        }
        
        info("日志系统初始化完成，级别: %d", level);
    }
}

void Utils_Logger::setLevel(LogLevel level) {
    currentLevel = level;
    info("日志级别已设置为: %d", level);
}

void Utils_Logger::error(const char* format, ...) {
    if (currentLevel >= LEVEL_ERROR) {
        va_list args;
        va_start(args, format);
        logInternal(LEVEL_ERROR, "[ERROR] ", format, args);
        va_end(args);
    }
}

void Utils_Logger::info(const char* format, ...) {
    if (currentLevel >= LEVEL_INFO) {
        va_list args;
        va_start(args, format);
        logInternal(LEVEL_INFO, "[INFO] ", format, args);
        va_end(args);
    }
}

void Utils_Logger::debug(const char* format, ...) {
    if (currentLevel >= LEVEL_DEBUG) {
        va_list args;
        va_start(args, format);
        logInternal(LEVEL_DEBUG, "[DEBUG] ", format, args);
        va_end(args);
    }
}

void Utils_Logger::print(const char* text) {
    Serial.print(text);
}

void Utils_Logger::println(const char* text) {
    Serial.println(text);
}

void Utils_Logger::print(int number) {
    Serial.print(number);
}

void Utils_Logger::println(int number) {
    Serial.println(number);
}

Utils_Logger::LogLevel Utils_Logger::getCurrentLevel() {
    return currentLevel;
}

void Utils_Logger::cleanup() {
    initialized = false;
    info("日志系统已清理");
}

void Utils_Logger::logInternal(LogLevel level, const char* prefix, const char* format, va_list args) {
    // 输出前缀
    Serial.print(prefix);
    
    // 格式化输出
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    // 输出内容
    Serial.println(buffer);
}