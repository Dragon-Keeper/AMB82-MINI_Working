# ILI9488 8位并行接口驱动技术文档

## 概述
本文档详细记录了STM32F103C8T6微控制器通过8位并行接口驱动ILI9488 TFT液晶显示屏的技术实现细节。该驱动支持320×480分辨率、18位色彩深度，提供完整的图形绘制、文字显示和触摸控制功能。

## 硬件接口设计

### 引脚配置
| 信号线 | STM32引脚 | 功能描述 |
|--------|-----------|----------|
| DB0-DB7 | PB0-PB7 | 8位数据总线 |
| CS | PA12 | 片选信号 |
| RS | PA15 | 命令/数据选择 |
| WR | PC13 | 写信号 |
| RD | PC14 | 读信号 |
| RST | PC15 | 复位信号 |
| LED | PA2 | 背光控制 |

### 触摸控制接口
- **电阻触摸屏(XPT2046)**: TPEN=PA5(中断), TSDO=PA3(数据输出), TSDI=PA4(数据输入), TCLK=PA6(时钟), TCS=PA7(片选)
- **电容触摸屏(FT6336)**: FT_INT=PA5(中断), CT_IIC_SDA=PA4(I2C数据), CT_IIC_SCL=PA6(I2C时钟), FT_RST=PA7(复位)

## 显示屏参数

### 基本参数
- **分辨率**: 320 × 480像素
- **色彩深度**: 18位(262K色)
- **显示方向**: 支持0°、90°、180°、270°旋转
- **驱动芯片**: ILI9488

### 颜色定义
```c
#define WHITE            0xFFFF
#define BLACK            0x0000
#define BLUE             0x001F
#define BRED             0XF81F
#define GRED             0XFFE0
#define GBLUE            0X07FF
#define RED              0xF800
#define MAGENTA          0xF81F
#define GREEN            0x07E0
#define CYAN             0x7FFF
#define YELLOW           0xFFE0
#define BROWN            0XBC40
#define BRRED            0XFC07
#define GRAY             0X8430
#define DARKBLUE         0X01CF
#define LIGHTBLUE        0X7D7C
#define GRAYBLUE         0X5458
#define LIGHTGREEN       0X841F
#define LGRAY            0XC618
#define LGRAYBLUE        0XA651
#define LBBLUE           0X2B12
```

## 初始化序列

### GPIO初始化
配置PA2、PA12、PA15、PC13-15、PB0-15引脚为推挽输出模式，设置初始电平。

### ILI9488初始化指令序列
```c
// 关键寄存器配置
LCD_WR_REG(0xF7); LCD_WR_DATA(0xA9); LCD_WR_DATA(0x51); LCD_WR_DATA(0x2C); LCD_WR_DATA(0x82);
LCD_WR_REG(0xC0); LCD_WR_DATA(0x11); LCD_WR_DATA(0x09);
LCD_WR_REG(0xC1); LCD_WR_DATA(0x41);
LCD_WR_REG(0xC5); LCD_WR_DATA(0x00); LCD_WR_DATA(0x00); LCD_WR_DATA(0x00); LCD_WR_DATA(0x00);
LCD_WR_REG(0xB1); LCD_WR_DATA(0xB0); LCD_WR_DATA(0x11);
LCD_WR_REG(0xB4); LCD_WR_DATA(0x02);
LCD_WR_REG(0xB6); LCD_WR_DATA(0x02); LCD_WR_DATA(0x02); LCD_WR_DATA(0x3B);
LCD_WR_REG(0xB7); LCD_WR_DATA(0xC6);
LCD_WR_REG(0x3A); LCD_WR_DATA(0x55);  // 设置16位色彩模式
LCD_WR_REG(0x21);                     // 显示反转
LCD_WR_REG(0x11);                     // 退出睡眠模式
delay_ms(120);
LCD_WR_REG(0x29);                     // 开启显示
```

## 数据传输协议

### 8位并行通信机制
- **命令写入**: RS=0时写入命令寄存器地址
- **数据写入**: RS=1时写入显示数据
- **16位数据传输**: 分两次传输(先高8位，后低8位)

### 核心通信函数
```c
void LCD_WR_DATA(u16 data) {
    LCD_RS_SET;
    LCD_CS_CLR;
    DATAOUT(data>>8);    // 发送高8位
    LCD_WR_CLR;
    LCD_WR_SET;
    DATAOUT(data);       // 发送低8位
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET;
}

void LCD_WR_REG(u8 reg) {
    LCD_RS_CLR;
    LCD_CS_CLR;
    DATAOUT(reg);
    LCD_WR_CLR;
    LCD_WR_SET;
    LCD_CS_SET;
}
```

## 图形绘制函数

