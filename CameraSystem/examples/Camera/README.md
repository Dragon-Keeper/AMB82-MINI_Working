# AMB82-MINI相机控制系统

这是一个基于AMB82-MINI开发板的相机控制系统，使用EC11旋转编码器控制菜单操作，集成实时预览、拍照、录像和图片回放等功能。

## 项目概述

- **开发板**：AMB82-MINI
- **显示模块**：ST7789 240x320 TFT屏幕
- **输入设备**：EC11旋转编码器
- **相机模块**：GC2053摄像头
- **存储设备**：Micro SD卡
- **时钟模块**：DS1307实时时钟
- **操作系统**：FreeRTOS

## 硬件要求

- AMB82-MINI开发板
- ST7789 240x320 TFT屏幕
- EC11旋转编码器(在 EC11 旋转编码器 的 CLK、DT、SW 引脚与 GND 之间并联了10nF陶瓷电容，核心是通过 RC 滤波抑制机械触点抖动和电磁噪声，电容值选择需兼顾滤波效果和信号响应速度，过大的电容会导致脉冲上升 / 下降沿延迟，造成单片机识别失效。)
- Micro SD卡（建议8GB以上）
- DS1307实时时钟模块
- 12V电源适配器

## 软件要求

- Arduino IDE 1.8.19或更高版本
- Realtek AmebaPro2开发板支持包
- JPEGDEC库
- AmebaFatFS库

## 功能特点

### 主菜单系统
- 使用EC11旋转编码器控制三角形光标在A-F位置间移动
- 按压编码器开关选择当前菜单项
- 6个独立功能模块，对应菜单A-F位置

### 相机功能
- 实时预览模式（VGA分辨率）
- 拍照功能（720P分辨率）
- 录像功能（720P分辨率）

### 图片回放功能
- 支持从SD卡读取JPG图片
- 使用旋转编码器控制前后切换照片
- 按压编码器开关返回主菜单
- 自动缩放适应屏幕显示
- 回放顺序从最新拍摄到最早拍摄
- 支持最大1000张照片的高效管理
- 优化内存使用，避免栈溢出问题
- 解决SD卡读写冲突，确保功能稳定

### 系统管理
- FreeRTOS多任务架构，支持任务优先级管理
- 模块化设计，易于扩展和维护
- 完善的错误处理和日志系统，支持分级日志输出
- DS1307实时时钟模块集成
  - 系统启动时自动设置初始时间
  - 通过串口监视器输出实时秒数
  - 支持固件刷写时保护时钟不被重置
  - 硬件I2C通信，稳定可靠
- 系统配置管理，支持配置项的持久化存储
- 系统资源管理，优化内存和资源分配
- 系统状态管理，实现状态机控制

## 系统架构

### FreeRTOS任务分配

| 菜单位置 | 功能描述 | 任务ID |
|---------|---------|--------|
| A | 实时预览 | EVENT_TASK_A_REQUESTED |
| B | 拍照功能 | EVENT_TASK_B_REQUESTED |
| C | 图片回放 | EVENT_TASK_C_REQUESTED |
| D | 录像功能 | EVENT_TASK_D_REQUESTED |
| E | 设置菜单 | EVENT_TASK_E_REQUESTED |
| F | 关于信息 | EVENT_TASK_F_REQUESTED |

### 事件驱动机制

系统使用FreeRTOS事件组实现任务间通信：

- `EVENT_TASK_X_REQUESTED`：请求启动对应任务
- `EVENT_RETURN_TO_MENU`：返回主菜单事件
- `EVENT_NEXT_PHOTO`：显示下一张照片
- `EVENT_PREVIOUS_PHOTO`：显示上一张照片

## 使用说明

### 系统启动

1. 连接所有硬件设备
2. 插入Micro SD卡
3. 上电启动系统
4. 系统将自动进入主菜单界面

### 菜单操作

1. **旋转编码器**：控制三角形光标在A-F位置间移动
2. **按压编码器开关**：选择当前菜单项并执行对应功能

### 图片回放操作

1. 在主菜单选择位置C进入回放模式
2. **顺时针旋转**：显示下一张照片
3. **逆时针旋转**：显示上一张照片
4. **按压开关**：返回主菜单

## 菜单功能详解

### 位置A：实时预览
- 启动相机实时预览
- 显示VGA分辨率视频流
- 按压开关返回主菜单

### 位置B：拍照功能
- 拍摄720P分辨率照片
- 保存到SD卡
- 自动命名为Image_X.jpg（X为序号）

### 位置C：图片回放
- 从SD卡读取并显示JPG图片
- 支持旋钮控制前后切换
- 自动缩放适应屏幕

### 位置D：录像功能
- 录制720P分辨率视频
- 保存到SD卡
- 支持开始/停止控制

### 位置E：设置菜单
- 进入系统设置子菜单
- 调整相机参数（分辨率、质量等）
- 设置显示选项（亮度、对比度等）
- 调整编码器灵敏度
- 设置系统超时时间
- 管理系统配置

### 位置F：关于信息
- 显示系统版本信息
- 显示硬件配置
- 显示版权信息

## 项目结构

