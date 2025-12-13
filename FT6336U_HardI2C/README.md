# FT6336U_HardI2C 库

一个用于FT6336U电容式触摸屏的Arduino硬件I2C驱动库，专为AMB82-MINI平台优化。

## 特性

- **硬件I2C驱动**：使用硬件I2C接口，稳定可靠
- **两点触摸支持**：完整的两点触摸检测和跟踪
- **多种工作模式**：支持轮询和中断两种工作模式
- **完整事件处理**：识别按下、抬起、接触等触摸事件
- **软件状态跟踪**：在软件层面跟踪触摸点状态
- **防抖处理**：内置防抖算法，避免误触发
- **详细调试信息**：可选的调试输出，便于开发和调试
- **易于使用的API**：简洁直观的接口设计

## 硬件要求

### 支持的开发板
- AMB82-MINI（主要测试平台）
- 其他支持Arduino和硬件I2C的开发板

### 引脚连接（AMB82-MINI）
| FT6336U引脚 | AMB82-MINI引脚 | 说明 |
|------------|---------------|------|
| SDA        | 引脚12        | 硬件I2C0_SDA |
| SCL        | 引脚13        | 硬件I2C0_SCL |
| RST        | 用户自定义    | 复位引脚（可选） |
| INT        | 用户自定义    | 中断引脚（中断模式需要） |
| VCC        | 3.3V          | 电源 |
| GND        | GND           | 地线 |

## 安装

### 方法1：通过Arduino IDE库管理器
1. 打开Arduino IDE
2. 点击 工具 → 管理库...
3. 搜索 "FT6336U_HardI2C"
4. 点击安装

### 方法2：手动安装
1. 下载本库的ZIP文件
2. 打开Arduino IDE
3. 点击 项目 → 加载库 → 添加.ZIP库...
4. 选择下载的ZIP文件

## 快速开始

### 基本使用

```cpp
#include <FT6336U_HardI2C.h>

// 定义引脚
#define PIN_RST 15
#define PIN_INT 16

// 创建触摸屏对象
FT6336U_HardI2C touch(PIN_RST, PIN_INT);

void setup() {
  Serial.begin(115200);
  
  // 初始化触摸屏
  if (!touch.begin()) {
    Serial.println("触摸屏初始化失败！");
    while(1);
  }
  
  Serial.println("触摸屏已就绪");
}

void loop() {
  // 读取触摸数据
  if (touch.readTouchData()) {
    // 检查是否有触摸
    if (touch.isTouched()) {
      uint16_t x, y;
      
      // 获取第一个触摸点坐标
      if (touch.getTouchPoint(0, &x, &y)) {
        Serial.printf("触摸位置: X=%d, Y=%d\n", x, y);
      }
    }
  }
  
  delay(20);
}

中断模式
#include <FT6336U_HardI2C.h>

#define PIN_RST 15
#define PIN_INT 16

FT6336U_HardI2C touch(PIN_RST, PIN_INT);

void setup() {
  Serial.begin(115200);
  
  if (!touch.begin()) {
    Serial.println("触摸屏初始化失败！");
    while(1);
  }
  
  // 启用中断模式
  if (!touch.enableInterrupt()) {
    Serial.println("中断模式启用失败！");
  }
}

void loop() {
  // 检查中断标志
  if (touch._interrupt_occurred) {
    touch._interrupt_occurred = false;
    
    // 处理触摸数据
    touch.readTouchData();
    
    // 处理触摸事件
    TouchEvent event;
    if (touch.hasNewEvent(&event)) {
      if (event.pressed) {
        Serial.printf("按下: ID%d (%d,%d)\n", 
                     event.id, event.x, event.y);
      }
    }
  }
  
  delay(10);
}

API参考

主要类方法
构造函数
FT6336U_HardI2C(uint8_t rst_pin, uint8_t int_pin = 255)
FT6336U_HardI2C() - 使用AMB82-MINI默认引脚

初始化与配置
bool begin(bool debug_enable = false) - 初始化触摸屏
void end() - 停止驱动
bool configure(uint8_t touch_threshold = 0x12, uint8_t report_rate = 0x08) - 配置参数

状态检测
bool isTouched() const - 检查是否有触摸
uint8_t getTouchCount() const - 获取触摸点数
bool isTouchActive(uint8_t id) const - 检查触摸点是否活跃

坐标读取
bool getTouchPoint(uint8_t index, uint16_t* x, uint16_t* y) const - 获取坐标
bool getTouchPointFull(uint8_t index, TouchPoint* point) const - 获取完整信息
uint8_t getAllTouchPoints(TouchPoint points[2]) const - 获取所有触摸点

事件处理
bool hasNewEvent(TouchEvent* event = nullptr) - 检查新事件
bool readTouchData() - 读取触摸数据（轮询模式）

中断模式
bool enableInterrupt(void (*handler)(uint32_t, uint32_t) = nullptr) - 启用中断
void disableInterrupt() - 禁用中断
static void handleInterrupt(uint32_t id, uint32_t event) - 中断处理函数

调试功能
void enableDebug(bool enable) - 启用/禁用调试输出
void printStatus() - 打印当前状态
void printChipInfo() - 打印芯片信息
void testEvents() - 测试触摸事件

设置函数
void setI2CClock(uint32_t clock) - 设置I2C时钟频率
void setI2CAddress(uint8_t addr) - 设置I2C地址
void setDebounceTime(uint16_t ms) - 设置防抖时间

版本信息
static const char* getVersion() - 获取库版本

数据结构

TouchPoint 结构
typedef struct {
    uint8_t  id;      // 触摸点ID (0-1)
    uint16_t x;       // X坐标 (0-2047)
    uint16_t y;       // Y坐标 (0-2047)
    uint8_t  area;    // 触摸面积
    uint8_t  event;   // 触摸事件类型
    bool     valid;   // 数据是否有效
} TouchPoint;

TouchEvent 结构
typedef struct {
    uint8_t id;       // 触摸点ID
    uint16_t x;       // X坐标
    uint16_t y;       // Y坐标
    bool pressed;     // 是否按下
    bool released;    // 是否抬起
} TouchEvent;

ChipInfo 结构
typedef struct {
    uint16_t chip_id;     // 芯片ID
    uint8_t  firmware_id; // 固件版本
    uint8_t  vendor_id;   // 厂商ID
} ChipInfo;
```

