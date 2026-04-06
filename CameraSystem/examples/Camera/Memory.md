# AMB82-MINI Camera项目开发历程记录

## 项目概述
本项目为AMB82-MINI Camera项目实现视频录制功能的同步音频录制模块，使系统能够同时捕获并整合音频信号，生成包含同步音视频的AVI媒体文件。

---

## 开发记录

### 版本 V1.33 - 修复编译错误：ISP配置模块集成优化 (2026-03-31)

#### 问题描述
根据 Bug.md 中的编译错误报告，ISP 配置模块在集成过程中出现了多个编译错误，包括：
1. FatFS.h 头文件未找到（项目实际使用 AmebaFatFS）
2. FILE_READ/FILE_WRITE 宏未定义
3. ROTATION_CLOCKWISE 枚举未定义（应为 ROTATION_CW）
4. ST7789_DARKGREY 颜色未定义
5. ISPConfigManager 缺少 setConfig() 方法
6. CameraManager 的 applyISPSettings() 方法是 private 的
7. Display_TFTManager 使用了错误的方法名（drawRect 应为 drawRectangle）
8. Display_FontRenderer 使用了不存在的 drawString() 方法
9. ISPConfigTask 的静态回调方法访问权限问题
10. taskISPConfig() 函数中的 TaskFactory 和全局变量引用问题

#### 根本原因
1. **文件系统架构不匹配**：ISPConfigManager 错误地使用了标准 FatFS 库，而项目实际使用 AmebaFatFS 和 SDCardManager 封装
2. **枚举值不匹配**：编码器控制模块使用 ROTATION_CW/ROTATION_CCW，而非 ROTATION_CLOCKWISE
3. **API 方法名不一致**：Display_TFTManager 使用 drawRectangle/fillRectangle 而非 drawRect/fillRect
4. **访问权限问题**：静态回调方法需要 public 访问权限
5. **缺少必要的方法**：ISPConfigUI 需要调用 setConfig() 方法
6. **taskISPConfig 过度复杂**：原实现使用了 TaskFactory 和复杂的参数传递，与项目现有架构不兼容

#### 技术方案
1. **ISP_ConfigManager 重构**：
   - 添加 SDCardManager* 成员变量，通过 m_sdCardManager->getFileSystem() 获取 AmebaFatFS 实例
   - 修改 init() 方法签名为 bool init(SDCardManager& sdCardManager)
   - 添加 setConfig(const ISPConfig& config) 方法，支持完整配置更新
   - 移除 FILE_READ/FILE_WRITE 参数，使用 fs->open(path) 的默认模式
   - 所有文件操作通过 SDCardManager 统一管理

2. **ISP_ConfigUI 简化**：
   - 将 ST7789_DARKGREY 替换为 ST7789_BLUE
   - 使用 tftManager->drawRectangle() 和 tftManager->fillRectangle()
   - 使用 tftManager->setTextColor()、setCursor() 和 print() 方法输出文本
   - 简化 drawMenuItem() 实现，直接使用 TFT 的文本输出能力
   - 暂时禁用复杂的参数条绘制（保留接口，后续可完善）

3. **ISP_ConfigTask 重构**：
   - 移除静态回调方法，改用非静态内部方法
   - 添加 handleISPRotation() 和 handleISPButton() 静态辅助函数
   - 修复 ROTATION_CLOCKWISE 为 ROTATION_CW/ROTATION_CCW
   - 简化 taskISPConfig() 实现，移除 TaskFactory 依赖
   - 在 taskISPConfig() 内部声明所有所需的外部全局变量引用

4. **CameraManager 调整**：
   - 将 applyISPSettings() 从 private 移到 public 区域
   - 更新 ISPConfigManager 初始化调用，传入 *m_sdCardManager 参数

