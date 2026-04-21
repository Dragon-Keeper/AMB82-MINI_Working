/*
 * DS3231_ClockModule.cpp - DS3231时钟模块实现文件
 * 提供DS3231实时时钟的初始化、读取、写入和时间格式化功能
 * DS3231具有温度补偿功能，精度比DS1307更高
 */

#include "DS3231_ClockModule.h"
#include "Utils_Logger.h"

// 构造函数
DS3231_ClockModule::DS3231_ClockModule() {
}

// 初始化DS3231模块
bool DS3231_ClockModule::initialize() {
    // 初始化Wire1库（使用Wire1实例，引脚9/10）
    Wire1.begin();
    Wire1.setClock(100000); // 设置I2C时钟频率为100kHz，兼容DS3231模块
    delay(100); // 等待I2C总线稳定

    // 检查DS3231是否存在
    Wire1.beginTransmission((uint8_t)DS3231_ADDRESS);
    uint8_t error = Wire1.endTransmission();

    Serial.print("DS3231 I2C初始化结果: ");
    switch (error) {
        case 0: Serial.println("成功"); break;
        case 1: Serial.println("数据溢出"); break;
        case 2: Serial.println("NACK地址"); break;
        case 3: Serial.println("NACK数据"); break;
        case 4: Serial.println("其他错误"); break;
        default: Serial.println("未知错误"); break;
    }

    if (error == 0) {
        // DS3231存在，先检查振荡器状态
        if (isOscillatorStopped()) {
            Serial.println("DS3231振荡器已停止，需要重新设置时间");
            clearOscillatorStoppedFlag();
        }

        // 启用时钟
        enableClock();
        Serial.println("DS3231时钟模块初始化成功");

        // 触发一次温度转换以确保时间准确
        triggerTemperatureConversion();

        return true;
    } else {
        // DS3231不存在
        Serial.println("DS3231时钟模块初始化失败");
        return false;
    }
}

// 触发温度转换
void DS3231_ClockModule::triggerTemperatureConversion() {
    uint8_t control;
    if (readRegister(DS3231_REG_CONTROL, control)) {
        // 设置CONV位触发温度转换
        if (writeRegister(DS3231_REG_CONTROL, control | DS3231_CONTROL_CONV)) {
            Serial.println("DS3231温度转换已触发");
        }
    }
}

