/*
 * DS1307_ClockModule.h - DS1307时钟模块头文件
 * 提供DS1307实时时钟的初始化、读取、写入和时间格式化功能
 * 为相机系统提供可靠的时间戳来源
 */

#ifndef DS1307_CLOCKMODULE_H
#define DS1307_CLOCKMODULE_H

#include <Arduino.h>
#include <Wire.h>

// DS1307 I2C地址
#define DS1307_ADDRESS 0x68

// DS1307寄存器地址
#define DS1307_REG_SECONDS 0x00
#define DS1307_REG_MINUTES 0x01
#define DS1307_REG_HOURS 0x02
#define DS1307_REG_DAY 0x03
#define DS1307_REG_DATE 0x04
#define DS1307_REG_MONTH 0x05
#define DS1307_REG_YEAR 0x06
#define DS1307_REG_CONTROL 0x07

// 时间结构体定义
typedef struct {
    uint8_t seconds;    // 秒 (0-59)
    uint8_t minutes;    // 分 (0-59)
    uint8_t hours;      // 时 (0-23)
    uint8_t day;        // 星期 (1-7, 1=Sunday)
    uint8_t date;       // 日 (1-31)
    uint8_t month;      // 月 (1-12)
    uint16_t year;      // 年 (完整年份，如2024)
} DS1307_Time;

class DS1307_ClockModule {
public:
    // 构造函数
    DS1307_ClockModule();
    
    // 初始化DS1307模块
    bool initialize();
    
    // 读取当前时间
    bool readTime(DS1307_Time &time);
    
    // 写入时间
    bool writeTime(const DS1307_Time &time);
    
    // 启用时钟
    bool enableClock();
    
    // 禁用时钟
    bool disableClock();
    
    // 格式化时间为字符串 (YYYY-MM-DD HH:MM:SS)
    void formatTime(const DS1307_Time &time, char *buffer, size_t bufferSize);
    
    // 格式化时间为文件名 (YYYYMMDD_HHMMSS)
    void formatTimeForFilename(const DS1307_Time &time, char *buffer, size_t bufferSize);
    
    // BCD转十进制
    static uint8_t bcdToDec(uint8_t bcd);
    
    // 十进制转BCD
    static uint8_t decToBcd(uint8_t dec);
    
private:
    // 向DS1307写入单个字节
    bool writeRegister(uint8_t regAddr, uint8_t data);
    
    // 从DS1307读取单个字节
    bool readRegister(uint8_t regAddr, uint8_t &data);
    
    // 从DS1307读取多个字节
    bool readRegisters(uint8_t startAddr, uint8_t *data, uint8_t length);
};

#endif // DS1307_CLOCKMODULE_H