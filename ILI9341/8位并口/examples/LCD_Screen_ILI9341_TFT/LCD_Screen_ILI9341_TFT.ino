/*
 * This sketch demonstrates how to use TFT LCD with ILI9341 api using 8-bit parallel bus
 *
 * Pre-requirement:
 *     an ILI9341 TFT LCD with 8-bit parallel interface
 *
 * An ILI9341 TFT LCD with 8-bit parallel interface can be used with GPIO pins
 * to send command and data. We can draw text, line, circle, and other picture on it.
 */

#include "AmebaParallel8.h"
#include "AmebaLogo.h"

// 一定要接5V电源，接3.3V会导致无法显示
// 8位并行总线引脚定义
// 控制引脚
#define TFT_CS    12   // 片选
#define TFT_DC    4    // 数据/命令选择
#define TFT_RESET 5    // 复位
#define TFT_WR    6    // 写使能
#define TFT_RD    7    // 读使能

// 数据引脚 (8位) - 为Ameba平台定义引脚数组
const PortPin dataPins[8] = {8, 9, 2, 3, 10, 11, 13, 14};

// 创建8位并行总线驱动对象 - 使用新的构造函数
AmebaParallel8 tft(dataPins, TFT_CS, TFT_DC, TFT_RESET, TFT_WR, TFT_RD);

void setup()
{
    Serial.begin(115200);
    Serial.println("ILI9341 8-bit Parallel Bus Test!");

    tft.begin();

    Serial.println("test filling screen");
    testFillScreen();
    delay(500);

    Serial.println("test Lines");
    testLines(0x07FF); // CYAN
    delay(500);

    Serial.println("test Circles");
    testCircles(5, 0x001F); // BLUE
    delay(500);

    Serial.println("test Rectangle");
    testRectangle(0xC618); // LIGHTGREY
    delay(500);

    Serial.println("test Bitmap");
    testBitmap(0, 0, logoWidth, logoHeight, logo);
    delay(500);

    Serial.println("test Text");
    testText();
    delay(500);
}

void loop()
{
    for (int i = 0; i < 4; i++) {
        tft.setRotation(i);
        testText();
        delay(500);
    }
}

// 原有的测试函数保持不变
unsigned long testFillScreen()
{
    tft.fillScreen(0x0000); // BLACK
    tft.fillScreen(0xF800); // RED
    tft.fillScreen(0x07E0); // GREEN
    tft.fillScreen(0x001F); // BLUE
    tft.fillScreen(0x0000); // BLACK

    return 0;
}

unsigned long testText()
{
    tft.clr();

    tft.setCursor(0, 0);

    tft.setForeground(0x07E0); // GREEN
    tft.setFontSize(5);
    tft.println("Ameba");

    tft.setForeground(0xFFFF); // WHITE
    tft.setFontSize(3);
    tft.println("Hello World!");

    tft.setForeground(0xFFE0); // YELLOW
    tft.setFontSize(2);
    tft.println(1234.56);

    tft.setForeground(0xF800); // RED
    tft.setFontSize(1);
    tft.println(0xDEADBEEF, HEX);

    tft.println();

    tft.setForeground(0xC618); // LIGHTGREY
    tft.setFontSize(2);
    tft.println("Alice in Wonderland");

    tft.setFontSize(1);
    tft.println("\"Where should I go?\" -Alice. \"That depends on where you want to end up.\" - The Cheshire Cat.");

    return 0;
}

unsigned long testLines(uint16_t color)
{
    int x0, y0, x1, y1;
    int w = tft.getWidth();
    int h = tft.getHeight();

    tft.clr();

    x0 = 0;
    x1 = w;
    for (y0 = y1 = 0; y0 <= h; y0 += 5, y1 += 5) {
        tft.drawLine(x0, y0, x1, y1, color);
    }
    y0 = 0;
    y1 = h;
    for (x0 = x1 = 0; x0 <= w; x0 += 5, x1 += 5) {
        tft.drawLine(x0, y0, x1, y1, color);
    }

    tft.fillScreen(0x0000); // BLACK

    x0 = w / 2;
    y0 = h / 2;
    x1 = 0;
    for (y1 = 0; y1 < h; y1 += 5) {
        tft.drawLine(x0, y0, x1, y1, color);
    }

    y1 = h;
    for (x1 = 0; x1 < w; x1 += 5) {
        tft.drawLine(x0, y0, x1, y1, color);
    }

    x1 = w;
    for (y1 = h; y1 >= 0; y1 -= 5) {
        tft.drawLine(x0, y0, x1, y1, color);
    }

    y1 = 0;
    for (x1 = w; x1 >= 0; x1 -= 5) {
        tft.drawLine(x0, y0, x1, y1, color);
    }

    return 0;
}

void testRectangle(uint16_t color)
{
    int rw, rh;
    int div = 60;
    int w = tft.getWidth();
    int h = tft.getHeight();

    tft.clr();

    for (rw = w / div, rh = h / div; rw < w; rw += w / div, rh += h / div) {
        tft.drawRectangle((w / 2 - rw), (h / 2 - rh), (rw * 2), (rh * 2), color);
    }
}

void testCircles(uint8_t radius, uint16_t color)
{
    int x, y;
    int w = tft.getWidth();
    int h = tft.getHeight();

    tft.clr();

    for (x = radius; x < w; x += radius * 2) {
        for (y = radius; y < h; y += radius * 2) {
            tft.drawCircle(x, y, radius, color);
        }
    }
}

void testBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color)
{
    tft.clr();
    tft.drawBitmap(x, y, w, h, color);
    delay(500);
}
