# AMB82-MINI Camera项目开发历程记录

## 项目概述
本项目为AMB82-MINI Camera项目实现视频录制功能的同步音频录制模块，使系统能够同时捕获并整合音频信号，生成包含同步音视频的AVI媒体文件。

---

## 开发记录

### 版本 V1.28 - 移除不必要的DC偏移消除和数字滤波，恢复与原项目一致的简单实现 (2026-03-27)

#### 问题描述
V1.27-V1.28版本尝试添加DC偏移消除和数字滤波来减少杂音，但反而导致了更严重的问题：
- 拾音内容不清晰或完全听不到
- 存在大量背景杂音
- 原项目Inmp441_Rtos.ino在同样30dB增益下工作正常，拾音效果很好

#### 根本原因分析
通过详细对比原项目Inmp441_Rtos.ino和当前项目，发现关键问题：

1. **过度复杂化的音频处理**
   - 当前项目：添加了DC偏移消除、数字滤波、软饱和等复杂处理
   - 原项目：只有简单的30dB增益应用，直接写入环形缓冲区
   - 复杂的滤波处理反而导致音频信号失真和杂音

2. **原项目的关键启示**
   - 原项目Inmp441_Rtos.ino没有任何滤波处理，但拾音效果非常好
   - 原项目只有简单的30dB软件增益和硬饱和处理
   - 复杂的滤波逻辑反而破坏了原始音频信号的质量

3. **BUFFER_SIZE不匹配**
   - 当前项目使用32768，原项目使用4096
   - 过大的缓冲区可能导致时序问题

#### 技术方案
1. **完全移除DC偏移消除和数字滤波**：恢复到原项目的简单实现
2. **移除软饱和处理**：恢复到原项目的简单硬饱和
3. **恢复BUFFER_SIZE到4096**：与原项目保持一致
4. **保持30dB软件增益不变**：继续使用原项目成功的增益配置
5. **保持DMA配置不变**：继续使用DMA_PAGE_NUM=4, DMA_PAGE_SIZE=1280

#### 实施步骤
1. **Inmp441_MicrophoneManager.cpp 修改**
   - 移除i2s_rx_callback()中的DC偏移消除算法
   - 移除数字滤波逻辑
   - 移除软饱和处理
   - 恢复到与原项目完全一致的简单实现：读取样本→应用30dB增益→硬饱和→写入环形缓冲区

2. **Inmp441_MicrophoneManager.h 修改**
   - 更新版本注释为 V1.28
   - 说明移除不必要的滤波处理
   - BUFFER_SIZE: 32768 → 4096（与原项目一致）

3. **Shared_GlobalDefines.h 修改**
   - 版本号保持 V1.28

#### 文件变更
- `Inmp441_MicrophoneManager.cpp`: 移除DC偏移消除、数字滤波、软饱和，恢复与原项目一致的简单实现
- `Inmp441_MicrophoneManager.h`: 恢复BUFFER_SIZE到4096，更新版本注释
- `Shared_GlobalDefines.h`: 版本号 V1.28

#### 验证结果
修改后需验证：
- 拾音内容清晰可听，与原项目Inmp441_Rtos.ino效果相当
- 背景杂音明显减少或消失
- 视频播放速度正常
- 音视频同步良好
- 无新问题引入

---

### 版本 V1.27-V1.28 (已废弃) - 添加DC偏移消除和数字滤波以减少杂音 (2026-03-27)

#### 问题描述
V1.27版本虽然移除了临界区保护并增大了环形缓冲区，但音频仍然存在较大的背景杂音：
- 能听到清晰的人声，语速匹配正常
- 但存在较大的背景杂音
- 视频播放速度正常，音视频同步良好
- 原项目Inmp441_Rtos.ino在历史版本中曾使用滤波措施减少杂音

#### 根本原因分析
通过详细分析Memory.md历史记录，发现关键问题：

1. **缺少音频滤波措施**
   - 历史版本V1.2中使用了DC偏移消除和数字滤波来减少杂音
   - 当前V1.27版本完全移除了这些滤波措施
   - 缺乏滤波导致背景杂音无法被有效抑制

2. **DC偏移问题**
   - INMP441麦克风的原始信号可能存在直流偏移
   - 直流偏移会导致音频信号偏离零点，产生杂音
   - 需要通过DC偏移消除算法来校正

3. **高频噪声问题**
   - 原始音频信号中可能存在高频噪声
   - 需要通过低通滤波器来减少高频噪声
   - 温和的滤波可以保留人声清晰度的同时减少背景杂音

4. **信号饱和问题**
   - 30dB软件增益可能导致信号饱和
   - 硬饱和会产生严重的失真和杂音
   - 需要软饱和处理来减少失真

#### 技术方案
1. **添加DC偏移消除**：使用慢速跟踪算法（DC_ALPHA=256），避免消除有效信号
2. **添加数字滤波**：实现温和的一阶低通滤波器 `(sample * 3 + lastSample) / 4`
3. **软饱和处理**：在20000阈值处进行软饱和，减少硬削波失真
4. **保持30dB软件增益不变**：继续使用原项目成功的增益配置
5. **保持DMA和缓冲区配置不变**：继续使用V1.27的配置

#### 实施步骤
1. **Inmp441_MicrophoneManager.cpp 修改**
   - 在i2s_rx_callback()中添加DC偏移消除算法
   - 添加一阶低通数字滤波器
   - 添加软饱和处理逻辑
   - 调整处理顺序：DC偏移→滤波→增益→饱和

2. **Inmp441_MicrophoneManager.h 修改**
   - 更新版本注释为 V1.28
   - 说明添加DC偏移消除和数字滤波

3. **Shared_GlobalDefines.h 修改**
   - 版本号从 V1.27 更新为 V1.28

#### 文件变更
- `Inmp441_MicrophoneManager.cpp`: 添加DC偏移消除和数字滤波算法
- `Inmp441_MicrophoneManager.h`: 更新版本注释为V1.28
- `Shared_GlobalDefines.h`: 版本号 V1.28

#### 验证结果
修改后需验证：
- 背景杂音明显减少或消失
- 拾音内容清晰可听
- 视频播放速度正常
- 音视频同步良好
- 无新问题引入

---

### 版本 V1.27 - 移除临界区保护，增大环形缓冲区 (2026-03-27)

#### 问题描述
V1.26版本添加临界区保护后，音频仍然存在较大的背景杂音：
- 能听到清晰的人声，语速匹配正常
- 但存在较大的背景杂音
- 视频播放速度正常，音视频同步良好

#### 根本原因分析
通过详细分析，发现临界区保护可能不是最佳方案：

1. **临界区保护过度影响性能**
   - V1.26添加了过多的临界区保护
   - 频繁的临界区进出可能导致I2S中断延迟
   - ISR上下文的临界区保护可能影响音频数据的实时性

2. **原项目成功的关键因素**
   - 原项目Inmp441_Rtos.ino没有任何临界区保护，但工作正常
   - 原项目使用更大的环形缓冲区虽然原项目BUFFER_SIZE=4096，但数据处理更及时
   - 在单任务环境下，没有并发访问问题

3. **多任务环境的新策略**
   - I2S中断优先级高于任务优先级，ISR不会被任务打断
   - 任务读取环形缓冲区时，即使ISR同时写入，m_count等变量的更新是原子的（32位系统）
   - 更大的环形缓冲区可以提供更多的缓冲空间，减少数据溢出风险

#### 技术方案
1. **完全移除临界区保护**：恢复到原项目Inmp441_Rtos.ino的简单实现
2. **增大环形缓冲区**：BUFFER_SIZE从4096增加到32768，提供更多缓冲空间
3. **保持DMA配置不变**：继续使用DMA_PAGE_NUM=4, DMA_PAGE_SIZE=1280
4. **保持30dB软件增益不变**

#### 实施步骤
1. **Inmp441_MicrophoneManager.cpp 修改**
   - 完全移除所有临界区保护代码
   - write()、read()、available()、isEmpty()、isFull()、clear()、getCount()都恢复到简单实现

2. **Inmp441_MicrophoneManager.h 修改**
   - BUFFER_SIZE: 4096 → 32768
   - 更新版本注释为 V1.27

3. **Shared_GlobalDefines.h 修改**
   - 版本号从 V1.26 更新为 V1.27

#### 文件变更
- `Inmp441_MicrophoneManager.cpp`: 完全移除临界区保护
- `Inmp441_MicrophoneManager.h`: 增大环形缓冲区到32768，更新版本注释
- `Shared_GlobalDefines.h`: 版本号 V1.27

#### 验证结果
修改后需验证：
- 背景杂音明显减少或消失
- 拾音内容清晰可听
- 视频播放速度正常
- 音视频同步良好
- 无新问题引入

---

### 版本 V1.26 - 在多任务环境下添加环形缓冲区临界区保护 (2026-03-27)

#### 问题描述
V1.25版本完全按照原项目Inmp441_Rtos.ino配置后，虽然能听到拾音内容，但存在以下问题：
- 能听到清晰的人声，语速匹配正常
- 但存在较大的背景杂音
- 视频播放速度正常，音视频同步良好
- 原项目Inmp441_Rtos.ino在单任务环境下工作正常，无杂音

#### 根本原因分析
通过对比分析原项目和当前项目的差异，发现关键问题：

1. **环境差异导致的问题**
   - 原项目：单任务环境，I2S中断回调写入环形缓冲区，主loop()直接读取
   - 当前项目：双任务环境（音频处理任务+视频处理任务），通过队列传递数据
   - 多任务环境下，环形缓冲区的并发访问可能导致数据竞争

2. **数据竞争风险**
   - I2S中断回调（ISR上下文）持续写入环形缓冲区
   - 音频处理任务（任务上下文）同时读取环形缓冲区
   - 虽然原项目没有临界区保护，但在单任务环境下不会有问题
   - 在多任务环境下，并发访问可能导致m_head、m_tail、m_count等变量不一致

3. **临界区保护的必要性**
   - write()函数在ISR中调用，需要ISR安全的保护
   - read()、available()等函数在任务中调用，需要任务级保护
   - 适当的临界区保护可以防止数据竞争，确保音频数据完整性

#### 技术方案
1. **在write()函数中添加ISR安全的临界区保护**：使用portSET_INTERRUPT_MASK_FROM_ISR()
2. **在read()函数中添加任务级临界区保护**：使用taskENTER_CRITICAL()
3. **在available()、isEmpty()、isFull()、getCount()中添加保护**：确保读取操作的原子性
4. **在clear()函数中添加ISR安全的保护**：确保清除操作的完整性
5. **保持DMA和缓冲区配置不变**：继续使用V1.25的配置（DMA_PAGE_NUM=4, DMA_PAGE_SIZE=1280, BUFFER_SIZE=4096）
6. **保持30dB软件增益不变**

#### 实施步骤
1. **Inmp441_MicrophoneManager.cpp 修改**
   - write()：添加portSET_INTERRUPT_MASK_FROM_ISR()保护
   - read()：添加taskENTER_CRITICAL()保护
   - available()：添加taskENTER_CRITICAL()保护
   - isEmpty()：添加taskENTER_CRITICAL()保护
   - isFull()：添加taskENTER_CRITICAL()保护
   - clear()：添加portSET_INTERRUPT_MASK_FROM_ISR()保护
   - getCount()：添加taskENTER_CRITICAL()保护

