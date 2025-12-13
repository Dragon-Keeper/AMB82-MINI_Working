#include "AmebaParallel8.h"

// 构造函数：初始化硬件端口
#if defined(ARDUINO_ARCH_AVR)
AmebaParallel8::AmebaParallel8(PortReg dataPort, uint8_t dataMask,
                               int csPin, int dcPin, int resetPin, int wrPin, int rdPin)
  : _dataPort(dataPort), _dataMask(dataMask),
    _csPin(csPin), _dcPin(dcPin), _resetPin(resetPin), _wrPin(wrPin), _rdPin(rdPin)
{
  _dataDDR = dataPort - 1; // AVR中DDR寄存器 = PORT寄存器 - 1（如PORTD - 1 = DDRD）
  // 初始化其他状态变量
  _width = 240;
  _height = 320;
  cursor_x = cursor_y = 0;
  foreground = 0xFFFF;
  background = 0x0000;
  fontsize = 1;
  rotation = 0;
}
#elif defined(ARDUINO_ARCH_ESP32)
AmebaParallel8::AmebaParallel8(const PortPin dataPins[8],
                               int csPin, int dcPin, int resetPin, int wrPin, int rdPin)
  : _csPin(csPin), _dcPin(dcPin), _resetPin(resetPin), _wrPin(wrPin), _rdPin(rdPin)
{
  memcpy(_dataPins, dataPins, 8 * sizeof(PortPin));
  // 初始化其他状态变量
  _width = 240;
  _height = 320;
  cursor_x = cursor_y = 0;
  foreground = 0xFFFF;
  background = 0x0000;
  fontsize = 1;
  rotation = 0;
}
#elif defined(ARDUINO_ARCH_RTL8730) || defined(AMB82_MINI) || defined(ARDUINO_AMEBA)
AmebaParallel8::AmebaParallel8(const PortPin dataPins[8],
                               int csPin, int dcPin, int resetPin, int wrPin, int rdPin)
  : _csPin(csPin), _dcPin(dcPin), _resetPin(resetPin), _wrPin(wrPin), _rdPin(rdPin)
{
  memcpy(_dataPins, dataPins, 8 * sizeof(PortPin));
  // 初始化其他状态变量
  _width = 240;
  _height = 320;
  cursor_x = cursor_y = 0;
  foreground = 0xFFFF;
  background = 0x0000;
  fontsize = 1;
  rotation = 0;
}
#else
AmebaParallel8::AmebaParallel8(int csPin, int dcPin, int resetPin, int wrPin, int rdPin,
                               int d0Pin, int d1Pin, int d2Pin, int d3Pin,
                               int d4Pin, int d5Pin, int d6Pin, int d7Pin)
{
    _csPin = csPin;
    _dcPin = dcPin;
    _resetPin = resetPin;
    _wrPin = wrPin;
    _rdPin = rdPin;
    _d0Pin = d0Pin;
    _d1Pin = d1Pin;
    _d2Pin = d2Pin;
    _d3Pin = d3Pin;
    _d4Pin = d4Pin;
    _d5Pin = d5Pin;
    _d6Pin = d6Pin;
    _d7Pin = d7Pin;

    _width = 240;
    _height = 320;
    cursor_x = cursor_y = 0;
    foreground = 0xFFFF;
    background = 0x0000;
    fontsize = 1;
    rotation = 0;
}
#endif

