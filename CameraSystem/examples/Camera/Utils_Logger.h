/*
 * Utils_Logger.h - 日志工具头文件
 * 提供系统日志记录和级别控制功能
 * 阶段五：相机模块开发 - 工具类开发
 */

#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H

#include <Arduino.h>

class Utils_Logger {
public:
    typedef enum {
        LEVEL_ERROR = 0,
        LEVEL_INFO = 1,
        LEVEL_DEBUG = 2
    } LogLevel;
    
    // 初始化日志系统
    static void init(LogLevel level = LEVEL_INFO);
    
    // 设置日志级别
    static void setLevel(LogLevel level);
    
    // 日志输出方法
    static void error(const char* format, ...);
    static void info(const char* format, ...);
    static void debug(const char* format, ...);
    
    // 原始输出（保持与Serial兼容）
    static void print(const char* text);
    static void println(const char* text);
    static void print(int number);
    static void println(int number);
    
    // 获取当前日志级别
    static LogLevel getCurrentLevel();
    
    // 清理资源
    static void cleanup();
    
private:
    static LogLevel currentLevel;
    static bool initialized;
    
    // 内部格式化输出方法
    static void logInternal(LogLevel level, const char* prefix, const char* format, va_list args);
};

#endif // UTILS_LOGGER_H