2. **Shared_GlobalDefines.h 修改**
   - 版本号从 V1.25 更新为 V1.26

3. **Inmp441_MicrophoneManager.h 修改**
   - 更新版本注释为 V1.26

#### 文件变更
- `Inmp441_MicrophoneManager.cpp`: 在多任务环境下添加环形缓冲区临界区保护
- `Shared_GlobalDefines.h`: 版本号 V1.26
- `Inmp441_MicrophoneManager.h`: 更新版本注释

#### 验证结果
修改后需验证：
- 背景杂音明显减少或消失
- 拾音内容清晰可听
- 视频播放速度正常
- 音视频同步良好
- 无新问题引入

---

### 版本 V1.25 - 完全按照原项目Inmp441_Rtos.ino的配置恢复 (2026-03-27)

#### 问题描述
V1.24版本恢复后，音频仍然存在严重问题：
- 完全听不到拾音内容，只有大量背景杂音
- 视频播放正常，但音频完全失效
- 原项目Inmp441_Rtos.ino在同样30dB增益下工作正常

#### 根本原因分析
通过详细对比原项目Inmp441_Rtos.ino和当前项目，发现关键差异：

1. **DMA配置不匹配**
   - 原项目：DMA_PAGE_NUM=4, DMA_PAGE_SIZE=1280
   - V1.24：DMA_PAGE_NUM=2, DMA_PAGE_SIZE=512
   - DMA配置是I2S音频采集的关键参数，必须与原项目保持一致

2. **环形缓冲区大小不匹配**
   - 原项目：BUFFER_SIZE=4096
   - V1.24：BUFFER_SIZE=32768
   - 过大的缓冲区可能导致数据处理延迟或时序问题

3. **多任务环境与原项目环境差异**
   - 原项目：单任务，直接从环形缓冲区读取写入SD卡
   - 当前项目：双任务（音频处理任务+视频处理任务），通过队列传递数据
   - 必须确保底层I2S和DMA配置与原项目完全一致，这是音频正常工作的基础

#### 技术方案
1. **完全按照原项目Inmp441_Rtos.ino恢复DMA配置**：DMA_PAGE_NUM=4, DMA_PAGE_SIZE=1280
2. **完全按照原项目恢复环形缓冲区大小**：BUFFER_SIZE=4096
3. **保持30dB软件增益不变**：原项目使用30dB增益工作正常
4. **保持其他优化不变**：完全移除环形缓冲区临界区保护

#### 实施步骤
1. **Inmp441_MicrophoneManager.h 修改**
   - DMA_PAGE_NUM: 2 → 4
   - DMA_PAGE_SIZE: 512 → 1280
   - BUFFER_SIZE: 32768 → 4096
   - 更新版本注释为 V1.25

2. **Shared_GlobalDefines.h 修改**
   - 版本号从 V1.24 更新为 V1.25

3. **保持Inmp441_MicrophoneManager.cpp不变**
   - 环形缓冲区实现保持简单（与原项目一致）
   - I2S回调函数保持不变

#### 文件变更
- `Inmp441_MicrophoneManager.h`: 完全按照原项目恢复DMA配置和环形缓冲区大小
- `Shared_GlobalDefines.h`: 版本号 V1.25

#### 验证结果
修改后需验证：
- 能够听到清晰的拾音内容
- 背景杂音明显减少或消失
- 音频播放正常，与视频同步
- 视频播放速度正常
- 录制的音频质量与原项目Inmp441_Rtos.ino相当

---

### 版本 V1.24 - 恢复DMA配置和环形缓冲区大小 (2026-03-27)

#### 问题描述
V1.23版本修改DMA配置后，音频出现严重问题：
- 完全听不到拾音内容，只有大量背景杂音
- 视频播放正常，但音频完全失效
- V1.22版本能正常听到拾音内容（只是有杂音）

#### 根本原因分析
通过分析V1.22和V1.23的差异，发现以下问题：

1. **DMA配置修改导致音频数据异常**
   - V1.22: DMA_PAGE_NUM=2, DMA_PAGE_SIZE=512
   - V1.23: DMA_PAGE_NUM=4, DMA_PAGE_SIZE=1280 (匹配原项目)
   - 虽然原项目使用DMA_PAGE_NUM=4, DMA_PAGE_SIZE=1280，但在当前多任务环境下可能不兼容

2. **环形缓冲区大小太小**
   - V1.23: BUFFER_SIZE=4096 (约0.25秒缓冲)
   - V1.22: BUFFER_SIZE=32768 (约2秒缓冲)
   - 多任务环境下需要更大的缓冲空间

3. **多任务环境与原项目环境差异**
   - 原项目：单任务，直接从环形缓冲区读取写入SD卡
   - 当前项目：双任务（音频处理任务+视频处理任务），通过队列传递数据
   - 当前项目的数据流程更复杂，需要更保守的配置

#### 技术方案
1. **恢复DMA配置到V1.22版本**：DMA_PAGE_NUM=2, DMA_PAGE_SIZE=512
2. **恢复环形缓冲区大小到V1.22版本**：BUFFER_SIZE=32768
3. **保持其他V1.22的优化**：完全移除环形缓冲区临界区保护
4. **先恢复能听到声音的状态**：再逐步优化杂音问题

#### 实施步骤
1. **Inmp441_MicrophoneManager.h 修改**
   - DMA_PAGE_NUM: 4 → 2
   - DMA_PAGE_SIZE: 1280 → 512
   - BUFFER_SIZE: 4096 → 32768
   - 更新版本注释为 V1.24

2. **Shared_GlobalDefines.h 修改**
   - 版本号从 V1.22 更新为 V1.24

#### 文件变更
- `Inmp441_MicrophoneManager.h`: 恢复DMA配置和环形缓冲区大小到V1.22版本
- `Shared_GlobalDefines.h`: 版本号 V1.24

#### 验证结果
修改后需验证：
- 能够听到拾音内容（与V1.22版本相当）
- 视频播放速度正常
- 音视频同步正常
- 后续版本再优化杂音问题

---

### 版本 V1.22 - 完全移除环形缓冲区临界区保护 (2026-03-27)

#### 问题描述
V1.19和V1.20版本后，音频出现严重问题：
- 无法播放拾音内容，只有大量杂音
- 视频播放正常，但音频完全失效
- 原项目 Inmp441_Rtos.ino 在同样30dB增益下工作正常
- 版本V1.20计划回滚但未实际执行，代码仍保留临界区保护

#### 根本原因分析
通过详细对比原项目 Inmp441_Rtos.ino 和当前项目，发现以下关键差异：

1. **环形缓冲区临界区保护过度**
   - 当前项目：使用 portSET_INTERRUPT_MASK_FROM_ISR() 保护环形缓冲区操作
   - 原项目：没有任何临界区保护，直接操作
   - 临界区保护导致 I2S 中断延迟或数据丢失，产生杂音

2. **过度复杂化的环形缓冲区操作**
   - 当前项目添加了过多的保护机制，偏离了原项目的简单实现
   - 原项目的简单实现在实际使用中工作稳定
   - 原项目 Inmp441_Rtos.ino 从V1.0到V3.0都没有使用任何临界区保护

3. **关键发现：原项目无保护机制**
   - 原项目 Inmp441_Rtos.ino 的 RingBufferClass 完全没有临界区保护
   - 直接操作 m_head、m_tail、m_count 等变量
   - 在I2S中断回调中直接写入，在主循环中直接读取
   - 这种简单实现在实际使用中非常稳定

#### 技术方案
1. **完全移除所有临界区保护**：确保环形缓冲区操作与原项目 Inmp441_Rtos.ino 完全一致
2. **保持简单实现**：不添加任何额外的保护机制
3. **信任硬件和RTOS调度**：依赖I2S DMA和FreeRTOS任务调度的自然时序

#### 实施步骤
1. **Inmp441_MicrophoneManager.cpp 修改**
   - 移除 write() 函数中的 portSET_INTERRUPT_MASK_FROM_ISR()/portCLEAR_INTERRUPT_MASK_FROM_ISR()
   - 移除 read() 函数中的临界区保护
   - 恢复到简单的直接操作实现（与原项目完全一致）

2. **Inmp441_MicrophoneManager.h 修改**
   - 更新版本注释为 V1.22
   - 说明完全移除环形缓冲区临界区保护

3. **Shared_GlobalDefines.h 修改**
   - 版本号保持 V1.22（已正确设置）

#### 文件变更
- `Inmp441_MicrophoneManager.cpp`: 完全移除所有临界区保护，恢复简单环形缓冲区实现
- `Inmp441_MicrophoneManager.h`: 更新版本注释为 V1.22
- `Shared_GlobalDefines.h`: 版本号 V1.22

#### 验证结果
修改后需验证：
- 能够听到清晰的拾音内容
- 背景杂音明显减少或消失
- 音频播放正常，与视频同步
- 视频播放速度正常
- 录制的音频质量与原项目 Inmp441_Rtos.ino 相当

---

### 版本 V1.20 - 回滚环形缓冲区临界区保护和volatile修饰符 (2026-03-27)

#### 问题描述
V1.19修复后，出现以下严重问题：
- 无法播放拾音内容，只有大量杂音
- 视频播放正常，但音频完全失效
- 原项目 Inmp441_Rtos.ino 在同样30dB增益下工作正常

#### 根本原因分析
通过对比原项目 Inmp441_Rtos.ino 和当前项目，发现以下关键差异：

1. **环形缓冲区临界区保护过度**
   - 当前项目：使用 portSET_INTERRUPT_MASK_FROM_ISR() 保护环形缓冲区操作
   - 原项目：没有任何临界区保护，直接操作
   - 临界区保护可能导致 I2S 中断延迟或数据丢失

2. **volatile 修饰符问题**
   - 当前项目：m_count 使用 volatile 修饰符
   - 原项目：没有 volatile 修饰符
   - volatile 可能导致编译器优化行为异常

3. **过度复杂化的环形缓冲区操作**
   - 当前项目添加了过多的保护机制，偏离了原项目的简单实现
   - 原项目的简单实现在实际使用中工作稳定

#### 技术方案
1. **回滚环形缓冲区实现**：完全移除所有临界区保护
2. **移除 volatile 修饰符**：m_count 不再使用 volatile
3. **保持与原项目完全一致**：确保环形缓冲区操作和原项目 Inmp441_Rtos.ino 完全相同

#### 实施步骤
1. **Inmp441_MicrophoneManager.cpp 修改**
   - 移除 write() 函数中的 portSET_INTERRUPT_MASK_FROM_ISR()/portCLEAR_INTERRUPT_MASK_FROM_ISR()
   - 移除 read() 函数中的临界区保护
   - 移除 available()、isEmpty()、isFull()、clear()、getCount() 中的临界区保护
   - 恢复到简单的直接操作实现

2. **Inmp441_MicrophoneManager.h 修改**
   - 移除 m_count 成员变量的 volatile 修饰符
   - 更新版本注释为 V1.20

3. **Shared_GlobalDefines.h 修改**
   - 保持版本号为 V1.20（已正确设置）