void AmebaParallel8::begin(void)
{
    // 初始化控制引脚
    pinMode(_csPin, OUTPUT);
    pinMode(_dcPin, OUTPUT);
    pinMode(_resetPin, OUTPUT);
    pinMode(_wrPin, OUTPUT);
    pinMode(_rdPin, OUTPUT);
    
    // 初始化数据端口为输出（硬件方式）
    #if defined(ARDUINO_ARCH_AVR)
      *_dataDDR |= _dataMask; // 数据端口方向设为输出
    #elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_RTL8730) || defined(AMB82_MINI) || defined(ARDUINO_AMEBA)
      for (int i = 0; i < 8; i++) {
        pinMode(_dataPins[i], OUTPUT);
      }
    #else
      pinMode(_d0Pin, OUTPUT);
      pinMode(_d1Pin, OUTPUT);
      pinMode(_d2Pin, OUTPUT);
      pinMode(_d3Pin, OUTPUT);
      pinMode(_d4Pin, OUTPUT);
      pinMode(_d5Pin, OUTPUT);
      pinMode(_d6Pin, OUTPUT);
      pinMode(_d7Pin, OUTPUT);
    #endif

    // 控制引脚默认电平
    digitalWrite(_csPin, HIGH);
    digitalWrite(_rdPin, HIGH);
    digitalWrite(_wrPin, HIGH);

    reset();

    // ILI9341初始化序列
    writeCommand(0xEF);
    writeData(0x03);
    writeData(0x80);
    writeData(0x02);

    writeCommand(0xCF);
    writeData(0x00);
    writeData(0xC1);
    writeData(0x30);

    writeCommand(0xED);
    writeData(0x64);
    writeData(0x03);
    writeData(0x12);
    writeData(0x81);

    writeCommand(0xE8);
    writeData(0x85);
    writeData(0x00);
    writeData(0x78);

    writeCommand(0xCB);
    writeData(0x39);
    writeData(0x2C);
    writeData(0x00);
    writeData(0x34);
    writeData(0x02);

    writeCommand(0xF7);
    writeData(0x20);

    writeCommand(0xEA);
    writeData(0x00);
    writeData(0x00);

    writeCommand(0xC0); // Power control
    writeData(0x23);

    writeCommand(0xC1); // Power control
    writeData(0x10);

    writeCommand(0xC5); // VCM control
    writeData(0x3e);
    writeData(0x28);

    writeCommand(0xC7); // VCM control2
    writeData(0x86);

    writeCommand(0x36); // Memory Access Control
    writeData(0x48);

    writeCommand(0x3A); // Pixel Format
    writeData(0x55);

    writeCommand(0xB1); // Frame Rate Control
    writeData(0x00);
    writeData(0x18);

    writeCommand(0xB6); // Display Function Control
    writeData(0x08);
    writeData(0x82);
    writeData(0x27);

    writeCommand(0xF2); // 3Gamma Function Disable
    writeData(0x00);

    writeCommand(0x26); // Gamma curve selected
    writeData(0x01);

    writeCommand(0xE0); // Set Gamma
    writeData(0x0F);
    writeData(0x31);
    writeData(0x2B);
    writeData(0x0C);
    writeData(0x0E);
    writeData(0x08);
    writeData(0x4E);
    writeData(0xF1);
    writeData(0x37);
    writeData(0x07);
    writeData(0x10);
    writeData(0x03);
    writeData(0x0E);
    writeData(0x09);
    writeData(0x00);

    writeCommand(0xE1); // Set Gamma
    writeData(0x00);
    writeData(0x0E);
    writeData(0x14);
    writeData(0x03);
    writeData(0x11);
    writeData(0x07);
    writeData(0x31);
    writeData(0xC1);
    writeData(0x48);
    writeData(0x08);
    writeData(0x0F);
    writeData(0x0C);
    writeData(0x31);
    writeData(0x36);
    writeData(0x0F);

    writeCommand(0x11); // Exit Sleep
    delay(120);

    writeCommand(0x29); // Display on
}

// 硬件并行数据写入（核心优化）
void AmebaParallel8::write8bitData(uint8_t data)
{
    // 1. 写入8位数据到硬件端口（一次性设置所有数据位）
    #if defined(ARDUINO_ARCH_AVR)
      // AVR：直接操作端口寄存器（如PORTD = data）
      *_dataPort = (*_dataPort & ~_dataMask) | (data & _dataMask);
    #elif defined(ARDUINO_ARCH_ESP32)
      // ESP32：通过GPIO矩阵一次性设置8个引脚（效率高于逐个digitalWrite）
      uint32_t out = 0;
      for (int i = 0; i < 8; i++) {
        if (data & (1 << i)) {
          out |= (1 << _dataPins[i]);
        }
      }
      GPIO.out_w1ts = out;               // 置位需要输出高的引脚
      GPIO.out_w1tc = ~out & GPIO.out;   // 清零需要输出低的引脚
    #elif defined(ARDUINO_ARCH_RTL8730) || defined(AMB82_MINI) || defined(ARDUINO_AMEBA)
      // Ameba平台：使用优化的digitalWrite序列进行并行写入
      digitalWrite(_dataPins[0], (data & 0x01) ? HIGH : LOW);
      digitalWrite(_dataPins[1], (data & 0x02) ? HIGH : LOW);
      digitalWrite(_dataPins[2], (data & 0x04) ? HIGH : LOW);
      digitalWrite(_dataPins[3], (data & 0x08) ? HIGH : LOW);
      digitalWrite(_dataPins[4], (data & 0x10) ? HIGH : LOW);
      digitalWrite(_dataPins[5], (data & 0x20) ? HIGH : LOW);
      digitalWrite(_dataPins[6], (data & 0x40) ? HIGH : LOW);
      digitalWrite(_dataPins[7], (data & 0x80) ? HIGH : LOW);
    #else
      // 兼容模式：逐个设置数据引脚
      digitalWrite(_d0Pin, (data & 0x01) ? HIGH : LOW);
      digitalWrite(_d1Pin, (data & 0x02) ? HIGH : LOW);
      digitalWrite(_d2Pin, (data & 0x04) ? HIGH : LOW);
      digitalWrite(_d3Pin, (data & 0x08) ? HIGH : LOW);
      digitalWrite(_d4Pin, (data & 0x10) ? HIGH : LOW);
      digitalWrite(_d5Pin, (data & 0x20) ? HIGH : LOW);
      digitalWrite(_d6Pin, (data & 0x40) ? HIGH : LOW);
      digitalWrite(_d7Pin, (data & 0x80) ? HIGH : LOW);
    #endif

    // 2. 产生WR脉冲（移除冗余延迟，用nop保证最小脉宽）
    digitalWrite(_wrPin, LOW);
    // 最小脉宽：根据ILI9341手册，通常≥15ns，此处用nop满足（1 nop ≈ 62.5ns@16MHz AVR）
    __asm__ __volatile__("nop\n"); 
    digitalWrite(_wrPin, HIGH);
    __asm__ __volatile__("nop\n"); 
}