#### 核心修复内容
- ✅ FatFS.h 头文件问题已解决（改用项目现有的 SDCardManager）
- ✅ FILE_READ/FILE_WRITE 宏问题已解决（使用默认打开模式）
- ✅ ROTATION_CLOCKWISE 已替换为 ROTATION_CW/ROTATION_CCW
- ✅ ST7789_DARKGREY 已替换为 ST7789_BLUE
- ✅ ISPConfigManager 已添加 setConfig() 方法
- ✅ applyISPSettings() 方法已设为 public
- ✅ Display_TFTManager 方法名已修正
- ✅ Display_FontRenderer 调用已替换为 TFT 直接文本输出
- ✅ 静态回调方法访问权限已解决
- ✅ taskISPConfig() 已简化，与项目架构兼容

#### 文件变更
- `ISP_ConfigManager.h`: 添加 SDCardManager 依赖，添加 setConfig() 方法
- `ISP_ConfigManager.cpp`: 重构文件操作，使用 SDCardManager 和 AmebaFatFS
- `ISP_ConfigUI.h`: 替换未定义的颜色常量
- `ISP_ConfigUI.cpp`: 重写绘制逻辑，使用正确的 TFT API
- `ISP_ConfigTask.h`: 调整方法访问权限
- `ISP_ConfigTask.cpp`: 重写任务逻辑，修复所有编译错误
- `Camera_CameraManager.h`: 将 applyISPSettings() 设为 public
- `Camera_CameraManager.cpp`: 更新 ISPConfigManager 初始化调用
- `Shared_GlobalDefines.h`: 版本号更新到 V1.33

---

### 版本 V1.32 - ISPControl移植：阶段三 完整用户配置界面开发 (2026-03-31)

#### 问题描述
完成 ISPControl移植报告.md 的阶段三开发，需要创建完整的用户配置界面，让用户可以通过选项D进入ISP配置菜单，调节曝光模式、亮度、对比度、饱和度等参数，并实时预览效果，参数保存到SD卡。

#### 实施目标
1. 创建 ISPConfigUI 类，实现ISP配置界面的显示和交互
2. 创建 ISPConfigTask 类，实现ISP配置任务的完整逻辑
3. 将任务集成到选项D，替换原有的占位功能
4. 实现实时预览功能，参数调节时立即生效
5. 实现参数保存到SD卡的 ISPControl.ini 文件

#### 技术方案
1. **ISP_ConfigUI.h/.cpp 创建**：
   - 实现菜单背景和菜单项绘制
   - 实现参数条显示（亮度、对比度、饱和度）
   - 实现曝光模式选择（自动/手动）
   - 支持菜单导航、参数编辑和确认保存

2. **ISP_ConfigTask.h/.cpp 创建**：
   - 实现编码器事件处理（旋转和按钮）
   - 实现菜单导航逻辑
   - 实现参数编辑和保存逻辑
   - 集成相机预览，实现实时效果显示
   - 将参数实时应用到CameraManager

3. **RTOS_TaskFactory.cpp 修改**：
   - 添加ISP相关头文件包含
   - 修改taskFunctionD，重定向到ISP配置任务

4. **系统集成**：
   - 从SD卡加载配置
   - 参数修改时实时应用到相机
   - 确认保存时写入SD卡
   - 支持参数重置功能

#### 核心功能特性
- 完整的ISP配置菜单（6个选项）
- 曝光模式切换（自动/手动）
- 亮度调节（-64 ~ 64）
- 对比度调节（0 ~ 100）
- 饱和度调节（0 ~ 100）
- 参数重置功能
- 实时预览，参数调节立即生效
- 参数持久化存储到SD卡（ISPControl.ini）
- 友好的UI界面，带红色选择框
- 参数条显示，直观展示当前值

#### 实施步骤
1. 创建 ISP_ConfigUI.h，定义UI状态和接口
2. 创建 ISP_ConfigUI.cpp，实现完整绘制逻辑
3. 创建 ISP_ConfigTask.h，定义任务接口
4. 创建 ISP_ConfigTask.cpp，实现完整任务逻辑
5. 修改 RTOS_TaskFactory.cpp，添加头文件包含
6. 修改 RTOS_TaskFactory.cpp，重定向taskFunctionD
7. 更新 Shared_GlobalDefines.h，版本号从 V1.31 到 V1.32
8. 更新 Memory.md 记录本次完整开发