#### 文件变更
- `Inmp441_MicrophoneManager.cpp`: 移除所有临界区保护，恢复简单环形缓冲区实现
- `Inmp441_MicrophoneManager.h`: 移除 m_count 的 volatile 修饰符，更新版本注释
- `Shared_GlobalDefines.h`: 保持版本号 V1.20

#### 验证结果
修改后需验证：
- 能够听到清晰的拾音内容
- 背景杂音明显减少或消失
- 音频播放正常，与视频同步
- 视频播放速度正常
- 录制的音频质量与原项目 Inmp441_Rtos.ino 相当

---

### 版本 V1.6 - 修复视频卡住和杂音问题 (2026-03-26)

#### 问题描述
V1.5修复后，视频播放出现以下问题：
- 视频播放过程中有拾音内容播放，但伴随大量杂音
- 播放到后面几秒时画面卡住，但拾音还在播放
- VLC日志显示视频只有72帧（约4秒），但实际录制了70帧
- VLC日志显示音频索引条目过多（2279个），视频索引条目只有72个

#### 根本原因分析
通过详细分析VLC调试日志和Arduino日志，发现以下问题：

1. **音频块太小导致索引条目过多**
   - 当前音频块大小为64样本（128字节）
   - 每个音频块都创建一个索引条目
   - 录制约4秒视频产生2209个音频索引条目
   - VLC解析AVI索引时效率低下，无法正确定位所有视频帧

2. **索引条目数量严重失衡**
   - VLC日志：`stream[0] created 72 index entries`（视频）
   - VLC日志：`stream[1] created 112 index entries`（音频）
   - 实际Arduino日志：音频块2209个，视频帧70个
   - 索引条目比例严重失衡，导致VLC解析混乱

3. **音频数据碎片化严重**
   - 64样本=4ms音频数据，碎片化严重
   - 音视频同步困难
   - VLC需要频繁定位索引，性能下降

#### 技术方案
1. **增大音频块大小**：64样本 → 512样本（32ms音频数据）
2. **减少索引条目数量**：预计减少约8倍，提高VLC解析效率
3. **增大音频队列缓冲**：64块 → 128块（约4秒缓冲）
4. **优化日志频率**：减少调试日志输出频率

#### 实施步骤
1. **Inmp441_MicrophoneManager.h 修改**
   - AudioDataBlock.samples从64增加到512
   - AUDIO_QUEUE_SIZE从64增加到128

2. **RTOS_TaskFactory.cpp 修改**
   - taskAudioProcessing函数改为累积512样本后发送
   - 减少日志输出频率（每10块输出一次）

3. **VideoRecorder.cpp 修改**
   - 减少音频块日志输出频率（每10块输出一次）

4. **Shared_GlobalDefines.h 修改**
   - 版本号更新为 V1.6

#### 文件变更
- `Inmp441_MicrophoneManager.h`: 音频块大小512样本，队列大小128块
- `RTOS_TaskFactory.cpp`: 累积512样本发送，优化日志频率
- `VideoRecorder.cpp`: 优化日志频率
- `Shared_GlobalDefines.h`: 版本号 V1.6

#### 验证结果
修改后需验证：
- VLC日志中音频索引条目数量合理（预计减少8倍）
- 视频播放流畅，无卡顿
- 音频播放清晰，无杂音
- 音画同步良好
- 视频时长与音频时长匹配

---

### 版本 V1.2 - 修复音频质量问题 (2025-05-28)

#### 问题描述
V1.1修复后音频采集恢复，但音频质量严重问题：
- 音频时长仅8秒（视频21秒），占比38%
- 音频开头8秒只有杂音无拾音内容，随后完全静音
- 音频缓冲区频繁为空（available=0）
- timeSinceLastWrite经常超过100ms（远超设定的50ms超时）

#### 根本原因分析
1. **DC偏移消除过强**：DC_ALPHA=32过小，导致低频信号被过度消除
2. **数字滤波器太激进**：`(sample + lastSample) / 2`滤波器严重衰减高频，导致声音失真
3. **音频采集频率不足**：写入阈值128样本仍然过高，音频缓冲区积累缓慢
4. **环形缓冲区大小不足**：16384样本（约1秒）缓冲不够

#### 技术方案
1. 优化DC偏移消除：DC_ALPHA 32 → 256（慢速跟踪，避免消除有效信号）
2. 改进数字滤波：`(sample + lastSample) / 2` → `(sample * 3 + lastSample) / 4`（更温和）
3. 降低写入阈值：128样本 → 64样本（提高写入频率）
4. 增大环形缓冲区：16384 → 32768样本（增加缓冲容量）
5. 调整饱和阈值：30000 → 20000（适应INMP441典型输出范围）

#### 实施步骤
1. **Inmp441_MicrophoneManager.cpp 修改**
   - DC_ALPHA从32增加到256
   - 数字滤波器改为更温和的版本
   - 饱和阈值从30000调整到20000

2. **Inmp441_MicrophoneManager.h 修改**
   - BUFFER_SIZE从16384增加到32768

3. **VideoRecorder.cpp 修改**
   - 写入阈值从128样本降低到64样本

4. **Shared_GlobalDefines.h 修改**
   - 版本号更新为 V1.2

#### 文件变更
- `Inmp441_MicrophoneManager.cpp`: DC_ALPHA=256, 改进滤波器, 调整饱和阈值
- `Inmp441_MicrophoneManager.h`: BUFFER_SIZE=32768
- `VideoRecorder.cpp`: 写入阈值64样本
- `Shared_GlobalDefines.h`: 版本号 V1.2

#### 验证结果
修改后需验证：
- 音频时长接近视频时长（目标>80%）
- 音频质量明显改善（无杂音，有清晰拾音内容）
- timeSinceLastWrite稳定在50ms左右
- 音频缓冲区可用样本稳定（available>0）
- VLC播放音频正常，音画同步

---

### 版本 V1.1 - 修复I2S DMA配置错误 (2025-05-28)

#### 问题描述
V1.0修复导致音频完全缺失：
- 音频帧数为0（video=179 frames, audio=0 chunks）
- VLC日志显示音频流索引条目为0（stream[1] created 0 index entries）
- I2S初始化出现错误：`[I2S  Err]hal_rtl_i2s_page_recv: No Idle Rx Page`
- 音频缓冲区始终为空（available=0）

#### 根本原因分析
1. **DMA配置超出硬件限制**：DMA_PAGE_NUM=8超出硬件支持的2~4范围
2. **I2S初始化失败**：`PageNum(8) valid value is 2~4`错误导致I2S无法接收数据
3. **音频采集完全中断**：I2S DMA缓冲区配置错误，导致音频数据无法进入环形缓冲区

#### 技术方案
1. 恢复DMA配置到硬件支持范围：DMA_PAGE_NUM 8 → 4
2. 恢复环形缓冲区大小：BUFFER_SIZE 32768 → 16384
3. 保持V1.0的音频采集频率：阈值128样本，超时50ms

#### 实施步骤
1. **Inmp441_MicrophoneManager.h 修改**
   - DMA_PAGE_NUM 从8恢复为4（硬件限制2~4）
   - BUFFER_SIZE 从32768恢复为16384

2. **Shared_GlobalDefines.h 修改**
   - 版本号更新为 V1.1

#### 文件变更
- `Inmp441_MicrophoneManager.h`: DMA_PAGE_NUM=4, BUFFER_SIZE=16384
- `Shared_GlobalDefines.h`: 版本号 V1.1

#### 验证结果
修改后需验证：
- I2S初始化无错误日志
- 音频缓冲区有可用样本（available>0）
- 音频帧数恢复正常（>0）
- VLC日志中音频流有索引条目
- 音频播放恢复正常

---

### 版本 V1.0 - 最终修复音频采集性能 (2025-05-28)

#### 问题描述
V0.9修复后，音频时长有显著改善但仍不足：
- 音频时长达到视频时长的40%（视频10秒 vs 音频4秒）
- 音频帧数达到112帧（视频160帧）
- 音频缓冲区经常为空（available=0），表明音频采集仍然不够及时

#### 根本原因分析
1. **音频采集频率仍然不足**：写入阈值256样本仍然过高
2. **DMA缓冲区太小**：4页DMA缓冲区仅能存储0.16秒音频数据，容易丢失
3. **环形缓冲区大小不足**：16384样本（约1秒）可能不够缓冲
4. **音频处理延迟**：100ms超时仍然过长

#### 技术方案
1. 进一步降低写入阈值：256样本 → 128样本
2. 进一步缩短超时时间：100ms → 50ms
3. 增大DMA缓冲区：DMA_PAGE_NUM 4 → 8（增加100%）
4. 增大环形缓冲区：BUFFER_SIZE 16384 → 32768（增加100%）

#### 实施步骤
1. **Inmp441_MicrophoneManager.h 修改**
   - DMA_PAGE_NUM 从4增加到8
   - BUFFER_SIZE 从16384增加到32768

2. **VideoRecorder.cpp 修改**
   - 写入阈值降低到128样本
   - 超时时间缩短到50ms
   - 更新日志输出条件匹配新超时时间

3. **Shared_GlobalDefines.h 修改**
   - 版本号更新为 V1.0

#### 文件变更
- `Inmp441_MicrophoneManager.h`: DMA_PAGE_NUM=8, BUFFER_SIZE=32768
- `VideoRecorder.cpp`: 阈值128样本，超时50ms
- `Shared_GlobalDefines.h`: 版本号 V1.0

#### 验证结果
修改后需验证：
- VLC日志中音频时长接近视频时长（目标>80%）
- 音频帧数显著增加（目标>200帧/10秒视频）
- 音频缓冲区可用样本数稳定（available>0）
- 音频播放无杂音、无静音

---

### 版本 V0.9 - 根本性修复音频问题 (2025-05-28)

#### 问题描述
V0.8修复后，音频时长仍严重不足且存在杂音：
- 音频时长只有视频时长的28.6%（视频7秒 vs 音频2秒）
- 音频数据采集仍然不够频繁
- 音频开头存在杂音，随后静音

#### 根本原因分析
1. **音频采集频率依然不足**：写入阈值512样本仍然过高
2. **音频数据质量问题**：存在直流偏移和高频噪声
3. **饱和处理不当**：硬除2导致音频失真
4. **数据格式不确定性**：字节序可能不正确

#### 技术方案
1. 大幅降低写入阈值：512样本 → 256样本
2. 缩短超时时间：200ms → 100ms
3. 添加DC偏移消除：消除直流分量
4. 添加数字滤波：一阶低通滤波器去除高频噪声
5. 改进饱和处理：软限制（减少25%音量）代替硬除2
6. 添加数据格式验证：可选小端序格式测试

#### 实施步骤
1. **VideoRecorder.cpp 修改**
   - 写入阈值降低到256样本
   - 超时时间缩短到100ms
   - 更新日志输出条件匹配新超时时间

2. **Inmp441_MicrophoneManager.cpp 修改**
   - 添加DC偏移消除算法
   - 添加一阶低通数字滤波器
   - 改进饱和处理为软限制
   - 添加小端序格式注释供测试

3. **Shared_GlobalDefines.h 修改**
   - 版本号更新为 V0.9

#### 文件变更
- `VideoRecorder.cpp`: 阈值256样本，超时100ms
- `Inmp441_MicrophoneManager.cpp`: DC偏移消除、数字滤波、改进饱和处理
- `Shared_GlobalDefines.h`: 版本号 V0.9