### 基本绘图功能
- **清屏**: `LCD_Clear(u16 color)` - 用指定颜色填充整个屏幕
- **区域填充**: `LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color)`
- **画点**: `LCD_DrawPoint(u16 x, u16 y)`
- **读点**: `LCD_ReadPoint(u16 x, u16 y)` - 读取指定位置颜色值

### 几何图形绘制
- **画线**: `LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)` - 使用Bresenham算法
- **画矩形**: `LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)`
- **画圆**: `LCD_Draw_Circle(u16 x0, u16 y0, u16 r)`
- **填充圆**: `gui_fill_circle(u16 x0, u16 y1, u16 r, u16 color)`

## 文字显示系统

### 字体支持
- **ASCII字体**: 6×12、8×16、12×24点阵
- **中文字体**: 16×16、24×24、32×32点阵
- **字体数据**: 存储在FONT.H文件中，包含完整的字模数组

### 文字显示函数
- **字符显示**: `LCD_ShowChar(u16 x, u16 y, u8 num, u8 size, u8 mode)`
- **数字显示**: `LCD_ShowNum(u16 x, u16 y, u32 num, u8 len, u8 size)`
- **字符串显示**: `LCD_ShowString(u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p)`
- **中英文混合**: `Show_Str(u16 x, u16 y, u8 *str, u8 size, u8 mode)`

## 屏幕控制命令

### 显示控制
- **开启显示**: `LCD_DisplayOn()` - 发送0x29命令
- **关闭显示**: `LCD_DisplayOff()` - 发送0x28命令
- **设置方向**: `LCD_Display_Dir(u8 dir)` - 0:0°, 1:180°, 2:270°, 3:90°

### 窗口设置
- **设置窗口**: `LCD_Set_Window(u16 xsta, u16 ysta, u16 xend, u16 yend)`
- **准备写GRAM**: `LCD_WriteRAM_Prepare()`
- **写GRAM数据**: `LCD_WriteRAM(u16 RGB_Code)`

## 设备识别与诊断

### 设备ID读取
```c
u16 LCD_Read_ID(u8 id) {
    u16 idval = 0;
    LCD_WR_REG(id);
    idval = LCD_RD_DATA();
    return idval;
}
```

### 诊断测试函数
- **颜色测试**: `Color_Test()` - 显示各种颜色条带
- **图形测试**: `FillRec_Test()` - 绘制几何图形
- **字体测试**: `English_Font_test()`, `Chinese_Font_test()`
- **旋转测试**: `Rotate_Test()` - 测试4个方向的显示
- **触摸测试**: `rtp_test()`, `ctp_test()` - 电阻/电容触摸测试

## 应用示例

### 主程序流程
```c
int main(void) {
    Stm32_Clock_Init(9);        // 系统时钟初始化
    uart_init(72,9600);         // 串口初始化
    delay_init(72);             // 延时初始化
    LCD_Init();                 // LCD初始化
    tp_dev.touchtype |= USE_TP_TYPE;  // 触摸类型设置
    LCD_Display_Dir(USE_LCM_DIR);     // 显示方向设置
    LCD_Clear(WHITE);           // 清屏
    LCD_LED = 1;                // 开启背光
    
    // 执行各项测试
    main_test("IC:ILI9488");
    Color_Test();
    Read_Test();
    FillRec_Test();
    English_Font_test();
    Chinese_Font_test();
    Pic_test();
    Switch_test();
    Rotate_Test();
    
    // 触摸测试
    LCD_Display_Dir(0);
    tp_dev.init();
    if(tp_dev.touchtype & 0X80) ctp_test();  // 电容触摸
    else rtp_test();                         // 电阻触摸
}
```

## 性能优化

### 数据传输优化
- 使用硬件GPIO直接操作，避免软件延时
- 批量数据传输时使用窗口设置减少命令开销
- 采用DMA传输优化大数据量操作

### 内存管理
- 显存操作直接映射到GRAM，减少中间缓冲区
- 字体数据采用常量数组存储，节省RAM空间

## 错误处理与调试

### 常见问题排查
1. **显示异常**: 检查初始化序列和时序参数
2. **触摸不响应**: 验证触摸芯片通信和中断配置
3. **颜色失真**: 确认色彩模式设置(0x3A寄存器)
4. **ID读取失败**: 检查硬件连接和通信协议

### 调试支持
- 通过串口输出调试信息
- 提供完整的测试函数验证各项功能
- 支持显示ID和颜色值读取诊断

## 总结
该8位并行接口驱动为ILI9488显示屏提供了完整的功能支持，包括基本的图形绘制、文字显示、触摸控制等。驱动设计合理，代码结构清晰，便于二次开发和功能扩展。通过优化的通信协议和丰富的测试函数，确保了驱动的稳定性和可靠性。