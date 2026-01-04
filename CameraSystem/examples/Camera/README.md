# AMB82-MINI 相机应用项目（模块化重构版）

这是一个基于AMB82-MINI开发板的综合应用项目，经过模块化重构后，项目采用模块化架构设计，集成了图形菜单系统和相机功能。项目使用EC11旋转编码器进行控制，通过TFT显示屏显示菜单界面，并支持相机预览和拍照功能。

## 🎯 项目特性

### 核心功能
- **模块化架构**：采用6大功能模块分离设计，提高代码可维护性
- **图形菜单系统**：支持主菜单和子菜单的多级页面切换
- **旋转编码器控制**：使用EC11编码器进行菜单导航和选项选择
- **相机功能集成**：在主菜单A选项中集成相机预览和拍照功能
- **实时图像显示**：支持VGA预览和720p高清拍照
- **16x16点阵字体**：支持中英文文字显示
- **SD卡存储**：支持拍照图片保存到SD卡

### 技术特点
- **模块化设计**：Display、Encoder、Camera、Menu、Shared、Utils六大模块
- **统一日志系统**：分级日志管理，替代直接Serial调用
- **时间管理工具**：统一时间函数接口，便于测试和维护
- **高级图形绘制**：支持三角形、圆形、圆角矩形等图形原语
- **中文字体渲染**：专门的字体渲染模块
- **显示硬件抽象**：TFT管理器封装显示硬件操作
- 基于AmebaST7789_DMA_SPI1显示驱动
- 使用JPEGDEC库进行JPEG图像解码
- 支持AmebaFatFS文件系统
- 中断驱动的旋转编码器处理
- 多状态机系统管理
- 硬件消抖和防抖处理

## 📁 项目结构（模块化重构版）

```
Camera/
├── Camera.ino                          # 主程序文件，集成所有模块
├── 
├── 模块化文件（按功能分类）：
│   
│   ├── Display模块（显示功能）
│   │   ├── Display_AmebaST7789_DMA_SPI1.h/.cpp   # 显示驱动封装（DMA版本）
│   │   ├── Display_AmebaST7789_SPI1.h/.cpp       # 显示驱动封装（普通SPI版本）
│   │   ├── Display_SPI.h/.cpp                    # SPI显示驱动基础
│   │   ├── Display_TFTManager.h/.cpp             # TFT显示管理器
│   │   ├── Display_FontRenderer.h/.cpp           # 字体渲染器
│   │   └── Display_GraphicsPrimitives.h/.cpp     # 图形绘制原语
│   
│   ├── Encoder模块（编码器控制）
│   │   └── Encoder_Control.h/.cpp                # 编码器控制
│   
│   ├── Camera模块（相机功能）
│   │   ├── Camera_CameraCore.h/.cpp               # 相机硬件驱动核心
│   │   ├── Camera_CameraManager.h/.cpp           # 相机管理器（预览/拍照控制）
│   │   ├── Camera_SDCardManager.h/.cpp           # SD卡管理器
│   │   └── Camera_ImageConfig.h                   # 图像配置
│   
│   ├── Menu模块（菜单系统）
│   │   ├── Menu_MenuManager.h                     # 菜单管理器
│   │   ├── Menu_MenuContext.h/.cpp               # 菜单上下文（子菜单处理）
│   │   ├── Menu_TriangleController.h/.cpp        # 三角形指示器控制
│   │   ├── Menu_MM.h                             # 主菜单图像数据
│   │   └── Menu_SM.h                             # 子菜单图像数据
│   
│   ├── Shared模块（共享资源）
│   │   ├── Shared_GlobalDefines.h                # 全局宏定义
│   │   ├── Shared_Types.h                        # 共享数据类型
│   │   └── Shared_SharedResources.h              # 共享资源声明
│   
│   └── Utils模块（工具函数）
│       ├── Utils_Logger.h/.cpp                   # 日志系统
│       ├── Utils_Timer.h/.cpp                    # 定时器工具
│       └── Utils_BufferManager.h/.cpp            # 缓冲区管理
│
├── Display_font16x16.h                # 16x16点阵字体库
├── font5x7.h                          # 5x7字体库
└── README.md                           # 项目说明文档
```

## 🔧 硬件要求

### 开发板
- AMB82-MINI 开发板

### 显示屏
- TFT显示屏（ST7789驱动芯片）
- 引脚定义：
  - TFT_CS: SPI1_SS
  - TFT_DC: 4
  - TFT_RST: 5
  - TFT_BL: 6

