/*
 * DS3231_ClockModule.h - DS3231时钟模块头文件
 * 提供DS3231实时时钟的初始化、读取、写入和时间格式化功能
 * DS3231具有温度补偿功能，精度比DS1307更高
 */

#ifndef DS3231_CLOCKMODULE_H
#define DS3231_CLOCKMODULE_H

#include <Arduino.h>
#include <Wire.h>

// DS3231 I2C地址
#define DS3231_ADDRESS 0x68

// DS3231寄存器地址（与DS1307兼容的部分）
#define DS3231_REG_SECONDS 0x00
#define DS3231_REG_MINUTES 0x01
#define DS3231_REG_HOURS 0x02
#define DS3231_REG_DAY 0x03
#define DS3231_REG_DATE 0x04
#define DS3231_REG_MONTH 0x05
#define DS3231_REG_YEAR 0x06

// DS3231特有的寄存器
#define DS3231_REG_CONTROL 0x0E
#define DS3231_REG_STATUS 0x0F
#define DS3231_REG_AGING_OFFSET 0x10
#define DS3231_REG_TEMP_MSB 0x11
#define DS3231_REG_TEMP_LSB 0x12

// DS3231控制寄存器位定义
#define DS3231_CONTROL_EOSC 0x80    // 振荡器停止标志
#define DS3231_CONTROL_BBSQW 0x40   // 电池备份方波使能
#define DS3231_CONTROL_CONV 0x20    // 温度转换触发
#define DS3231_CONTROL_RS2 0x10     // 方波频率选择位2
#define DS3231_CONTROL_RS1 0x08      // 方波频率选择位1
#define DS3231_CONTROL_INTCN 0x04   // 中断控制
#define DS3231_CONTROL_A2IE 0x02    // 闹钟2中断使能
#define DS3231_CONTROL_A1IE 0x01    // 闹钟1中断使能

// DS3231状态寄存器位定义
#define DS3231_STATUS_OSF 0x80     // 振荡器停止标志
#define DS3231_STATUS_EN32KHZ 0x08  // 32kHz输出使能
#define DS3231_STATUS_BSY 0x04     // 忙标志（温度转换）
#define DS3231_STATUS_A2F 0x02     // 闹钟2标志
#define DS3231_STATUS_A1F 0x01     // 闹钟1标志

// 时间结构体定义
typedef struct {
    uint8_t seconds;    // 秒 (0-59)
    uint8_t minutes;    // 分 (0-59)
    uint8_t hours;      // 时 (0-23)
    uint8_t day;        // 星期 (1-7, 1=Sunday)
    uint8_t date;       // 日 (1-31)
    uint8_t month;      // 月 (1-12)
    uint16_t year;      // 年 (完整年份，如2024)
} DS3231_Time;

class DS3231_ClockModule {
public:
    // 构造函数
    DS3231_ClockModule();

    // 初始化DS3231模块
    bool initialize();

    // 读取当前时间
    bool readTime(DS3231_Time &time);

    // 写入时间
    bool writeTime(const DS3231_Time &time);

    // 启用时钟
    bool enableClock();

    // 禁用时钟
    bool disableClock();

    // 格式化时间为字符串 (YYYY-MM-DD HH:MM:SS)
    void formatTime(const DS3231_Time &time, char *buffer, size_t bufferSize);

    // 格式化时间为文件名 (YYYYMMDD_HHMMSS)
    void formatTimeForFilename(const DS3231_Time &time, char *buffer, size_t bufferSize);

    // 读取温度（DS3231特有）
    float readTemperature();

    // BCD转十进制
    static uint8_t bcdToDec(uint8_t bcd);

    // 十进制转BCD
    static uint8_t decToBcd(uint8_t dec);

    // 检查振荡器是否停止（断电后需要重新设置时间）
    bool isOscillatorStopped();

    // 清除振荡器停止标志
    bool clearOscillatorStoppedFlag();

private:
    // 向DS3231写入单个字节
    bool writeRegister(uint8_t regAddr, uint8_t data);

    // 从DS3231读取单个字节
    bool readRegister(uint8_t regAddr, uint8_t &data);

    // 从DS3231读取多个字节
    bool readRegisters(uint8_t startAddr, uint8_t *data, uint8_t length);

    // 强制温度转换（DS3231需要）
    void triggerTemperatureConversion();
};

// 全局DS3231时间读取函数，提供统一的时间读取接口
extern DS3231_ClockModule clockModule;
void readDS3231Time(DS3231_Time& time);
#endif // DS3231_CLOCKMODULE_H