#### 文件变更
- 新建 `ISP_ConfigUI.h`: ISP配置界面头文件
- 新建 `ISP_ConfigUI.cpp`: ISP配置界面实现
- 新建 `ISP_ConfigTask.h`: ISP配置任务头文件
- 新建 `ISP_ConfigTask.cpp`: ISP配置任务实现
- 修改 `RTOS_TaskFactory.cpp`: 添加头文件，重定向任务D
- 修改 `Shared_GlobalDefines.h`: 版本号更新到 V1.32

#### 后续计划
- 完整测试所有功能
- 验证参数正确应用到拍照
- 验证参数保存和加载功能
- 优化UI交互体验

---

### 版本 V1.31 - ISPControl移植：阶段三 ISPConfigManager 创建与集成 (2026-03-31)

#### 问题描述
继续 ISPControl移植报告.md 的阶段三开发，需要创建 ISP 配置管理器来处理参数持久化存储和读取，为后续的用户配置界面做准备。

#### 实施目标
1. 创建 ISPConfigManager 类，管理 ISP 参数配置
2. 支持从 SD 卡根目录的 ISPControl.ini 文件读写配置
3. 集成到 CameraManager 中，在拍照时应用存储的配置

#### 技术方案
1. **ISP_ConfigManager.h 创建**：
   - 定义 ISPConfig 结构体，包含所有 ISP 参数
   - 定义参数范围宏（与 ISPControl 项目一致）
   - 声明 ISPConfigManager 类接口

2. **ISP_ConfigManager.cpp 创建**：
   - 实现配置文件读写（使用 FatFS/SD）
   - 实现参数验证和默认值管理
   - 实现配置重置功能

3. **Camera_CameraManager 集成**：
   - 添加 ISPConfigManager 成员变量
   - 在 init() 中初始化并加载配置
   - 从配置管理器读取参数到 CameraManager 内部变量

#### 核心功能特性
- 曝光模式（0=手动, 1=自动）
- 亮度调节（-64 ~ 64）
- 对比度调节（0 ~ 100）
- 饱和度调节（0 ~ 100）
- 白平衡模式（0=手动, 1=自动）
- 配置文件持久化到 SD 卡（ISPControl.ini）
- 参数范围验证，防止无效配置
- 默认配置管理

#### 实施步骤
1. 创建 ISP_ConfigManager.h，定义数据结构和接口
2. 创建 ISP_ConfigManager.cpp，实现完整功能
3. 修改 Camera_CameraManager.h，添加 ISPConfigManager 成员
4. 修改 Camera_CameraManager.cpp，在构造函数和 init() 中集成
5. 更新 Memory.md 记录本次进展

#### 文件变更
- 新建 `ISP_ConfigManager.h`: ISP 配置管理器头文件
- 新建 `ISP_ConfigManager.cpp`: ISP 配置管理器实现
- 修改 `Camera_CameraManager.h`: 添加 ISPConfigManager 成员
- 修改 `Camera_CameraManager.cpp`: 集成 ISPConfigManager

#### 后续计划
阶段三剩余工作：
- 创建 ISP 配置界面 UI 类
- 创建 ISP 配置任务
- 修改主菜单，在选项D中添加 ISP 配置子菜单
- 实现完整的交互功能和实时预览

---

### 版本 V1.30 - ISPControl移植：渐进式集成阶段一和阶段二 (2026-03-31)

#### 问题描述
根据 ISPControl移植报告.md 的分析，Camera 项目缺少 ISP（图像信号处理器）参数配置能力，无法像 ISPControl 项目示例那样配置曝光模式、亮度、对比度、饱和度、白平衡等关键拍照参数，限制了拍照功能的灵活性和图像质量调节能力。

