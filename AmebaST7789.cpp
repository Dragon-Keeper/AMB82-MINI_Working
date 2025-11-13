
#include "AmebaST7789.h"
#include "font5x7.h"
#include "spi_ex_api.h"
// 静态帧缓存定义
pixel_t __attribute__((aligned(4))) AmebaST7789::frameBuffer[2][AmebaST7789::MAX_FRAME_SIZE];

AmebaST7789::AmebaST7789(int csPin, int dcPin, int resetPin, uint16_t width, uint16_t height)
{
    _csPin = csPin;
    _dcPin = dcPin;
    _resetPin = resetPin;
    _width = width;
    _height = height;
    cursor_x = 0;
    cursor_y = 0;
    foreground = ST7789_WHITE;
    background = ST7789_WHITE;
    fontsize = 1;
    rotation = 0;
    // pinMode(_csPin, OUTPUT);
    // digitalWrite(_csPin, HIGH);

    pinMode(_dcPin, OUTPUT);
}

void AmebaST7789::begin()
{   if(_resetPin != -1){
      pinMode(_resetPin, OUTPUT);
      digitalWrite(_resetPin, LOW);
      delay(10);
      digitalWrite(_resetPin, HIGH);
      delay(100);
    }
    
    _csPin = (PinName)g_APinDescription[_csPin].pinname;
    // 初始化 SPI
    spi_init(&spi_obj, PE_3, PE_2, PE_1, (PinName)(_csPin));  // MOSI, MISO, SCLK, CSN
    spi_format(&spi_obj, 8, 0, 0);              // 8-bit, mode 0
    spi_frequency(&spi_obj, 25000000);          // 最大频率
    spi_enable(&spi_obj);
    // 注册中断回调
    spi_irq_hook(&spi_obj, spi_dma_complete_callback, (uint32_t)this);

    
    // 屏幕初始化命令
    writecommand(ST7789_SWRESET);
    delay(150);
    writecommand(ST7789_SLPOUT);
    delay(120);

    writecommand(ST7789_COLMOD);
    writedata(0x55);  // RGB565

    writecommand(ST7789_MADCTL);
    writedata(ST7789_MADCTL_RGB);   //根据需要选择命令 ST7789_MADCTL_RGB 和 ST7789_MADCTL_BGR
    
    
    writecommand(ST7789_INVON); // 加入反色控制，请根据需要选择
    writecommand(ST7789_NORON);    // Normal display on
    writecommand(ST7789_DISPON);

    
   
}


void AmebaST7789::spi_dma_complete_callback(uint32_t id, SpiIrq event)
{
    AmebaST7789* tft = reinterpret_cast<AmebaST7789*>(id);
    if (event == SpiTxIrq) {
        //digitalWrite(tft->_csPin, HIGH);  // 结束传输
        // 此处可以触发下一次刷新或释放锁
        tft->dmaInProgress = false;
    }
}



void AmebaST7789::setRotation(uint8_t m)
{
    writecommand(ST7789_MADCTL);
    rotation = m % 4;
    switch (rotation) {
        case 0: // Portrait
            writedata(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
            break;
        case 1: // Landscape
            writedata(ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
            break;
        case 2: // Inverted portrait
            writedata(ST7789_MADCTL_RGB);
            break;
        case 3: // Inverted landscape
            writedata(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
            break;
    }
}





void AmebaST7789::fillScreen(pixel_t color)
{
    pixel_t* fb = frameBuffer[frontBufferIndex];
    size_t total = _width * _height;
    for (size_t i = 0; i < total; i++) {
        fb[i] = color;
    }
}

void AmebaST7789::drawPixel(int16_t x, int16_t y, pixel_t color)
{
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    pixel_t* fb = frameBuffer[frontBufferIndex];
    fb[y * _width + x] = color;
}

void AmebaST7789::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const pixel_t* data)
{
    if (x < 0 || y < 0 || x + w > _width || y + h > _height) return;
    pixel_t* fb = frameBuffer[frontBufferIndex];
    for (int j = 0; j < h; j++) {
        memcpy(fb + (y + j) * _width + x, data + j * w, w * sizeof(pixel_t));
    }
}


void AmebaST7789::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    int16_t temp;
    bool exchange_xy = false;
    int16_t dx, dy, err, ystep, x, y;

    // 交换坐标使 x0 <= x1
    if (x0 > x1) {
        temp = x0; x0 = x1; x1 = temp;
        temp = y0; y0 = y1; y1 = temp;
    }

    // 处理垂直线
    if (x0 == x1) {
        if (y0 > y1) { temp = y0; y0 = y1; y1 = temp; }
        for (int16_t y = y0; y <= y1; y++) {
            drawPixel(x0, y, color);
        }
        return;
    }

    // 处理水平线
    if (y0 == y1) {
        for (int16_t x = x0; x <= x1; x++) {
            drawPixel(x, y0, color);
        }
        return;
    }

    // Bresenham's line algorithm
    dx = x1 - x0;
    dy = abs(y1 - y0);
    err = dx / 2;
    ystep = (y0 < y1) ? 1 : -1;
    y = y0;

    if (abs(y1 - y0) > dx) {
        exchange_xy = true;
        // 交换 x 和 y
        temp = x0; x0 = y0; y0 = temp;
        temp = x1; x1 = y1; y1 = temp;
        dx = x1 - x0;
        dy = abs(y1 - y0);
        err = dx / 2;
        ystep = (y0 < y1) ? 1 : -1;
        y = y0;
    }

    for (x = x0; x <= x1; x++) {
        if (exchange_xy)
            drawPixel(y, x, color);
        else
            drawPixel(x, y, color);

        err -= dy;
        if (err < 0) {
            y += ystep;
            err += dx;
        }
    }
}

void AmebaST7789::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawLine(x, y, x+w-1, y, color);
    drawLine(x, y+h-1, x+w-1, y+h-1, color);
    drawLine(x, y, x, y+h-1, color);
    drawLine(x+w-1, y, x+w-1, y+h-1, color);
}