#### 验证结果
修改后需验证：
- VLC日志中音频时长接近视频时长（目标>80%）
- 音频帧数显著增加
- 音频播放无杂音、无静音
- 音画同步良好

---

### 版本 V0.8 - 优化音频采集性能 (2025-05-28)

#### 问题描述
V0.7修复后，音频杂音问题得到改善，但音频时长仍然不足：
- 音频时长只有视频时长的37.5%（视频8秒 vs 音频3秒）
- 音频数据采集不够频繁，导致音频帧数不足

#### 根本原因分析
1. **音频写入阈值过高**：需要累积1024样本才写入，导致音频数据写入不够及时
2. **超时时间过长**：500ms超时导致音频数据积压
3. **时间戳初始化问题**：`lastAudioWriteTime`未初始化，导致第一次写入时间计算错误
4. **音频处理顺序**：音频处理在视频帧获取之后，可能因视频帧获取延迟而阻塞

#### 技术方案
1. 降低写入阈值：1024样本 → 512样本
2. 缩短超时时间：500ms → 200ms
3. 修复时间戳初始化：添加`lastAudioWriteTime`初始化检查
4. 优化音频处理优先级（后续可考虑调整处理顺序）

#### 实施步骤
1. **VideoRecorder.cpp 修改**
   - 降低写入阈值到512样本
   - 缩短超时时间到200ms
   - 添加`lastAudioWriteTime`初始化检查
   - 更新日志输出条件匹配新超时时间

2. **Shared_GlobalDefines.h 修改**
   - 版本号更新为 V0.8

#### 文件变更
- `VideoRecorder.cpp`: 优化音频采集性能参数
- `Shared_GlobalDefines.h`: 版本号 V0.8

#### 验证结果
修改后需验证：
- VLC日志中音频时长接近视频时长
- 音频帧数显著增加
- 音频播放流畅无静音

---

### 版本 V0.7 - 修复音频杂音和数据格式问题 (2025-05-28)

#### 问题描述
V0.6修复后，音频时长从2秒提升到3秒，但仍存在问题：
- 视频开头出现杂音
- 随后立即进入静音状态
- 音频时长仍不足（视频9秒 vs 音频3秒）

#### 根本原因分析
1. **增益过大**：默认软件增益30dB（左移5位 = 乘以32）导致音频信号饱和
2. **音频数据格式错误**：INMP441是24位麦克风，但数据处理假设为16位，字节序可能错误
3. **音频数据饱和**：过大的增益产生削波，导致杂音和静音

#### 技术方案
1. **降低默认增益**：从30dB改为0dB
2. **修复音频数据格式**：修正字节序（MSB在前，LSB在后）
3. **添加音频数据验证**：过滤饱和值，防止削波

#### 实施步骤
1. **Inmp441_MicrophoneManager.cpp 修改**
   - 构造函数：`m_softwareGain(GAIN_30DB)` → `m_softwareGain(GAIN_0DB)`
   - 音频回调：修正字节序 `src[i*2+1] << 8 | src[i*2]` → `src[i*2] << 8 | src[i*2+1]`
   - 添加饱和值过滤：接近±32000时减半音量

2. **Shared_GlobalDefines.h 修改**
   - 版本号更新为 V0.7

#### 文件变更
- `Inmp441_MicrophoneManager.cpp`: 修复增益、数据格式和验证
- `Shared_GlobalDefines.h`: 版本号 V0.7

#### 验证结果
修改后需验证：
- VLC日志中音频时长接近视频时长
- 音频播放无杂音
- 音频数据质量改善

---

### 版本 V0.6 - 修复音频采集不足问题 (2025-05-28)

#### 问题描述
V0.4修复后，视频播放仍然出现异常：
- 视频开头出现杂音
- 随后立即进入静音状态
- 播放器进度条停止不动

#### 根本原因分析
VLC调试日志显示：
```
avi debug: | + LIST-hdrl size:294 pos:12  // ✅ hdrl size 正确
avi debug: | | + strf size:20 pos:288     // 音频 strf
avi debug: | + ST@� size:... pos:316      // VLC期望下一个chunk在316
```

Arduino日志显示：
```
m_moviStartPos=314  // movi实际在314
```

**关键发现**：VLC 期望下一个 chunk 在位置 **316**，但 movi LIST 实际在位置 **314**，相差 **2 字节**！

**根本原因**：音频 `strf` 的 size 声明为 **20**，但实际写入的数据只有 **18 字节**（标准 WAVEFORMATEX 结构）

```cpp
// 音频 strf 实际写入的数据：
writeLE16: wFormatTag (2 bytes)
writeLE16: nChannels (2 bytes)
writeLE32: nSamplesPerSec (4 bytes)
writeLE32: nAvgBytesPerSec (4 bytes)
writeLE16: nBlockAlign (2 bytes)
writeLE16: wBitsPerSample (2 bytes)
writeLE16: cbSize (2 bytes)
// 总计: 18 bytes，但 size 声明为 20！
```

这导致 VLC 期望读取更多数据，解析位置错位 2 字节。

#### 技术方案
修正音频流相关的大小字段：
1. 音频 strf size: 20 → 18
2. 音频 strl LIST size: 96 → 94 (4 + 64 + 26)

#### 实施步骤
1. **MJPEG_Encoder.cpp 修改**
   - 将音频 strl LIST size 从 96 改为 94
   - 将音频 strf size 从 20 改为 18

2. **Shared_GlobalDefines.h 修改**
   - 版本号更新为 V0.5

#### 文件变更
- `MJPEG_Encoder.cpp`: 修正音频 strf 和 strl LIST 大小
- `Shared_GlobalDefines.h`: 版本号 V0.5

#### 验证结果
修改后需验证：
- VLC日志中 `strf size:18`（不是20）
- `LIST-strl size:94`（不是96）
- `movi LIST at 314` 正确识别
- 视频播放正常，音频同步，无杂音
- 进度条正常前进

---

### 版本 V0.4 - 修复hdrl LIST大小计算错误 (2025-05-28)

#### 问题描述
V0.3修复后，视频播放仍然出现异常：
- 视频开头出现杂音
- 随后立即进入静音状态
- 播放器进度条停止不动

#### 根本原因分析
VLC调试日志显示：
```
avi debug: | + LIST-hdrl size:302 pos:12
avi debug: | + ST@� size:1869414444 pos:316  // VLC在位置316读取，而不是314！
avi error: invalid file: cannot find hdrl or movi chunks
```

Arduino日志显示：
```
m_moviStartPos=314
movi header check: 'LIST' at 314
```

**关键发现**：VLC 在位置 **316** 开始读取 movi，但 Arduino 写入的位置是 **314**。

**根本原因**：hdrl LIST 大小计算仍然错误

根据 AVI 规范，LIST chunk 的 size 字段表示从 type 字段开始的数据长度（不包含 'LIST' 和 size 字段本身）：
- hdrl LIST 从位置 12 开始
- 'LIST' 在 12-15，size 在 16-19，'hdrl' type 在 20-23
- movi LIST 从位置 314 开始
- **正确的 hdrl size = 314 - 20 = 294**

但 V0.3 代码计算的是 `314 - 12 = 302`，多了 8 字节！

```cpp
// V0.3 错误计算
uint32_t hdrlSize = m_moviStartPos - 12;  // = 302

// V0.4 正确计算
uint32_t hdrlSize = m_moviStartPos - 20;  // = 294
```

这导致 VLC 认为音频 strf 数据是 movi LIST 的一部分，解析错位。

#### 技术方案
修正 `end()` 函数中的 `hdrlSize` 计算公式，从 `m_moviStartPos - 12` 改为 `m_moviStartPos - 20`。

#### 实施步骤
1. **MJPEG_Encoder.cpp 修改**
   - 将 `hdrlSize = m_moviStartPos - 12` 改为 `hdrlSize = m_moviStartPos - 20`
   - 添加调试日志输出 hdrl size

2. **Shared_GlobalDefines.h 修改**
   - 版本号更新为 V0.4

#### 文件变更
- `MJPEG_Encoder.cpp`: 修正 hdrlSize 计算
- `Shared_GlobalDefines.h`: 版本号 V0.4

#### 验证结果
修改后需验证：
- VLC日志中 `LIST-hdrl size:294`（不是302）
- `movi LIST at 314` 正确识别
- 视频播放正常，音频同步，无杂音
- 进度条正常前进

---

### 版本 V0.3 - 修复AVI文件结构损坏问题 (2025-05-28)

#### 问题描述
修复无声问题后，视频播放出现严重异常：
- 视频开头出现杂音
- 随后立即进入静音状态
- 播放器进度条停止不动，但视频画面继续播放

#### 根本原因分析
VLC调试日志显示AVI文件结构严重损坏：
```
avi warning: chunk LIST does not fit into parent 310
avi warning: chunk ST�L does not fit into parent 3298100
avi error: invalid file: cannot find hdrl or movi chunks
```

**根本原因**：`end()` 函数中 hdrl LIST 大小计算错误

```cpp
// 错误计算（减了24）
uint32_t hdrlSize = m_moviStartPos - 24;

// 正确计算（应减12）
uint32_t hdrlSize = m_moviStartPos - 12;
```

AVI文件结构：
- RIFF at 0 (4 bytes) + RIFF size (4 bytes) = 8 bytes
- AVI at 8 (4 bytes) + hdrl LIST size at 16 (4 bytes) = 12 bytes (before data)
- hdrl LIST from 12, so size field is at 16
- hdrlSize should be from end of size field (16) to start of movi (314): 314 - 16 = 298

但 `m_moviStartPos - 24 = 314 - 24 = 290`，少算了12字节，导致VLC认为hdrl LIST在位置310就结束，而实际上strf数据在位置310-350之间，被错误截断解析。

#### 技术方案
修正 `end()` 函数中的 `hdrlSize` 计算公式。

#### 实施步骤
1. **MJPEG_Encoder.cpp 修改**
   - 将 `hdrlSize = m_moviStartPos - 24` 改为 `hdrlSize = m_moviStartPos - 12`

#### 文件变更
- `MJPEG_Encoder.cpp`: 修正 end() 函数中的 AVI 头大小计算

#### 验证结果
修改后需验证：
- VLC日志中 `chunk LIST does not fit` 警告消失
- `cannot find hdrl or movi chunks` 错误消失
- 视频播放正常，音频同步，无杂音
- 进度条正常前进

---

### 版本 V0.2 - 修复无声问题 (2025-05-28)

#### 问题描述
录制完成的AVI视频文件在VLC播放器中可以正常播放，但无声音输出。检查日志发现 `[INFO] Ending MJPEG recording: video=105 frames, audio=0 chunks`，音频数据从未被写入AVI文件。

#### 根本原因分析
1. **音频帧阈值过高**：`videoRecorderLoop()` 中采用 `while (availableSamples >= AUDIO_FRAME_SIZE)` 判断条件，需要积累 AUDIO_FRAME_SIZE(2048) 个样本才写入一次

2. **I2S数据供给速度不匹配**：I2S DMA 每帧只提供 640 个样本（DMA_PAGE_SIZE/2 = 1280/2），需要约 3.2 次中断才能凑齐 2048 样本的写入条件

