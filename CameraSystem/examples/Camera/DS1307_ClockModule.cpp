/*
 * DS1307_ClockModule.cpp - DS1307时钟模块实现文件
 * 提供DS1307实时时钟的初始化、读取、写入和时间格式化功能
 * 为相机系统提供可靠的时间戳来源
 */

#include "DS1307_ClockModule.h"

// 构造函数
DS1307_ClockModule::DS1307_ClockModule() {
    // 构造函数不需要特殊初始化
}

// 初始化DS1307模块
bool DS1307_ClockModule::initialize() {
    // 初始化Wire1库（使用Wire1实例，引脚9/10）
    Wire1.begin();
    Wire1.setClock(100000); // 设置I2C时钟频率为100kHz，兼容DS1307模块
    delay(100); // 等待I2C总线稳定
    
    // 检查DS1307是否存在
    Wire1.beginTransmission((uint8_t)DS1307_ADDRESS);
    uint8_t error = Wire1.endTransmission();
    
    if (error == 0) {
        // DS1307存在，启用时钟
        enableClock();
        return true;
    } else {
        // DS1307不存在
        return false;
    }
}

// 读取当前时间
bool DS1307_ClockModule::readTime(DS1307_Time &time) {
    uint8_t buffer[7];
    bool readSuccess = false;
    
    // 重试机制：最多尝试3次读取
    for (int retry = 0; retry < 3; retry++) {
        // 读取DS1307的7个时间寄存器
        if (readRegisters(DS1307_REG_SECONDS, buffer, 7)) {
            readSuccess = true;
            break;
        }
        delay(50); // 等待50ms后重试
    }
    
    if (readSuccess) {
        // 转换BCD到十进制并存储到时间结构体
        time.seconds = bcdToDec(buffer[0] & 0x7F); // 清除时钟停止位
        time.minutes = bcdToDec(buffer[1]);
        time.hours = bcdToDec(buffer[2] & 0x3F);   // 24小时制
        time.day = bcdToDec(buffer[3]);
        time.date = bcdToDec(buffer[4]);
        time.month = bcdToDec(buffer[5]);
        time.year = 2000 + bcdToDec(buffer[6]);    // 年份从2000年开始
        
        // 数据有效性检查：确保时间值在合理范围内
        if (time.seconds > 59 || time.minutes > 59 || time.hours > 23 || 
            time.date < 1 || time.date > 31 || time.month < 1 || time.month > 12) {
            readSuccess = false;
        }
    }
    
    if (!readSuccess) {
        // 读取失败或数据无效时，设置合理的默认值
        time.seconds = 0;
        time.minutes = 0;
        time.hours = 0;
        time.day = 1;
        time.month = 1;
        time.year = 2026;
    }
    
    return readSuccess;
}

// 写入时间
bool DS1307_ClockModule::writeTime(const DS1307_Time &time) {
    Serial.print("正在设置DS1307时间: ");
    Serial.print(time.year);
    Serial.print("-");
    Serial.print(time.month);
    Serial.print("-");
    Serial.print(time.date);
    Serial.print(" ");
    Serial.print(time.hours);
    Serial.print(":");
    Serial.print(time.minutes);
    Serial.print(":");
    Serial.println(time.seconds);
    
    // 检查时间值的有效性
    if (time.seconds > 59 || time.minutes > 59 || time.hours > 23 ||
        time.day < 1 || time.day > 7 || time.date < 1 || time.date > 31 ||
        time.month < 1 || time.month > 12 || time.year < 2000 || time.year > 2100) {
        Serial.println("DS1307时间设置失败：时间值无效");
        return false;
    }
    
    // 输出BCD转换结果（用于调试）
    Serial.print("BCD转换结果 - 秒: "); Serial.print(time.seconds); Serial.print(" -> "); Serial.print(decToBcd(time.seconds), HEX); Serial.print("  ");
    Serial.print("分: "); Serial.print(time.minutes); Serial.print(" -> "); Serial.print(decToBcd(time.minutes), HEX); Serial.print("  ");
    Serial.print("时: "); Serial.print(time.hours); Serial.print(" -> "); Serial.print(decToBcd(time.hours), HEX); Serial.print("  ");
    Serial.print("日: "); Serial.print(time.date); Serial.print(" -> "); Serial.print(decToBcd(time.date), HEX); Serial.print("  ");
    Serial.print("月: "); Serial.print(time.month); Serial.print(" -> "); Serial.print(decToBcd(time.month), HEX); Serial.print("  ");
    Serial.print("年: "); Serial.print(time.year - 2000); Serial.print(" -> "); Serial.print(decToBcd(time.year - 2000), HEX); Serial.println();
    
    // 准备写入的数据（十进制转BCD）
    uint8_t buffer[8];
    buffer[0] = DS1307_REG_SECONDS; // 起始寄存器地址
    buffer[1] = decToBcd(time.seconds);
    buffer[2] = decToBcd(time.minutes);
    buffer[3] = decToBcd(time.hours);
    buffer[4] = decToBcd(time.day);
    buffer[5] = decToBcd(time.date);
    buffer[6] = decToBcd(time.month);
    buffer[7] = decToBcd(time.year - 2000); // 年份只存储后两位
    
    // 写入DS1307
    Wire1.beginTransmission((uint8_t)DS1307_ADDRESS);
    for (uint8_t i = 0; i < 8; i++) {
        Wire1.write(buffer[i]);
    }
    uint8_t error = Wire1.endTransmission();
    
    if (error == 0) {
        Serial.println("DS1307时间设置成功");
        return true;
    } else {
        Serial.println("DS1307时间设置失败");
        return false;
    }
}