### 输入设备
- EC11旋转编码器
- 引脚定义：
  - ENCODER_CLK: 7
  - ENCODER_DT: 8
  - ENCODER_SW: 9

### 相机模块
- 兼容AMB82-MINI的相机模块

### 存储设备
- MicroSD卡（用于保存拍照图片）

## 🚀 快速开始

### 1. 硬件连接
按照引脚定义连接TFT显示屏、EC11旋转编码器和相机模块到AMB82-MINI开发板。

### 2. 图片配置准备
如果需要自定义菜单图片，使用图片处理工具：

1. 进入 `生成图片数组` 目录
2. 将PNG格式的菜单图片放入该目录（如：MM.png, SM.png）
3. 双击运行 `自动生成配置文件.bat`
4. 将生成的 `ImageConfig.h` 复制到项目根目录
5. 将生成的 `.h` 文件（如MM.h, SM.h）复制到项目根目录

### 3. 模块化项目编译上传
1. 使用Arduino IDE打开 `Camera.ino`
2. 选择正确的开发板和端口
3. 确保所有必要的库已安装：
   - AmebaST7789_DMA_SPI1
   - JPEGDEC_Libraries
   - AmebaFatFS
4. **模块化特性**：项目已自动包含所有模块化文件，无需额外配置
5. 编译并上传程序

### 4. 模块化项目验证
上传完成后，系统将自动启动并显示：
- ✅ 主菜单界面正常显示
- ✅ 中文提示信息正确
- ✅ 三角形指示器移动正常
- ✅ 相机预览功能正常
- ✅ 模块化日志系统工作正常

## 📋 使用说明

### 菜单操作
- **旋转编码器**：顺时针旋转向下移动，逆时针旋转向上移动
- **按下编码器**：根据当前状态执行不同操作
- **三角形指示器**：显示当前选中的菜单项

### 详细操作说明

#### 主菜单操作
- **旋转旋钮**：在A-F位置之间移动三角形
- **按压开关**：
  - A位置：进入相机预览模式
  - B-E位置：仅显示位置信息，无其他操作
  - F位置：进入系统设置子菜单

#### 系统设置子菜单操作
- **旋转旋钮**：在子菜单选项之间移动三角形，不会返回主菜单
- **按压开关**：
  - F选项位置：返回主菜单
  - 其他选项位置：仅显示位置信息，无其他操作

#### 相机预览模式操作
- **旋转旋钮**：返回主菜单
- **按压开关**：拍照并保存图片

### 菜单项说明
- **A选项**：进入相机预览模式
- **B-F选项**：其他功能选项（可根据需要扩展）

### 相机功能
在A选项下按下编码器进入相机预览模式：
- **预览模式**：实时显示相机画面（VGA分辨率，640x480）
- **拍照功能**：按下编码器进行手动拍照（720p高清，1280x720）
- **图片保存**：拍照后图片自动保存到SD卡
- **状态提示**：显示中文状态提示（"实时预览"、"拍照成功"、"保存失败"、"拍照失败"）
- **返回菜单**：旋转旋钮返回主菜单

## 🔌 引脚配置

### TFT显示屏引脚
```cpp
#define TFT_CS   SPI1_SS    // SPI片选
#define TFT_DC   4          // 数据/命令选择
#define TFT_RST  5          // 复位引脚
#define TFT_BL   6          // 背光控制
```

### EC11编码器引脚
```cpp
#define ENCODER_CLK 7       // 时钟引脚
#define ENCODER_DT  8       // 数据引脚
#define ENCODER_SW  9       // 开关引脚
```

### 相机配置
```cpp
#define PREVIEW_CH  0       // 预览通道：VGA
#define STILL_CH    1       // 拍照通道：720p
VideoSetting configPreview(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);
VideoSetting configStill(VIDEO_HD, CAM_FPS, VIDEO_JPEG, 1);
```

## 🛠️ 开发指南

### 模块化架构设计
项目采用模块化架构，分为6大功能模块：

#### 1. Display模块（显示功能）
- **Display_TFTManager**: TFT显示硬件抽象层
- **Display_FontRenderer**: 中文字体渲染系统
- **Display_GraphicsPrimitives**: 高级图形绘制原语
- **使用方式**: `tftManager.fillScreen(ST7789_BLACK);`

#### 2. Encoder模块（编码器控制）
- **Encoder_Control**: EC11旋转编码器驱动
- **硬件消抖**: 100ms间隔处理
- **方向检测**: 精确的旋转方向识别
- **回调机制**: 状态感知的事件处理系统