void AmebaParallel8::writeCommand(uint8_t command)
{
    digitalWrite(_dcPin, LOW);
    digitalWrite(_csPin, LOW);
    write8bitData(command);
    digitalWrite(_csPin, HIGH);
}

void AmebaParallel8::writeData(uint8_t data)
{
    digitalWrite(_dcPin, HIGH);
    digitalWrite(_csPin, LOW);
    write8bitData(data);
    digitalWrite(_csPin, HIGH);
}

void AmebaParallel8::writeData16(uint16_t data)
{
    writeData(data >> 8);
    writeData(data & 0xFF);
}

void AmebaParallel8::setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    writeCommand(0x2A); // CASET
    writeData16(x0);
    writeData16(x1);

    writeCommand(0x2B); // PASET
    writeData16(y0);
    writeData16(y1);

    writeCommand(0x2C); // RAMWR
}

void AmebaParallel8::reset(void)
{
    if (_resetPin > 0) {
        digitalWrite(_resetPin, HIGH);
        delay(5);
        digitalWrite(_resetPin, LOW);
        delay(20);
        digitalWrite(_resetPin, HIGH);
        delay(150);
    }
}

void AmebaParallel8::setRotation(uint8_t m)
{
    writeCommand(0x36); // MADCTL
    rotation = m % 4;
    switch (rotation) {
        case 0:
            writeData(0x48); // MX | BGR
            _width = 240;
            _height = 320;
            break;
        case 1:
            writeData(0x28); // MV | BGR
            _width = 320;
            _height = 240;
            break;
        case 2:
            writeData(0x88); // MY | BGR
            _width = 240;
            _height = 320;
            break;
        case 3:
            writeData(0xE8); // MX | MY | MV | BGR
            _width = 320;
            _height = 240;
            break;
    }
}

void AmebaParallel8::fillScreen(uint16_t color)
{
    // 优化全屏填充，确保正确刷新
    setAddress(0, 0, _width - 1, _height - 1);
    
    digitalWrite(_dcPin, HIGH);
    digitalWrite(_csPin, LOW);
    
    // 批量写入优化：减少函数调用开销
    uint32_t pixelCount = (uint32_t)_width * (uint32_t)_height;
    uint8_t hi = color >> 8, lo = color & 0xFF;
    for (uint32_t i = 0; i < pixelCount; i++) {
        write8bitData(hi);
        write8bitData(lo);
    }
    
    digitalWrite(_csPin, HIGH);
    
    // 确保显示刷新完成
    delay(1);
    
    // 额外的刷新确保
    digitalWrite(_dcPin, HIGH);
    digitalWrite(_csPin, LOW);
    digitalWrite(_csPin, HIGH);
}

void AmebaParallel8::clr()
{
    fillScreen(background);
    // 确保显示刷新完成
    delay(1);
    
    // 额外的刷新确保
    digitalWrite(_dcPin, HIGH);
    digitalWrite(_csPin, LOW);
    digitalWrite(_csPin, HIGH);
}