// 读取当前时间
bool DS3231_ClockModule::readTime(DS3231_Time &time) {
    uint8_t buffer[7];
    bool readSuccess = false;

    // 重试机制：最多尝试3次读取
    for (int retry = 0; retry < 3; retry++) {
        // 读取DS3231的7个时间寄存器
        if (readRegisters(DS3231_REG_SECONDS, buffer, 7)) {
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
        time.month = bcdToDec(buffer[5] & 0x1F); // 清除 century 位
        time.year = 2000 + bcdToDec(buffer[6]);    // 年份从2000年开始

        // 检查century位（bit 7 of month register）
        if (buffer[5] & 0x80) {
            time.year += 100;
        }

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
bool DS3231_ClockModule::writeTime(const DS3231_Time &time) {
    Serial.print("正在设置DS3231时间: ");
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
        Serial.println("DS3231时间设置失败：时间值无效");
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
    uint8_t buffer[7];
    buffer[0] = decToBcd(time.seconds) & 0x7F; // 清除时钟停止位（BIT7），确保时钟运行
    buffer[1] = decToBcd(time.minutes);
    buffer[2] = decToBcd(time.hours) & 0x3F; // 确保使用24小时制（清除12/24小时制选择位和上午/下午位）
    buffer[3] = decToBcd(time.day);
    buffer[4] = decToBcd(time.date);
    buffer[5] = decToBcd(time.month); // DS3231的月份寄存器不使用 century 位，写入时清除
    buffer[6] = decToBcd(time.year - 2000); // 年份只存储后两位

    // 写入DS3231
    Wire1.beginTransmission((uint8_t)DS3231_ADDRESS);
    Wire1.write(DS3231_REG_SECONDS); // 起始寄存器地址
    for (uint8_t i = 0; i < 7; i++) {
        Wire1.write(buffer[i]);
    }
    uint8_t error = Wire1.endTransmission();

    if (error == 0) {
        Serial.println("DS3231时间设置成功");
        // 清除振荡器停止标志
        clearOscillatorStoppedFlag();
        return true;
    } else {
        Serial.println("DS3231时间设置失败");
        return false;
    }
}

// 启用时钟
bool DS3231_ClockModule::enableClock() {
    uint8_t control;

    // 读取当前控制寄存器值
    if (!readRegister(DS3231_REG_CONTROL, control)) {
        return false;
    }

    // 清除EOSC位（bit 7）使能振荡器
    control &= ~DS3231_CONTROL_EOSC;

    // 写回控制寄存器
    return writeRegister(DS3231_REG_CONTROL, control);
}

// 禁用时钟
bool DS3231_ClockModule::disableClock() {
    uint8_t control;

    // 读取当前控制寄存器值
    if (!readRegister(DS3231_REG_CONTROL, control)) {
        return false;
    }

    // 设置EOSC位（bit 7）禁用振荡器
    control |= DS3231_CONTROL_EOSC;

    // 写回控制寄存器
    return writeRegister(DS3231_REG_CONTROL, control);
}

// 格式化时间为字符串 (YYYY-MM-DD HH:MM:SS)
void DS3231_ClockModule::formatTime(const DS3231_Time &time, char *buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d",
             time.year, time.month, time.date,
             time.hours, time.minutes, time.seconds);
}

// 格式化时间为文件名 (YYYYMMDD_HHMMSS)
void DS3231_ClockModule::formatTimeForFilename(const DS3231_Time &time, char *buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%04d%02d%02d_%02d%02d%02d",
             time.year, time.month, time.date,
             time.hours, time.minutes, time.seconds);
}

// 读取温度（DS3231特有）
float DS3231_ClockModule::readTemperature() {
    uint8_t tempMsb, tempLsb;

    if (readRegister(DS3231_REG_TEMP_MSB, tempMsb) &&
        readRegister(DS3231_REG_TEMP_LSB, tempLsb)) {
        // 温度为12位有符号数，MSB为整数部分，LSB的高4位为小数部分
        int16_t temp = (int16_t)(tempMsb << 8 | tempLsb);
        // 右移6位以获得实际温度值（保留2个小数位）
        float temperature = (float)(temp >> 6);
        temperature += (tempLsb >> 4) * 0.25; // 添加小数部分
        return temperature;
    }

    return 0.0f;
}

// 检查振荡器是否停止（断电后需要重新设置时间）
bool DS3231_ClockModule::isOscillatorStopped() {
    uint8_t status;
    if (readRegister(DS3231_REG_STATUS, status)) {
        return (status & DS3231_STATUS_OSF) != 0;
    }
    return false;
}

// 清除振荡器停止标志
bool DS3231_ClockModule::clearOscillatorStoppedFlag() {
    uint8_t status;
    if (readRegister(DS3231_REG_STATUS, status)) {
        // 清除OSF位
        status &= ~DS3231_STATUS_OSF;
        return writeRegister(DS3231_REG_STATUS, status);
    }
    return false;
}

// BCD转十进制
uint8_t DS3231_ClockModule::bcdToDec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// 十进制转BCD
uint8_t DS3231_ClockModule::decToBcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// 向DS3231写入单个字节
bool DS3231_ClockModule::writeRegister(uint8_t regAddr, uint8_t data) {
    Wire1.beginTransmission((uint8_t)DS3231_ADDRESS);
    Wire1.write(regAddr);
    Wire1.write(data);
    uint8_t error = Wire1.endTransmission();

    return (error == 0);
}

// 从DS3231读取单个字节
bool DS3231_ClockModule::readRegister(uint8_t regAddr, uint8_t &data) {
    Wire1.beginTransmission((uint8_t)DS3231_ADDRESS);
    Wire1.write(regAddr);
    uint8_t error = Wire1.endTransmission();

    if (error != 0) {
        return false;
    }

    Wire1.requestFrom((uint8_t)DS3231_ADDRESS, (uint8_t)1);
    if (Wire1.available() == 1) {
        data = Wire1.read();
        return true;
    } else {
        return false;
    }
}

// 从DS3231读取多个字节
bool DS3231_ClockModule::readRegisters(uint8_t startAddr, uint8_t *data, uint8_t length) {
    Wire1.beginTransmission((uint8_t)DS3231_ADDRESS);
    Wire1.write(startAddr);
    uint8_t error = Wire1.endTransmission();

    if (error != 0) {
        return false;
    }

    // 检查requestFrom的返回值，确保成功读取了预期数量的字节
    int receivedBytes = Wire1.requestFrom((uint8_t)DS3231_ADDRESS, (uint8_t)length);
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

// 全局DS3231时间读取函数
void readDS3231Time(DS3231_Time& time) {
    // 使用全局clockModule读取时间
    if (!clockModule.readTime(time)) {
        Utils_Logger::error("Failed to read time from DS3231");
        // 设置默认值
        time.seconds = 0;
        time.minutes = 0;
        time.hours = 0;
        time.day = 1;
        time.month = 1;
        time.year = 2026;
    }
}