```
Camera/
├── Camera.ino                           # 主程序入口
├── README.md                            # 项目说明文档
├── Shared_GlobalDefines.h               # 全局定义
├── Shared_Types.h                       # 共享数据类型
├── Shared_SharedResources.h             # 共享资源
├── Menu_MenuContext.cpp                 # 菜单上下文管理
├── Menu_MenuContext.h                   # 菜单上下文管理头文件
├── Menu_MenuManager.cpp                 # 菜单管理器
├── Menu_MenuManager.h                   # 菜单管理器头文件
├── Menu_MenuItems.cpp                   # 菜单项管理
├── Menu_MenuItems.h                     # 菜单项管理头文件
├── MenuPages.cpp                        # 菜单页面数据
├── MenuPages.h                          # 菜单页面数据头文件
├── Menu_TriangleController.cpp          # 三角形光标控制器
├── Menu_TriangleController.h            # 三角形光标控制器头文件
├── Menu_MM.h                            # 主菜单配置
├── Menu_SM.h                            # 设置菜单配置
├── Display_AmebaST7789_DMA_SPI1.cpp     # DMA SPI显示驱动
├── Display_AmebaST7789_DMA_SPI1.h       # DMA SPI显示驱动头文件
├── Display_AmebaST7789_SPI1.cpp         # SPI显示驱动
├── Display_AmebaST7789_SPI1.h           # SPI显示驱动头文件
├── Display_FontRenderer.cpp             # 字体渲染
├── Display_FontRenderer.h               # 字体渲染头文件
├── Display_GraphicsPrimitives.cpp       # 图形基元
├── Display_GraphicsPrimitives.h         # 图形基元头文件
├── Display_SPI.cpp                      # SPI显示接口
├── Display_SPI.h                        # SPI显示接口头文件
├── Display_TFTManager.cpp               # TFT屏幕管理器
├── Display_TFTManager.h                 # TFT屏幕管理器头文件
├── Display_font16x16.h                  # 16x16字体数据
├── font5x7.h                            # 5x7字体数据
├── Encoder_Control.cpp                  # 编码器控制
├── Encoder_Control.h                    # 编码器控制头文件
├── RTOS_TaskFactory.cpp                 # 任务工厂
├── RTOS_TaskFactory.h                   # 任务工厂头文件
├── RTOS_TaskManager.cpp                 # 任务管理器
├── RTOS_TaskManager.h                   # 任务管理器头文件
├── Camera_CameraManager.cpp             # 相机管理
├── Camera_CameraManager.h               # 相机管理头文件
├── Camera_CameraCore.cpp                # 相机核心功能
├── Camera_CameraCore.h                  # 相机核心功能头文件
├── Camera_SDCardManager.cpp             # SD卡管理
├── Camera_SDCardManager.h               # SD卡管理头文件
├── Camera_ImageConfig.h                 # 图像配置
├── DS1307_ClockModule.cpp               # DS1307时钟模块
├── DS1307_ClockModule.h                 # DS1307时钟模块头文件
├── System_ConfigManager.cpp             # 系统配置管理
├── System_ConfigManager.h               # 系统配置管理头文件
├── System_ResourceManager.cpp           # 系统资源管理
├── System_ResourceManager.h             # 系统资源管理头文件
├── System_StateManager.cpp              # 系统状态管理
├── System_StateManager.h                # 系统状态管理头文件
├── Utils_Logger.cpp                     # 日志工具
├── Utils_Logger.h                       # 日志工具头文件
├── Utils_BufferManager.cpp              # 缓冲区管理
├── Utils_BufferManager.h                # 缓冲区管理头文件
└── Utils_Timer.cpp                      # 定时器工具
└── Utils_Timer.h                        # 定时器工具头文件
```

## 编译和上传

1. 打开Arduino IDE
2. 安装Realtek AmebaPro2开发板支持包
3. 导入项目文件
4. 选择AMB82-MINI开发板
5. 连接开发板到电脑
6. 编译并上传程序

## 故障排除

### SD卡错误
- 检查SD卡是否正确插入
- 确认SD卡格式为FAT32
- 尝试重新格式化SD卡

### 相机预览失败
- 检查相机模块连接
- 确认电源电压稳定
- 尝试重新启动系统

### 图片显示失败
- 确认SD卡中有正确命名的JPG文件
- 检查图片格式是否兼容
- 确认图片大小不超过系统处理能力

### DS1307时钟错误
- 检查I2C连接（SCL和SDA引脚）
- 确认DS1307模块电源正常
- 检查是否存在引脚冲突
- 确认UPDATE_DS1307_TIME宏配置正确

## 版本历史

### V1.3.0 (2026-01-11)
- 更新README.md，完善项目结构描述
- 增强系统管理功能，添加配置、资源和状态管理
- 优化系统架构文档
- 改进菜单模块，添加菜单项管理和三角形光标控制器

### V1.2.0 (2026-01-10)
- 改进图片回放功能，支持最大1000张照片
- 将回放顺序从最早拍摄到最新拍摄改为最新拍摄到最早拍摄
- 修复栈溢出问题，将String数组从栈空间改为堆空间分配
- 解决SD卡读写冲突问题，确保文件系统正确释放
- 增加目录缓冲区大小，优化内存使用
- 添加内存使用监控日志

### V1.1.0 (2026-01-07)
- 集成DS1307实时时钟模块
- 实现硬件I2C通信驱动
- 添加时钟超时机制防止程序阻塞
- 支持固件刷写时保护时钟设置

### V1.0.0 (2026-01-06)
- 初始版本发布
- 实现主菜单系统
- 集成相机预览和拍照功能
- 添加图片回放功能
- 支持旋钮控制照片导航

## 注意事项

1. 使用12V电源适配器供电，确保系统稳定运行
2. 避免在高温、潮湿环境下使用
3. 定期备份SD卡中的照片和视频
4. 请勿在系统运行时拔插SD卡

## 许可证

本项目采用MIT许可证，详见LICENSE文件。