void AmebaParallel8::fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if ((x >= _width) || (y >= _height)) return;
    if ((x + w - 1) >= _width) w = _width - x;
    if ((y + h - 1) >= _height) h = _height - y;

    setAddress(x, y, x + w - 1, y + h - 1);

    digitalWrite(_dcPin, HIGH);
    digitalWrite(_csPin, LOW);
    
    // 批量写入优化：减少函数调用开销
    uint32_t pixelCount = (uint32_t)w * (uint32_t)h;
    uint8_t hi = color >> 8, lo = color & 0xFF;
    for (uint32_t i = 0; i < pixelCount; i++) {
        write8bitData(hi);
        write8bitData(lo);
    }
    
    digitalWrite(_csPin, HIGH);
    
    // 确保显示刷新完成
    delayMicroseconds(10);
    
    // 额外的刷新确保
    digitalWrite(_dcPin, HIGH);
    digitalWrite(_csPin, LOW);
    digitalWrite(_csPin, HIGH);
}

// 获取显示宽度
int16_t AmebaParallel8::getWidth()
{
    return _width;
}

// 获取显示高度
int16_t AmebaParallel8::getHeight()
{
    return _height;
}

// 设置光标位置
void AmebaParallel8::setCursor(int16_t x, int16_t y)
{
    cursor_x = x;
    cursor_y = y;
}

// 设置前景色
void AmebaParallel8::setForeground(uint16_t color)
{
    foreground = color;
}

// 设置背景色
void AmebaParallel8::setBackground(uint16_t color)
{
    background = color;
}

// 设置字体大小
void AmebaParallel8::setFontSize(uint8_t size)
{
    fontsize = size;
}

// 绘制像素点
void AmebaParallel8::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x >= _width) || (y >= _height)) return;
    
    setAddress(x, y, x, y);
    writeData16(color);
    
    // 确保显示刷新完成
    delayMicroseconds(1);
    
    // 额外的刷新确保
    digitalWrite(_dcPin, HIGH);
    digitalWrite(_csPin, LOW);
    digitalWrite(_csPin, HIGH);
}

// 绘制直线
void AmebaParallel8::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        int16_t temp = x0; x0 = y0; y0 = temp;
        temp = x1; x1 = y1; y1 = temp;
    }

    if (x0 > x1) {
        int16_t temp = x0; x0 = x1; x1 = temp;
        temp = y0; y0 = y1; y1 = temp;
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            drawPixel(y0, x0, color);
        } else {
            drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

// 绘制矩形边框
void AmebaParallel8::drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    drawLine(x, y, x + w - 1, y, color);
    drawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
    drawLine(x, y, x, y + h - 1, color);
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
}

// 绘制圆形
void AmebaParallel8::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

// 绘制位图
void AmebaParallel8::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color)
{
    if ((x >= _width) || (y >= _height)) return;
    if ((x + w - 1) >= _width) w = _width - x;
    if ((y + h - 1) >= _height) h = _height - y;

    setAddress(x, y, x + w - 1, y + h - 1);

    digitalWrite(_dcPin, HIGH);
    digitalWrite(_csPin, LOW);
    
    // 优化：批量写入，减少函数调用开销
    uint32_t pixelCount = (uint32_t)w * (uint32_t)h;
    for (uint32_t i = 0; i < pixelCount; i++) {
        writeData16(color[i]);
    }
    
    digitalWrite(_csPin, HIGH);
    
    // 确保显示刷新完成
    delayMicroseconds(10);
    
    // 额外的刷新确保
    digitalWrite(_dcPin, HIGH);
    digitalWrite(_csPin, LOW);
    digitalWrite(_csPin, HIGH);
}

// 优化的绘制位图函数 - 专门针对摄像头视频流
void AmebaParallel8::drawBitmapOptimized(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color)
{
    if ((x >= _width) || (y >= _height)) return;
    if ((x + w - 1) >= _width) w = _width - x;
    if ((y + h - 1) >= _height) h = _height - y;

    setAddress(x, y, x + w - 1, y + h - 1);

    digitalWrite(_dcPin, HIGH);
    digitalWrite(_csPin, LOW);
    
    // 高度优化的批量写入：减少函数调用和延迟
    uint32_t pixelCount = (uint32_t)w * (uint32_t)h;
    
    // 使用优化的循环，减少循环开销
    for (uint32_t i = 0; i < pixelCount; i++) {
        uint16_t pixel = color[i];
        
        // 直接写入高位和低位字节，减少函数调用
        write8bitData(pixel >> 8);
        write8bitData(pixel & 0xFF);
    }
    
    digitalWrite(_csPin, HIGH);
    
    // 最小化延迟，仅保证基本刷新
    delayMicroseconds(2);
}

