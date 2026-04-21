# 第二层：项目级RULE.md

> **层级定位**：项目「宪法」，定义项目特定的规则和约定
> **加载范围**：对应项目文件夹及其子目录
> **核心作用**：约定目录结构、命名规范、引脚分配、硬件配置

---

## 一、定义与定位

项目级RULE.md是AI Agent规则体系的第二层级，相当于项目的「宪法」。它位于项目根目录下，对该项目及其所有子目录具有约束力。项目级RULE.md在全局RULE.md的基础上，定义项目特定的规则和约定，是项目开发的核心指导文档。

### 1.1 核心特征

| 特征 | 说明 |
|------|------|
| **项目特定** | 针对具体项目定义规则，不同项目可有不同规则 |
| **继承全局** | 继承全局RULE.md的规则，可在此基础上细化 |
| **硬件约束** | 定义开发板型号、引脚分配、硬件连接 |
| **库管理** | 定义项目依赖的库及版本 |

### 1.2 文件位置

```
project-root/
├── RULE.md              # 项目级配置文件
├── project.ino          # Arduino主程序文件
├── src/                 # 源代码目录
├── lib/                 # 自定义库目录
├── docs/                # 文档目录
└── hardware/            # 硬件相关文档
```

### 1.3 与全局RULE.md的关系

| 关系 | 说明 |
|------|------|
| **继承** | 继承全局RULE.md的所有规则 |
| **细化** | 在全局规则基础上进行项目特定的细化 |
| **补充** | 补充项目特定的规则和约定 |
| **覆盖** | 在明确说明的情况下，可覆盖全局规则 |

---

## 二、核心内容框架

### 2.1 项目概述

项目概述提供项目的基本信息，帮助AI Agent理解项目背景和目标。

```markdown
## 项目概述

- 项目名称：[项目名称]
- 项目描述：[项目简介]
- 目标开发板：[ESP32/ESP8266/BW21-CBV-Kit]
- 开发环境：Trae IDE + Arduino框架
- 当前状态：[开发阶段：规划/开发/测试/完成]
```

**示例**：

```markdown
## 项目概述

- 项目名称：智能温控器
- 项目描述：基于ESP32的智能温度控制系统，支持OLED显示、旋转编码器调节、RTC定时
- 目标开发板：ESP32-DevKitC
- 开发环境：Trae IDE + Arduino框架
- 当前状态：开发阶段
```

### 2.2 硬件配置

硬件配置是Arduino项目的核心内容，必须详细记录。

#### 2.2.1 开发板信息

```markdown
## 开发板信息

| 项目 | 内容 |
|------|------|
| 开发板型号 | ESP32-DevKitC / ESP8266 NodeMCU / BW21-CBV-Kit |
| CPU频率 | 240MHz / 160MHz / ... |
| Flash大小 | 4MB / 16MB / ... |
| RAM大小 | 520KB / 80KB / ... |
| 工作电压 | 3.3V |
| 特殊功能 | WiFi / Bluetooth / ... |
```

#### 2.2.2 引脚分配表

引脚分配表是项目最重要的参考文档，必须详细记录每个引脚的用途。

```markdown
## 引脚分配表

### ESP32引脚分配

| GPIO | 功能 | 连接设备 | 说明 |
|------|------|----------|------|
| 4 | I2C_SDA | OLED/RTC | I2C数据线 |
| 5 | I2C_SCL | OLED/RTC | I2C时钟线 |
| 18 | OUTPUT | 旋转编码器CLK | 编码器时钟信号 |
| 19 | INPUT | 旋转编码器DT | 编码器数据信号 |
| 21 | INPUT | 旋转编码器SW | 编码器按键 |
| 22 | OUTPUT | LED指示灯 | 状态指示 |
| 23 | OUTPUT | 继电器控制 | 加热器控制 |
| 34 | INPUT | 温度传感器 | DS18B20数据线 |
| 35 | INPUT | 按键 | 用户按键 |

### 特殊引脚说明

- GPIO0: 启动模式选择，正常运行时悬空或接高电平
- GPIO2: 启动时输出，避免连接下拉设备
- GPIO12: 启动时影响Flash电压，谨慎使用
- GPIO34-39: 仅输入模式，无上拉电阻
```

#### 2.2.3 硬件清单

```markdown
## 硬件清单

| 序号 | 名称 | 型号 | 数量 | 说明 |
|------|------|------|------|------|
| 1 | 开发板 | ESP32-DevKitC | 1 | 主控制器 |
| 2 | OLED屏幕 | SSD1306 0.96寸 | 1 | I2C接口 |
| 3 | RTC模块 | DS3231 | 1 | 高精度时钟 |
| 4 | 旋转编码器 | KY-040 | 1 | 用户输入 |
| 5 | 温度传感器 | DS18B20 | 1 | 单总线接口 |
| 6 | 继电器模块 | 5V继电器 | 1 | 控制加热器 |
```

