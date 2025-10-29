#include "AmebaILI9488.h"

/* 构造函数 */
AmebaILI9488::AmebaILI9488(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t sck, uint8_t mosi, uint8_t led)
  : _cs(cs), _dc(dc), _rst(rst), _sck(sck), _mosi(mosi), _led(led), 
    _madctl(0), _cursorX(0), _cursorY(0), _fontSize(1), 
    _foreground(ILI9488_WHITE), _background(ILI9488_BLACK),
    _width(320), _height(480) {}

/* 初始化显示器 */
void AmebaILI9488::begin(void) {
  // 初始化所有引脚
  pinMode(_cs, OUTPUT);
  pinMode(_dc, OUTPUT);
  pinMode(_rst, OUTPUT);
  pinMode(_sck, OUTPUT);
  pinMode(_mosi, OUTPUT);
  pinMode(_led, OUTPUT);
  
  // 设置初始状态
  CS_HIGH();
  DC_HIGH();
  SCK_LOW();
  MOSI_LOW();
  
  // 打开背光
  digitalWrite(_led, HIGH);
  
  // 硬件复位
  hardwareReset();
  
  // 初始化寄存器
  initRegisters();
}

/* 硬件复位 */
void AmebaILI9488::hardwareReset(void) {
  digitalWrite(_rst, HIGH);
  delay(5);
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(120);
}

/* 初始化寄存器配置 */
void AmebaILI9488::initRegisters(void) {
  writeCommand(CMD_SWRESET);   // 01h - 软件复位
  delay(5);

  writeCommand(CMD_SLPOUT);    // 11h - 退出睡眠模式
  delay(120);

  writeCommand(CMD_COLMOD);    // 3Ah - 设置像素格式
  writeData(0x66);             // 18-bit 6-6-6格式

  writeCommand(CMD_MADCTL);    // 36h - 内存数据访问控制
  writeData(0x48);             // RGB顺序，默认扫描方向

  writeCommand(CMD_PWCTRL1);   // C0h - 电源控制1
  writeData(0x0E);
  writeData(0x0E);

  writeCommand(CMD_PWCTRL2);   // C1h - 电源控制2
  writeData(0x44);

  writeCommand(CMD_VMCTRL1);   // C5h - VCOM控制
  writeData(0x00);
  writeData(0x40);
  writeData(0x00);
  writeData(0x40);

  /* 正伽马校正 */
  writeCommand(CMD_PGAMCTRL);  // E0h
  const uint8_t pgam[] = {
    0x0F,0x1F,0x1C,0x0C,0x0F,0x08,0x48,0x98,
    0x37,0x0A,0x13,0x04,0x11,0x0D,0x00
  };
  writeBytes((uint8_t*)pgam, 15);

  /* 负伽马校正 */
  writeCommand(CMD_NGAMCTRL);  // E1h
  const uint8_t ngam[] = {
    0x0F,0x32,0x2E,0x0B,0x0D,0x05,0x47,0x75,
    0x37,0x06,0x10,0x03,0x24,0x20,0x00
  };
  writeBytes((uint8_t*)ngam, 15);

  writeCommand(CMD_INVOFF);    // 20h - 关闭反转显示
  writeCommand(CMD_DISPON);    // 29h - 打开显示
}

/* 设置旋转方向 */
void AmebaILI9488::setRotation(uint8_t r) {
  _madctl = 0x48;              // 基础值
  switch (r & 3) {
    case 0: 
      _madctl |= 0x00; 
      _width = 320; 
      _height = 480; 
      break;
    case 1: 
      _madctl |= 0xA0; 
      _width = 480; 
      _height = 320; 
      break;
    case 2: 
      _madctl |= 0xC0; 
      _width = 320; 
      _height = 480; 
      break;
    case 3: 
      _madctl |= 0x60; 
      _width = 480; 
      _height = 320; 
      break;
  }
  writeCommand(CMD_MADCTL);
  writeData(_madctl);
}

/* 设置GRAM窗口 */
void AmebaILI9488::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  writeCommand(CMD_CASET);
  writeData16(x0);
  writeData16(x1);
  writeCommand(CMD_PASET);
  writeData16(y0);
  writeData16(y1);
}

/* 清屏 */
void AmebaILI9488::fillScreen(uint16_t color) {
  setAddrWindow(0, 0, _width - 1, _height - 1);
  writeCommand(CMD_RAMWR);
  for (uint32_t i = 0; i < (uint32_t)_width * _height; i++) {
    writePixel(color);
  }
}

/* 绘制像素点 */
void AmebaILI9488::drawPixel(uint16_t x, uint16_t y, uint16_t color) {
  if (x >= _width || y >= _height) return;
  setAddrWindow(x, y, x, y);
  writeCommand(CMD_RAMWR);
  writePixel(color);
}

/* 写入像素颜色，18-bit模式 */
void AmebaILI9488::writePixel(uint16_t color) {
  uint8_t r = (color >> 11) & 0x1F;
  uint8_t g = (color >> 5)  & 0x3F;
  uint8_t b =  color        & 0x1F;
  writeData((r << 1) | (g >> 5));   // R[5:1] + G[5:4]
  writeData((g << 3) | (b >> 3));   // G[3:0] + B[5:1]
  writeData(b << 5);                // B[0] 和 0
}

