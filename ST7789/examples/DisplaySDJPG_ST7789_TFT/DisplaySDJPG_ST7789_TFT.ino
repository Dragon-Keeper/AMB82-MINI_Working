/*

 This sketch shows the example of displaying jpg image from micro SD card on ILI9341 TFT display module.

 Example guide:
 https://ameba-doc-arduino-sdk.readthedocs-hosted.com/en/latest/ameba_pro2/amb82-mini/Example_Guides/SPI/Display%20SD%20JPG%20ILI9341%20TFT.html

*/

#include "SPI.h"
#include "AmebaST7789_DMA.h"   // 使用DMA版本的ST7789库
// Include the jpeg decoder library
#include "TJpg_Decoder.h"
#include "AmebaFatFS.h"

#define TFT_RESET 5
#define TFT_DC    4
#define TFT_CS    SPI_SS

#define ST7789_SPI_FREQUENCY 50000000   // 50 MHz (DMA版本需要更高频率)

AmebaST7789_DMA tft = AmebaST7789_DMA(TFT_CS, TFT_DC, TFT_RESET);

AmebaFatFS fs;

uint32_t currentImageIndex = 0;
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 3000; // 3秒更新间隔
String filenamePrefix = "Image_";
String filenameSuffix = ".jpg";

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    tft.drawBitmap(x, y, w, h, bitmap);

    // Return 1 to decode next block
    return 1;
}

void setup()
{
    Serial.begin(115200);

    Serial.println("TFT ST7789 DMA - 自动播放 Image_X.jpg 照片");

    SPI.setDefaultFrequency(ST7789_SPI_FREQUENCY);

    tft.begin();
    tft.setRotation(3);          // 设置为逆时针90度旋转（修正显示方向）
    tft.setColorOrder(true);    // 设置为RGB模式（修正颜色偏差）

    // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(2);

    tft.clr();
    tft.setCursor(0, 0);

    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(tft_output);

    lastUpdateTime = millis(); // 初始化更新时间
}

void loop()
{
    unsigned long currentTime = millis();
    
    // 每3秒更新一次照片
    if (currentTime - lastUpdateTime >= updateInterval) {
        displayNextImage();
        lastUpdateTime = currentTime;
    }
}

void displayNextImage() {
    fs.begin();
    String file_path = String(fs.getRootPath());
    
    // 尝试查找当前索引的照片
    String currentFilename = filenamePrefix + String(currentImageIndex) + filenameSuffix;
    
    if (fs.exists(file_path + "/" + currentFilename)) {
        // 显示当前照片
        File file = fs.open(file_path + "/" + currentFilename);
        unsigned char *file_data;
        uint32_t file_size;
        
        if (file.readFile(file_data, file_size)) {
            tft.clr();
            tft.setCursor(0, 0);
            
            TJpgDec.getJpgSize(0, 0, (const uint8_t *)file_data, file_size);
            TJpgDec.drawJpg(0, 0, (const uint8_t *)file_data, file_size);
            
            Serial.println("正在显示: " + currentFilename);
            free(file_data);
            
            // 增加索引，准备显示下一张照片
            currentImageIndex++;
        } else {
            Serial.println("读取文件失败: " + currentFilename);
            // 如果读取失败，也增加索引尝试下一张
            currentImageIndex++;
        }
        file.close();
    } else {
        // 如果当前索引的照片不存在，重置索引到0
        Serial.println("照片不存在，重置索引: " + currentFilename);
        currentImageIndex = 0;
        
        // 检查是否有照片文件
        String firstFilename = filenamePrefix + "0" + filenameSuffix;
        if (!fs.exists(file_path + "/" + firstFilename)) {
            Serial.println("SD卡中没有找到 Image_X.jpg 格式的照片");
        }
    }
    
    fs.end();
}