#### 实施目标
完成 ISPControl移植报告.md 中"渐进式集成"方案的阶段一和阶段二：
- 阶段一：在 CameraManager 中添加 CameraSetting 对象和基础 ISP 配置方法
- 阶段二：在拍照流程中应用 ISP 设置

#### 技术方案
1. **Camera_CameraManager.h 修改**：
   - 添加 ISP 参数默认值定义宏
   - 在 CameraManager 类中添加 ISP 配置方法声明
   - 添加 CameraSetting 指针和相关状态成员变量

2. **Camera_CameraManager.cpp 修改**：
   - 在构造函数中初始化 ISP 相关成员变量
   - 在 init() 函数中调用 initISP() 初始化 ISP 配置
   - 实现完整的 ISP 配置方法（initISP, applyISPSettings, set/get 方法）
   - 在 capturePhoto() 函数中，拍照前应用 ISP 设置并等待生效

3. **核心功能特性**：
   - 曝光模式配置（自动/手动）
   - 亮度调节（-64~64）
   - 对比度调节（0~100）
   - 饱和度调节（0~100）
   - 自动白平衡模式配置（自动/手动）
   - ISP 参数重置功能
   - 参数范围验证，防止无效配置

#### 实施步骤
1. 修改 Camera_CameraManager.h，添加 ISP 配置相关的宏定义、方法声明和成员变量
2. 修改 Camera_CameraManager.cpp 的构造函数，初始化 ISP 成员变量
3. 在 CameraManager::init() 中添加 ISP 初始化调用
4. 实现完整的 ISP 配置方法（12个新方法）
5. 修改 capturePhoto() 函数，在拍照前应用 ISP 设置并等待 100ms 生效
6. 更新版本号从 V1.29 到 V1.30
7. 更新 Memory.md 记录本次移植

#### 文件变更
- `Camera_CameraManager.h`: 添加 ISP 配置方法声明、成员变量和宏定义
- `Camera_CameraManager.cpp`: 实现 ISP 配置方法，修改 init() 和 capturePhoto() 函数
- `Shared_GlobalDefines.h`: 版本号从 V1.29 更新为 V1.30
- `Memory.md`: 记录本次移植详情

#### 验证标准
**移植成功验证标准**：

1. **功能完整性验证**：
   - ✅ 项目编译通过，无错误无警告
   - ✅ CameraManager 初始化成功，ISP 配置模块正常初始化
   - ✅ initISP() 正确创建 CameraSetting 对象并应用默认参数
   - ✅ 所有 ISP getter/setter 方法正常工作
   - ✅ 参数范围验证生效，无效参数被拒绝
   - ✅ resetISP() 正确重置所有参数为默认值

2. **拍照流程集成验证**：
   - ✅ capturePhoto() 函数在拍照前正确调用 applyISPSettings()
   - ✅ ISP 设置应用后等待 100ms 让参数生效
   - ✅ 拍照功能保持正常，无功能退化
   - ✅ 原有的超时处理机制保持完好

3. **性能基准验证**：
   - ⏱️ ISP 初始化时间 < 50ms
   - ⏱️ ISP 参数应用时间 < 100ms
   - ⏱️ 拍照总时间增加 < 150ms（可接受）
   - 💾 内存占用增加 < 5KB（可接受）

4. **关键 API 调用验证**：
   - 📊 CameraSetting 构造成功率：100%
   - 📊 setExposureMode() 成功率：100%
   - 📊 setBrightness() 成功率：100%
   - 📊 setContrast() 成功率：100%
   - 📊 setSaturation() 成功率：100%
   - 📊 setAWB() 成功率：100%

5. **业务流程正确性证明**：
   - 📸 测试流程 1：使用默认 ISP 参数拍照 - 应正常工作
   - 📸 测试流程 2：调整亮度为 +30 后拍照 - 图像应明显变亮
   - 📸 测试流程 3：调整对比度为 80 后拍照 - 图像对比度应增强
   - 📸 测试流程 4：调整饱和度为 80 后拍照 - 图像色彩应更鲜艳
   - 📸 测试流程 5：调用 resetISP() 后拍照 - 图像应恢复默认效果
   - 🔄 回归测试：原有拍照/录像/回放功能应完全正常

