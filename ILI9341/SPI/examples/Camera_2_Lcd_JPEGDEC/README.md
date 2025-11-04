# AMB82 Mini Camera to LCD JPEG Decoder

This project demonstrates how to display camera video stream on an ILI9341 TFT LCD using the AMB82 Mini development board. It uses JPEG decoding and hardware parallel interface for optimized performance.

## Features

- Real-time camera video streaming to ILI9341 TFT LCD
- Optimized JPEG decoding with hardware parallel 8-bit interface
- High frame rate performance (target 30 FPS)
- Real-time FPS display in the top-left corner
- Full-screen display with 320x240 resolution
- Proper aspect ratio without stretching or deformation

## Hardware Requirements

- AMB82 Mini development board
- ILI9341 TFT LCD (320x240 resolution)
- Camera module compatible with AMB82 Mini

## Pin Connections

The following pin connections are used for the parallel interface:

- Data pins (D0-D7): 8, 9, 2, 3, 10, 11, 13, 14
- Control pins:
  - TFT_CS: 12
  - TFT_DC: 4
  - TFT_RESET: 5
  - TFT_WR: 6
  - TFT_RD: 7

## Optimizations

- Uses hardware parallel 8-bit interface for faster data transfer
- Optimized JPEG decoding with increased iMaxMCUs value
- Improved frame rate control with dynamic delay adjustment
- Image caching mechanism to reduce redundant decoding
- Enhanced character rendering with 5x7 font for FPS display

## Usage

1. Connect the hardware as per the pin connections
2. Upload the sketch to the AMB82 Mini board
3. Open the Serial Monitor to view performance information
4. The camera feed will be displayed on the LCD with real-time FPS counter

## Performance

The optimized version achieves up to 30 FPS with proper image quality. The FPS counter in the top-left corner provides real-time performance feedback.

## Common Compilation Errors and Solutions

### 1. 'dtostrf' was not declared in this scope
**Cause:** Arduino环境下dtostrf函数可能不可用或需要特定配置
**Solution:** 使用Arduino String类替代dtostrf函数进行浮点数到字符串的转换

### 2. 显示内容分辨率过低问题

**问题描述：**
显示的JPEG图像分辨率过低，无法全屏显示，图像较小且有明显缩放痕迹。

**问题原因：**
1. 代码中硬编码了JPEG图像尺寸(640x480)，而非从实际JPEG文件中获取
2. 使用固定的JPEG_SCALE_QUARTER缩放参数，强制将图像缩小到1/4大小
3. 缺乏自适应屏幕尺寸的图像缩放算法

**解决方案：**
1. 使用jpeg.getWidth()和jpeg.getHeight()动态获取JPEG图像实际尺寸
2. 根据屏幕尺寸和图像尺寸计算最优缩放比例
3. 根据计算结果选择合适的JPEG解码缩放选项(JPEG_SCALE_HALF/QUARTER/EIGHTH)
4. 计算图像居中显示位置，确保图像在屏幕中居中显示