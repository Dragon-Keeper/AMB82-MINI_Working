#include "AmebaILI9488.h"

/* 构造函数 */
AmebaILI9488::AmebaILI9488(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t sck, uint8_t mosi, uint8_t led)
  : _cs(cs), _dc(dc), _rst(rst), _sck(sck), _mosi(mosi), _led(led), 
    _rotation(0), _cursorX(0), _cursorY(0), _fontSize(1), 
    _foreground(ILI9488_WHITE), _background(ILI9488_BLACK),
    _width(ILI9488_TFTWIDTH), _height(ILI9488_TFTHEIGHT) {}

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

/* 初始化寄存器配置 - 18位RGB666模式优化 */
void AmebaILI9488::initRegisters(void) {
  // 软件复位
  writeCommand(ILI9488_SWRESET);   // 01h - 软件复位
  delay(5);

  // 退出睡眠模式
  writeCommand(ILI9488_SLPOUT);    // 11h - 退出睡眠模式
  delay(120);
  
  // 帧率控制
  writeCommand(ILI9488_FRMCTR1);
  writeData(0xA0);  // 60Hz
  
  writeCommand(ILI9488_FRMCTR2);
  writeData(0x00);
  
  writeCommand(ILI9488_FRMCTR3);
  writeData(0x00);
  
  // 电源控制
  writeCommand(ILI9488_PWCTR1);
  writeData(0x17);  // 设置VRH1[4:0] & VC[2:0]
  writeData(0x15);  // 设置BT[2:0]
  
  writeCommand(ILI9488_PWCTR2);
  writeData(0x41);  // 设置AP[7:0]
  
  writeCommand(ILI9488_VMCTR1);
  writeData(0x00);
  writeData(0x12);
  writeData(0x80);
  
  // 内存访问控制 - 尝试BGR颜色顺序
  writeCommand(ILI9488_MADCTL);
  writeData(MADCTL_BGR | MADCTL_MX);
  
  // 像素格式设置 - 18位RGB666模式
  writeCommand(ILI9488_PIXFMT);
  writeData(0x66);  // 18-bit RGB666格式
  
  // 显示反转控制
  writeCommand(ILI9488_INVCTR);
  writeData(0x02);  // 2-dot inversion
  
  // 显示功能控制
  writeCommand(ILI9488_DFUNCTR);
  writeData(0x02);
  writeData(0x02);
  
  // 颜色增强控制
  writeCommand(0xF7);
  writeData(0xA9);
  writeData(0x51);
  writeData(0x2C);
  writeData(0x82);
  
  // 正伽马校正
  writeCommand(ILI9488_GMCTRP1);
  writeData(0x00); writeData(0x03); writeData(0x09); writeData(0x08); writeData(0x16);
  writeData(0x0A); writeData(0x3F); writeData(0x78); writeData(0x4C); writeData(0x09);
  writeData(0x0A); writeData(0x08); writeData(0x16); writeData(0x1A); writeData(0x0F);
  
  // 负伽马校正
  writeCommand(ILI9488_GMCTRN1);
  writeData(0x00); writeData(0x16); writeData(0x19); writeData(0x03); writeData(0x0F);
  writeData(0x05); writeData(0x32); writeData(0x45); writeData(0x46); writeData(0x04);
  writeData(0x0E); writeData(0x0D); writeData(0x35); writeData(0x37); writeData(0x0F);
  
  // 正常显示模式
  writeCommand(ILI9488_NORON);
  delay(10);
  
  // 关闭显示反转
  writeCommand(ILI9488_INVOFF);
  delay(10);
  
  // 打开显示
  writeCommand(ILI9488_DISPON);
  delay(100);
}