/* 绘制线段 */
void AmebaILI9488::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
  // Bresenham算法实现
  int16_t dx = abs((int16_t)x1 - (int16_t)x0);
  int16_t dy = abs((int16_t)y1 - (int16_t)y0);
  int16_t sx = x0 < x1 ? 1 : -1;
  int16_t sy = y0 < y1 ? 1 : -1;
  int16_t err = dx - dy;
  
  while (true) {
    drawPixel(x0, y0, color);
    
    if (x0 == x1 && y0 == y1) break;
    
    int16_t e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

/* 绘制矩形 */
void AmebaILI9488::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  drawLine(x, y, x + w - 1, y, color);
  drawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
  drawLine(x, y, x, y + h - 1, color);
  drawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
}

/* 填充矩形 */
void AmebaILI9488::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
  if (x >= _width || y >= _height) return;
  if (x + w > _width) w = _width - x;
  if (y + h > _height) h = _height - y;
  
  setAddrWindow(x, y, x + w - 1, y + h - 1);
  writeCommand(CMD_RAMWR);
  
  for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
    writePixel(color);
  }
}

/* 绘制圆形 */
void AmebaILI9488::drawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color) {
  int16_t x = r;
  int16_t y = 0;
  int16_t err = 0;
  
  while (x >= y) {
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 - y, y0 - x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 + x, y0 - y, color);
    
    y++;
    err += 1 + 2 * y;
    if (2 * (err - x) + 1 > 0) {
      x--;
      err += 1 - 2 * x;
    }
  }
}

/* 填充圆形 */
void AmebaILI9488::fillCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color) {
  int16_t x = r;
  int16_t y = 0;
  int16_t err = 0;
  
  while (x >= y) {
    drawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
    drawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
    drawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
    drawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);
    
    y++;
    err += 1 + 2 * y;
    if (2 * (err - x) + 1 > 0) {
      x--;
      err += 1 - 2 * x;
    }
  }
}

/* 设置光标位置 */
void AmebaILI9488::setCursor(uint16_t x, uint16_t y) {
  _cursorX = x;
  _cursorY = y;
}

/* 设置字体大小 */
void AmebaILI9488::setFontSize(uint8_t size) {
  _fontSize = size;
}

/* 设置前景色 */
void AmebaILI9488::setForeground(uint16_t color) {
  _foreground = color;
}

/* 设置背景色 */
void AmebaILI9488::setBackground(uint16_t color) {
  _background = color;
}

/* 绘制字符 */
void AmebaILI9488::drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
  if ((x >= _width) || (y >= _height)) return;
  if ((c < 32) || (c > 127)) c = ' ';
  
  c -= 32; // 偏移到字体数组的开始位置
  
  for (int8_t i = 0; i < 5; i++) { // 5列宽的字体
    uint8_t line = PGM_READ_BYTE(&font5x7[c * 5 + i]);
    for (int8_t j = 0; j < 7; j++) { // 7行高的字体
      if (line & (1 << j)) {
        if (size == 1) {
          drawPixel(x + i, y + j, color);
        } else {
          fillRect(x + i * size, y + j * size, size, size, color);
        }
      } else if (bg != color) {
        if (size == 1) {
          drawPixel(x + i, y + j, bg);
        } else {
          fillRect(x + i * size, y + j * size, size, size, bg);
        }
      }
    }
  }
}

/* 写入单个字符 */
size_t AmebaILI9488::write(uint8_t c) {
  if (c == '\n') {
    _cursorY += 8 * _fontSize;
    _cursorX = 0;
  } else if (c == '\r') {
    // 回车不做任何操作
  } else {
    drawChar(_cursorX, _cursorY, c, _foreground, _background, _fontSize);
    _cursorX += 6 * _fontSize; // 字符宽度 + 1个像素间距
    if (_cursorX >= _width) {
      _cursorY += 8 * _fontSize;
      _cursorX = 0;
    }
  }
  return 1;
}

/* 打印字符串 */
void AmebaILI9488::print(const char* str) {
  while (*str) {
    write(*str++);
  }
}

/* 打印字符串并换行 */
void AmebaILI9488::println(const char* str) {
  print(str);
  _cursorY += 8 * _fontSize;
  _cursorX = 0;
}

/* 软件SPI传输函数 */
inline void AmebaILI9488::SPI_Transfer(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    SCK_LOW();
    if (data & 0x80) {
      MOSI_HIGH();
    } else {
      MOSI_LOW();
    }
    data <<= 1;
    SCK_HIGH();
  }
}

/* 写入命令 */
void AmebaILI9488::writeCommand(uint8_t cmd) {
  DC_LOW();
  CS_LOW();
  SPI_Transfer(cmd);
  CS_HIGH();
}

/* 写入数据 */
void AmebaILI9488::writeData(uint8_t data) {
  DC_HIGH();
  CS_LOW();
  SPI_Transfer(data);
  CS_HIGH();
}

/* 写入16位数据 */
void AmebaILI9488::writeData16(uint16_t data) {
  writeData(data >> 8);
  writeData(data & 0xFF);
}

/* 写入多个字节 */
void AmebaILI9488::writeBytes(uint8_t *data, uint32_t len) {
  DC_HIGH();
  CS_LOW();
  for (uint32_t i = 0; i < len; i++) {
    SPI_Transfer(data[i]);
  }
  CS_HIGH();
}