// 启用时钟
bool DS1307_ClockModule::enableClock() {
    uint8_t seconds;
    
    // 读取当前秒寄存器值
    if (!readRegister(DS1307_REG_SECONDS, seconds)) {
        return false;
    }
    
    // 清除时钟停止位（BIT7）
    seconds &= 0x7F;
    
    // 写回秒寄存器
    return writeRegister(DS1307_REG_SECONDS, seconds);
}

// 禁用时钟
bool DS1307_ClockModule::disableClock() {
    uint8_t seconds;
    
    // 读取当前秒寄存器值
    if (!readRegister(DS1307_REG_SECONDS, seconds)) {
        return false;
    }
    
    // 设置时钟停止位（BIT7）
    seconds |= 0x80;
    
    // 写回秒寄存器
    return writeRegister(DS1307_REG_SECONDS, seconds);
}

// 格式化时间为字符串 (YYYY-MM-DD HH:MM:SS)
void DS1307_ClockModule::formatTime(const DS1307_Time &time, char *buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d",
             time.year, time.month, time.date,
             time.hours, time.minutes, time.seconds);
}

// 格式化时间为文件名 (YYYYMMDD_HHMMSS)
void DS1307_ClockModule::formatTimeForFilename(const DS1307_Time &time, char *buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%04d%02d%02d_%02d%02d%02d",
             time.year, time.month, time.date,
             time.hours, time.minutes, time.seconds);
}

// BCD转十进制
uint8_t DS1307_ClockModule::bcdToDec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// 十进制转BCD
uint8_t DS1307_ClockModule::decToBcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// 向DS1307写入单个字节
bool DS1307_ClockModule::writeRegister(uint8_t regAddr, uint8_t data) {
    Wire1.beginTransmission((uint8_t)DS1307_ADDRESS);
    Wire1.write(regAddr);
    Wire1.write(data);
    uint8_t error = Wire1.endTransmission();
    
    return (error == 0);
}

// 从DS1307读取单个字节
bool DS1307_ClockModule::readRegister(uint8_t regAddr, uint8_t &data) {
    Wire1.beginTransmission((uint8_t)DS1307_ADDRESS);
    Wire1.write(regAddr);
    uint8_t error = Wire1.endTransmission();
    
    if (error != 0) {
        return false;
    }
    
    Wire1.requestFrom((uint8_t)DS1307_ADDRESS, (uint8_t)1);
    if (Wire1.available() == 1) {
        data = Wire1.read();
        return true;
    } else {
        return false;
    }
}

// 从DS1307读取多个字节
bool DS1307_ClockModule::readRegisters(uint8_t startAddr, uint8_t *data, uint8_t length) {
    Wire1.beginTransmission((uint8_t)DS1307_ADDRESS);
    Wire1.write(startAddr);
    uint8_t error = Wire1.endTransmission();
    
    if (error != 0) {
        return false;
    }
    
    // 检查requestFrom的返回值，确保成功读取了预期数量的字节
    int receivedBytes = Wire1.requestFrom((uint8_t)DS1307_ADDRESS, (uint8_t)length);
    if (receivedBytes != length) {
        return false;
    }
    
    uint32_t startTime = millis();
    while (Wire1.available() < length) {
        if (millis() - startTime > 100) {  // 100ms超时
            return false;
        }
        delay(1);
    }
    
    for (uint8_t i = 0; i < length; i++) {
        data[i] = Wire1.read();
    }
    return true;
}