### 2.3 目录结构规范

Arduino项目的目录结构应该清晰明了，便于管理。

```
project-root/
├── RULE.md                    # 项目规则文档
├── project.ino                # Arduino主程序（与项目文件夹同名）
├── src/                       # 源代码目录
│   ├── config.h              # 配置文件（引脚定义、常量）
│   ├── display.h/.cpp        # 显示模块
│   ├── encoder.h/.cpp        # 编码器模块
│   ├── rtc.h/.cpp            # RTC模块
│   └── utils.h/.cpp          # 工具函数
├── lib/                       # 自定义库目录
│   └── MyLibrary/            # 自定义库
├── docs/                      # 文档目录
│   ├── wiring.md             # 接线说明
│   └── notes.md              # 开发笔记
├── hardware/                  # 硬件相关
│   ├── schematic.png         # 电路图
│   └── pinout.md             # 引脚说明
└── Memory.md                  # 项目记忆文件
```

### 2.4 命名规范

#### 2.4.1 文件命名

| 类型 | 规范 | 示例 |
|------|------|------|
| 主程序 | 项目名.ino | smart_thermostat.ino |
| 头文件 | 模块名.h | display.h |
| 源文件 | 模块名.cpp | display.cpp |
| 配置文件 | config.h | config.h |

#### 2.4.2 变量命名

```cpp
// 引脚定义 - 使用大写+下划线
#define PIN_SDA 4
#define PIN_SCL 5
const int ENCODER_CLK = 18;

// 全局变量 - 使用小写+下划线，加g_前缀
float g_current_temp = 0.0;
int g_target_temp = 25;

// 局部变量 - 使用小写+下划线
int encoder_count = 0;
bool button_pressed = false;

// 函数名 - 使用小写+下划线
void update_display();
void read_temperature();
void handle_encoder();

// 类名 - 使用大驼峰
class TemperatureSensor {
  // ...
};

// 常量 - 使用大写+下划线
#define MAX_TEMPERATURE 100
const float TEMP_OFFSET = 0.5;
```

#### 2.4.3 注释规范

```cpp
/**
 * @brief 初始化OLED显示屏
 * @param sda_pin I2C数据线引脚
 * @param scl_pin I2C时钟线引脚
 * @return true 初始化成功
 * @return false 初始化失败
 */
bool init_display(int sda_pin, int scl_pin) {
  // 初始化I2C通信
  Wire.begin(sda_pin, scl_pin);
  
  // 初始化OLED显示屏
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED初始化失败！");
    return false;
  }
  
  // 清屏并显示启动画面
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("System Starting...");
  display.display();
  
  return true;
}
```

### 2.5 库依赖管理

#### 2.5.1 库列表

```markdown
## 库依赖

| 库名称 | 版本 | 用途 | 安装方式 |
|--------|------|------|----------|
| Adafruit_GFX | 1.11.5 | 图形库 | 库管理器 |
| Adafruit_SSD1306 | 2.5.7 | OLED驱动 | 库管理器 |
| RTClib | 2.1.1 | RTC库 | 库管理器 |
| OneWire | 2.3.8 | 单总线 | 库管理器 |
| DallasTemperature | 3.11.0 | DS18B20 | 库管理器 |
| ESP32Encoder | 0.11.5 | 旋转编码器 | 库管理器 |
```

#### 2.5.2 库安装说明

```markdown
## 库安装方法

### 方法一：Arduino库管理器（推荐）
1. 打开Arduino IDE
2. 菜单：工具 -> 管理库
3. 搜索库名称
4. 点击安装

### 方法二：手动安装
1. 下载库的ZIP文件
2. 菜单：项目 -> 加载库 -> 添加.ZIP库
3. 选择下载的ZIP文件

### 方法三：从GitHub安装
1. 克隆或下载库到 ~/Arduino/libraries/
2. 重启Arduino IDE
```

### 2.6 开发流程规范

```markdown
## 开发流程

1. **需求分析**
   - 明确功能需求
   - 确定硬件清单
   - 规划引脚分配

2. **环境准备**
   - 安装开发板支持包
   - 安装所需库
   - 配置开发环境

3. **硬件连接**
   - 按照接线图连接
   - 检查电源和地线
   - 确认无短路

4. **代码开发**
   - 先写基础框架
   - 逐模块添加功能
   - 每步都测试验证

5. **调试测试**
   - 使用Serial输出调试信息
   - 验证每个功能模块
   - 记录问题和解决方案

6. **优化完善**
   - 优化代码结构
   - 减少资源占用
   - 添加错误处理
```