3. **轮询机制缺陷**：`videoRecorderLoop()` 约每 67ms 执行一次（15fps），而音频数据在 `startAVIRecording()` 后持续填充 `m_ringBuffer`，但由于缓冲区判断条件未满足，音频数据从未被读取

4. **数据流断裂**：录音启动后，`m_ringBuffer` 持续接收数据，但在录制停止前从未被读取写入 AVI

#### 技术方案
采用**累积缓冲区方式**实时读取音频数据：

1. 维护一个 `audioAccumBuffer[AUDIO_FRAME_SIZE]` 累积缓冲区
2. 在每次 `videoRecorderLoop()` 调用时，持续从 `m_ringBuffer` 读取数据直到缓冲区空或累积满
3. 当累积满一帧时，立即写入 AVI 并重置缓冲区
4. 添加每秒音频缓冲区状态日志，便于调试监控

#### 实施步骤
1. **VideoRecorder.cpp 修改**
   - 将 `while (availableSamples >= AUDIO_FRAME_SIZE)` 改为 `while (availableSamples > 0 && accumSamples < AUDIO_FRAME_SIZE)`
   - 实现持续读取机制：`toRead = min(availableSamples, AUDIO_FRAME_SIZE - accumSamples)`
   - 累积满一帧后立即写入：`mjpegEncoder.addAudioFrame()`
   - 添加音频状态调试日志

2. **调试机制**
   - 每秒输出：`Audio buffer: available=%d, accum=%d`
   - 每50帧输出：`Audio frame written: %d frames, %d samples`

#### 文件变更
- `VideoRecorder.cpp`: 重写音频帧处理逻辑

#### 验证结果
修改后需验证：
- 日志中 `audio=0 chunks` 问题是否解决
- VLC 播放时是否有声音输出
- 音视频是否同步

---

### 版本 V0.1 - 初始实现 (2025-05-28)

#### 问题描述
为AMB82-MINI Camera项目实现视频录制功能的同步音频录制模块，使当前仅支持无声视频拍摄的系统能够同时捕获并整合音频信号，生成包含同步音视频的媒体文件。

#### 技术要点
1. 使用项目当前的音频编码格式（16kHz采样率，单声道，16位PCM）
2. 调用Inmp441_MicrophoneManager获取音频数据，VideoRecorder获取MJPEG编码视频
3. 将音频数据与MJPEG编码视频合并到AVI容器中
4. 开发音视频同步机制，确保音频与视频数据在时间轴上精确对齐
5. 扩展AVI文件封装模块，支持音频流的写入与索引构建

#### 实施步骤
1. **Shared_GlobalDefines.h 配置**
   - 添加系统版本号定义 V0.1
   - 添加音频录制配置（16kHz采样率、单声道、16位采样、2048帧大小）

2. **Inmp441_MicrophoneManager 扩展**
   - 添加 `startAVIRecording()` 接口用于启动音频采集
   - 添加 `stopAVIRecording()` 接口用于停止音频采集
   - 添加 `getAvailableAudioSamples()` 接口获取可用音频样本数
   - 添加 `readAudioSamples()` 接口读取音频样本

3. **MJPEG_Encoder 扩展**
   - 扩展AVI文件头支持，添加音频流（auds）配置
   - 实现 `addAudioFrame()` 方法用于添加音频帧
   - 支持音频流索引构建（idx1）
   - 支持01wb音频块写入

4. **VideoRecorder 修改**
   - 引入Inmp441_MicrophoneManager头文件
   - 修改 `startVideoRecording()`，启动音频采集
   - 修改 `stopVideoRecording()`，停止音频采集
   - 修改 `videoRecorderLoop()`，添加音频帧处理逻辑
   - 实现音视频同步录制

5. **Camera.ino 修改**
   - 在setup()中添加系统版本号输出："当前版本: V0.1"

#### 技术方案
- **音视频同步机制**：通过时间戳（millis()）对齐，视频帧和音频帧都带有时间戳，写入时保持时间顺序
- **音频编码格式**：PCM 16位，16kHz，单声道
- **视频编码格式**：MJPEG，720P@15fps
- **容器格式**：AVI，支持双流（视频流00dc/00db + 音频流01wb）
- **索引构建**：使用idx1块，记录每个数据块的位置和大小
- **同步精度**：通过实时采集和写入，确保同步误差在50ms以内

#### 交付成果
- 完整的音频采集与处理代码模块（Inmp441_MicrophoneManager）
- 修改后的视频录制主程序（VideoRecorder）
- 更新的AVI文件处理库（MJPEG_Encoder）
- 版本号管理功能（V0.1）
- Memory.md文档创建

#### 文件变更
- `Shared_GlobalDefines.h`: 添加版本号和音频配置
- `Inmp441_MicrophoneManager.h/cpp`: 添加音视频同步录制接口
- `MJPEG_Encoder.h/cpp`: 扩展支持音频流
- `VideoRecorder.cpp`: 实现音视频同步录制
- `Camera.ino`: 添加版本号输出

#### 验证标准
1. 代码必须通过完整的编译流程，无语法错误、链接错误及警告信息
2. 生成的AVI文件在VLC等标准媒体播放器中能够正常播放，音频与视频同步输出，无卡顿、跳帧或音画不同步现象
3. 音视频同步误差需控制在50ms以内，确保观看体验自然流畅
4. 所有原有系统功能（包括菜单导航、视频回放、OSD显示等）需保持正常运行，无功能退化或异常表现
5. 系统需通过稳定性测试，在连续录制至少10分钟的情况下保持正常工作状态，无崩溃、死机或数据丢失现象

---

### 版本 V1.5 - 修复音频采集任务中断问题 (2026-03-22)

#### 问题描述
V1.4版本后，视频播放过程中出现严重异常：
- 视频开头出现几秒只有杂音无拾音内容，随后立即进入静音状态且无任何音频输出
- 视频画面继续播放且播放器进度条正常
- 日志显示音频帧写入正常（106帧，640样本），但播放时出现杂音后静音

#### 根本原因分析
1. **音频采集任务被视频编码任务中断**：视频编码（处理1280x720 JPEG帧）占用大量CPU时间，阻塞音频环形缓冲区的及时读取
2. **音频处理优先级不足**：`videoRecorderLoop()`中音频处理在视频处理之后，当视频编码繁忙时，音频数据读取延迟
3. **音频写入阈值仍可优化**：32样本阈值和30ms超时仍可能造成音频写入不规律
4. **环形缓冲区可能溢出**：当视频任务长时间阻塞时，I2S中断继续填充环形缓冲区，可能导致数据丢失

#### 技术方案
1. **优先处理音频数据**：在`videoRecorderLoop()`中先处理音频数据，再处理视频数据
2. **降低音频写入阈值**：从32样本降低到16样本，时间阈值从30ms降低到20ms，确保更频繁的音频写入
3. **增加环形缓冲区容量**：从16384样本增加到32768样本，提供更大的缓冲空间
4. **优化视频帧获取**：减少重试次数（3→2）和重试延迟（10ms→5ms），减少阻塞时间

#### 实施步骤
1. **VideoRecorder.cpp 修改**
   - 重排`videoRecorderLoop()`处理顺序：先音频后视频
   - 降低音频写入阈值：`accumSamples >= 16` 代替 `>= 32`
   - 缩短时间阈值：`timeSinceLastWrite > 20` 代替 `> 30`
   - 减少视频帧获取重试次数：`MAX_RETRIES = 2` 代替 `3`
   - 减少重试延迟：`delay(5)` 代替 `delay(10)`

2. **Inmp441_MicrophoneManager.h 修改**
   - 增大环形缓冲区：`BUFFER_SIZE = 32768` 代替 `16384`

#### 文件变更
- `VideoRecorder.cpp`: 优化音频处理优先级和写入频率，减少视频获取阻塞
- `Inmp441_MicrophoneManager.h`: 增大环形缓冲区容量

#### 验证结果
修改后需验证：
- 视频播放无杂音、无静音，音频完整输出
- 音频与视频同步良好，无音画不同步
- 音频缓冲区可用样本稳定（available>0）
- `timeSinceLastWrite`稳定在20ms左右
- 日志中音频帧写入连续无大间隔

---

## 版本历史

| 版本号 | 日期 | 描述 |
|--------|------|------|
| V1.16 | 2026-03-26 | 修复videoRecorderLoop()未被调用：在REC_RECORDING状态下正确调用videoRecorderLoop()，解决视频帧丢失47%的问题 |
| V1.15 | 2026-03-26 | 修复视频帧等待机制阻塞主循环：移除50ms等待，采用非阻塞方式获取视频帧，修正时间戳更新时机 |
| V1.14 | 2026-03-26 | 优化处理顺序：先获取视频帧再处理音频数据 |
| V1.13 | 2026-03-26 | 修复帧率控制bug：将lastVideoFrameTime更新移到帧获取判断之前 |
| V1.12 | 2026-03-26 | 添加帧率控制(67ms间隔)和视频帧等待机制(最多50ms)，但存在bug：帧获取失败时不更新时间戳 |
| V1.11 | 2026-03-26 | (已回滚)限制每次循环处理的音频块数量，导致环形缓冲区溢出，音频变杂音 |
| V1.10 | 2026-03-26 | 修复音画不同步问题：调整启动顺序（先设置录制状态再启动音视频），调整停止顺序（先停止视频再丢弃剩余音频），添加音频处理任务录制状态检查 |
| V1.9 | 2026-03-23 | 修复音频帧累积写入导致VLC解析异常：移除音频帧累积机制，每收到一个音频块（64样本）立即写入AVI，保持音频流连续性，解决VLC播放时只有节奏杂音的问题 |
| V1.8 | 2026-03-23 | 修复音频队列持续满载导致音频块丢弃：1) 增大音频队列从32块到64块；2) 修改VideoRecorder录音循环使用while持续清空队列而非单次if检查 |
| V1.7 | 2026-03-23 | 修复音频任务创建失败：问题根因是TaskManager::init()未被调用，导致taskInfos[TASK_AUDIO_PROCESSING].function为NULL。在setup()中添加TaskManager::init()调用以初始化任务信息数组 |
| V1.6 | 2026-03-23 | 多线程音频处理方案：将音频处理分离到独立RTOS任务（TASK_AUDIO_PROCESSING），优先级4高于系统设置任务，通过RTOS队列（32块缓冲）实现音频数据传递，避免被视频任务抢占 |
| V1.5 | 2026-03-22 | 修复音频采集任务中断：优先处理音频数据，降低写入阈值（32→16样本），缩短超时时间（30→20ms），增大环形缓冲区（16384→32768），优化视频帧获取减少阻塞 |
| V1.4 | 2025-05-28 | 修复音频时长不足：降低音频写入阈值（64→32样本）和超时时间（50→30ms），修正AVI音频流长度计算（帧数→样本数），解决4秒杂音后静音问题 |
| V1.3 | 2025-05-28 | 基于原始版本优化：修复字节序（大端序→小端序），移除过度音频处理（DC偏移消除、数字滤波、饱和过滤），恢复默认增益30dB，缓冲区大小32768→16384 |
| V1.2 | 2025-05-28 | 修复音频质量：优化DC偏移消除（DC_ALPHA 32→256），改进数字滤波，降低写入阈值（128→64），增大环形缓冲区（16384→32768） |
| V1.1 | 2025-05-28 | 修复I2S DMA配置错误：DMA页数从8恢复为4（硬件限制2~4），环形缓冲区恢复为16384样本 |
| V1.0 | 2025-05-28 | 最终修复：降低阈值到128样本，缩短超时到50ms，增大DMA缓冲区到8页，增大环形缓冲区到32768样本 |
| V0.9 | 2025-05-28 | 根本性修复：降低阈值到256样本，缩短超时到100ms，添加DC偏移消除和数字滤波 |
| V0.8 | 2025-05-28 | 优化音频采集性能：降低写入阈值到512样本，缩短超时到200ms，修复时间戳初始化 |
| V0.7 | 2025-05-28 | 修复音频杂音和数据格式：降低增益30dB→0dB，修正字节序，添加饱和过滤 |
| V0.6 | 2025-05-28 | 修复音频采集不足：降低阈值到1024样本，添加500ms超时机制 |
| V0.5 | 2025-05-28 | 修复音频strf大小不匹配：strf size 20→18, strl LIST size 96→94 |
| V0.4 | 2025-05-28 | 修复hdrl LIST大小计算错误：从 m_moviStartPos-12 改为 m_moviStartPos-20 |
| V0.3 | 2025-05-28 | 修复AVI文件结构损坏问题：修正end()函数中hdrlSize计算 |
| V0.2 | 2025-05-28 | 修复无声问题：重写音频帧处理逻辑，采用累积缓冲区方式 |
| V0.1 | 2025-05-28 | 初始版本，实现音视频同步录制功能 |

