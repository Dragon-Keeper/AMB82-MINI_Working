# FT6336U 电容触摸屏驱动库

一个专为 AMB82-MINI 开发板设计的 FT6336U 电容触摸屏驱动库，支持 GPIO 模拟 I2C 通信和多点触摸功能。

## 特性

- ✅ **GPIO 模拟 I2C** - 无需硬件 I2C，使用任意 GPIO 引脚
- ✅ **多点触摸支持** - 支持最多 2 点同时触摸
- ✅ **中断模式** - 支持中断驱动，降低 CPU 占用率
- ✅ **轮询模式** - 兼容传统轮询方式
- ✅ **事件回调** - 提供触摸按下/抬起事件回调
- ✅ **AMB82-MINI 优化** - 专门适配 AMB82-MINI 的中断机制
- ✅ **调试信息** - 可配置的调试输出功能

## 硬件要求

### 开发板
- AMB82-MINI 开发板
- 兼容其他 Arduino 兼容开发板

### 触摸屏
- FT6336U 电容触摸屏控制器
- 支持 I2C 通信接口

### 引脚连接
| 触摸屏引脚 | AMB82-MINI 引脚 | 说明 |
|-----------|----------------|------|
| SDA       | GPIO12         | 数据线 |
| SCL       | GPIO13         | 时钟线 |
| RST       | GPIO15         | 复位线 |
| INT       | GPIO16         | 中断线 |

## 安装

### Arduino IDE 安装
1. 下载本库的 ZIP 文件
2. 在 Arduino IDE 中选择：`项目` → `加载库` → `添加 .ZIP 库`
3. 选择下载的 ZIP 文件

### 手动安装
1. 将 `src` 文件夹复制到 Arduino 库目录：
   - Windows: `Documents\Arduino\libraries\FT6336U_Touch`
   - Linux/Mac: `~/Arduino/libraries/FT6336U_Touch`

## 快速开始

### 基本用法

```cpp
#include <FT6336U_SoftI2C.h>

// 创建触摸屏对象（使用默认引脚）
FT6336U_SoftI2C touch;

void setup() {
    Serial.begin(115200);
    
    // 初始化触摸屏
    touch.begin(true);  // 启用调试信息
    
    // 读取芯片信息
    touch.readChipInfo();
    
    // 配置触摸参数
    touch.configure(0x12);
}

void loop() {
    // 更新触摸数据
    touch.update();
    
    // 检查触摸状态
    if (touch.isTouched()) {
        Serial.print("触摸点数: ");
        Serial.println(touch.getTouchCount());
        
        // 获取触摸点坐标
        uint16_t x, y;
        if (touch.getTouchPoint(0, &x, &y)) {
            Serial.print("坐标: X=");
            Serial.print(x);
            Serial.print(", Y=");
            Serial.println(y);
        }
    }
    
    delay(100);
}
```

### 使用事件回调

```cpp
#include <FT6336U_SoftI2C.h>

FT6336U_SoftI2C touch;

// 触摸事件回调函数
void handleTouchEvent(TouchEvent event) {
    if (event.pressed) {
        Serial.print("[按下] ID:");
        Serial.print(event.id);
        Serial.print(" (");
        Serial.print(event.x);
        Serial.print(",");
        Serial.print(event.y);
        Serial.println(")");
    } else if (event.released) {
        Serial.print("[抬起] ID:");
        Serial.println(event.id);
    }
}

void setup() {
    Serial.begin(115200);
    touch.begin(true);
    
    // 设置触摸事件回调
    touch.setTouchCallback(handleTouchEvent);
}

void loop() {
    touch.update();
    delay(10);
}
```

## API 参考

### 构造函数

- `FT6336U_SoftI2C()` - 使用默认引脚 (SDA=12, SCL=13, RST=15, INT=16)
- `FT6336U_SoftI2C(sda_pin, scl_pin, rst_pin, int_pin)` - 自定义引脚

### 主要方法

- `begin(debug)` - 初始化触摸屏
- `update()` - 更新触摸数据
- `getTouchCount()` - 获取当前触摸点数
- `getTouchPoint(index, x, y)` - 获取指定触摸点的坐标
- `isTouched()` - 检查是否有触摸
- `setTouchCallback(callback)` - 设置触摸事件回调
- `readChipInfo()` - 读取芯片信息
- `configure(threshold)` - 配置触摸参数

### 配置选项

在包含头文件前定义以下宏来配置库行为：

```cpp
#define USE_INTERRUPT_MODE   1     // 0=轮询模式，1=中断模式
#define ENABLE_DEBUG_INFO    1     // 启用调试信息
#define MAX_TOUCH_POINTS     2     // 最大触摸点数
```

## 示例程序

### 基础示例 (`Examples/BasicDemo/`)
- 简单的触摸检测和坐标输出
- 适合初学者快速上手

### 完整示例 (`Examples/FT6336U/`)
- 包含中断模式配置
- 事件回调功能演示
- 完整的调试信息输出

## 中断模式配置

### AMB82-MINI 专用中断

库已针对 AMB82-MINI 开发板优化中断处理：

```cpp
// 在 setup() 函数中配置中断
#if USE_INTERRUPT_MODE
    pinMode(16, INPUT_IRQ_FALL);
    digitalSetIrqHandler(16, FT6336U_SoftI2C::amebaInterruptHandler);
#endif
```

### 中断工作流程

1. 触摸屏产生中断信号
2. `amebaInterruptHandler` 被调用
3. 设置中断标志位
4. `update()` 函数检测到中断标志
5. 读取触摸数据并处理事件

## 故障排除

### 常见问题

1. **编译错误：找不到 FT6336U 库**
   - 检查库是否正确安装
   - 确认 Arduino IDE 已重启

2. **触摸无响应**
   - 检查引脚连接是否正确
   - 确认触摸屏电源正常
   - 检查 I2C 地址是否匹配 (0x38)

3. **中断模式不工作**
   - 确认 `USE_INTERRUPT_MODE` 设置为 1
   - 检查 INT 引脚连接
   - 验证中断引脚配置

### 调试技巧

启用调试信息查看详细运行状态：

```cpp
touch.begin(true);  // 启用调试信息
```

## 技术规格

- **I2C 地址**: 0x38 (7位地址)
- **触摸点数**: 最多 2 点
- **分辨率**: 12位 (0-2047)
- **通信方式**: GPIO 模拟 I2C
- **工作模式**: 轮询/中断可选

## 版本历史

- **v1.0.0** (当前版本)
  - 初始版本发布
  - 支持 GPIO 模拟 I2C
  - 支持多点触摸
  - 适配 AMB82-MINI 中断

## 许可证

本项目采用 MIT 许可证。详见 [LICENSE](LICENSE) 文件。

## 贡献

欢迎提交 Issue 和 Pull Request 来改进这个库。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 项目仓库: [GitHub Repository](https://github.com/yourusername/FT6336U_Touch)
- 邮箱: your.email@example.com

---

**注意**: 本库专为 AMB82-MINI 开发板优化，在其他平台上可能需要调整引脚配置和中断处理机制。