---

## 三、最佳实践示例

以下是一个完整的项目级RULE.md示例：

```markdown
# 项目级RULE.md - 智能温控器

## 项目概述

- 项目名称：智能温控器
- 项目描述：基于ESP32的智能温度控制系统
- 目标开发板：ESP32-DevKitC (ESP32-WROOM-32)
- 开发环境：Trae IDE + Arduino框架
- 当前状态：开发阶段

## 开发板信息

| 项目 | 内容 |
|------|------|
| 开发板型号 | ESP32-DevKitC |
| CPU频率 | 240MHz |
| Flash大小 | 4MB |
| RAM大小 | 520KB |
| 工作电压 | 3.3V |

## 引脚分配表

| GPIO | 功能 | 连接设备 | 说明 |
|------|------|----------|------|
| 4 | I2C_SDA | OLED/RTC | I2C数据线 |
| 5 | I2C_SCL | OLED/RTC | I2C时钟线 |
| 18 | INPUT | 编码器CLK | 旋转编码器 |
| 19 | INPUT | 编码器DT | 旋转编码器 |
| 21 | INPUT | 编码器SW | 编码器按键 |
| 22 | OUTPUT | LED | 状态指示灯 |
| 23 | OUTPUT | RELAY | 继电器控制 |
| 34 | INPUT | DS18B20 | 温度传感器 |

## 硬件清单

| 名称 | 型号 | 数量 |
|------|------|------|
| 开发板 | ESP32-DevKitC | 1 |
| OLED | SSD1306 0.96寸 | 1 |
| RTC | DS3231 | 1 |
| 编码器 | KY-040 | 1 |
| 温度传感器 | DS18B20 | 1 |
| 继电器 | 5V模块 | 1 |

## 库依赖

| 库名称 | 版本 | 用途 |
|--------|------|------|
| Adafruit_GFX | 1.11.5 | 图形库 |
| Adafruit_SSD1306 | 2.5.7 | OLED驱动 |
| RTClib | 2.1.1 | RTC库 |
| DallasTemperature | 3.11.0 | DS18B20 |

## 命名规范

- 引脚定义：PIN_前缀，大写
- 全局变量：g_前缀，小写+下划线
- 函数名：小写+下划线
- 类名：大驼峰

## 开发流程

1. 先完成基础框架（setup/loop）
2. 逐模块添加功能
3. 每步测试验证
4. 记录问题和解决方案
```

---

## 四、与全局RULE.md的协作

### 4.1 规则继承机制

项目级RULE.md自动继承全局RULE.md的所有规则，无需重复定义。继承的规则包括：

- 用户画像
- 第一性原理
- 约束先行原则
- 交互设计原则
- 工作方式规范
- 开发习惯规范

### 4.2 规则细化机制

项目级RULE.md可以在全局规则基础上进行细化：

| 全局规则 | 项目级细化 |
|----------|------------|
| 代码添加注释 | 具体注释格式和内容要求 |
| 提供接线说明 | 具体的引脚分配表 |
| 说明库安装方法 | 具体的库名称和版本 |

---

## 五、维护与更新

### 5.1 更新时机

- 硬件变更时（更换开发板、添加模块）
- 引脚重新分配时
- 库版本更新时
- 功能需求变更时

### 5.2 更新原则

- 先更新RULE.md，再更新代码
- 保持引脚分配表与实际一致
- 记录变更原因和日期

---

## 六、常见问题

### Q1: 项目级RULE.md应该放在哪里？

**A**: 放在项目根目录下，与.ino主程序文件同级。

### Q2: 引脚分配冲突怎么办？

**A**: 
1. 检查开发板的特殊引脚限制
2. 优先使用通用GPIO
3. 避免使用启动时影响运行的引脚

### Q3: 如何管理多个开发板的项目？

**A**: 
1. 使用条件编译切换不同开发板
2. 在RULE.md中记录不同开发板的配置
3. 使用不同的头文件管理引脚定义

### Q4: 库版本冲突怎么办？

**A**: 
1. 在RULE.md中记录已验证的库版本
2. 使用库管理器安装指定版本
3. 必要时修改库源码适配

---

## 七、参考资料

- [ESP32引脚参考](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [ESP8266引脚参考](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/)
- [Arduino编程参考](https://www.arduino.cc/reference/en/)
- [BW21-CBV-Kit文档](https://www.amebaiot.com/)