/* 设置旋转方向 */
void AmebaILI9488::setRotation(uint8_t r) {
  _rotation = r % 4;  // 确保旋转角度在0-3范围内
  
  writeCommand(ILI9488_MADCTL);
  switch (_rotation) {
    case 0:
      writeData(MADCTL_BGR | MADCTL_MX);
      _width = ILI9488_TFTWIDTH;
      _height = ILI9488_TFTHEIGHT;
      break;
    case 1:
      writeData(MADCTL_BGR | MADCTL_MV);
      _width = ILI9488_TFTHEIGHT;
      _height = ILI9488_TFTWIDTH;
      break;
    case 2:
      writeData(MADCTL_BGR | MADCTL_MY);
      _width = ILI9488_TFTWIDTH;
      _height = ILI9488_TFTHEIGHT;
      break;
    case 3:
      writeData(MADCTL_BGR | MADCTL_MX | MADCTL_MY | MADCTL_MV);
      _width = ILI9488_TFTHEIGHT;
      _height = ILI9488_TFTWIDTH;
      break;
  }
}

/* 反转显示 */
void AmebaILI9488::invertDisplay(boolean i) {
  writeCommand(i ? ILI9488_INVON : ILI9488_INVOFF);
}

/* 设置GRAM窗口 */
void AmebaILI9488::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  writeCommand(ILI9488_CASET);
  // 在18位模式下，仍使用16位地址
  writeData(x0 >> 8);
  writeData(x0 & 0xFF);
  writeData(x1 >> 8);
  writeData(x1 & 0xFF);
  
  writeCommand(ILI9488_PASET);
  writeData(y0 >> 8);
  writeData(y0 & 0xFF);
  writeData(y1 >> 8);
  writeData(y1 & 0xFF);
  
  // 准备写入RAM
  writeCommand(ILI9488_RAMWR);
}

/* 清屏 - 18位RGB666优化版 */
void AmebaILI9488::fillScreen(uint32_t color) {
  fillRect(0, 0, _width, _height, color);
}

/* 快速绘制水平线 */
void AmebaILI9488::drawFastHLine(uint16_t x, uint16_t y, uint16_t w, uint32_t color) {
  if ((x >= _width) || (y >= _height) || (y < 0)) return;
  if ((x + w - 1) < 0) return;
  
  if (x < 0) {
    w += x;
    x = 0;
  }
  if (x + w > _width) {
    w = _width - x;
  }
  
  setAddrWindow(x, y, x + w - 1, y);
  
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  
  CS_LOW();
  DC_HIGH();
  
  for (uint16_t i = 0; i < w; i++) {
    SPI_Transfer(r);
    SPI_Transfer(g);
    SPI_Transfer(b);
  }
  
  CS_HIGH();
}

/* 快速绘制垂直线 */
void AmebaILI9488::drawFastVLine(uint16_t x, uint16_t y, uint16_t h, uint32_t color) {
  if ((x >= _width) || (x < 0) || (y >= _height)) return;
  if ((y + h - 1) < 0) return;
  
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (y + h > _height) {
    h = _height - y;
  }
  
  setAddrWindow(x, y, x, y + h - 1);
  
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  
  CS_LOW();
  DC_HIGH();
  
  for (uint16_t i = 0; i < h; i++) {
    SPI_Transfer(r);
    SPI_Transfer(g);
    SPI_Transfer(b);
  }
  
  CS_HIGH();
}

/* 绘制像素 */
void AmebaILI9488::drawPixel(uint16_t x, uint16_t y, uint32_t color) {
  if (x >= _width || y >= _height || x < 0 || y < 0) return;
  setAddrWindow(x, y, x, y);
  writePixelData(color);
}

/* 写入像素颜色，18-bit RGB666模式 - 按照文档规范实现 */
void AmebaILI9488::writePixelData(uint32_t color) {
  // 确保RGB666格式的正确数据顺序：MSB为R5，LSB为B0
  // 根据文档SPI模式下的18位像素数据顺序规范
  DC_HIGH();
  CS_LOW();
  
  // 提取RGB各6位数据，确保正确的位对齐
  uint8_t r_high = (color >> 16) & 0xFF;  // R6位 (R5-R0)
  uint8_t g_high = (color >> 8) & 0xFF;   // G6位 (G5-G0)
  uint8_t b_high = color & 0xFF;          // B6位 (B5-B0)
  
  // 尝试调整颜色通道发送顺序 - 交换红蓝通道
  // 注意：与MADCTL_BGR设置配合使用时，这里仍按R→G→B发送
  // 控制器会根据MADCTL设置自动调整内部映射
  SPI_Transfer(r_high);   // R6位（高字节）
  SPI_Transfer(g_high);   // G6位（中字节）
  SPI_Transfer(b_high);   // B6位（低字节）
  
  CS_HIGH();
}