**核心功能**:
- `init()` - 初始化编码器硬件和中断
- `setRotationCallback()` - 注册旋转事件回调函数
- `setButtonCallback()` - 注册按钮事件回调函数
- `checkButton()` - 按钮状态检测和消抖处理
- `getLastRotation()` - 获取最后一次旋转方向
- `isButtonPressed()` - 检查按钮是否按下
- `getEncoderCount()` - 获取编码器计数

**技术特性**:
- **中断驱动**: 使用GPIO中断检测旋转事件
- **静态实例**: 支持中断服务例程的对象访问
- **消抖处理**: 50ms按钮消抖，300ms旋转消抖
- **状态感知**: 支持不同系统状态下的差异化处理
- **事件回调**: 解耦硬件事件和业务逻辑

**使用示例**:
```cpp
// 创建编码器实例
EncoderControl encoder;

// 初始化编码器
encoder.init(ENCODER_CLK, ENCODER_DT, ENCODER_SW);

// 设置回调函数
encoder.setRotationCallback(handleEncoderRotation);
encoder.setButtonCallback(handleEncoderButton);

// 在主循环中检查按钮状态
void loop() {
    encoder.checkButton();
    // 其他代码...
}
```

**回调函数示例**:
```cpp
// 旋转事件回调（支持状态感知）
void handleEncoderRotation(RotationDirection direction) {
    // 根据当前系统状态处理旋转事件
    switch (currentState) {
        case STATE_MAIN_MENU:
            // 主菜单模式：移动三角形
            if (direction == ROTATION_CW) moveTriangleDown();
            else if (direction == ROTATION_CCW) moveTriangleUp();
            break;
        case STATE_CAMERA_PREVIEW:
            // 相机预览模式：返回主菜单
            xEventGroupSetBits(xEventGroup, EVENT_RETURN_TO_MENU);
            break;
    }
}

// 按钮事件回调
void handleEncoderButton() {
    // 处理按钮按下事件
    switch (currentState) {
        case STATE_MAIN_MENU:
            // 主菜单：选择当前菜单项
            handleMainMenuSelection();
            break;
        case STATE_CAMERA_PREVIEW:
            // 相机预览：拍照
            takePhoto();
            break;
    }
}
```

#### 3. Camera模块（相机功能）
- **Camera_ImageConfig**: 图像配置管理
- **预览/拍照**: VGA预览 + 720p高清拍照
- **SD卡存储**: 自动保存拍照图片

#### 4. Menu模块（菜单系统）
- **Menu_MenuManager**: 多级菜单状态管理
- **Menu_MenuContext**: 菜单上下文管理，处理子菜单逻辑和状态
- **Menu_TriangleController**: 三角形指示器控制，负责菜单选择可视化
- **Menu_MM/Menu_SM**: 主菜单/子菜单图像数据
- **核心功能**: 支持主菜单和子菜单的多级页面切换、编码器控制的菜单导航

#### 5. Shared模块（共享资源）
- **Shared_GlobalDefines**: 全局宏定义和引脚配置
- **Shared_Types**: 共享数据类型定义
- **Shared_SharedResources**: 事件组、信号量等共享资源

#### 6. Utils模块（工具函数）
- **Utils_Logger**: 分级日志系统（替代Serial调用）
- **Utils_Timer**: 统一时间管理接口
- **Utils_BufferManager**: 内存缓冲区管理

### 系统状态机
项目使用状态机管理不同模式：
- `STATE_MAIN_MENU`：主菜单状态
- `STATE_SUB_MENU`：子菜单状态
- `STATE_CAMERA_PREVIEW`：相机预览状态

### 图像处理流程
1. 图片通过工具转换为C数组格式（RGB565）
2. 使用ImageConfig.h统一管理图像配置
3. 显示屏直接显示图像数据
4. 相机图像通过JPEGDEC库解码显示

### 旋转编码器处理
- 使用中断处理旋转检测
- 硬件消抖处理（100ms间隔）
- 方向检测和计数处理
- 按钮去抖动处理（50ms延迟）
- 状态机驱动的菜单导航系统
- 严格的软件消抖处理（300ms间隔）

### 16x16点阵字体系统
- 支持中英文混合显示
- 包含字母、数字和常用符号
- 支持中文字符显示（实时预览、拍照成功、保存失败、拍照失败等）
- 支持字符旋转显示（90度顺时针旋转）
- 可扩展新的字符

### 扩展功能
- **添加新的菜单页面**：创建新的图片和对应的.h文件
- **扩展相机功能**：修改相机配置和图像处理逻辑
- **添加其他外设**：通过GPIO引脚连接其他传感器或模块
- **自定义字体**：扩展font16x16.h字库
- **模块化扩展**：遵循现有模块化架构添加新功能