// 绘制字符
void AmebaParallel8::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t fontcolor, uint16_t background, uint8_t fontsize)
{
    // 简单的5x7字体实现，支持数字和基本字符
    static const uint8_t font5x7[95][5] = {
        {0x00, 0x00, 0x00, 0x00, 0x00}, // 空格
        {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
        {0x00, 0x07, 0x00, 0x07, 0x00}, // "
        {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
        {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
        {0x23, 0x13, 0x08, 0x64, 0x62}, // %
        {0x36, 0x49, 0x55, 0x22, 0x50}, // &
        {0x00, 0x05, 0x03, 0x00, 0x00}, // '
        {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
        {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
        {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // *
        {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
        {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
        {0x08, 0x08, 0x08, 0x08, 0x08}, // -
        {0x00, 0x60, 0x60, 0x00, 0x00}, // .
        {0x20, 0x10, 0x08, 0x04, 0x02}, // /
        {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
        {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
        {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
        {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
        {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
        {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
        {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
        {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
        {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
        {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
        {0x00, 0x36, 0x36, 0x00, 0x00}, // :
        {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
        {0x00, 0x08, 0x14, 0x22, 0x41}, // <
        {0x14, 0x14, 0x14, 0x14, 0x14}, // =
        {0x41, 0x22, 0x14, 0x08, 0x00}, // >
        {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
        {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
        {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
        {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
        {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
        {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
        {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
        {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
        {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
        {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
        {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
        {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
        {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
        {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
        {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
        {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
        {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
        {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
        {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
        {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
        {0x46, 0x49, 0x49, 0x49, 0x31}, // S
        {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
        {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
        {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
        {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
        {0x63, 0x14, 0x08, 0x14, 0x63}, // X
        {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
        {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
        {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
        {0x02, 0x04, 0x08, 0x10, 0x20}, // \
        {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
        {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
        {0x40, 0x40, 0x40, 0x40, 0x40}, // _
        {0x00, 0x01, 0x02, 0x04, 0x00}, // `
        {0x20, 0x54, 0x54, 0x54, 0x78}, // a
        {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
        {0x38, 0x44, 0x44, 0x44, 0x20}, // c
        {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
        {0x38, 0x54, 0x54, 0x54, 0x18}, // e
        {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
        {0x0C, 0x52, 0x52, 0x52, 0x3E}, // g
        {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
        {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
        {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
        {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
        {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
        {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
        {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
        {0x38, 0x44, 0x44, 0x44, 0x38}, // o
        {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
        {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
        {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
        {0x48, 0x54, 0x54, 0x54, 0x20}, // s
        {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
        {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
        {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
        {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
        {0x44, 0x28, 0x10, 0x28, 0x44}, // x
        {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
        {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
        {0x00, 0x08, 0x36, 0x41, 0x00}, // {
        {0x00, 0x00, 0x7F, 0x00, 0x00}, // |
        {0x00, 0x41, 0x36, 0x08, 0x00}, // }
        {0x08, 0x08, 0x2A, 0x1C, 0x08}, // ~
    };
    
    if (c < 32 || c > 126) return; // 只支持可打印字符
    
    uint8_t charIndex = c - 32;
    const uint8_t *charData = font5x7[charIndex];
    
    // 绘制字符
    for (int col = 0; col < 5; col++) {
        uint8_t line = charData[col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                // 绘制前景色像素
                for (int fsx = 0; fsx < fontsize; fsx++) {
                    for (int fsy = 0; fsy < fontsize; fsy++) {
                        drawPixel(x + col * fontsize + fsx, y + row * fontsize + fsy, fontcolor);
                    }
                }
            } else if (background != fontcolor) {
                // 绘制背景色像素
                for (int fsx = 0; fsx < fontsize; fsx++) {
                    for (int fsy = 0; fsy < fontsize; fsy++) {
                        drawPixel(x + col * fontsize + fsx, y + row * fontsize + fsy, background);
                    }
                }
            }
        }
    }
    
    // 字符间距
    for (int fsx = 0; fsx < fontsize; fsx++) {
        for (int fsy = 0; fsy < 7 * fontsize; fsy++) {
            if (background != fontcolor) {
                drawPixel(x + 5 * fontsize + fsx, y + fsy, background);
            }
        }
    }
}

// Print类的write方法实现
size_t AmebaParallel8::write(uint8_t c)
{
    if (c == '\n') {
        cursor_y += 8 * fontsize;
        cursor_x = 0;
    } else if (c == '\r') {
        // 回车，不做处理
    } else {
        drawChar(cursor_x, cursor_y, c, foreground, background, fontsize);
        cursor_x += 6 * fontsize;
        if (cursor_x > (_width - 6 * fontsize)) {
            cursor_x = 0;
            cursor_y += 8 * fontsize;
        }
    }
    return 1;
}

// 交换两个16位整数的辅助函数
void _swap_int16_t(int16_t &a, int16_t &b)
{
    int16_t t = a;
    a = b;
    b = t;
}