/* 绘制线段 */
void AmebaILI9488::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color) {
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
void AmebaILI9488::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
  drawLine(x, y, x + w - 1, y, color);
  drawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
  drawLine(x, y, x, y + h - 1, color);
  drawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
}

/* 填充矩形 - 18位RGB666优化版 */
void AmebaILI9488::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
  if (x >= _width || y >= _height) return;
  if (x + w > _width) w = _width - x;
  if (y + h > _height) h = _height - y;
  if (w <= 0 || h <= 0) return;
  
  setAddrWindow(x, y, x + w - 1, y + h - 1);
  
  // 提取RGB各6位数据，预计算以提高性能
  uint8_t r_high = (color >> 16) & 0xFF;  // R6位 (R5-R0)
  uint8_t g_high = (color >> 8) & 0xFF;   // G6位 (G5-G0)
  uint8_t b_high = color & 0xFF;          // B6位 (B5-B0)
  
  // 优化：保持片选信号低直到所有像素都写入完成，提高填充效率
  DC_HIGH();
  CS_LOW();
  
  // 按照RGB666格式要求写入所有像素
  for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
    SPI_Transfer(r_high);   // R6位
    SPI_Transfer(g_high);   // G6位
    SPI_Transfer(b_high);   // B6位
  }
  
  CS_HIGH();
}

/* 设置滚动位置 */
void AmebaILI9488::scrollTo(uint16_t y) {
  writeCommand(0x37);  // Vertical Scrolling Start Address
  writeData(y >> 8);
  writeData(y & 0xFF);
}

/* 设置滚动边距 */
void AmebaILI9488::setScrollMargins(uint16_t top, uint16_t bottom) {
  if (top + bottom <= _height) {
    uint16_t middle = _height - top - bottom;
    
    writeCommand(0x33);  // Vertical Scrolling Definition
    writeData(top >> 8);
    writeData(top & 0xFF);
    writeData(middle >> 8);
    writeData(middle & 0xFF);
    writeData(bottom >> 8);
    writeData(bottom & 0xFF);
  }
}

/* 绘制圆形 */
void AmebaILI9488::drawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint32_t color) {
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
void AmebaILI9488::fillCircle(uint16_t x0, uint16_t y0, uint8_t r, uint32_t color) {
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
void AmebaILI9488::setForeground(uint32_t color) {
  _foreground = color;
}

/* 设置背景色 */
void AmebaILI9488::setBackground(uint32_t color) {
  _background = color;
}

/* 绘制字符 */
void AmebaILI9488::drawChar(uint16_t x, uint16_t y, char c, uint32_t color, uint32_t bg, uint8_t size) {
  if ((x >= _width) || (y >= _height) || ((x + 6 * size - 1) < 0) || ((y + 8 * size - 1) < 0)) {
    return;
  }
  
  if (c >= 32 && c <= 127) {
    for (int8_t i = 0; i < 6; i++) { // 6列宽，包括右边距
      uint8_t line = 0;
      if (i < 5) {
        line = pgm_read_byte(font5x7 + (c - 32) * 5 + i);
      }
      
      for (int8_t j = 0; j < 8; j++) {
        if (line & 0x1) {
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
        line >>= 1;
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

/* 写入24位数据 */
void AmebaILI9488::writeData24(uint32_t data) {
  // 保持片选信号低直到三个字节都发送完成
  DC_HIGH();
  CS_LOW();
  SPI_Transfer((data >> 16) & 0xFF);
  SPI_Transfer((data >> 8) & 0xFF);
  SPI_Transfer(data & 0xFF);
  CS_HIGH();
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

/* RGB888到RGB666的转换方法实现 */
uint32_t AmebaILI9488::color666(uint8_t r, uint8_t g, uint8_t b) {
  return rgb888_to_rgb666(r, g, b);
}