## 📊 技术规格

### 显示规格
- 分辨率：根据TFT显示屏规格
- 颜色深度：16位RGB565
- 刷新率：根据SPI配置
- 字体大小：16x16像素

### 相机规格
- 预览分辨率：VGA (640x480)
- 拍照分辨率：HD (1280x720)
- 图像格式：JPEG
- 帧率：可配置的CAM_FPS

### 模块化架构规格
- **模块数量**: 6大功能模块
- **代码文件**: 26个模块化文件
- **代码行数**: 约3,500行重构代码
- **Serial调用替换**: 100%完成
- **时间函数迁移**: 100%完成

### 性能指标
- 菜单响应时间：< 50ms
- 图像显示延迟：< 100ms
- 相机启动时间：< 2s
- 拍照保存时间：< 1s
- 模块初始化时间：< 100ms
- 图形绘制性能：支持实时三角形移动

## 🐛 故障排除

### 常见问题
1. **显示屏无显示**
   - 检查引脚连接是否正确
   - 验证SPI配置和电源连接
   - 检查背光控制引脚

2. **编码器无响应**
   - 检查编码器引脚连接
   - 验证中断配置和上拉电阻
   - 检查硬件消抖设置

3. **相机功能异常**
   - 检查相机模块连接
   - 验证相机驱动库版本
   - 检查内存分配情况

4. **SD卡无法识别**
   - 检查SD卡格式是否正确
   - 验证文件系统初始化
   - 检查存储空间是否充足

5. **模块化编译错误**
   - 检查所有模块化文件是否在正确目录
   - 验证头文件包含路径是否正确
   - 确认模块初始化顺序正确

6. **图形绘制异常**
   - 检查Display_TFTManager是否已初始化
   - 验证图形原语模块是否正确集成
   - 确认三角形位置计算逻辑

### 调试建议
- 使用Utils_Logger输出调试信息（替代Serial）
- 检查内存使用情况
- 验证中断处理逻辑
- 使用示波器检查信号时序
- 检查模块初始化状态
- 验证模块间接口调用

## 📝 版本历史

### v2.0.1（模块化重构版 - 修复更新）
- **第五阶段第三步完成**：Camera.ino重构集成完成
- **三角形指示器修复**：解决了拍照后旋转旋钮返回主菜单时，三角形一直停留在A选项的问题
- **状态管理优化**：改进系统状态转换逻辑，确保createTask返回值正确处理
- **函数声明修复**：修正createTask函数声明和定义的一致性
- **语法错误修复**：解决函数原型缺失和语法错误问题

### v2.0.2（模块化重构版 - Camera.ino清理完成）
- **Phase 8.4完成**：Camera.ino主程序重构完成，遵循软件工程最佳实践
- **模块化拆分**：将相关功能代码进行模块化拆分，模块间职责清晰、接口明确
- **代码清理**：删除已模块化的注释代码，保留有效代码及其说明注释
- **单一职责原则**：Camera.ino仅负责系统初始化和任务协调
- **模块集成**：MenuContext和TriangleController模块完成集成
- **系统验证**：固件刷入后系统运行正常

### v2.0.0（模块化重构版）
- **模块化架构重构**：6大功能模块分离设计
- **Display模块**：TFT管理器、字体渲染器、图形原语
- **Encoder模块**：编码器控制驱动
- **Camera模块**：图像配置管理
- **Menu模块**：菜单系统状态管理
- **Shared模块**：共享资源和全局定义
- **Utils模块**：日志系统、定时器、缓冲区管理
- **Serial调用替换**：100%使用Utils_Logger替代
- **时间函数迁移**：100%使用Utils_Timer替代
- **高级图形绘制**：支持三角形、圆形、圆角矩形
- **系统稳定性验证**：固件刷入后运行正常

### v1.0.0
- 初始版本发布
- 基础菜单系统
- 相机功能集成
- 图片处理工具
- SD卡存储功能
- 16x16点阵字体系统

## 📄 许可证

本项目基于Realtek AMB82-MINI SDK开发，具体许可证信息请参考相关SDK文档。

## 🤝 贡献

欢迎提交Issue和Pull Request来改进这个项目。

## 📞 支持

如有问题请参考：
- Realtek官方文档
- AMB82-MINI开发板手册
- 项目Issue页面

---

**注意**：本项目需要特定的硬件环境，请确保使用兼容的AMB82-MINI开发板和外围设备。