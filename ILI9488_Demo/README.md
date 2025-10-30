# ILI9488显示屏驱动项目

## 项目介绍

这是一个基于AMB82-MINI开发板的ILI9488 TFT显示屏驱动项目，实现了18位RGB666颜色模式的显示功能，支持基本图形绘制、文本显示和颜色测试等功能。

## 硬件连接

### 引脚定义

| 显示屏引脚 | AMB82-MINI引脚 | 描述 |
|---------|--------------|------|
| CS      | 12 (SPI2_CS)  | 片选信号 |
| DC      | 16 (PD_18)    | 数据/命令控制 |
| RST     | 4 (IOF11)     | 复位信号 |
| SCK     | 15 (SPI2_SCL) | 时钟信号 |
| MOSI    | 13 (SPI2_MOSI)| 数据输出 |
| LED     | 11 (GPIOE_0)  | 背光控制 |
| VCC     | 3.3V          | 电源 |
| GND     | GND           | 接地 |

> 注意：显示屏使用软件SPI方式通信，不需要MISO引脚

## 项目结构

```
ILI9488_Demo/
├── AmebaILI9488.cpp    # 驱动实现文件
├── AmebaILI9488.h      # 驱动头文件和颜色定义
├── ILI9488_Demo.ino    # 示例程序
├── font5x7.h           # 5x7字体数据
└── README.md           # 本说明文件
```

## 主要功能

1. **基本图形绘制**
   - 点、线、矩形、圆形绘制
   - 填充矩形、填充圆形
   - 水平和垂直线快速绘制

2. **文本显示**
   - 字符和字符串显示
   - 可调整字体大小
   - 可设置前景色和背景色

3. **屏幕控制**
   - 屏幕旋转控制 (0°、90°、180°、270°)
   - 显示方向控制
   - 屏幕滚动功能

4. **颜色模式**
   - 18位RGB666颜色模式
   - 内置常用颜色定义
   - RGB888到RGB666颜色转换

## 驱动特点

### 颜色处理

1. **颜色定义**
   - 使用18位RGB666模式，每个颜色通道6位
   - 颜色值范围：0x000000 ~ 0xFCFCFC
   - 内置常用颜色：黑色、白色、红色、绿色、蓝色、黄色、青色、洋红色

2. **颜色修正**
   - 提供`fixColor()`函数处理颜色映射
   - 支持RGB888到RGB666的转换
   - 通过MADCTL寄存器配置确保正确的颜色通道顺序

### 初始化和配置

1. **初始化流程**
   - 硬件复位
   - 寄存器配置
   - 显示方向设置
   - 颜色模式配置

2. **关键设置**
   - 使用BGR颜色顺序以确保正确的颜色显示
   - 禁用显示反转以避免颜色显示相反
   - 配置像素格式为18位RGB666模式

## 使用方法

### 基本初始化

```cpp
#include "AmebaILI9488.h"

// 创建显示屏对象
AmebaILI9488 tft = AmebaILI9488(TFT_CS, TFT_DC, TFT_RST, TFT_SCK, TFT_MOSI, TFT_LED);

void setup() {
  // 初始化显示屏
  tft.begin();
  
  // 设置显示方向
  tft.setRotation(0);  // 0-3: 不同的旋转角度
  
  // 清除屏幕
  tft.fillScreen(fixColor(ILI9488_BLACK));
}
```

### 颜色使用

```cpp
// 使用预定义颜色
uint32_t red = fixColor(ILI9488_RED);
uint32_t green = fixColor(ILI9488_GREEN);
uint32_t blue = fixColor(ILI9488_BLUE);

// 自定义颜色 (RGB888转RGB666)
uint32_t customColor = rgb888_to_rgb666(100, 150, 200);
customColor = fixColor(customColor);
```

### 图形绘制

```cpp
// 绘制像素点
tft.drawPixel(100, 100, red);

// 绘制线段
tft.drawLine(0, 0, 320, 240, blue);

// 绘制矩形
tft.drawRect(50, 50, 200, 150, green);

// 填充矩形
tft.fillRect(60, 60, 180, 130, blue);

// 绘制圆形
tft.drawCircle(160, 120, 50, yellow);

// 填充圆形
tft.fillCircle(160, 120, 40, red);
```

### 文本显示

```cpp
// 设置文本属性
tft.setCursor(20, 20);
tft.setFontSize(2);
tft.setForeground(white);
tft.setBackground(black);

// 显示文本
tft.print("Hello, World!");
tft.println("This is ILI9488 Demo");
```

## 注意事项

1. **颜色显示**
   - 由于硬件特性，本驱动已经调整了颜色定义以确保正确显示
   - 使用`fixColor()`函数处理所有颜色值

2. **内存使用**
   - 软件SPI实现可能比硬件SPI慢，但具有更好的兼容性
   - 文本绘制和图形绘制会占用一定的内存

3. **性能优化**
   - 填充大区域时使用`fillRect()`而不是逐像素填充
   - 连续绘制时减少设置操作以提高速度

## 故障排除

### 颜色显示问题

1. **颜色反转**
   - 确保MADCTL寄存器配置正确（BGR颜色顺序）
   - 检查INVOFF命令是否已在初始化中执行

2. **颜色通道错位**
   - 确保所有颜色值都通过`fixColor()`函数处理
   - 检查RGB666颜色格式是否正确

### 显示方向问题

1. **屏幕旋转**
   - 使用`setRotation()`函数调整显示方向
   - 旋转角度应在0-3范围内

2. **图像颠倒或镜像**
   - 检查MADCTL寄存器中的MX、MY、MV位设置

## 版本历史

- **v1.0**
  - 初始版本
  - 基本图形和文本功能
  - RGB666颜色模式支持

## 许可证

本项目基于MIT许可证开源。