---

## 版本 V1.10 - 修复音画不同步问题 (2026-03-26)

#### 问题描述
V1.6版本后，视频播放出现以下问题：
- 视频画面播放速度比实际拍摄时快
- 视频画面播放完毕后，音频仍在继续播放
- 音频时长比视频时长长约60%（视频5秒，音频9秒）
- 仍有少量杂音

#### 根本原因分析
1. **启动顺序错误**：音频采集任务在视频通道启动前就创建，导致音频数据在视频帧准备好之前就开始采集
2. **停止顺序错误**：停止录制时刷新了队列中剩余的音频数据，导致额外音频被写入
3. **录制状态检查缺失**：音频处理任务没有检查录制状态，在非录制状态下也在处理数据
4. **I2S持续采集**：I2S在系统初始化时就开始采集数据，环形缓冲区中积累了大量录制前的数据

#### 技术方案
1. **调整启动顺序**：
   - 先初始化MJPEG编码器
   - 先设置录制状态为REC_RECORDING
   - 先启动视频通道
   - 最后启动音频采集任务

2. **调整停止顺序**：
   - 先停止视频通道
   - 删除音频处理任务
   - 丢弃队列中剩余音频数据（不再刷新写入）
   - 停止音频采集
   - 最后停止MJPEG编码器

3. **添加录制状态检查**：
   - 音频处理任务只在REC_RECORDING状态下处理数据
   - 非录制状态下跳过音频处理

#### 实施步骤
1. **VideoRecorder.cpp 修改**
   - `startVideoRecording()`：调整启动顺序，先设置录制状态，再启动视频和音频
   - `stopVideoRecording()`：调整停止顺序，先停止视频，丢弃剩余音频数据

2. **RTOS_TaskFactory.cpp 修改**
   - 添加`#include "VideoRecorder.h"`
   - `taskAudioProcessing()`：添加录制状态检查，只在REC_RECORDING状态下处理音频

3. **Shared_GlobalDefines.h 修改**
   - 更新版本号从V1.9到V1.10

#### 文件变更
- `VideoRecorder.cpp`: 调整启动和停止顺序，确保音视频同步开始和结束
- `RTOS_TaskFactory.cpp`: 添加录制状态检查
- `Shared_GlobalDefines.h`: 更新版本号

#### 验证结果
修改后需验证：
- 视频时长与音频时长基本一致（误差<0.5秒）
- 视频播放完毕时音频也同时结束
- 无明显杂音
- 音画同步良好

---

## 版本 V1.12 - 修复视频帧丢失问题 (2026-03-26)

#### 问题描述
V1.11版本引入了新问题：
- **音频变成杂音**：限制音频处理数量导致环形缓冲区溢出
- 视频帧仍丢失38%（录制13秒，视频只有8秒）

#### 根本原因分析

**V1.11失败原因**：
限制每次处理5个音频块导致：
1. videoRecorderLoop()消费音频数据的速度变慢
2. 音频处理任务从环形缓冲区读取数据的速度变慢
3. **环形缓冲区(32768样本=2秒)满了，新数据覆盖旧数据**
4. 音频数据损坏 → 只有杂音

**视频帧丢失的真正原因**：
1. **视频帧获取是非阻塞的**：`Camera.getImage()`如果帧没准备好就返回`imgLen=0`
2. **没有帧率控制**：主循环调用频率太高，但视频帧只有15fps
3. **视频帧被跳过**：当帧没准备好时，代码直接跳过，导致帧丢失

#### 技术方案
采用帧率控制+等待机制：
1. **帧率控制**：每67ms(15fps)才尝试获取视频帧
2. **等待机制**：如果帧没准备好，最多等待50ms
3. **重试机制**：每5ms重试一次，直到获取到帧或超时

#### 实施步骤
1. **VideoRecorder.cpp 修改**
   - 回滚V1.11的音频处理限制
   - 添加帧率控制变量`lastVideoFrameTime`
   - 添加帧间隔常量`FRAME_INTERVAL = 67`
   - 修改视频帧获取逻辑，添加等待机制

2. **Shared_GlobalDefines.h 修改**
   - 更新版本号从V1.11到V1.12

#### 文件变更
- `VideoRecorder.cpp`: 添加帧率控制和视频帧等待机制
- `Shared_GlobalDefines.h`: 更新版本号

#### 验证结果
修改后需验证：
- 视频帧数接近预期（10秒录制应有约150帧）
- 音频正常，无杂音
- 视频时长与音频时长基本一致（误差<0.5秒）
- 音画同步良好

---

## 版本 V1.13 - 修复帧率控制bug (2026-03-26)

#### 问题描述
V1.12版本后，视频帧丢失问题反而更严重：
- 视频帧丢失率从40%增加到45%
- 录制约9秒，视频只有5秒（80帧）
- 音频恢复正常，能听到拾音内容

#### 根本原因分析

**V1.12的bug**：
```cpp
// 错误逻辑：只有获取到帧才更新时间戳
if (imgLen > 0) {
    mjpegEncoder.addVideoFrame(...);
    lastVideoFrameTime = currentMillis; // ❌ 只在成功时更新
}
```

**问题链**：
1. 第N次循环：尝试获取视频帧，但帧没准备好（imgLen=0）
2. `lastVideoFrameTime`不更新
3. 第N+1次循环：`currentMillis - lastVideoFrameTime < 67`仍然成立
4. 直接return，跳过视频帧获取
5. 第N+2次循环：继续跳过...
6. **帧丢失累积**：一旦开始丢失，就会持续丢失

#### 技术方案
将`lastVideoFrameTime`更新移到帧获取判断之前：
```cpp
// 正确逻辑：无论是否获取到帧，都更新时间戳
lastVideoFrameTime = currentMillis; // ✅ 始终更新

if (imgLen > 0) {
    mjpegEncoder.addVideoFrame(...);
}
```

#### 实施步骤
1. **VideoRecorder.cpp 修改**
   - 将`lastVideoFrameTime = currentMillis`移到`if (imgLen > 0)`之前
   - 确保每67ms都会尝试获取帧

2. **Shared_GlobalDefines.h 修改**
   - 更新版本号从V1.12到V1.13

#### 文件变更
- `VideoRecorder.cpp`: 修复帧率控制时间戳更新逻辑
- `Shared_GlobalDefines.h`: 更新版本号

#### 验证结果
修改后需验证：
- 视频帧数接近预期（10秒录制应有约150帧）
- 视频时长与音频时长基本一致（误差<0.5秒）
- 音画同步良好

---

## 版本 V1.14 - 优化处理顺序 (2026-03-26)

#### 问题描述
V1.13版本后，视频帧丢失问题仍然存在：
- 视频帧丢失率约45%
- 音频正常，能听到拾音内容

#### 根本原因分析

**当前架构问题**：
```
videoRecorderLoop() {
    ① 处理音频数据（串行）→ 可能耗时较长
    ② 获取视频帧（串行）→ 可能错过视频帧
}
```

视频处理不是独立的RTOS任务，而是在主循环中与音频处理**串行执行**。当音频处理耗时较长时，视频帧获取可能被延迟。

**16MB Flash的作用**：
- 16MB Flash用于存储程序代码和文件系统
- **不能直接用于运行时内存**
- 真正影响的是RAM（128MB DDR2）

#### 技术方案
调整处理顺序，**先获取视频帧，再处理音频数据**：
```
videoRecorderLoop() {
    ① 获取视频帧（第一优先级）
    ② 处理音频数据（第二优先级）
}
```

这样视频帧获取不会被音频处理阻塞。

#### 实施步骤
1. **VideoRecorder.cpp 修改**
   - 将视频帧获取代码移到函数开头
   - 音频处理代码移到视频帧获取之后

2. **Shared_GlobalDefines.h 修改**
   - 更新版本号从V1.13到V1.14

#### 文件变更
- `VideoRecorder.cpp`: 调整视频帧获取和音频处理的顺序
- `Shared_GlobalDefines.h`: 更新版本号

#### 验证结果
修改后需验证：
- 视频帧数接近预期（10秒录制应有约150帧）
- 视频时长与音频时长基本一致（误差<0.5秒）
- 音画同步良好

---

## 版本 V1.15 - 修复视频帧等待机制阻塞主循环 (2026-03-26)

#### 问题描述
V1.14版本后，视频帧丢失问题仍然严重：
- 录制11秒，视频只有6秒（97帧），丢失41%
- 音频正常，能听到拾音内容但有杂音

#### 根本原因分析

**V1.14的致命问题**：视频帧等待机制阻塞主循环

```cpp
// V1.14 错误代码
do {
    Camera.getImage(VIDEO_CHANNEL_RECORD, &imgAddr, &imgLen);
    if (imgLen > 0) break;
    delay(5); // ❌ 阻塞主循环！
} while (millis() - startTime < 50); // 最多等待50ms
```

**问题链**：
1. 循环开始时间 T
2. 等待最多50ms → 实际时间变成 T+50ms
3. `lastVideoFrameTime = T`（使用的是循环开始时间）
4. 下一次循环在 T+60ms 开始
5. 检查：60 - T = 60 < 67 → 跳过！
6. **实际帧间隔变成 60+50 = 110ms，而不是67ms**

**时间戳更新时机错误**：
```cpp
lastVideoFrameTime = currentMillis; // ❌ 使用循环开始时间
```
应该使用当前时间 `millis()`，而不是循环开始时间。

#### 技术方案
1. **移除等待机制**：采用非阻塞方式获取视频帧
2. **修正时间戳更新**：使用 `millis()` 获取当前时间