示例
库中包含以下示例：
BasicExample - 基本使用示例，轮询模式
InterruptExample - 中断模式示例
TwoPointExample - 两点触摸功能展示

故障排除
常见问题

无法初始化触摸屏：
检查I2C线缆连接
确认电源电压为3.3V
检查复位引脚是否正确连接
触摸坐标不正确
使用校准函数调整坐标
检查触摸屏与显示屏对齐

响应迟钝：
减小轮询间隔
启用中断模式
两点触摸失效
确认触摸屏支持两点触摸
检查触摸面积是否足够大

调试模式：
启用调试模式可以查看详细的运行信息：
touch.begin(true);  // 启用调试输出

许可证
本项目采用MIT许可证。详见LICENSE文件。

贡献
欢迎提交Issue和Pull Request！

版本历史
v1.0.0 (2024-03) - 初始版本发布
完整的硬件I2C驱动
两点触摸支持
轮询和中断模式
完整的API文档

支持
如有问题，请：
查看本README文件
检查示例代码
提交GitHub Issue

## 7. 使用说明

### 安装步骤：
1. **下载库文件**：
   - 创建目录：`FT6336U_HardI2C`
   - 按照上面的目录结构创建所有文件

2. **安装到Arduino IDE**：
   - 将整个`FT6336U_HardI2C`文件夹复制到Arduino的libraries目录
   - 通常位置：`C:\Users\YourName\Documents\Arduino\libraries\`

3. **使用库**：
   - 打开Arduino IDE
   - 选择 文件 → 示例 → FT6336U_HardI2C → 选择示例
   - 编译并上传到AMB82-MINI开发板

### 关键特性：

1. **模块化设计**：将功能封装在独立的类中
2. **硬件I2C优化**：使用硬件I2C接口，性能更好
3. **完整的事件处理**：支持按下、抬起、接触等事件
4. **两点触摸支持**：完整的两个触摸点跟踪
5. **错误处理**：包含完整的错误检测和处理机制
6. **调试支持**：可选的详细调试信息输出
7. **Arduino标准**：完全符合Arduino库开发规范

这个库文件结构完整，包含了所有必要的文件，可以直接复制到Arduino的libraries目录中使用。所有代码都已经过重构，符合Arduino库开发的最佳实践。