void AmebaST7789::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
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

void AmebaST7789::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    // Draw vertical line for the center column of the circle
    for (int16_t y = y0 - r; y <= y0 + r; y++) {
        drawPixel(x0, y, color);
    }

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        // Fill the horizontal lines at each step
        for (int16_t dy = -y; dy <= y; dy++) {
            drawPixel(x0 + x, y0 + dy, color);
            drawPixel(x0 - x, y0 + dy, color);
            drawPixel(x0 + y, y0 + dy, color); 
            drawPixel(x0 - y, y0 + dy, color);
        }
    }
}
void AmebaST7789::fillRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if ((x >= _width) || (y >= _height)) return;
    if ((x + w - 1) >= _width) w = _width - x;
    if ((y + h - 1) >= _height) h = _height - y;

    // Fill the rectangle in framebuffer
    for (int16_t i = x; i < x + w; i++) {
        for (int16_t j = y; j < y + h; j++) {
            drawPixel(i, j, color);
        }
    }
}


void AmebaST7789::directWrite(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t *pixels) {
    setAddress(x, y, w-1, h-1);
    digitalWrite(_dcPin, HIGH);
    spi_master_write_stream_dma(&spi_obj, (char*)pixels, w*h*sizeof(uint16_t )); // 16-bit 数据
}

void AmebaST7789::setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    writecommand(ST7789_CASET);
    writedata(x0 >> 8);
    writedata(x0 & 0xFF);
    writedata(x1 >> 8);
    writedata(x1 & 0xFF);

    writecommand(ST7789_RASET);
    writedata(y0 >> 8);
    writedata(y0 & 0xFF);
    writedata(y1 >> 8);
    writedata(y1 & 0xFF);

    writecommand(ST7789_RAMWR);
    digitalWrite(_dcPin, HIGH);
}

void AmebaST7789::writecommand(uint8_t command)
{
    digitalWrite(_dcPin, 0);
    spi_master_write(&spi_obj, command);
}

void AmebaST7789::writedata(uint8_t data)
{
    digitalWrite(_dcPin, 1);
    spi_master_write(&spi_obj, data);
}
void AmebaST7789::drawChar(unsigned char c)
{
    drawChar(cursor_x, cursor_y, c, foreground, background, fontsize);
}

void AmebaST7789::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t _fontcolor, uint16_t _background, uint8_t _fontsize)
{
    int i, j;
    uint8_t line;

    foreground = _fontcolor;
    background = _background;
    fontsize = _fontsize;

    if ((x >= _width) || (y >= _height) || (x + 6 * fontsize - 1) < 0 || (y + 8 * fontsize - 1) < 0) {
        return;
    }

    for (i = 0; i < 6; i++) {
        if (i < 5) {
            line = font5x7[(c * 5 + i)];
        } else {
            line = 0x00;
        }
        for (j = 0; j < 8; j++, line >>= 1) {
            if (line & 0x01) {
                if (fontsize == 1) {
                    drawPixel((x + i), (y + j), foreground);
                } else {
                    fillRectangle((x + i * fontsize), (y + j * fontsize), fontsize, fontsize, foreground);
                }
            } else if (background != foreground) {
                if (fontsize == 1) {
                    drawPixel((x + i), (y + j), background);
                } else {
                    fillRectangle((x + i * fontsize), (y + j * fontsize), fontsize, fontsize, background);
                }
            }
        }
    }

    cursor_x += fontsize * 6;
    cursor_y = y;
}
void AmebaST7789::drawString(const char *str, int16_t x, int16_t y, uint16_t _fontcolor, uint16_t _background, uint8_t _fontsize) {

    int16_t char_width = 8 * _fontsize;
    int16_t char_height = 16 * _fontsize; // 假设字符高度为16像素

    while (*str) { // 遍历字符串中的每个字符
        if (*str == '\n') { // 如果遇到换行符
            // 移动到下一行开始的位置
            y += char_height;
            x = 0; // 或者你希望的初始x位置
        } else {
            // 绘制当前字符
            drawChar(x, y, *str, _fontcolor, _background, _fontsize);
            // 更新x坐标以绘制下一个字符
            x += char_width;
        }
        str++; // 移动到下一个字符
    }
}
void AmebaST7789::drawString(const char *str)
{
    drawString(str,cursor_x, cursor_y, foreground, background, fontsize);
}
void AmebaST7789::flush()
{   while (dmaInProgress) {
      delayMicroseconds(1);  // 如果上一次 DMA 还未完成，等待
    }

    

    setAddress(0, 0, _width - 1, _height - 1);

    const uint8_t* data = reinterpret_cast<const uint8_t*>(frameBuffer[frontBufferIndex]);
    size_t length = _width * _height * sizeof(pixel_t);
    dmaInProgress = true;              // ⬅️ 设置为 true，表示开始 DMA 传输
    start_dma_transfer(data, length);
}

void AmebaST7789::start_dma_transfer(const uint8_t* data, size_t len)
{
    digitalWrite(_dcPin, HIGH);
    //digitalWrite(_csPin, LOW);

    spi_master_write_stream_dma(&spi_obj, (char*)data, len);
}

void AmebaST7789::setFrontBufferIndex(uint8_t index)
{
    frontBufferIndex = index % 2;
}
void AmebaST7789::reset(void)
{
    digitalWrite(_resetPin, HIGH);
    delay(5);
    digitalWrite(_resetPin, LOW);
    delay(20);
    digitalWrite(_resetPin, HIGH);
    delay(150);
}