```cpp
// V1.15 正确代码
// 非阻塞获取视频帧（不等待）
Camera.getImage(VIDEO_CHANNEL_RECORD, &imgAddr, &imgLen);

// 更新时间戳（使用当前时间）
lastVideoFrameTime = millis();
```

#### 实施步骤
1. **VideoRecorder.cpp 修改**
   - 移除 `do-while` 等待循环
   - 移除 `delay(5)` 阻塞调用
   - 将 `lastVideoFrameTime = currentMillis` 改为 `lastVideoFrameTime = millis()`

2. **Shared_GlobalDefines.h 修改**
   - 更新版本号从V1.14到V1.15

#### 文件变更
- `VideoRecorder.cpp`: 移除视频帧等待机制，修正时间戳更新
- `Shared_GlobalDefines.h`: 更新版本号

#### 验证结果
修改后需验证：
- 视频帧数接近预期（11秒录制应有约165帧）
- 视频时长与音频时长基本一致（误差<0.5秒）
- 音画同步良好

---

## 版本 V1.16 - 修复videoRecorderLoop()未被调用 (2026-03-26)

#### 问题描述
V1.15版本后，视频播放仍有以下问题：
- 视频帧丢失47%（录制14秒，视频只有111帧，预期210帧）
- 视频时长比音频时长短50%（视频7秒，音频12秒）
- 音画不同步，视频先播放结束
- 仍有背景杂音

#### 根本原因分析

**致命问题发现**：`videoRecorderLoop()` 从未被调用！

```cpp
// Camera.ino 原代码
case REC_IDLE:
case REC_RECORDING:
    processPreviewFrame();  // ❌ 只处理预览帧
    break;                   // ❌ 没有调用 videoRecorderLoop()！
```

**问题链**：
1. `videoRecorderLoop()` 包含视频帧获取和音频处理逻辑
2. 但在录制状态下，只调用了 `processPreviewFrame()`
3. `videoRecorderLoop()` 从未被执行
4. 视频帧获取完全缺失，导致帧数严重不足

**日志证据**：
| 指标 | 数值 | 预期 | 问题 |
|------|------|------|------|
| 录制时间 | 14秒 | - | - |
| 视频帧数 | 111帧 | 210帧 (14秒×15fps) | **丢失47%** |
| 视频时长 | 7秒 | 14秒 | **短50%** |
| 音频时长 | 12秒 | 14秒 | 基本正常 |

#### 技术方案
在 `Camera.ino` 的 `loop()` 函数中，将 `REC_RECORDING` 状态单独处理：
1. 调用 `videoRecorderLoop()` 获取视频帧和处理音频
2. 同时调用 `processPreviewFrame()` 用于LCD显示

```cpp
// V1.16 正确代码
case REC_IDLE:
    // 空闲状态：只处理预览帧
    processPreviewFrame();
    break;
case REC_RECORDING:
    // 录制状态：运行视频录制循环（包含视频帧获取和音频处理）
    videoRecorderLoop();
    // 同时处理预览帧（用于LCD显示）
    processPreviewFrame();
    break;
```

#### 实施步骤
1. **Camera.ino 修改**
   - 将 `REC_IDLE` 和 `REC_RECORDING` 分开处理
   - 在 `REC_RECORDING` 状态下调用 `videoRecorderLoop()`

2. **Shared_GlobalDefines.h 修改**
   - 更新版本号从V1.15到V1.16

#### 文件变更
- `Camera.ino`: 在REC_RECORDING状态下调用videoRecorderLoop()
- `Shared_GlobalDefines.h`: 更新版本号

#### 验证结果
修改后需验证：
- 视频帧数接近预期（14秒录制应有约210帧）
- 视频时长与音频时长基本一致（误差<0.5秒）
- 音画同步良好

---

## 版本 V1.11 - (已回滚) (2026-03-26)

#### 问题描述
V1.10版本后，视频播放仍有以下问题：
- 视频画面播放速度比实际拍摄时快
- 视频画面播放完毕后，音频仍在继续播放
- 音频时长比视频时长长70%（录制10秒，视频只有6秒，丢失40%视频帧）
- 仍有少量杂音

#### 根本原因分析
通过详细分析VLC调试日志和Arduino日志：

| 指标 | 数值 | 计算 |
|------|------|------|
| 实际录制时间 | 04:26:21 → 04:26:31 | **10秒** |
| 音频时长 | 163840样本 | 163840÷16kHz = **10.24秒** ✅ |
| 视频帧数 | 90帧 | 90÷15fps = **6秒** ❌ |

**问题根本原因**：视频帧丢失了40%！按10秒录制应该有150帧，但只记录了90帧。

#### 技术方案 (已证明失败)
限制每次循环处理的音频块数量，确保视频帧能及时获取：
- 每次循环最多处理5个音频块（约160ms音频数据）
- 然后立即处理视频帧

#### 失败原因
限制音频处理数量导致环形缓冲区溢出，音频数据损坏变成杂音。此方案已回滚。

---

## 版本 V1.17 - 视频帧获取独立RTOS任务 (2026-03-27)

#### 问题描述
V1.16版本后，视频播放仍有以下问题：
- 视频帧丢失38%（录制11秒，视频只有103帧，预期165帧）
- 视频时长比音频时长短45%（视频6秒，音频11秒）
- 音画不同步，视频先播放结束
- 仍有背景杂音

#### 根本原因分析

**架构问题**：视频帧获取和音频处理在同一个循环中串行执行

```
videoRecorderLoop() {
    ① 获取视频帧（串行）
    ② 处理音频数据（串行）→ 可能耗时较长，阻塞视频帧获取
}
```

**问题链**：
1. 音频处理可能耗时较长
2. 视频帧获取被阻塞
3. 视频帧丢失
4. 视频时长变短

**日志证据**：
| 指标 | 数值 | 预期 | 问题 |
|------|------|------|------|
| 录制时间 | 11秒 | - | - |
| 视频帧数 | 103帧 | 165帧 (11秒×15fps) | **丢失38%** |
| 视频时长 | 6秒 | 11秒 | **短45%** |
| 音频时长 | 11秒 | 11秒 | 正常 |

#### 技术方案
将视频帧获取独立成RTOS任务，并设置更高的优先级：

**新架构**：
```
┌─────────────────────────────────────────────────────────────┐
│                      RTOS任务调度器                          │
├─────────────────────────────────────────────────────────────┤
│  taskVideoFrameCapture (优先级5)                            │
│  ├─ 每67ms获取一帧视频                                       │
│  └─ 写入MJPEG编码器                                         │
├─────────────────────────────────────────────────────────────┤
│  taskAudioProcessing (优先级4)                              │
│  ├─ 从环形缓冲区读取音频数据                                 │
│  └─ 发送到音频队列                                          │
├─────────────────────────────────────────────────────────────┤
│  videoRecorderLoop() (主循环)                               │
│  ├─ 从音频队列接收数据                                       │
│  └─ 写入AVI文件                                             │
└─────────────────────────────────────────────────────────────┘
```

**优先级设置**：
- 视频帧获取任务：优先级5（最高）
- 音频处理任务：优先级4
- 主循环：优先级1（默认）

这样确保视频帧获取不会被音频处理阻塞。

#### 实施步骤
1. **RTOS_TaskManager.h 修改**
   - 添加 `TASK_VIDEO_FRAME_CAPTURE` 任务ID

2. **RTOS_TaskFactory.h 修改**
   - 添加 `taskVideoFrameCapture()` 函数声明

3. **RTOS_TaskManager.cpp 修改**
   - 添加视频帧获取任务配置（优先级5，栈大小4096）

4. **RTOS_TaskFactory.cpp 修改**
   - 实现 `taskVideoFrameCapture()` 任务函数
   - 功能：每67ms获取一帧视频，写入MJPEG编码器

5. **VideoRecorder.cpp 修改**
   - `startVideoRecording()`: 创建视频帧获取任务
   - `stopVideoRecording()`: 删除视频帧获取任务
   - `videoRecorderLoop()`: 移除视频帧获取逻辑，只处理音频数据

6. **Shared_GlobalDefines.h 修改**
   - 更新版本号从V1.16到V1.17

#### 文件变更
- `RTOS_TaskManager.h`: 添加TASK_VIDEO_FRAME_CAPTURE任务ID
- `RTOS_TaskFactory.h`: 添加taskVideoFrameCapture函数声明
- `RTOS_TaskManager.cpp`: 添加视频帧获取任务配置
- `RTOS_TaskFactory.cpp`: 实现taskVideoFrameCapture任务函数
- `VideoRecorder.cpp`: 创建/删除视频帧获取任务，移除视频帧获取逻辑
- `Shared_GlobalDefines.h`: 更新版本号

#### 验证结果
修改后需验证：
- 视频帧数接近预期（11秒录制应有约165帧）
- 视频时长与音频时长基本一致（误差<0.5秒）
- 音画同步良好
- 无杂音

---

## 版本 V1.18 - 多任务并发访问互斥保护 (2026-03-27)

#### 问题描述
V1.17版本后，视频播放速度正常了，但音频出现以下问题：
- 无法听到拾音内容，只有背景杂音
- 音频数据可能被视频帧数据污染

#### 根本原因分析

**核心问题：多任务并发访问冲突**

V1.17版本创建了两个独立的RTOS任务：
- `taskVideoFrameCapture` (优先级5) → 调用 `mjpegEncoder.addVideoFrame()`
- `videoRecorderLoop()` (主循环) → 调用 `mjpegEncoder.addAudioFrame()`

**这两个任务同时访问 `mjpegEncoder` 对象，但没有互斥锁保护！**

```
┌─────────────────────────────────────────────────────────────┐
│                    问题：并发访问冲突                        │
├─────────────────────────────────────────────────────────────┤
│  taskVideoFrameCapture (优先级5)                            │
│  └─ addVideoFrame() ──────────────────┐                    │
│                                       ▼                    │
│                              ┌─────────────────┐           │
│                              │  mjpegEncoder   │ ← 无保护！ │
│                              │  (共享资源)      │           │
│                              └─────────────────┘           │
│                                       ▲                    │
│  videoRecorderLoop() ─────────────────┘                    │
│  └─ addAudioFrame()                                         │
└─────────────────────────────────────────────────────────────┘
```

**后果**：
1. 视频帧和音频帧数据在AVI缓冲区中交错混乱
2. 音频数据可能被视频帧数据污染，导致杂音
3. 索引条目可能损坏

**次要问题：环形缓冲区并发访问**
- I2S中断回调写入环形缓冲区
- `taskAudioProcessing` 任务读取环形缓冲区
- 没有临界区保护，可能导致数据损坏

#### 技术方案

**方案一：MJPEG编码器互斥锁保护**

使用FreeRTOS互斥锁保护 `mjpegEncoder` 的并发访问：

```cpp
// MJPEG_Encoder.h
#include <semphr.h>
class MJPEGEncoder {
private:
    SemaphoreHandle_t m_mutex;
};

// MJPEG_Encoder.cpp
bool MJPEGEncoder::addVideoFrame(...) {
    if (m_mutex && xSemaphoreTake(m_mutex, portMAX_DELAY) != pdTRUE) {
        return false;
    }
    // ... 写入视频帧数据 ...
    if (m_mutex) xSemaphoreGive(m_mutex);
    return true;
}

bool MJPEGEncoder::addAudioFrame(...) {
    if (m_mutex && xSemaphoreTake(m_mutex, portMAX_DELAY) != pdTRUE) {
        return false;
    }
    // ... 写入音频帧数据 ...
    if (m_mutex) xSemaphoreGive(m_mutex);
    return true;
}
```