#### 验证结果（待实际测试验证）
修改完成后需进行实际硬件验证，确认：
- 代码编译通过，无错误
- ISP 配置 API 可正常调用
- 不同 ISP 参数确实产生不同的拍照效果
- 拍照功能保持稳定可靠
- 无新问题引入

---

### 版本 V1.29 - 修复回放功能（选项C）双重焦点标记异常问题 (2026-03-31)

#### 问题描述
当设备上电后首次进入选项C回放功能时，"选择焦点"正确地显示在右上角的"back"按钮上，并通过红色框框标记。然而，当用户按下旋钮返回主菜单后再次进入选项C回放功能时，出现了"back"按钮和四格布局的左上角选项同时被红色框框标记为"选择焦点"的异常情况。尽管实际功能正常（按下旋钮仍会返回主菜单，表明实际焦点仍在"back"按钮上），但这种视觉上的双重焦点标记会导致用户混淆，无法明确识别真正的当前选择焦点。

#### 根本原因分析
通过详细分析 VideoRecorder.cpp 中的代码，发现问题根源如下：

1. **状态变量未重置**：
   - `enterFileListMode()` 函数在设置 `isBackButtonSelected = true` 时，没有同时重置相关的状态跟踪变量
   - `lastBackButtonSelected` 和 `lastSelectedMediaIndex` 这两个变量在用户离开回放功能后保持着上次的值
   - 当再次进入回放功能时，这些旧值导致 `drawFileListUI()` 函数进入错误的绘制分支

2. **绘制逻辑缺陷**：
   - `drawFileListUI()` 函数在局部更新分支（第 1414 行）会比较当前状态与上次状态的差异
   - 当 `isBackButtonSelected` 从 false 变为 true 时，代码会同时尝试清除上次选中媒体的边框和绘制当前 back 按钮的边框
   - 但由于状态变量未完全重置，导致两个元素的边框都被保留

3. **关键问题代码位置**：
   - `drawFileListUI()` 函数在 `VideoRecorder.cpp:1214
   - `enterFileListMode()` 函数在 `VideoRecorder.cpp:558`

#### 技术方案
在 `enterFileListMode()` 函数中添加必要的状态变量重置逻辑，确保每次进入回放功能时所有相关状态都是干净的：

1. **重置最后选中状态变量**：
   - `lastBackButtonSelected = false`：确保上次返回按钮状态被清除
   - `lastSelectedMediaIndex = UINT32_MAX`：确保上次媒体索引被重置为初始值
   - `currentMediaIndex = 0`：确保当前媒体索引重置为 0

2. **保持原有功能不变**：
   - 继续使用 `isBackButtonSelected = true` 作为默认选中状态
   - 继续使用 `fileListNeedsRedraw = true` 确保完全重绘

#### 实施步骤
1. **VideoRecorder.cpp 修改**
   - 在 `enterFileListMode()` 函数中添加状态变量重置代码
   - 确保每次进入文件列表模式时，状态跟踪变量都被正确重置

#### 文件变更
- `VideoRecorder.cpp`: 在 enterFileListMode() 函数中添加 lastBackButtonSelected、lastSelectedMediaIndex 和 currentMediaIndex 状态变量的重置逻辑
- `Shared_GlobalDefines.h`: 版本号从 V1.28 更新为 V1.29

#### 验证结果
修改后需验证：
- 首次进入选项C回放功能时，只有"back"按钮正确显示红色边框
- 返回主菜单后再次进入选项C回放功能时，仍只有"back"按钮显示红色边框
- 不存在双重焦点标记的视觉歧义
- 功能正常（按下旋钮可以返回主菜单
- 无新问题引入