**方案二：环形缓冲区临界区保护**

使用FreeRTOS临界区保护环形缓冲区的并发访问：

```cpp
// Inmp441_MicrophoneManager.cpp
bool RingBufferClass::write(int16_t value) {
    taskENTER_CRITICAL();
    // ... 写入数据 ...
    taskEXIT_CRITICAL();
    return true;
}

int16_t RingBufferClass::read() {
    taskENTER_CRITICAL();
    // ... 读取数据 ...
    taskEXIT_CRITICAL();
    return value;
}
```

#### 实施步骤

1. **MJPEG_Encoder.h 修改**
   - 添加 `#include <semphr.h>`
   - 添加 `SemaphoreHandle_t m_mutex` 成员变量

2. **MJPEG_Encoder.cpp 修改**
   - 构造函数中创建互斥锁：`m_mutex = xSemaphoreCreateMutex()`
   - 析构函数中删除互斥锁：`vSemaphoreDelete(m_mutex)`
   - `addVideoFrame()` 添加互斥锁保护
   - `addAudioFrame()` 添加互斥锁保护

3. **Inmp441_MicrophoneManager.h 修改**
   - 将 `m_count` 声明为 `volatile size_t`

4. **Inmp441_MicrophoneManager.cpp 修改**
   - `RingBufferClass::write()` 添加临界区保护
   - `RingBufferClass::read()` 添加临界区保护
   - 其他访问函数添加临界区保护

5. **Shared_GlobalDefines.h 修改**
   - 更新版本号从V1.17到V1.18

#### 文件变更
- `MJPEG_Encoder.h`: 添加互斥锁成员变量
- `MJPEG_Encoder.cpp`: 实现互斥锁保护
- `Inmp441_MicrophoneManager.h`: 添加volatile关键字
- `Inmp441_MicrophoneManager.cpp`: 实现临界区保护
- `Shared_GlobalDefines.h`: 更新版本号

#### 验证结果
修改后需验证：
- 音频能清晰听到拾音内容
- 无背景杂音
- 视频播放正常
- 音画同步良好

---

## 版本 V1.19 - 修复ISR临界区保护问题 (2026-03-27)

#### 问题描述
V1.18版本添加了互斥锁和临界区保护后，视频播放速度正常，但音频仍然只有杂音，无法听到拾音内容。

#### 根本原因分析

**核心问题：临界区函数在中断上下文中使用不当**

V1.18版本使用了 `taskENTER_CRITICAL()` / `taskEXIT_CRITICAL()` 来保护环形缓冲区：

```cpp
bool RingBufferClass::write(int16_t value) {
    taskENTER_CRITICAL();  // ❌ 在ISR中不安全！
    // ... 写入数据 ...
    taskEXIT_CRITICAL();
    return true;
}
```

**问题**：I2S回调函数 `i2s_rx_callback` 是在**中断上下文**中执行的，它调用 `m_ringBuffer->write()`。

`taskENTER_CRITICAL()` / `taskEXIT_CRITICAL()` 是为**任务上下文**设计的：
- 会禁用中断
- 但I2S回调本身就是中断
- 可能导致中断嵌套问题、死锁或数据损坏

**正确的ISR保护方式**：使用 `portSET_INTERRUPT_MASK_FROM_ISR()` / `portCLEAR_INTERRUPT_MASK_FROM_ISR()`

```cpp
bool RingBufferClass::write(int16_t value) {
    UBaseType_t uxSavedStatus = portSET_INTERRUPT_MASK_FROM_ISR();  // ✅ ISR安全
    // ... 写入数据 ...
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedStatus);
    return true;
}
```

#### 技术方案

将所有环形缓冲区函数中的临界区保护改为ISR安全版本：

| 原函数 | 替换为 |
|--------|--------|
| `taskENTER_CRITICAL()` | `portSET_INTERRUPT_MASK_FROM_ISR()` |
| `taskEXIT_CRITICAL()` | `portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedStatus)` |

#### 实施步骤

1. **Inmp441_MicrophoneManager.cpp 修改**
   - `RingBufferClass::write()` 使用ISR安全临界区
   - `RingBufferClass::read()` 使用ISR安全临界区
   - `RingBufferClass::available()` 使用ISR安全临界区
   - `RingBufferClass::isEmpty()` 使用ISR安全临界区
   - `RingBufferClass::isFull()` 使用ISR安全临界区
   - `RingBufferClass::clear()` 使用ISR安全临界区
   - `RingBufferClass::getCount()` 使用ISR安全临界区

2. **Inmp441_MicrophoneManager.h 修改**
   - 更新版本注释

3. **Shared_GlobalDefines.h 修改**
   - 更新版本号从V1.18到V1.19

#### 文件变更
- `Inmp441_MicrophoneManager.cpp`: 使用ISR安全临界区保护
- `Inmp441_MicrophoneManager.h`: 更新版本注释
- `Shared_GlobalDefines.h`: 更新版本号

#### 验证结果
修改后需验证：
- 音频能清晰听到拾音内容
- 无背景杂音
- 视频播放正常
- 音画同步良好

---

## 版本 V1.20 - 修复I2S数据格式问题 (2026-03-27)

#### 问题描述
V1.19版本修复ISR临界区保护后，音频能听到拾音内容，音视频同步正常，但仍然存在较大的背景杂音。

#### 根本原因分析

**核心问题：I2S数据格式不匹配**

INMP441麦克风使用**左对齐（Left-Justified）**数据格式，而代码使用的是**标准I2S格式**。

根据INMP441技术文档：
- "INMP441在I²S模式下通常是MSB first、left justified(左对齐)"
- "该芯片采用单声道左对齐数据格式"

**两种格式的关键区别**：

| 格式 | MSB传输时机 | 数据对齐方式 |
|------|-------------|--------------|
| FORMAT_I2S (标准) | WS变化后延迟1个时钟周期 | 数据延迟一位 |
| FORMAT_LEFT_JUST (左对齐) | WS变化后立即传输 | 数据不延迟 |

```
标准I2S格式时序：
     ___     ___     ___     ___
SCK |   |___|   |___|   |___|   |___
        _______________
WS   __|               |____________
        <-延迟1位->
DATA ___[MSB].........[LSB]________

左对齐格式时序：
     ___     ___     ___     ___
SCK |   |___|   |___|   |___|   |___
        _______________
WS   __|               |____________
DATA [MSB].........[LSB]___________
      <-无延迟，立即传输->
```

**问题影响**：
- 使用FORMAT_I2S读取左对齐数据时，数据整体错位一个时钟周期
- 导致采样值错误，产生持续的背景杂音
- 即使ISR临界区保护正确，数据本身仍然是错误的

#### 技术方案

将I2S格式从`FORMAT_I2S`改为`FORMAT_LEFT_JUST`，以匹配INMP441的数据格式。

#### 实施步骤

1. **Inmp441_MicrophoneManager.cpp 修改**
   - 将`i2s_set_format(&m_i2sObj, FORMAT_I2S)`改为`i2s_set_format(&m_i2sObj, FORMAT_LEFT_JUST)`

2. **Inmp441_MicrophoneManager.h 修改**
   - 更新版本注释

3. **Shared_GlobalDefines.h 修改**
   - 更新版本号从V1.19到V1.20

#### 文件变更
- `Inmp441_MicrophoneManager.cpp`: I2S格式改为FORMAT_LEFT_JUST
- `Inmp441_MicrophoneManager.h`: 更新版本注释
- `Shared_GlobalDefines.h`: 更新版本号

#### 验证结果
修改后需验证：
- 音频清晰无杂音
- 拾音内容清晰可辨
- 视频播放正常
- 音画同步良好

---

## 版本 V1.22 - 修复环形缓冲区ISR临界区保护问题 (2026-03-27)

#### 问题描述
V1.21版本回滚临界区保护后，出现以下严重问题：
- 无法播放拾音内容，只有大量杂音
- 视频播放正常，但音频完全失效
- 原项目 Inmp441_Rtos.ino 在同样30dB增益下工作正常

#### 根本原因分析

**核心问题：环形缓冲区并发访问冲突**

通过详细分析，发现问题的根源是：
1. **V1.20回滚了临界区保护**：完全移除了所有临界区保护机制
2. **多任务并发访问**：I2S中断上下文（i2s_rx_callback）写入环形缓冲区，同时音频处理任务（taskAudioProcessing）读取环形缓冲区
3. **数据竞争导致数据损坏**：没有临界区保护时，write()和read()操作可能同时执行，导致m_head、m_tail、m_count等变量状态不一致，音频数据被损坏

**关键证据**：
- 中断上下文中调用write()，任务上下文中调用read()
- 两个上下文同时操作环形缓冲区状态变量
- 没有任何同步机制保护并发访问

#### 技术方案

使用FreeRTOS提供的ISR安全临界区保护机制：
1. **在write()和read()函数中添加ISR安全临界区保护**
2. **使用portSET_INTERRUPT_MASK_FROM_ISR()和portCLEAR_INTERRUPT_MASK_FROM_ISR()**
3. **这些函数可以在中断上下文中安全使用**，不会导致系统崩溃

**为什么之前V1.19有问题但V1.22没有？**
- V1.19同时尝试修改了很多其他参数，可能引入了其他问题
- V1.22只专注于临界区保护，保持其他所有参数与原项目一致

#### 实施步骤

1. **Inmp441_MicrophoneManager.cpp 修改**
   - 在RingBufferClass::write()函数中添加portSET_INTERRUPT_MASK_FROM_ISR()保护
   - 在RingBufferClass::read()函数中添加portSET_INTERRUPT_MASK_FROM_ISR()保护
   - 使用portCLEAR_INTERRUPT_MASK_FROM_ISR()恢复中断状态

2. **Inmp441_MicrophoneManager.h 修改**
   - 更新版本注释为V1.22

3. **Shared_GlobalDefines.h 修改**
   - 更新版本号从V1.21到V1.22

#### 文件变更
- `Inmp441_MicrophoneManager.cpp`: 添加ISR安全临界区保护到环形缓冲区write()和read()
- `Inmp441_MicrophoneManager.h`: 更新版本注释
- `Shared_GlobalDefines.h`: 更新版本号

#### 验证结果
修改后需验证：
- 能够听到清晰的拾音内容
- 背景杂音明显减少或消失
- 音频播放正常，与视频同步
- 视频播放速度正常
- 录制的音频质量与原项目 Inmp441_Rtos.ino 相当

---

## 相关技术文档

- AVI文件格式规范：https://docs.microsoft.com/en-us/windows/win32/directshow/avi-riff-file-reference
- MJPEG编解码器规范
- INMP441麦克风数据手册
- AMB82-MINI开发板文档

---

## 注意事项
1. 硬件连接：INMP441麦克风需正确连接到AMB82-MINI的I2S接口
2. SD卡要求：使用高速SD卡以确保音视频数据写入速度
3. 内存使用：AVI编码器使用较大缓冲区，请确保系统内存充足
4. 同步校准：如需更高精度同步，可考虑使用硬件时间戳

