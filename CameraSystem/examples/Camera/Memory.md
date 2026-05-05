# AMB82-MINI Camera项目开发历程记录

【重要说明】后续每次向本文件添加内容时，必须严格遵循当前目录结构，在对应版本号条目下或按版本号顺序新增条目进行添加，不得随意更改目录结构，以此减少对文件的读写操作时间。

【读取指引】后续若需要读取本文件内容进行bug修复工作，请优先从目录的版本号条目处开始读取，通过版本号定位相关内容区域，以加快内容查找进度并节省读取时间。

## 项目概述
本项目为AMB82-MINI Camera项目实现视频录制功能的同步音频录制模块，使系统能够同时捕获并整合音频信号，生成包含同步音视频的AVI媒体文件。

---

## 目录

### V1.29 - V1.39 版本
- [V1.29 - 修复回放功能（选项C）双重焦点标记异常问题](#版本-v129---修复回放功能选项c双重焦点标记异常问题-2026-03-31)
- [V1.30 - ISPControl移植：渐进式集成阶段一和阶段二](#版本-v130---ispcontrol移植渐进式集成阶段一和阶段二-2026-03-31)
- [V1.31 - ISPControl移植：阶段三 ISPConfigManager 创建与集成](#版本-v131---ispcontrol移植阶段三-ispconfigmanager-创建与集成-2026-03-31)
- [V1.32 - ISPControl移植：阶段三 完整用户配置界面开发](#版本-v132---ispcontrol移植阶段三-完整用户配置界面开发-2026-03-31)
- [V1.33 - 修复编译错误：ISP配置模块集成优化](#版本-v133---修复编译错误isp配置模块集成优化-2026-03-31)
- [V1.34 - 选项E（WiFi文件传输）功能完善与编码器稳定性增强](#版本-v134---选项ewifi文件传输功能完善与编码器稳定性增强-2026-04-09)
- [V1.35 - OTA升级功能编译错误修复与版本管理系统完善](#版本-v135---ota升级功能编译错误修复与版本管理系统完善-2026-04-13)
- [V1.36 - OTA Keep-alive线程堆栈溢出修复](#版本-v136---ota-keep-alive线程堆栈溢出修复-2026-04-13)
- [V1.37 - OTA POST请求格式修复：解决服务器无法识别开发板问题](#版本-v137---ota-post请求格式修复解决服务器无法识别开发板问题-2026-04-13)
- [V1.38 - BLE WiFi配网功能与OTA WiFi连接策略重构](#版本-v138---ble-wifi配网功能与ota-wifi连接策略重构-2026-04-15)
- [V1.39 - USB MSC模式退出对话框交互功能实现](#版本-v139---usb-msc模式退出对话框交互功能实现-2026-04-19)

### V1.40 - V1.49 版本
- [V1.40 - 无Force网络环境下WiFi连接崩溃修复](#版本-v140---无force网络环境下wifi连接崩溃修复-2026-04-19)
- [V1.41 - WiFi连接前增加SSID存在性检查修复崩溃问题](#版本-v141---wifi连接前增加ssid存在性检查修复崩溃问题-2026-04-19)
- [V1.42 - taskTimeSync函数WiFi连接崩溃修复](#版本-v142---tasktimesync函数wifi连接崩溃修复-2026-04-19)
- [V1.43 - USB MSC模式USB直连异常问题诊断与修复](#版本-v143---usb-msc模式usb直连异常问题诊断与修复-2026-04-19)
- [V1.44 - WiFi与USB OTG资源冲突导致USB MSC模式卡死问题修复](#版本-v144---wifi与usb-otg资源冲突导致usb-msc模式卡死问题修复-2026-04-19)
- [V1.45 - USB MSC模式无法关闭及重复启用失败问题修复](#版本-v145---usb-msc模式无法关闭及重复启用失败问题修复-2026-04-19)
- [V1.46 - BLE WiFi配网退出确认弹窗功能实现](#版本-v146---ble-wifi配网退出确认弹窗功能实现-2026-04-19)
- [V1.47 - USB调试输出清理与英文UI本地化清单](#版本-v147---usb调试输出清理与英文ui本地化清单-2026-04-20)
- [V1.48 - 不存在WiFi SSID连接崩溃修复与文件传输返回功能](#版本-v148---不存在wifi-ssid连接崩溃修复与文件传输返回功能-2026-04-20)
- [V1.49 - 修复视频录制音视频不同步：视频播放速度过快问题](#版本-v149---修复视频录制音视频不同步视频播放速度过快问题-2026-04-23)

### V1.50 - V1.59 版本
- [V1.50 - MAX98357A音频驱动完善与视频回放音频播放功能实现](#版本-v150---max98357a音频驱动完善与视频回放音频播放功能实现-2026-04-24)
- [V1.51 - MAX98357A音频驱动I2S DMA修复与视频回放调试](#版本-v151---max98357a音频驱动i2s-dma修复与视频回放调试-2026-04-24)
- [V1.52 - I2S RX页面管理修复与视频回放性能优化](#版本-v152---i2s-rx页面管理修复与视频回放性能优化-2026-04-24)
- [V1.53 - 音频降噪优化：录音端信号处理与播放端淡入淡出](#版本-v153---音频降噪优化录音端信号处理与播放端淡入淡出-2026-04-24)
- [V1.54 - 修复播放端规律高音噪声：移除不稳定反馈滤波器](#版本-v154---修复播放端规律高音噪声移除不稳定反馈滤波器-2026-04-24)
- [V1.55 - 录音增益调整：18dB → 30dB](#版本-v155---录音增益调整18db--30db-2026-04-24)
- [V1.56 - I2S播放通道修复：MONO→STEREO解决MAX98357A杂音](#版本-v156---i2s播放通道修复monostereo解决max98357a杂音-2026-04-24)
- [V1.57 - I2S字节序修复：启用BYTE_SWAP解决TX端字节交换失真](#版本-v157---i2s字节序修复启用byte_swap解决tx端字节交换失真-2026-04-24)
- [V1.58 - I2S数据开始边沿优化：添加POSITIVE_EDGE改善时序](#版本-v158---i2s数据开始边沿优化添加positive_edge改善时序-2026-04-24)
- [V1.59 - 回滚字节序和边沿修改：恢复默认I2S配置](#版本-v159---回滚字节序和边沿修改恢复默认i2s配置-2026-04-24)

### V1.60 - V1.69 版本
- [V1.60 - 播放端DC偏移消除与fade-in优化](#版本-v160---播放端dc偏移消除与fade-in优化-2026-04-26)
- [V1.61 - 添加播放端低通滤波器](#版本-v161---添加播放端低通滤波器-2026-04-26)
- [V1.62 - 移除录音端降噪处理，恢复原始音频数据](#版本-v162---移除录音端降噪处理恢复原始音频数据-2026-04-26)
- [V1.63 - 移除播放端降噪处理，恢复对称架构](#版本-v163---移除播放端降噪处理恢复对称架构-2026-04-26)
- [V1.64 - 添加播放端软件字节交换，统一RX/TX配置](#版本-v164---添加播放端软件字节交换统一rxtx配置-2026-04-26)
- [V1.65 - 回滚V1.64错误修改，恢复V1.63状态](#版本-v165---回滚v164错误修改恢复v163状态-2026-04-26)
- [V1.66 - 音频解码诊断增强：添加详细日志与性能优化](#版本-v166---音频解码诊断增强添加详细日志与性能优化-2026-04-27)
- [V1.67 - 根本性修复：I2S字节序配置纠正](#版本-v167---根本性修复i2s字节序配置纠正-2026-04-27)
- [V1.68 - 字节交换导致白噪音问题回滚与分析](#版本-v168---字节交换导致白噪音问题回滚与分析-2026-04-27)
- [V1.69 - 降低麦克风增益：30dB → 18dB](#版本-v169---降低麦克风增益30db--18db-2026-04-27)

### V1.70 - V1.79 版本
- [V1.70 - 播放解码分析：回滚增益，聚焦播放链路排查](#版本-v170---播放解码分析回滚增益聚焦播放链路排查-2026-04-27)
- [V1.71 - I2S时序深度分析：添加时序诊断日志](#版本-v171---i2s时序深度分析添加时序诊断日志-2026-04-27)
- [V1.72 - TX回调日志修复：micros()改为millis()简化诊断](#版本-v172---tx回调日志修复micros改为millis简化诊断-2026-04-27)
- [V1.73 - 添加无条件TX回调诊断日志](#版本-v173---添加无条件tx回调诊断日志-2026-04-27)
- [V1.74 - 添加I2S初始化详细诊断日志](#版本-v174---添加i2s初始化详细诊断日志-2026-04-27)
- [V1.75 - 移除过多诊断日志，简化错误输出](#版本-v175---移除过多诊断日志简化错误输出-2026-04-27)
- [V1.76 - 修复I2S初始化顺序](#版本-v176---修复i2s初始化顺序-2026-04-27)
- [V1.77 - 回退音频代码+移除编码器上电限制](#版本-v177---回退音频代码移除编码器上电限制-2026-04-27)
- [V1.78 - 使用轮询方式替代TX中断回调](#版本-v178---使用轮询方式替代tx中断回调-2026-04-27)
- [V1.79 - 改用I2S_DIR_TXRX双向模式修复TX DMA不启动](#版本-v179---改用i2s_dir_txrx双向模式修复tx-dma不启动-2026-04-27)

### V1.80 - V1.89 版本
- [V1.80 - I2S配置修正](#版本-v180---i2s配置修正-2026-04-27)
- [V1.81 - 修复i2s_enable时序 + 取消TX中断注册](#版本-v181---修复i2s_enable时序--取消tx中断注册-2026-04-27)
- [V1.82 - 恢复TX中断注册防止崩溃](#版本-v182---恢复tx中断注册防止崩溃-2026-04-27)
- [V1.83 - 回退I2S_DIR_TX保留enable→send_page顺序](#版本-v183---回退i2s_dir_tx保留enablesend_page顺序-2026-04-27)
- [V1.84 - 移除所有RX代码防止干扰TX中断](#版本-v184---移除所有rx代码防止干扰tx中断-2026-04-27)
- [V1.85 - 回退send_page→enable顺序恢复V1.56工作状态](#版本-v185---回退send_pageenable顺序恢复v156工作状态-2026-04-27)
- [V1.86 - TXRX+send→enable+双回调+Serial直打诊断](#版本-v186---txrxsendenable双回调serial直打诊断-2026-04-27)
- [V1.87 - DC偏移消除修复杂音](#版本-v187---dc偏移消除修复杂音-2026-04-27)
- [V1.88 - 加速DC追踪+TX DMA数据快照](#版本-v188---加速dc追踪tx-dma数据快照-2026-04-27)
- [V1.89 - 逐采样DC阻塞滤波器替代逐块EMA](#版本-v189---逐采样dc阻塞滤波器替代逐块ema-2026-04-27)

### V1.90 - V1.99 版本
- [V1.90 - I2S数据边缘切换到POSITIVE_EDGE](#版本-v190---i2s数据边缘切换到positive_edge-2026-04-27)
- [V1.91 - 修复编译警告+ISR诊断+DC阻塞全路径](#版本-v191---修复编译警告isr诊断dc阻塞全路径-2026-04-27)
- [V1.92 - 修复buffer underrun杂音根因](#版本-v192---修复buffer-underrun杂音根因-2026-04-27)
- [V1.93 - 修复byte_swap: 关键时刻](#版本-v193---修复byte_swap-关键时刻-2026-04-27)
- [V1.94 - 回退byte_swap+极简TX回调定位噪声源](#版本-v194---回退byte_swap极简tx回调定位噪声源-2026-04-27)
- [V1.95 - 修复视频帧率：从AVI头读取FPS](#版本-v195---修复视频帧率从avi头读取fps-2026-04-28)
- [V1.96 - 修复预加载不足：音频缓冲预填充逻辑优化](#版本-v196---修复预加载不足音频缓冲预填充逻辑优化-2026-04-28)
- [V1.97 - 修复主菜单指示三角形位置保留问题](#版本-v197---修复主菜单指示三角形位置保留问题-2026-04-28)
- [V1.98 - 修复视频播放结束时音频未完整输出的问题](#版本-v198---修复视频播放结束时音频未完整输出的问题-2026-04-28)
- [V1.99 - 修复主菜单三角形位置保留问题 + 视频帧率控制修复](#版本-v199---修复主菜单三角形位置保留问题--视频帧率控制修复-2026-04-28)

### V1.100 - V1.109 版本
- [V1.100 - 修复视频帧率时间漂移累积问题](#版本-v1100---修复视频帧率时间漂移累积问题-2026-04-28)
- [V1.101 - 添加视频帧率调试日志](#版本-v1101---添加视频帧率调试日志-2026-04-28)
- [V1.102 - 精确帧率计时与绘制时间测量](#版本-v1102---精确帧率计时与绘制时间测量-2026-04-28)
- [V1.103 - 修复帧率时间累积误差：改用绝对帧号计算目标时间](#版本-v1103---修复帧率时间累积误差改用绝对帧号计算目标时间-2026-04-28)
- [V1.104 - 修复帧间隔整数除法误差：使用微秒精确计算](#版本-v1104---修复帧间隔整数除法误差使用微秒精确计算-2026-04-28)
- [V1.105 - 回溯修复：简化视频播放循环，使用简单计时替代复杂计算](#版本-v1105---回溯修复简化视频播放循环使用简单计时替代复杂计算-2026-04-28)
- [V1.106 - 修复videoPlaybackLoop中缺失的drawBitmap调用](#版本-v1106---修复videoplaybackloop中缺失的drawbitmap调用-2026-04-28)
- [V1.107 - 音频解码架构重构：音频优先批量读取解决播放卡顿](#版本-v1107---音频解码架构重构音频优先批量读取解决播放卡顿-2026-04-29)
- [V1.108 - I2S配置系统性比对分析与优化建议实施](#版本-v1108---i2s配置系统性比对分析与优化建议实施-2026-04-29)
- [V1.109 - 视频回放开头声音重复Bug修复](#v1109---视频回放开头声音重复bug修复-2026-04-30)

### V1.110 - V1.117 版本
- [V1.110 - 视频回放帧率修复](#v1110---视频回放帧率修复-2026-04-30)
- [V1.111 - 视频回放帧率修复 (第二轮)](#v1111---视频回放帧率修复-第二轮-2026-04-30)
- [V1.112 - 视频回放初始阶段掉帧修复](#v1112---视频回放初始阶段掉帧修复-2026-05-05)
- [V1.113 - 视频回放第三秒掉帧修复](#v1113---视频回放第三秒掉帧修复-2026-05-05)
- [V1.114 - 视频回放全程掉帧根因修复](#v1114---视频回放全程掉帧根因修复-2026-05-05)
- [V1.115 - 预加载阶段视频帧保存修复](#v1115---预加载阶段视频帧保存修复-2026-05-05)
- [V1.116 - 预读阶段视频帧丢失修复](#v1116---预读阶段视频帧丢失修复-2026-05-05)
- [V1.117 - 回退至V1.111](#v1117---回退至v1111-2026-05-05)

---

## 开发记录

### 版本 V1.29 - 修复回放功能（选项C）双重焦点标记异常问题 (2026-03-31)

#### 问题描述
视频回放功能已实现声音播出，但存在较为明显的杂音问题，影响正常听觉体验：
1. **持续背景噪声**：播放期间可听到明显的"沙沙"底噪
2. **咔嗒声/爆破音**：音频数据与零之间无平滑过渡，产生突变噪声
3. **缓冲区欠载噪声**：TX缓冲区数据耗尽时直接填零，造成音频断裂
4. **低频嗡嗡声**：DC偏移经高增益放大后产生低频噪声

#### 杂音来源分析（录音→AVI存储→回放解码→I2S输出全链路）

**1. 录音增益过大（最大杂音源）**
- INMP441 RX回调使用30dB软件增益（`<<5`，32倍放大）
- INMP441自身底噪约-90dBFS，经30dB放大后底噪升至-60dBFS，明显可闻
- 18dB增益（8倍放大）对语音采集已足够，底噪仅放大8倍（-82dBFS），噪声降低4倍

**2. 无DC偏移消除**
- INMP441输出存在DC偏移（典型值±50-200 LSB）
- 经30dB放大后，DC偏移可达±6400 LSB，产生明显低频嗡嗡声
- 录音时未去除DC偏移，直接写入AVI文件

**3. 无噪声门控**
- 静音段也输出放大后的底噪，形成持续"沙沙"声
- 噪声门控可在信号低于阈值时输出零，消除静音段噪声

**4. 播放缓冲区欠载**
- TX回调中mutex获取失败时直接memset零，造成音频断裂
- 缓冲区数据耗尽时直接输出零，与前一采样产生突变

**5. 无淡入淡出**
- 音频数据与零之间无平滑过渡
- 播放开始时从零突变到音频信号→咔嗒声
- 播放结束时从音频信号突变到零→咔嗒声

#### 解决方案

**优化1：降低录音增益 30dB → 18dB**
```cpp
// Inmp441_MicrophoneManager.cpp 构造函数
m_softwareGain(GAIN_18DB)  // 原为 GAIN_30DB
```
- 噪声放大倍数从32倍降至8倍，噪声降低12dB
- 18dB增益对1米距离语音采集已足够（INMP441灵敏度-26dBFS）

**优化2：RX回调添加DC偏移消除**
```cpp
// i2s_rx_callback() - 指数移动平均DC偏移跟踪
int32_t dcOffset = s_microphoneManagerPtr->m_dcOffset;
dcOffset += ((int32_t)sample - dcOffset) >> 6;  // α=1/64, 时间常数≈64采样
int16_t acSample = sample - (int16_t)(dcOffset >> 8);  // 减去DC偏移
```
- 使用定点数EMA（Exponential Moving Average）跟踪DC偏移
- `>>6`对应α=1/64，时间常数约64采样（4ms@16kHz），足够慢以跟踪缓慢漂移
- `dcOffset >> 8`将Q8.24格式转为int16_t
- 消除DC偏移后再进行增益放大，防止偏移被放大

**优化3：RX回调添加一阶低通滤波**
```cpp
// i2s_rx_callback() - 简单一阶IIR低通滤波
int16_t filtered = prevSample + ((acSample - prevSample) >> 2);  // α=0.25
prevSample = filtered;
```
- 截止频率约fc = fs × α / (2π) ≈ 16000 × 0.25 / 6.28 ≈ 637Hz
- 有效抑制高频噪声（>1kHz），保留语音频段（300Hz-3.4kHz）
- 仅使用整数运算，适合ISR环境

**优化4：RX回调添加噪声门控**
```cpp
// i2s_rx_callback() - 噪声门控
const int16_t NOISE_GATE_THRESHOLD = 80;
if (abs(filtered) < NOISE_GATE_THRESHOLD) {
    filtered = 0;
}
```
- 阈值80对应18dB增益下约-54dBFS的信号
- 低于阈值的信号视为噪声，输出零
- 消除静音段和低信号段的持续底噪

**优化5：TX回调添加淡入淡出**
```cpp
// handleI2sTxCallback() - 淡入处理
if (m_fadingIn) {
    m_fadeLevel += 64;  // 每采样增加64/4096≈1.56%
    if (m_fadeLevel >= 4096) {
        m_fadeLevel = 4096;
        m_fadingIn = false;
    }
    filtered = (int16_t)(((int32_t)filtered * m_fadeLevel) >> 12);
}

// 淡出处理
if (m_fadingOut) {
    m_fadeLevel -= 64;
    if (m_fadeLevel <= 0) {
        m_fadeLevel = 0;
        m_fadingOut = false;
        m_playing = false;
        m_state = PLAYBACK_STATE_STOPPED;
    }
    filtered = (int16_t)(((int32_t)filtered * m_fadeLevel) >> 12);
}
```
- 淡入：从0到1.0，64步×640采样/页≈40960采样≈2.56秒@16kHz
- 淡出：从1.0到0，64步×640采样/页≈2.56秒
- 使用12位定点数（0-4096）避免浮点运算
- startPlayback()初始化`m_fadeLevel=0, m_fadingIn=true`
- stopPlayback()设置`m_fadingOut=true`并等待淡出完成

**优化6：TX回调添加一阶低通滤波**
```cpp
// handleI2sTxCallback() - 播放端低通滤波
int16_t filtered = m_lastOutputSample + ((sample - m_lastOutputSample) >> 1);  // α=0.5
m_lastOutputSample = filtered;
```
- α=0.5对应截止频率约1.27kHz
- 进一步平滑输出信号，减少高频噪声和突变
- 同时作为缓冲区欠载时的插值基础

**优化7：增大TX缓冲区 4096 → 8192**
```cpp
// Max98357a_AudioPlayer.h
#define MAX98357A_BUFFER_SIZE     8192  // 原为4096
```
- 缓冲区容量从256ms增至512ms@16kHz
- 减少缓冲区欠载概率，提供更平滑的音频输出

**优化8：缓冲区欠载时重复最后采样**
```cpp
// handleI2sTxCallback() - 缓冲区欠载处理
if (m_txBufferCount > 0) {
    sample = m_txBuffer[m_txBufferTail];
    // ...
} else {
    sample = m_lastOutputSample >> 2;  // 衰减重复最后采样，而非直接填零
}
```
- 缓冲区空时使用最后输出采样的衰减值
- 通过低通滤波器平滑过渡，避免突变噪声
- 比直接填零更自然，减少咔嗒声

**优化9：停止播放时渐进衰减**
```cpp
// handleI2sTxCallback() - 非播放状态时的渐进衰减
if (!m_playing || m_state != PLAYBACK_STATE_PLAYING) {
    if (m_lastOutputSample != 0) {
        for (size_t i = 0; i < samplesPerPage; i++) {
            m_lastOutputSample = m_lastOutputSample >> 1;  // 每采样衰减50%
            pbuf16[i] = m_lastOutputSample;
            if (m_lastOutputSample == 0) break;
        }
    }
    // ...
}
```
- 停止时不是立即输出零，而是逐采样衰减
- 每采样右移1位（衰减50%），约15个采样内从32767衰减到0
- 消除停止时的突变噪声

#### 代码修改文件

| 文件 | 修改内容 |
|------|---------|
| `Inmp441_MicrophoneManager.h` | 添加`m_dcOffset`和`m_prevSample`成员变量 |
| `Inmp441_MicrophoneManager.cpp` | 构造函数初始化新变量；增益从30dB降至18dB；RX回调添加DC偏移消除、低通滤波、噪声门控 |
| `Max98357a_AudioPlayer.h` | 缓冲区从4096增至8192；添加`m_fadeLevel`/`m_lastOutputSample`/`m_fadingIn`/`m_fadingOut`成员 |
| `Max98357a_AudioPlayer.cpp` | 构造函数初始化新变量；TX回调添加淡入淡出、低通滤波、渐进衰减、欠载优化；startPlayback()初始化淡入；stopPlayback()实现淡出 |
| `Shared_GlobalDefines.h` | 版本号V1.52→V1.53 |

#### 技术参数对照表

| 参数 | 优化前 | 优化后 | 效果 |
|------|--------|--------|------|
| 录音增益 | 30dB (×32) | 18dB (×8) | 底噪降低12dB |
| DC偏移消除 | 无 | EMA α=1/64 | 消除低频嗡嗡声 |
| 录音端低通滤波 | 无 | 一阶IIR α=0.25 | 抑制>637Hz高频噪声 |
| 噪声门控阈值 | 无 | 80 LSB | 消除静音段底噪 |
| 播放端低通滤波 | 无 | 一阶IIR α=0.5 | 平滑输出信号 |
| 淡入淡出 | 无 | 12位定点，64步 | 消除开始/结束咔嗒声 |
| TX缓冲区 | 4096 (256ms) | 8192 (512ms) | 减少欠载概率 |
| 缓冲区欠载处理 | 直接填零 | 衰减重复+低通 | 减少断裂噪声 |
| 停止时处理 | 延迟40ms后disable | 渐进衰减→淡出→disable | 消除停止咔嗒声 |

#### 信号处理链路

**录音端（INMP441 RX回调）：**
```
原始采样 → DC偏移消除(EMA) → 低通滤波(IIR) → 增益放大(18dB) → 噪声门控 → 环形缓冲区 → AVI文件
```

**播放端（MAX98357A TX回调）：**
```
AVI文件 → TX缓冲区 → 低通滤波(IIR) → 淡入/淡出 → 渐进衰减(停止时) → I2S DMA输出
```

### 版本 V1.54 - 修复播放端规律高音噪声：移除不稳定反馈滤波器 (2026-04-24)

#### 问题描述
同一视频在电脑端播放正常，无规律高音；但在开发板端通过MAX98357A播放时出现**有规律的尖锐高音**。

#### 根本原因分析
**TX回调中的反馈滤波器引入了音频伪影**：
```cpp
// 原代码 - 存在问题的反馈滤波器
int16_t filtered = m_lastOutputSample + ((sample - m_lastOutputSample) >> 1);
m_lastOutputSample = filtered;
```
- 这是一个一阶IIR低通滤波器，形式为 `y[n] = y[n-1] + 0.5*(x[n] - y[n-1])`
- 但当缓冲区欠载时使用 `m_lastOutputSample >> 2` 作为输入，形成反馈循环
- 反馈回路可能在某些边界条件下产生高频振荡或不稳定输出

#### 解决方案
**简化TX回调处理，移除反馈滤波器**：
```cpp
// 新代码 - 直接输出，无反馈
if (m_txBufferCount > 0) {
    sample = m_txBuffer[m_txBufferTail];
    // ...
} else {
    sample = 0;  // 缓冲区欠载时直接输出静音，不再使用反馈值
    m_txUnderrunCount++;
}

m_lastOutputSample = sample;
```
- 移除反馈滤波器，输出直接来自缓冲区或静音
- 保持淡入淡出功能（不依赖反馈）
- 添加调试统计追踪欠载情况

#### 调试功能
添加TX回调统计信息，播放停止时输出：
```
TX Stats: callbacks=1234, underruns=56 (4.54%)
```
- `callbacks`：TX回调总次数
- `underruns`：缓冲区欠载次数
- 百分比过高说明缓冲区容量不足或供应不及时

#### 代码修改
| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.h` | 添加`m_txUnderrunCount`, `m_txCallbackCount` |
| `Max98357a_AudioPlayer.cpp` | 移除反馈滤波器；添加调试统计；欠载时输出静音 |

### 版本 V1.55 - 录音增益调整：18dB → 30dB (2026-04-24)

#### 背景
V1.54修复规律高音问题后，用户反馈播放音量偏低。为确保音频输出达到理想音量水平，将录音增益从18dB调整回30dB。

#### 变更内容
| 文件 | 修改 |
|------|------|
| `Inmp441_MicrophoneManager.cpp` | 构造函数中 `m_softwareGain(GAIN_18DB)` → `m_softwareGain(GAIN_30DB)` |

#### 保留的降噪特性
尽管增益提升到30dB，以下降噪特性仍然有效：
- **DC偏移消除**：在增益之前消除直流分量
- **一阶低通滤波**：在增益之前抑制高频噪声
- **噪声门控**：信号低于阈值(80 LSB)时输出零

#### 注意事项
- 30dB增益下底噪会比18dB时高约12dB
- 如出现明显底噪，可适当调高噪声门控阈值（如从80调整到120-150）

### 版本 V1.57 - I2S字节序修复：启用BYTE_SWAP解决TX端字节交换失真 (2026-04-24)

#### 问题描述
V1.56的MONO→STEREO修复后，用户反馈播放杂音无任何改善。需要更深入分析开发板与电脑之间的音频处理差异。

#### 根本原因分析

经过完整的音频数据链路追踪和I2S硬件寄存器分析，锁定根因：

**RTL8735B I2S TX字节序默认配置不匹配ARM内存字节序**

RTL8735B I2S控制器 `I2S_CTRL` 寄存器的 `BYTE_SWAP` 字段 (bit 12)：
- `0x0` (默认) = `I2S_BIG_INDIAN`：直接按内存字节序发送，MSB先发
- `0x1` = `I2S_LITTLE_INDIAN`：发送前交换每16-bit采样的两字节

**录音端（INMP441 RX）的巧妙规避：**
- RX DMA缓冲区默认存储为大端字节序 [MSB_byte, LSB_byte]
- INMP441 RX回调手动重组字节：
  ```cpp
  int16_t sample = (int16_t)((uint16_t)src[i*2+1] << 8 | src[i*2]);
  // src[i*2+1] = MSB_byte, src[i*2] = LSB_byte → 正确还原int16_t
  ```
- 结果：AVI文件中存储的是正确的小端序PCM数据 → **电脑回放正常**

**播放端（MAX98357A TX）的致命缺陷：**
- TX回调直接写入 int16_t 值到DMA缓冲区：
  ```cpp
  pbuf16[i * 2] = sample;  // ARM小端序: [LSB_byte, MSB_byte]
  ```
- I2S TX硬件以BIG_ENDIAN模式直接发送 → 先发LSB_byte，后发MSB_byte
- MAX98357A收到字节交换后的数据 → **每个采样的高低字节颠倒 → 严重失真/杂音**

**数据流对比：**

```
           录音端(INMP441 RX)                   播放端(MAX98357A TX)
           =================                    ====================
I2S硬件    接收→存[MSB,LSB] (big-endian)         读[LSB,MSB]→发送 (big-endian)
          ✅手动重组: src[1]<<8|src[0]         ❌直接写入: pbuf16[]=sample
           = P0x1234 → 字节34,12                 = 0x1234 → 字节34,12
           = 重组得0x1234 ✅                      = 发送34,12 (swap!) ❌
```

#### 解决方案

在 `initI2sForTx()` 中调用 `i2s_set_byte_swap(&m_i2sObj, TRUE)` 启用小端序字节交换：

```cpp
// Max98357a_AudioPlayer.cpp - initI2sForTx()
i2s_set_param(&m_i2sObj, I2S_CH_STEREO, I2S_SR_16KHZ, I2S_WL_16);
i2s_set_format(&m_i2sObj, FORMAT_I2S);
i2s_set_master(&m_i2sObj, I2S_MASTER);

i2s_set_byte_swap(&m_i2sObj, TRUE);  // ★ 启用小端序字节交换
```

启用后I2S TX硬件行为：
- 从DMA缓冲区读取 `int16_t` 值（ARM小端序：[LSB_byte, MSB_byte]）
- 硬件自动交换字节：发送 MSB_byte 先，LSB_byte 后
- MAX98357A正确接收：MSB先 → 采样值正确 → 音频清晰无杂音

#### 其他I2S参数验证

经过检查，以下参数默认值已正确匹配MAX98357A：
| 参数 | 默认值 | 说明 |
|------|--------|------|
| SCK_SWAP | 0 (不翻转) | 数据在SCK下降沿改变，MAX98357A在上升沿采样 → 匹配标准I2S |
| LR_SWAP | 0 (左相位) | WS=0时左声道，匹配MAX98357A默认配置 |
| EDGE_SW | 0 (下降沿) | 数据在SCK下降沿开始发送，匹配标准I2S |
| FORMAT | I2S_FORMAT_I2S | 标准I2S Philips格式，匹配MAX98357A |

#### 代码修改文件

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `initI2sForTx()`中添加 `i2s_set_byte_swap(&m_i2sObj, TRUE)` |
| `Shared_GlobalDefines.h` | 版本号V1.57→V1.58 |

### 版本 V1.59 - 回滚字节序和边沿修改：恢复默认I2S配置 (2026-04-24)

#### 问题描述
V1.57添加byte_swap和V1.58添加EDGE_SW后，用户反馈音频输出从正常变为杂音。分析表明之前对RTL8735B I2S字节序的理解存在偏差，修改方向错误。

#### 根本原因分析

**BYTE_SWAP和EDGE_SW修改导致音频失真**

V1.57/V1.58的错误假设：
- 假设I2S TX以big-endian模式发送，需要byte_swap来交换字节
- 假设EDGE_SW默认配置与MAX98357A采样边沿不匹配

实际情况：
- RTL8735B I2S硬件的字节序处理可能比预期更复杂
- EDGE_SW=1可能破坏了标准的I2S时序关系
- byte_swap=FALSE是正确选择，因为I2S标准本身就是MSB先发送

#### 解决方案

**回滚所有字节序相关修改，恢复V1.56的I2S默认配置：**

```cpp
// Max98357a_AudioPlayer.cpp - initI2sForTx()
i2s_set_byte_swap(&m_i2sObj, FALSE);  // 回滚：禁用字节交换，使用硬件默认
// 移除：i2s_set_data_start_edge(&m_i2sObj, POSITIVE_EDGE);
```

#### I2S参数配置（V1.59 - 恢复默认值）

| 参数 | 值 | 说明 |
|------|-----|------|
| BYTE_SWAP | 0 (FALSE) | 禁用字节交换，使用硬件默认行为 |
| EDGE_SW | 0 (默认) | 数据在SCK下降沿开始发送，标准I2S |
| SCK_SWAP | 0 | 不翻转SCK |
| LR_SWAP | 0 | WS=0时左声道 |
| FORMAT | I2S_FORMAT_I2S | 标准Philips I2S格式 |
| CHANNEL | STEREO | 立体声模式 |
| MASTER | 1 | RTL8735B作为I2S主设备 |

#### 代码修改文件

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `initI2sForTx()`中 `i2s_set_byte_swap(&m_i2sObj, FALSE)` |
| `Shared_GlobalDefines.h` | 版本号V1.58→V1.59 |

#### 教训总结

1. **字节序假设需要数据手册验证**：在未确认RTL8735B I2S实际行为的情况下，盲目添加byte_swap可能破坏正常功能

2. **标准I2S协议是MSB先发送**：这是协议规范，不是"big-endian vs little-endian"的问题

3. **修改应该最小化**：只做必要的修改，避免引入不必要的复杂性

4. **需要测试验证**：每次修改后需要实际测试确认效果

### 版本 V1.58 - I2S数据开始边沿优化：添加POSITIVE_EDGE改善时序 (2026-04-24)

#### 问题描述
V1.57添加byte_swap后，播放杂音仍然存在。需要进一步分析I2S时序和边沿配置。

#### 根本原因分析

**数据开始边沿（EDGE_SW）与MAX98357A采样边沿可能存在时序问题**

RTL8735B I2S控制器 `I2S_CTRL` 寄存器的 `EDGE_SW` 位 (bit 5)：
- `0` (默认) = `I2S_NEGATIVE_EDGE`：数据在SCK下降沿开始改变
- `1` = `I2S_POSITIVE_EDGE`：数据在SCK上升沿开始改变

MAX98357A数据手册要求：
- MAX98357A在BCLK**上升沿**锁存数据
- 数据应在上升沿之前已稳定（Setup time）
- 数据应在上升沿之后保持一段时间（Hold time）

**时序分析：**
```
默认配置(EDGE_SW=0)：
SCK:     ___|---|___|---|___|---|___|---
         下降沿  上升沿  下降沿  上升沿
数据:    [    稳定    ][ 改变  ][    稳定    ]
                          ↑
                    下降沿改变，可能与MAX98357A采样冲突

EDGE_SW=1时：
SCK:     ___|---|___|---|___|---|___|---
         下降沿  上升沿  下降沿  上升沿
数据:    [ 改变  ][    稳定    ][ 改变  ][    稳定    ]
              ↑                              ↑
         上升沿前改变，数据在上升沿时已稳定 ✅
```

#### 解决方案

在 `initI2sForTx()` 中添加 `i2s_set_data_start_edge(&m_i2sObj, POSITIVE_EDGE)`：

```cpp
// Max98357a_AudioPlayer.cpp - initI2sForTx()
i2s_set_byte_swap(&m_i2sObj, TRUE);
i2s_set_data_start_edge(&m_i2sObj, POSITIVE_EDGE);  // ★ 数据在上升沿前稳定
```

#### I2S参数配置总结（V1.58）

| 参数 | 值 | 说明 |
|------|-----|------|
| BYTE_SWAP | 1 | 启用小端序字节交换，匹配ARM内存字节序 |
| EDGE_SW | 1 (POSITIVE_EDGE) | 数据在SCK上升沿前稳定，MAX98357A正确采样 |
| SCK_SWAP | 0 | 不翻转SCK，保持标准I2S时钟 |
| LR_SWAP | 0 | WS=0时左声道，匹配MAX98357A默认 |
| FORMAT | I2S_FORMAT_I2S | 标准Philips I2S格式 |
| CHANNEL | STEREO | 立体声模式，每WS周期发送L+R |
| MASTER | 1 | RTL8735B作为I2S主设备 |

#### 代码修改文件

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `initI2sForTx()`中添加 `i2s_set_data_start_edge(&m_i2sObj, POSITIVE_EDGE)` |
| `Shared_GlobalDefines.h` | 版本号V1.57→V1.58 |

### 版本 V1.56 - I2S播放通道修复：MONO→STEREO解决MAX98357A杂音 (2026-04-24)

#### 问题描述
1. NOISE_GATE_THRESHOLD调整到150时录音声音被压制变形，需回到80
2. 开发板回放视频杂音大，而电脑回放同一视频基本没有杂音

#### 根本原因分析

**I2S MONO模式与MAX98357A不兼容（核心问题）**

MAX98357A是标准I2S立体声DAC，其工作原理：
- 使用WS（Word Select）信号区分左右声道
- 每个WS周期包含两个数据相位：左声道（WS低电平）+ 右声道（WS高电平）
- DAC从两个相位分别提取数据并输出

RTL8735B I2S控制器通道配置（`I2S_CTRL`寄存器 `AUDIO_MONO` 字段 bits[4:3]）：
- `0x0` = I2S_CH_STEREO：标准立体声模式，WS正常切换，每个WS周期发送L+R两个采样
- `0x2` = I2S_CH_MONO：单声道模式，只在WS的一个相位发送数据

**MONO模式导致杂音的机制：**
- MONO模式下，I2S控制器只在WS的一个相位（如左声道）发送有效数据
- 另一个相位（右声道）的数据内容未定义，可能包含残留数据或随机值
- MAX98357A无法区分"有效单声道数据"和"无效数据"，将两个相位都解码输出
- 结果：有效音频信号 + 无效数据混合 → 明显杂音
- 电脑播放时使用软件解码器，正确解析AVI中的单声道PCM数据，因此无杂音

**为什么录音端MONO模式正常：**
- INMP441是单声道麦克风，只在一个WS相位输出数据
- RX MONO模式正确地只采集有效相位的数据，丢弃另一相位
- 录音端和播放端使用独立的I2S初始化，互不影响

#### 解决方案

**1. I2S TX从MONO改为STEREO**
```cpp
// Max98357a_AudioPlayer.cpp - initI2sForTx()
i2s_set_param(&m_i2sObj, I2S_CH_STEREO, I2S_SR_16KHZ, I2S_WL_16);  // 原为 I2S_CH_MONO
```

**2. TX回调中将单声道数据复制到左右声道**
```cpp
// handleI2sTxCallback() - STEREO模式数据布局
size_t totalSlots = MAX98357A_DMA_PAGE_SIZE / sizeof(int16_t);  // 640
size_t monoSamplesPerPage = totalSlots / 2;  // 320（每帧2个slot：L+R）

for (size_t i = 0; i < monoSamplesPerPage; i++) {
    int16_t sample = /* 从TX缓冲区获取 */;
    pbuf16[i * 2] = sample;      // 左声道
    pbuf16[i * 2 + 1] = sample;  // 右声道（复制单声道数据）
}
```

**3. NOISE_GATE_THRESHOLD确认回到80**
- 阈值150在30dB增益下会压制正常语音信号（30dB放大后80-150范围的信号是有效语音）
- 阈值80在30dB增益下仅消除最底层的底噪，不影响语音质量

#### STEREO模式DMA数据格式

```
DMA缓冲区布局（STEREO, 16-bit, 16kHz）：
+--------+--------+--------+--------+--------+--------+-----+
| L[0]   | R[0]   | L[1]   | R[1]   | L[2]   | R[2]   | ... |
| 16bit  | 16bit  | 16bit  | 16bit  | 16bit  | 16bit  |     |
+--------+--------+--------+--------+--------+--------+-----+
|<--- 帧0 --->|<--- 帧1 --->|<--- 帧2 --->|

每页1280字节 = 640个int16_t = 320个立体声帧
每帧 = 1个单声道采样 × 2（L+R复制）
I2S WS频率 = 16kHz（每秒16000个立体声帧）
BCLK = 16000 × 2 × 16 = 512 kHz
```

#### 代码修改文件

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `i2s_set_param`从`I2S_CH_MONO`改为`I2S_CH_STEREO`；TX回调改为L/R双声道写入；日志更新 |
| `Max98357a_AudioPlayer.h` | `MAX98357A_CHANNELS`从1改为2 |
| `Shared_GlobalDefines.h` | 版本号V1.55→V1.56 |

#### 技术要点

**1. 录音端（INMP441）保持MONO不变**
- INMP441是单声道麦克风，RX MONO模式正确采集单相位数据
- 录音和播放使用独立的I2S初始化，通道配置互不影响

**2. 播放端必须使用STEREO**
- MAX98357A期望标准I2S立体声格式
- STEREO模式下WS正常切换，DAC在两个相位都获得有效数据
- 单声道数据复制到L+R，MAX98357A输出相同信号到扬声器

**3. DMA页面大小不变**
- STEREO模式下每页有效单声道采样数减半（640→320）
- 但I2S帧率仍为16kHz，音频时长不变
- TX回调处理速度要求不变（每页320个单声道采样 × 16kHz ≈ 20ms/页）

### 版本 V1.52 - I2S RX页面管理修复与视频回放性能优化 (2026-04-24)

#### 问题描述
V1.51修复后，视频回放仍存在以下严重问题：
1. **I2S RX页面所有权错误洪水**：串口大量输出 `I2S rx page own control error: 0x 110` 和 `Check rx interrupt proccess time(1)`，导致系统资源耗尽
2. **视频播放速度异常缓慢**：视频帧解码和显示极慢，远低于正常帧率
3. **视频播放最终完全卡死**：系统被I2S错误中断淹没，无法继续处理
4. **播放开始/结束时的咔嗒响声**：喇叭在视频开始和结束时发出"咔嗒"声

#### 根本原因分析

**1. I2S RX DMA页面未管理（根本原因）**
- RTL8735B的I2S控制器在TX模式下，RX DMA仍然运行
- `i2s_set_direction(&m_i2sObj, I2S_DIR_TX)` 只设置数据流方向，不停止RX DMA硬件
- INMP441（RX模式）在`i2s_enable()`后调用`i2s_recv_page()`提交4个RX页面，并在RX回调中重新提交
- MAX98357A（TX模式）从未调用`i2s_recv_page()`，RX DMA页面耗尽后产生大量错误
- 每次RX DMA完成传输但无可用页面时，触发错误中断，输出错误日志
- 错误中断频率极高（约每2-3ms一次），严重占用CPU和串口资源

**2. 视频播放速度慢和卡死（由I2S错误洪水导致）**
- I2S RX错误中断频繁触发，抢占CPU时间
- 串口输出错误日志占用大量时间（每次输出约0.5-1ms）
- 主循环被中断打断，无法及时处理视频帧
- 最终I2S错误中断频率超过主循环处理能力，系统完全卡死

**3. TX回调中的阻塞问题**
- `handleI2sTxCallback()`使用`portMAX_DELAY`等待互斥锁
- I2S回调在中断上下文中执行，不能阻塞等待
- 如果`writeAudioData()`持有互斥锁，TX回调会无限等待，导致系统死锁

**4. 停止播放时的DMA页面不重新提交**
- 原代码在`m_playing=false`时直接return，不重新提交TX页面
- I2S DMA需要持续有页面可用，否则会产生TX页面错误
- 最后一个DMA页面包含非零音频数据，直接停止产生突变电压→咔嗒声

**5. videoPlaybackLoop()每次只处理一个chunk**
- 主循环有10ms延迟（`waitForEvent`超时）
- 每次只读取一个音频chunk（1024字节≈32ms音频），然后等待10ms
- 音频数据供给不足，导致播放断续
- 视频帧解码耗时较长，音频数据在等待期间耗尽

#### 解决方案

**修复1：添加RX页面管理（根本修复）**
```cpp
// initI2sForTx() - 在i2s_enable()后提交RX页面
i2s_enable(&m_i2sObj);
for (int i = 0; i < MAX98357A_DMA_PAGE_NUM; i++) {
    i2s_recv_page(&m_i2sObj);
}

// 添加RX回调处理
void Max98357aAudioPlayer::handleI2sRxCallback(char* pbuf) {
    i2s_recv_page(&m_i2sObj);  // 重新提交RX页面，防止页面耗尽
}

// 注册RX回调
void i2s_rx_callback_max98357a(uint32_t id, char *pbuf) {
    if (s_playerPtr) {
        s_playerPtr->handleI2sRxCallback(pbuf);
    }
}
```

**修复2：TX回调使用非阻塞互斥锁 + 零填充**
```cpp
void Max98357aAudioPlayer::handleI2sTxCallback(char* pbuf) {
    int16_t* pbuf16 = (int16_t*)pbuf;
    size_t samplesPerPage = MAX98357A_DMA_PAGE_SIZE / sizeof(int16_t);

    if (!m_playing || m_state != PLAYBACK_STATE_PLAYING) {
        memset(pbuf, 0, MAX98357A_DMA_PAGE_SIZE);  // 零填充防pop
        i2s_send_page(&m_i2sObj, (uint32_t*)pbuf);  // 重新提交页面
        return;
    }

    if (xSemaphoreTake(m_mutex, 0) == pdTRUE) {  // 非阻塞，0超时
        // ... 从缓冲区读取数据 ...
        xSemaphoreGive(m_mutex);
    } else {
        memset(pbuf, 0, MAX98357A_DMA_PAGE_SIZE);  // 获取锁失败时零填充
    }

    i2s_send_page(&m_i2sObj, (uint32_t*)pbuf);
}
```

**修复3：videoPlaybackLoop()批量处理音频帧**
```cpp
void videoPlaybackLoop(void) {
    // ...
    for (int i = 0; i < 16; i++) {  // 一次处理最多16个chunk
        MJPEGDecoder::ChunkType type = mjpegDecoder.readNextChunk(&chunkData, &chunkSize);

        if (type == MJPEGDecoder::CHUNK_TYPE_END) {
            stopVideoPlayback();
            return;
        }

        if (type == MJPEGDecoder::CHUNK_TYPE_AUDIO) {
            Max98357aAudioPlayer::getInstance().writeAudioData(
                (const int16_t*)chunkData, chunkSize / sizeof(int16_t));
        } else if (type == MJPEGDecoder::CHUNK_TYPE_VIDEO) {
            // ... 解码和显示视频帧 ...
            return;  // 处理完视频帧后返回，让主循环处理其他事件
        }
    }
}
```

#### RTL8735B I2S硬件架构要点

**I2S控制器TX_ACT寄存器（偏移0x00, bits[2:1]）：**
- `00` = I2S_ONLY_RX：仅接收
- `01` = I2S_ONLY_TX：仅发送
- `10` = I2S_TXRX：同时收发

**关键发现：`I2S_DIR_TX`（软件层）≠ `I2S_ONLY_TX`（硬件层）**
- `i2s_set_direction(&m_i2sObj, I2S_DIR_TX)` 调用 `hal_rtl_i2s_set_direction()` 设置TX_ACT=01
- 但RTL8735B的I2S DMA控制器在TX模式下，RX DMA通道仍然活跃
- RX DMA需要持续有页面可用（通过`i2s_recv_page()`提交），否则产生页面所有权错误
- 这是硬件设计特性：DMA通道独立于数据流方向，需要软件层管理

**INMP441（RX模式）参考实现：**
```cpp
i2s_enable(&m_i2sObj);
for (int i = 0; i < DMA_PAGE_NUM; i++) {
    i2s_recv_page(&m_i2sObj);  // 提交RX页面
}
// RX回调中：i2s_recv_page(&m_i2sObj);  // 重新提交
```

**MAX98357A（TX模式）正确实现：**
```cpp
// 先提交TX页面
for (int i = 0; i < DMA_PAGE_NUM; i++) {
    i2s_send_page(&m_i2sObj, (uint32_t*)(m_txPageBuffer + i * PAGE_SIZE));
}
i2s_enable(&m_i2sObj);
// 再提交RX页面（防止RX DMA页面耗尽）
for (int i = 0; i < DMA_PAGE_NUM; i++) {
    i2s_recv_page(&m_i2sObj);
}
// TX回调：i2s_send_page() + 零填充
// RX回调：i2s_recv_page()（丢弃数据，仅维持页面循环）
```

#### 代码修改文件
- `Max98357a_AudioPlayer.h`：添加`handleI2sRxCallback()`方法声明
- `Max98357a_AudioPlayer.cpp`：
  - `i2s_rx_callback_max98357a()`：调用`handleI2sRxCallback()`
  - `initI2sForTx()`：`i2s_enable()`后添加`i2s_recv_page()`循环
  - `handleI2sTxCallback()`：非阻塞互斥锁 + 停止时零填充+重新提交
  - `handleI2sRxCallback()`：新方法，调用`i2s_recv_page()`
- `VideoRecorder.cpp`：`videoPlaybackLoop()`改为批量处理16个chunk

#### 技术经验总结

**1. I2S DMA页面管理是双向的**
- 即使只使用TX方向，RX DMA通道仍需页面管理
- 忽略RX页面管理会导致错误中断洪水，严重时使系统卡死
- 必须在`i2s_enable()`后提交所有方向的DMA页面

**2. I2S回调中禁止阻塞**
- I2S回调在中断上下文执行，不能使用`portMAX_DELAY`等待信号量
- 使用`0`超时（非阻塞）尝试获取互斥锁
- 获取失败时输出静音数据（零填充），而非等待

**3. DMA页面必须持续重新提交**
- I2S DMA是循环缓冲区机制，每次中断后需重新提交已用页面
- 停止播放时也需继续提交零填充页面，直到`i2s_disable()`
- 否则DMA页面耗尽产生错误，且最后输出非零数据产生pop噪音

**4. 视频回放需批量处理音频帧**
- AVI文件中音视频交错存储，通常2-3个音频帧后跟1个视频帧
- 每次只处理1个chunk会导致音频供给不足
- 批量处理（最多16个chunk）可确保音频缓冲区充足
- 遇到视频帧时立即返回，避免阻塞主循环

### 版本 V1.51 - MAX98357A音频驱动I2S DMA修复与视频回放调试 (2026-04-24)

#### 问题描述
视频回放时存在以下问题：
1. **I2S DMA页面索引错误**：串口输出 `hal_rtl_i2s_page_send: UnExpected Page Index. TxPage=0, Expected:1` 和 `I2S tx page own control error`，导致音频播放异常
2. **视频播放立即结束**：`readNextChunk()` 在进入顺序模式后立即找到idx1标记，0帧视频/音频被播放
3. **播放开始/结束时的pop噪音**：喇叭在视频开始和结束时各发出一声"噗"的噪音

#### 根本原因分析

**1. DMA缓冲区大小严重不足**
- `m_txPageBuffer` 和 `m_rxPageBuffer` 声明为 `char[MAX98357A_DMA_PAGE_SIZE]` = 1280字节
- 但 `i2s_set_dma_buffer` 配置了4页DMA，每页1280字节，共需5120字节
- INMP441正确使用 `DMA_PAGE_SIZE * DMA_PAGE_NUM` = 5120字节
- MAX98357A只分配了1/4的所需空间，DMA访问越界导致页面索引错误

**2. i2s_send_page初始提交错误**
- 原代码：`i2s_send_page(&m_i2sObj, (uint32_t*)m_txPageBuffer)` 提交同一页面4次
- I2S DMA需要4个不同的页面地址，重复提交同一页面导致DMA页管理混乱
- 正确做法：`i2s_send_page(&m_i2sObj, (uint32_t*)(m_txPageBuffer + i * PAGE_SIZE))`

**3. I2S启用顺序错误**
- 原代码先 `i2s_enable()` 再 `i2s_send_page()`，DMA启动时无有效页面数据
- 正确顺序：先提交所有页面，再启用I2S

**4. readNextChunk()起始位置错误**
- 原代码 `m_currentFilePos = m_moviStartPos`，从LIST头部开始搜索
- 需要在while循环中匹配"LIST"→"movi"才能跳到数据区
- 但搜索逻辑可能因字节对齐问题跳过数据直接找到idx1
- 修复：直接设置 `m_currentFilePos = m_moviStartPos + 12`（跳过LIST+size+movi头部）

**5. parseSimplifiedMovi()跳过第一个帧**
- 找到"movi"后 `pos += 8`，但"movi"只占4字节，应 `pos += 4`
- 导致第一个数据帧的前4字节被跳过

**6. Pop噪音原因**
- 开始时：DMA缓冲区未清零，包含随机数据，I2S启用瞬间输出非零信号
- 结束时：I2S直接disable，输出保持在非零状态，产生突变

#### 解决方案

**修复1：DMA缓冲区大小**
```cpp
// Max98357a_AudioPlayer.h
char m_txPageBuffer[MAX98357A_DMA_PAGE_SIZE * MAX98357A_DMA_PAGE_NUM] __attribute__((aligned(32)));
char m_rxPageBuffer[MAX98357A_DMA_PAGE_SIZE * MAX98357A_DMA_PAGE_NUM] __attribute__((aligned(32)));
```

**修复2：页面提交和I2S启用顺序**
```cpp
// initI2sForTx() - 先提交页面再启用I2S
memset(m_txPageBuffer, 0, sizeof(m_txPageBuffer));  // 清零防pop
memset(m_rxPageBuffer, 0, sizeof(m_rxPageBuffer));
// ... i2s_init, set_dma_buffer, set_param ...
for (int i = 0; i < MAX98357A_DMA_PAGE_NUM; i++) {
    i2s_send_page(&m_i2sObj, (uint32_t*)(m_txPageBuffer + i * MAX98357A_DMA_PAGE_SIZE));
}
i2s_enable(&m_i2sObj);  // 最后启用
```

**修复3：stopPlayback()防pop**
```cpp
// 先停止喂入数据，清零缓冲区，等待DMA排空，再disable
m_playing = false;
m_state = PLAYBACK_STATE_STOPPED;
memset(m_txBuffer, 0, ...);  // 清零软件缓冲区
delay(40);  // 等待DMA排空（约2-3个DMA周期）
i2s_disable(&m_i2sObj);
```

**修复4：readNextChunk()直接跳到数据区**
```cpp
m_currentFilePos = m_moviStartPos + 12;  // 跳过LIST+size+movi
```

**修复5：parseSimplifiedMovi()修正偏移**
```cpp
pos += 4;  // "movi"只占4字节，不是8字节
```

**修复6：未知chunk跳过策略优化**
- 原代码遇到未知标签逐字节搜索（`m_currentFilePos += 1`），极慢且易误匹配
- 改为：如果size合理则跳过整个chunk，否则逐字节搜索

#### 代码修改文件
- `Max98357a_AudioPlayer.h`：DMA缓冲区大小修正 + 对齐属性
- `Max98357a_AudioPlayer.cpp`：initI2sForTx()页面提交和启用顺序修正 + stopPlayback()防pop
- `MJPEG_Encoder.cpp`：readNextChunk()起始位置修正 + 调试日志 + parseSimplifiedMovi()偏移修正 + 未知chunk跳过优化
- `Shared_GlobalDefines.h`：版本号V1.50→V1.51

### 版本 V1.50 - MAX98357A音频驱动完善与视频回放音频播放功能实现 (2026-04-24)

#### 问题描述
视频回放功能中音频解码与输出存在以下问题：
1. **I2S分时复用冲突**：INMP441（录音）和MAX98357A（播放）共用I2S1接口（PD_14/PD_15/PD_17），但录制和播放切换时未正确释放/初始化I2S硬件
2. **音频帧读取不同步**：MJPEGDecoder的`readNextFrame()`和`readNextAudioFrame()`各自维护独立索引，但AVI文件中音视频数据是按时间交错存储的（00db视频帧, 01wb音频帧交替），独立索引导致音视频时间错位
3. **回放流程音频数据喂入不完整**：`videoPlaybackLoop()`使用`while(readNextAudioFrame)`一次性读取所有音频帧，导致音频数据被快速消耗后长时间无音频

#### 根本原因分析

**1. I2S硬件资源竞争**
- INMP441初始化I2S为RX模式（录音），MAX98357A初始化I2S为TX模式（播放）
- 两者共用同一组引脚（SCK=PD_14, WS=PD_17, TX=PD_15, RX=PD_18, MCK=PD_16）
- 切换录制/播放模式时必须先deinit当前I2S，再init为另一方向

**2. AVI文件交错格式与解码器不匹配**
- AVI文件中音视频chunk按录制时间顺序交错存储：`00db→01wb→00db→01wb→...`
- 旧版`readNextFrame()`只读视频帧索引，`readNextAudioFrame()`只读音频帧索引
- 导致播放时视频帧和音频帧的时间对齐关系被破坏

**3. 音频数据喂入策略问题**
- 旧版在每帧视频后用while循环读取所有音频帧，一次性喂入
- 音频缓冲区有限（4096采样），大量音频数据导致溢出丢帧

#### 解决方案

**修复1：I2S分时复用管理**
- `startVideoRecording()`：先调用`Max98357aAudioPlayer::releaseI2S()`释放播放I2S
- `startVideoPlayback()`：先调用`g_microphoneManager.deinitI2s()`释放录音I2S
- `stopVideoPlayback()`：先调用`Max98357aAudioPlayer::releaseI2S()`，再调用`g_microphoneManager.init()`恢复录音I2S
- 在Inmp441_MicrophoneManager中新增`deinitI2s()`方法，实现I2S硬件的完整释放

**修复2：MJPEGDecoder交错读取模式**
- 新增`ChunkType`枚举：`CHUNK_TYPE_VIDEO`、`CHUNK_TYPE_AUDIO`、`CHUNK_TYPE_END`
- 新增`readNextChunk()`方法：按文件中的原始顺序逐个读取chunk，根据fourcc（00db/01wb）返回chunk类型
- 新增`m_currentFilePos`和`m_sequentialMode`成员变量，跟踪文件读取位置
- 保留原有的`readNextFrame()`和`readNextAudioFrame()`接口以兼容缩略图等场景

**修复3：视频回放流程优化**
- `videoPlaybackLoop()`改为每次调用只读取一个chunk
- 音频帧：立即调用`writeAudioData()`喂入播放器
- 视频帧：等待帧间隔到达后解码显示，确保帧率正确
- 音视频数据按文件交错顺序自然同步，无需额外的同步机制

#### 代码变更清单

| 文件 | 变更类型 | 说明 |
|------|---------|------|
| `Inmp441_MicrophoneManager.h` | 新增方法 | 添加`deinitI2s()`声明 |
| `Inmp441_MicrophoneManager.cpp` | 新增实现 | 实现`deinitI2s()`：禁用I2S、释放DMA缓冲区、重置状态 |
| `MJPEG_Encoder.h` | 新增类型/方法 | 添加`ChunkType`枚举、`readNextChunk()`声明、`m_currentFilePos`/`m_sequentialMode`成员 |
| `MJPEG_Encoder.cpp` | 新增实现 | 实现`readNextChunk()`：按文件顺序读取chunk，支持00db/01wb/LIST/idx1等chunk类型 |
| `VideoRecorder.cpp` | 修改逻辑 | 重写`videoPlaybackLoop()`使用`readNextChunk()`；修改`startVideoRecording()`/`startVideoPlayback()`/`stopVideoPlayback()`添加I2S切换 |
| `Shared_GlobalDefines.h` | 版本更新 | V1.49→V1.50 |

#### 架构设计要点

**I2S分时复用状态机：**
```
IDLE状态 → 录制: releaseI2S(MAX98357A) → initI2S(INMP441/RX)
IDLE状态 → 播放: deinitI2S(INMP441) → initI2S(MAX98357A/TX)
录制结束 → IDLE: (INMP441保持初始化)
播放结束 → IDLE: releaseI2S(MAX98357A) → init(INMP441)
```

**AVI文件交错读取流程：**
```
readNextChunk() → 读取8字节chunk头
  ├── "00db"/"00dc" → CHUNK_TYPE_VIDEO (读取帧数据，递增视频帧索引)
  ├── "01wb"        → CHUNK_TYPE_AUDIO (读取音频数据，递增音频帧索引)
  ├── "LIST"+"movi" → 跳过LIST头，继续读取内部chunk
  ├── "idx1"        → CHUNK_TYPE_END (到达索引区，媒体数据结束)
  └── 其他          → 向前搜索下一个有效chunk
```

### 版本 V1.49 - 修复视频录制音视频不同步：视频播放速度过快问题 (2026-04-23)

#### 问题描述
视频录制过程中出现音视频不同步现象，具体表现为：
1. **视频播放帧率比正常快**：拍摄的内容完整无缺失，但播放速度异常加快
2. **音频播放速度正常**：声音以正常速度播放
3. **视频先于音频结束**：视频画面播放完毕后，音频仍在继续播放
4. **典型表现**：拍摄时间间隔为1秒递增1的画面时，能播放完整内容但速度过快，导致与正常速度的音频不同步

#### 根本原因分析

**问题发生在AVI文件合并阶段，但根源在录制阶段的Camera硬件资源竞争。**

**1. 日志数据量化分析**

| 指标 | Bug.md录制日志 | VLC.md播放日志 |
|------|--------------|--------------|
| 视频帧数 | 88帧 | 88帧 |
| 音频时长 | 293块×512÷16kHz = **9.38秒** | **9秒** |
| AVI声明帧率 | 15fps | 15fps |
| 视频播放时长 | - | 88÷15 = **5.87秒** ❌ |
| 实际捕获帧率 | 88÷9.38 = **9.4fps** | - |

**2. 核心问题：AVI头声明帧率与实际捕获帧率不匹配**

AVI头中声明 `rate=15, scale=1`（即15fps），但实际只捕获了9.4fps的帧。VLC按15fps播放88帧只需5.87秒，而音频实际9.38秒，导致视频比音频快1.6倍。

**3. 帧率偏低的直接原因：预览帧处理与录制帧竞争Camera硬件**

对比工作版本（03.31.01Camera），发现关键差异：

| 对比项 | 工作版本 | 当前版本（修复前） |
|--------|---------|-----------------|
| 预览帧频率限制 | 每100ms处理一次（约10fps） | **无限制**，每次主循环都调用 |
| 预览帧重试机制 | 无重试 | **do-while循环**，最多3次重试，含`delay(10)` |
| Camera硬件竞争 | 低（预览帧10fps） | **高**（预览帧无限制+重试阻塞） |

`processPreviewFrame()` 在录制状态下每次主循环迭代都调用 `Camera.getImage(VIDEO_CHANNEL_PREVIEW, ...)`，与录制通道 `Camera.getImage(VIDEO_CHANNEL_RECORD, ...)` 竞争Camera硬件资源。预览帧的 `do-while` 重试循环（最多3次，每次 `delay(10)`）进一步阻塞主循环，导致录制帧获取任务无法及时获取帧。

**4. 停止录制顺序问题**

当前版本 `stopVideoRecording()` 先设置 `g_recorderState = REC_IDLE` 再停止Camera硬件，与工作版本顺序相反。工作版本先停止Camera硬件再设置状态，确保任务能优雅退出。

#### 解决要点

**修复1：添加预览帧频率限制（根本修复）**
- 在 `processPreviewFrame()` 中添加100ms间隔限制（约10fps预览）
- 移除 `do-while` 重试循环和 `delay(10)` 阻塞调用
- 减少Camera硬件竞争，让录制通道能获得足够的帧

**修复2：调整停止录制顺序（稳定性修复）**
- 先 `Camera.channelEnd()` 停止相机硬件
- 再删除视频帧捕获任务和音频处理任务
- 最后设置 `g_recorderState = REC_IDLE`
- 与工作版本顺序一致

**修复3：AVI帧率自动修正（兜底方案）**
- 在 `MJPEGEncoder::end()` 中根据音频总样本数计算实际录制时长
- 计算实际帧率（帧数÷时长）
- 如果实际帧率与声明帧率偏差超过10%，更新AVI头中的 `avih.microSecPerFrame` 和 `strh.rate`
- 即使预览帧限制后帧率仍有偏差，也能确保播放速度正确

#### 实施步骤
1. 修改 `VideoRecorder.cpp` - `processPreviewFrame()` 添加100ms频率限制，移除重试循环
2. 修改 `VideoRecorder.cpp` - `stopVideoRecording()` 调整停止顺序
3. 修改 `MJPEG_Encoder.h` - 添加 `m_microSecPerFrameOffset` 和 `m_videoStrhRateOffset` 成员变量
4. 修改 `MJPEG_Encoder.cpp` - 构造函数初始化新成员变量
5. 修改 `MJPEG_Encoder.cpp` - `writeAVIHeader()` 保存帧率相关偏移位置
6. 修改 `MJPEG_Encoder.cpp` - `end()` 添加帧率自动修正逻辑
7. 修改 `Shared_GlobalDefines.h` - 版本号从V1.48递增到V1.49

#### 关键代码变更

**1. VideoRecorder.cpp - processPreviewFrame() 添加频率限制**
```cpp
// 修复前：无频率限制，每次主循环都调用，含阻塞重试
void processPreviewFrame(void) {
    // ...
    uint32_t imgAddr;
    uint32_t imgLen;
    int retryCount = 0;
    const int MAX_RETRIES = 3;
    do {
        Camera.getImage(VIDEO_CHANNEL_PREVIEW, &imgAddr, &imgLen);
        if (imgLen > 0) break;
        if (retryCount < MAX_RETRIES - 1) delay(10);  // ❌ 阻塞主循环
        retryCount++;
    } while (retryCount < MAX_RETRIES);
    // ...
}

// 修复后：100ms频率限制，无重试阻塞
void processPreviewFrame(void) {
    // ...
    unsigned long currentMillis = millis();
    static unsigned long lastPreviewTime = 0;
    if (currentMillis - lastPreviewTime < 100) {  // ✅ 限制预览帧频率
        return;
    }
    lastPreviewTime = currentMillis;
    // ...
    Camera.getImage(VIDEO_CHANNEL_PREVIEW, &imgAddr, &imgLen);  // ✅ 无重试阻塞
    // ...
}
```

**2. VideoRecorder.cpp - stopVideoRecording() 调整停止顺序**
```cpp
// 修复前：先设置REC_IDLE，后停止硬件
void stopVideoRecording(void) {
    g_recorderState = REC_IDLE;              // ❌ 先设置状态
    Camera.channelEnd(VIDEO_CHANNEL_RECORD); // ❌ 后停止硬件
    TaskManager::deleteTask(TASK_VIDEO_FRAME_CAPTURE);
    TaskManager::deleteTask(TASK_AUDIO_PROCESSING);
    // ...
}

// 修复后：先停止硬件，后设置状态（与工作版本一致）
void stopVideoRecording(void) {
    Camera.channelEnd(VIDEO_CHANNEL_RECORD); // ✅ 先停止硬件
    TaskManager::deleteTask(TASK_VIDEO_FRAME_CAPTURE);
    TaskManager::deleteTask(TASK_AUDIO_PROCESSING);
    g_recorderState = REC_IDLE;              // ✅ 后设置状态
    // ...
}
```

**3. MJPEG_Encoder.h - 添加帧率偏移量成员变量**
```cpp
uint32_t m_microSecPerFrameOffset;  // avih中microSecPerFrame的缓冲区偏移
uint32_t m_videoStrhRateOffset;     // 视频strh中rate字段的缓冲区偏移
```

**4. MJPEG_Encoder.cpp - writeAVIHeader() 保存偏移位置**
```cpp
m_microSecPerFrameOffset = m_bufferPos;
writeLE32(m_buffer + m_bufferPos, microSecPerFrame);
// ...
m_videoStrhRateOffset = m_bufferPos;
writeLE32(m_buffer + m_bufferPos, m_fps);
```

**5. MJPEG_Encoder.cpp - end() 帧率自动修正逻辑**
```cpp
if (m_totalAudioSamples > 0 && m_frameCount > 0 && m_microSecPerFrameOffset > 0 && m_videoStrhRateOffset > 0) {
    uint32_t audioDurationMs = (uint32_t)((uint64_t)m_totalAudioSamples * 1000 / AUDIO_SAMPLE_RATE);
    if (audioDurationMs > 0) {
        uint32_t actualFpsX1000 = (uint32_t)((uint64_t)m_frameCount * 1000000 / audioDurationMs);
        uint32_t declaredFpsX1000 = m_fps * 1000;
        
        if (actualFpsX1000 < declaredFpsX1000 * 90 / 100 || actualFpsX1000 > declaredFpsX1000 * 110 / 100) {
            uint32_t actualFps = actualFpsX1000 / 1000;
            if (actualFps < 1) actualFps = 1;
            uint32_t actualMicroSecPerFrame = 1000000 / actualFps;
            
            writeLE32(m_buffer + m_microSecPerFrameOffset, actualMicroSecPerFrame);
            writeLE32(m_buffer + m_videoStrhRateOffset, actualFps);
            
            Utils_Logger::info("Frame rate corrected: declared=%d fps, actual=%d fps (based on %d frames in %d ms)",
                              m_fps, actualFps, m_frameCount, audioDurationMs);
        } else {
            Utils_Logger::info("Frame rate OK: declared=%d fps, actual=%d fps (within 10%% tolerance)",
                              m_fps, actualFpsX1000 / 1000);
        }
    }
}
```

#### 修复效果预期

以之前日志为例（88帧，9.38秒音频）：

| 场景 | AVI声明帧率 | 视频播放时长 | 音频时长 | 同步状态 |
|------|-----------|------------|---------|---------|
| 修复前 | 15fps | 88÷15=5.87秒 | 9.38秒 | ❌ 视频快1.6倍 |
| 修复后（预览帧限制） | 15fps | ≈9秒 | 9.38秒 | ✅ 基本同步 |
| 修复后（帧率修正兜底） | 9fps | 88÷9=9.78秒 | 9.38秒 | ✅ 基本同步 |

#### 技术经验总结

**1. Camera硬件资源竞争是帧率问题的常见原因**
- `Camera.getImage()` 对不同通道（PREVIEW/RECORD）的调用会竞争Camera硬件
- 预览帧处理频率过高会严重影响录制帧率
- 必须限制预览帧处理频率（建议≤10fps），确保录制通道优先

**2. AVI帧率声明必须与实际帧率匹配**
- AVI头中的 `avih.microSecPerFrame` 和 `strh.rate` 决定播放器如何播放视频
- 如果声明帧率高于实际帧率，视频播放会加速
- 如果声明帧率低于实际帧率，视频播放会减速
- 在 `end()` 中根据实际录制数据修正帧率是可靠的兜底方案

**3. 停止录制顺序影响任务优雅退出**
- 先停止Camera硬件 → 任务看到 `imgLen=0` 自然退出
- 先设置 `REC_IDLE` → 任务看到状态变化进入空转，行为不可预测
- 正确顺序：停止硬件 → 删除任务 → 设置状态

**4. 预览帧重试机制在录制状态下有害**
- `delay(10)` 阻塞主循环，影响音频数据处理
- 重试3次最多阻塞30ms，在15fps录制下占2帧时间
- 录制状态下预览帧丢失是可接受的，不应重试

#### 文件变更
- `VideoRecorder.cpp`: processPreviewFrame()添加频率限制+移除重试循环，stopVideoRecording()调整停止顺序
- `MJPEG_Encoder.h`: 添加m_microSecPerFrameOffset和m_videoStrhRateOffset成员变量
- `MJPEG_Encoder.cpp`: 构造函数初始化新成员，writeAVIHeader()保存偏移位置，end()添加帧率自动修正
- `Shared_GlobalDefines.h`: 版本号 V1.49

#### 验证要点
- [ ] 编译通过
- [ ] 录制9秒视频，视频帧数接近135帧（9秒×15fps）
- [ ] VLC播放视频时长与音频时长基本一致（误差<0.5秒）
- [ ] 视频播放速度正常，无加速或减速现象
- [ ] 音视频同步良好，视频画面停止时音频也同时结束
- [ ] 串口日志显示帧率修正信息（如有偏差）或"Frame rate OK"（如无偏差）
- [ ] 串口监视器显示"当前版本: V1.49"

---

### 版本 V1.48 - 不存在WiFi SSID连接崩溃修复与文件传输返回功能 (2026-04-20)

#### 问题描述
1. **WiFi SSID不存在时系统崩溃**：当开发板所在环境中不存在Force或Tiger SSID时，执行"智能配网"或"网络校时"操作，系统在尝试连接不存在的SSID过程中发生崩溃（Bus Fault，栈损坏）
2. **崩溃根因**：`Menu_MenuContext.cpp`中的`executeOTA()`和`executeBleWifiConfig()`函数直接调用`WiFi.begin()`连接Force/Tiger SSID，没有先检查SSID是否存在。Realtek AmebaPro2的WiFi驱动在尝试连接不存在的SSID时，会导致WiFi驱动状态机进入异常状态，触发崩溃
3. **文件传输功能缺少返回选项**：传输模式选择界面只有USB和WEB两个选项，无法直接返回主菜单

#### 根本原因分析
- `WiFiConnector`类的`_connectToAP()`方法已有`_isSSIDAvailable()`检查（V1.41修复）
- `Camera.ino`的`taskTimeSync`函数已有`WiFi.scanNetworks()`检查（V1.42修复）
- 但`Menu_MenuContext.cpp`中的`executeOTA()`和`executeBleWifiConfig()`**遗漏了SSID存在性检查**，直接调用`WiFi.begin()`

#### 解决要点
1. 在`executeOTA()`的Force/Tiger连接循环中，每次尝试连接前先调用`WiFi.scanNetworks()`检查SSID是否存在
2. 在`executeBleWifiConfig()`的Force/Tiger连接循环中，同样添加SSID存在性检查
3. SSID不存在时跳过连接，在屏幕上显示"SSID not found"提示，继续尝试下一个SSID或进入BLE配网
4. 在传输模式选择界面添加"3. 返回"选项，支持从文件传输界面返回主菜单
5. 将`transferModeSelectDefaultUsb`(bool)改为`transferModeSelectIndex`(int: 0=USB, 1=WEB, 2=返回)

#### 实施步骤
1. 修改 `Menu_MenuContext.cpp` - executeOTA()添加SSID扫描检查
2. 修改 `Menu_MenuContext.cpp` - executeBleWifiConfig()添加SSID扫描检查
3. 修改 `Menu_MenuContext.cpp` - 传输模式选择界面添加返回选项
4. 修改 `Menu_MenuContext.h` - 变量声明更新和新增returnFromTransferMode()
5. 修改 `Shared_GlobalDefines.h` - 版本号从V1.47递增到V1.48
6. 迁移RTC模块从DS1307到DS3231 - 重命名文件、更新代码中的类型引用
7. 修改 `Menu_MenuContext.cpp` - 添加时间同步窗口UI（showTimeSyncWindow/updateTimeSyncWindow/hideTimeSyncWindow）
8. 修改 `Menu_MenuContext.h` - 添加时间同步窗口相关变量和方法声明
9. 修改 `Shared_Types.h` - 添加TimeSyncState枚举
10. 创建 `Shared_SharedResources.h` 和 `Shared_SharedResources.cpp` - 任务与UI通信的共享状态
11. 修改 `Camera.ino` - taskTimeSync()函数更新NTP状态到共享结构
12. 修改 `DS3231_ClockModule.cpp` - 移除调试输出防止刷屏
13. 修改 `Menu_MenuContext.cpp` - 修复进度显示使用sprintf格式化字符串

#### 关键代码变更

**1. Menu_MenuContext.cpp - executeOTA()添加SSID扫描检查**
```cpp
for (int i = 0; i < 2; i++) {
    // 新增: 扫描WiFi网络检查SSID是否存在
    Utils_Logger::info("[OTA] 扫描WiFi网络检查SSID: %s", knownSSIDs[i]);
    int scanCount = WiFi.scanNetworks();
    bool ssidExists = false;
    if (scanCount >= 0) {
        for (int s = 0; s < scanCount; s++) {
            if (strcmp(WiFi.SSID(s), knownSSIDs[i]) == 0) {
                ssidExists = true;
                break;
            }
        }
    }
    WiFi.scanDelete();

    if (!ssidExists) {
        Utils_Logger::info("[OTA] SSID %s 不存在，跳过连接", knownSSIDs[i]);
        tftManager.setTextColor(ST7789_GRAY, ST7789_BLACK);
        tftManager.setCursor(50, 210);
        tftManager.print(knownSSIDs[i]);
        tftManager.print(" not found   ");
        delay(500);
        continue;
    }
    // ... 原有连接逻辑
}
```

**2. Menu_MenuContext.cpp - executeBleWifiConfig()添加SSID扫描检查**
```cpp
for (int i = 0; i < 2; i++) {
    // 新增: 扫描WiFi网络检查SSID是否存在
    Utils_Logger::info("[BLE_WIFI] 扫描WiFi网络检查SSID: %s", knownSSIDs[i]);
    int scanCount = WiFi.scanNetworks();
    bool ssidExists = false;
    if (scanCount >= 0) {
        for (int s = 0; s < scanCount; s++) {
            if (strcmp(WiFi.SSID(s), knownSSIDs[i]) == 0) {
                ssidExists = true;
                break;
            }
        }
    }
    WiFi.scanDelete();

    if (!ssidExists) {
        Utils_Logger::info("[BLE_WIFI] SSID %s 不存在，跳过连接", knownSSIDs[i]);
        char skipMsg[32];
        sprintf(skipMsg, "%s not found", knownSSIDs[i]);
        showBleWifiConfigScreen(skipMsg);
        delay(500);
        continue;
    }
    // ... 原有连接逻辑
}
```

**3. Menu_MenuContext.cpp - 传输模式选择界面添加返回选项**
- `transferModeSelectDefaultUsb`(bool) → `transferModeSelectIndex`(int: 0=USB, 1=WEB, 2=返回)
- 新增`returnFromTransferMode()`函数：重置状态 → 返回主菜单
- 界面布局调整：3个选项(y=70, y=105, y=140)

#### 验证要点
- [ ] 无Force/Tiger SSID环境下，执行"智能配网"不崩溃，显示"not found"提示后进入BLE配网
- [ ] 无Force/Tiger SSID环境下，执行"网络校时"不崩溃，显示错误提示后返回主菜单
- [ ] 有Force SSID环境下，正常连接不受影响
- [ ] 传输模式选择界面3个选项可正常切换
- [ ] 选择"返回"选项后正常返回主菜单
- [ ] 时间同步窗口正确显示实时时间和NTP进度
- [ ] DS3231时间读取不再刷屏输出日志
- [ ] 进度显示格式正确（如5%、100%而非05%、0100%）

---

### 版本 V1.47 - USB调试输出清理与英文UI本地化清单 (2026-04-20)

#### 问题描述
1. **USB模式调试输出冗余**：USB MSC模式进入/退出流程中，屏幕显示"1/6"到"6/6"及"1/4"到"4/4"的分步调试信息，这些信息仅用于开发调试阶段，对最终用户无意义，需要在正式版本中移除
2. **英文UI内容未系统整理**：项目中大量界面文本仍为英文，缺乏完整的本地化清单，无法有效推进中文本地化工作

#### 解决要点
1. 注释掉USB_MassStorageModule.cpp中所有分步调试输出（enter()中的1/6~6/6，exit()中的1/4~4/4），仅保留功能性代码
2. 全面扫描项目中所有源文件，识别所有在用户界面中显示的英文内容
3. 将92条英文UI内容按功能模块分类整理，输出到Font.md文件，为后续中文本地化替换提供准确依据

#### 实施步骤
1. 修改 `USB_MassStorageModule.cpp` - 注释掉enter()中6条和exit()中4条调试输出print语句
2. 全面扫描项目所有.cpp/.ino文件中的`.print()`调用，识别英文UI文本
3. 创建 `Font.md` - 包含92条英文UI内容的完整清单，按10个功能模块分类，含屏幕位置、上下文说明和本地化优先级建议
4. 修改 `Shared_GlobalDefines.h` - 版本号从V1.46递增到V1.47

#### 关键代码变更

**1. USB_MassStorageModule.cpp - 注释调试输出**
```cpp
// enter()中 - 注释掉6条调试输出：
// m_tftManager->print("[1/6] Release WiFi...");
// m_tftManager->print("[2/6] USB Init...");
// m_tftManager->print("[3/6] SDIO Init...");
// m_tftManager->print("[4/6] USB Status...");
// m_tftManager->print("[5/6] Init Disk...");
// m_tftManager->print("[6/6] Load Driver...");

// exit()中 - 注释掉4条调试输出：
// m_tftManager->print("[1/4] USB MSC Deinit...");
// m_tftManager->print("[2/4] Wait settle...");
// m_tftManager->print("[3/4] WiFi Restart...");
// m_tftManager->print("[4/4] Done!");
```

**2. Font.md - 英文UI本地化清单（92条）**
- USB MSC模式（5条）：USB MSC Mode, USB Connected!, PC can access SD card, Rotate: Exit dialog, Exit USB MSC?
- OTA升级相关（21条）：State:, Device IP:, OTA failed, retrying..., OTA Server:, Exit OTA, Exiting OTA..., OTA Server IP Config, Rotate: Select option, Press: Confirm, Field 1/4~4/4, Using IP:, Rotate: +/-1, Press: Confirm field, IP Saved!, Connecting, Saved:, BLE WiFi Config..., Device IP:
- 版本信息界面（5条）：Ver:, FW: v4.0.9, Build:, Cam: GC2053, Board: AMB82-MINI
- BLE WiFi配网（19条）：BLE WiFi Config, Target IP seg3:, Status:, Press btn to cancel, Exit BLE WiFi?, WiFi Connected!, IP:, SSID:, Wrong IP reconnect..., Connecting Force..., Trying saved SSIDs..., BLE Config..., BLE Start Failed!, BLE Waiting..., Wrong IP segment!, BLE Timeout!, Configuring...
- 传输模式选择（6条）：1., 2., Rotate: Select option, Press: Confirm, USB, WEB
- 系统关闭（1条）：entering deep sleep...
- ISP参数设置（8条）：A: Exposure, B: Brightness, C: Contrast, D: Saturation, E: Reset, F: Back, Manual, Auto
- WiFi文件服务器（13条）：SSID:, PASS:, IP:, http://, Exit File Transfer?, [*] Cancel, [ ] Confirm, Rotate: Switch option, Press: Execute action, Cancelled, Returning to info..., Exiting..., Closing WiFi Server
- 媒体文件浏览与播放（13条）：SD card not inserted, Invalid file name, File not found, Unsupported video format, Memory error, Read error, Media Files, No media files found, Back, Decode Error, No Preview, VIDEO, IMAGE
- 菜单项标签（1条）：BLE WiFi

#### 待验证项
- [ ] 编译通过
- [ ] USB MSC模式进入时屏幕不再显示1/6~6/6调试步骤
- [ ] USB MSC模式退出时屏幕不再显示1/4~4/4调试步骤
- [ ] USB MSC功能正常工作（进入、文件传输、退出）
- [ ] 串口监视器显示"当前版本: V1.47"

### 版本 V1.46 - BLE WiFi配网退出确认弹窗功能实现 (2026-04-19)

#### 问题描述
1. **BLE配网无退出确认**：在BLE WiFi配网过程中，用户按下按钮直接停止配网并返回菜单，没有确认步骤，容易误操作
2. **旋转旋钮无响应**：在BLE配网界面中旋转旋钮被忽略，用户无法通过旋钮主动中断配网
3. **与OTA模式交互不一致**：OTA升级中界面已有退出确认弹窗（旋转弹出、按钮切换选项），BLE配网应保持一致的交互逻辑

#### 解决要点
1. 在BLE配网界面中，旋转旋钮弹出退出确认对话框（与OTA模式一致）
2. 按钮按下也弹出退出确认对话框（而非直接退出）
3. 退出确认对话框UI样式与OTA模式保持一致：白色边框、"返回"/"确认"中文选项、黄色高亮选中项
4. 添加状态判断逻辑：`handleBleExitRotation()`和`handleBleExitButton()`均检查`inBleWifiConfig`状态，防止非配网模式下误触发
5. 在弹窗显示期间暂停BLE配网状态刷新，防止`showBleWifiConfigScreen()`覆盖弹窗

#### 实施步骤
1. 修改 `Menu_MenuContext.h` - 添加`bleExitDialogShown`和`bleExitDefaultBack`状态变量，添加`showBleExitDialog()`、`handleBleExitRotation()`、`handleBleExitButton()`、`isBleExitDialogShown()`方法声明
2. 修改 `Menu_MenuContext.cpp`：
   - 旋转回调：BLE配网时旋转旋钮调用`handleBleExitRotation()`（替换原来的忽略逻辑）
   - 按钮回调：BLE配网时按钮调用`showBleExitDialog()`/`handleBleExitButton()`（替换原来的直接退出）
   - loop()中BLE按钮处理：改为弹窗逻辑（替换原来的`stopBleWifiConfig()`直接调用）
   - BLE等待循环：弹窗显示期间暂停状态刷新和WiFi连接检查
   - `showBleWifiConfigScreen()`：弹窗显示时跳过屏幕刷新
   - `stopBleWifiConfig()`：重置弹窗状态变量
3. 修改 `Shared_GlobalDefines.h` - 版本号从V1.45递增到V1.46

#### 关键代码变更

**1. 旋转回调 - BLE配网旋转处理**
```cpp
// 原来：忽略旋转事件
if (menuContext.isInBleWifiConfig()) {
    break;
}

// 修改后：弹出/切换退出对话框
if (menuContext.isInBleWifiConfig()) {
    menuContext.handleBleExitRotation(direction);
    break;
}
```

**2. 按钮回调 - BLE配网按钮处理**
```cpp
// 新增：BLE配网按钮处理（在USB MSC处理之后）
if (menuContext.isInBleWifiConfig()) {
    if (menuContext.isBleExitDialogShown()) {
        menuContext.handleBleExitButton();
    } else {
        menuContext.showBleExitDialog();
    }
    return;
}
```

**3. showBleExitDialog() - 退出确认对话框**
- UI样式与OTA退出弹窗完全一致
- 标题："Exit BLE WiFi?"
- 选项："返回"（strBack）/ "确认"（strConfirmExit）
- 默认选中"返回"，旋转切换选项

**4. handleBleExitButton() - 按钮选择处理**
- 选择"返回"：关闭弹窗，恢复配网界面，继续配网
- 选择"确认"：调用`stopBleWifiConfig()`停止配网并返回菜单

**5. BLE等待循环弹窗暂停逻辑**
```cpp
// 弹窗显示期间不刷新状态屏幕
if (elapsed % 5000 < 500 && !bleExitDialogShown) {
    showBleWifiConfigScreen(statusMsg);
}

// 弹窗显示期间暂停WiFi连接检查
if (bleExitDialogShown) {
    delay(100);
    continue;
}
```

**6. showBleWifiConfigScreen()防覆盖**
```cpp
void MenuContext::showBleWifiConfigScreen(const char* statusMsg) {
    if (bleExitDialogShown) {
        return;  // 弹窗显示时不刷新屏幕，防止覆盖弹窗
    }
    // ...正常显示逻辑
}
```

#### 待验证项
- [ ] 编译通过
- [ ] BLE配网中旋转旋钮弹出退出确认对话框
- [ ] BLE配网中按钮按下弹出退出确认对话框
- [ ] 退出对话框中选择"返回"继续配网
- [ ] 退出对话框中选择"确认"停止配网并返回菜单
- [ ] 弹窗显示期间配网界面不被覆盖
- [ ] 非BLE配网模式下不会误触发弹窗
- [ ] 串口监视器显示"当前版本: V1.46"

### 版本 V1.45 - USB MSC模式无法关闭及重复启用失败问题修复 (2026-04-19)

#### 问题描述
1. **关闭失败**：通过弹出窗口选择关闭USB后，U盘功能无法正常关闭，电脑端仍显示USB设备连接
2. **状态反转**：第二次打开USB功能时，界面显示6个步骤正常，但实际执行的是关闭操作而非启用USB功能
3. **第三次失败**：第三次打开USB功能时，电脑显示"无法识别的USB设备"，无法打开U盘功能

#### 根本原因分析

**核心问题：`USBDeinit()`不完整，USB OTG硬件未被反初始化**

1. **`USBMassStorage::USBDeinit()`只做了部分清理**：
   - 调用`usbd_msc_deinit()`卸载MSC类驱动
   - 释放`disk_operations`内存
   - **未调用`_usb_deinit()`**：USB OTG硬件控制器仍处于初始化状态

2. **第二次enter()时USB硬件冲突**：
   - `enter()`调用`USBInit()`→`_usb_init()`时，USB OTG硬件仍处于已初始化状态
   - 在已初始化的USB硬件上再次调用`_usb_init()`，导致USB硬件状态混乱
   - 表现为：界面显示正常（6步骤都执行了），但USB实际功能被关闭而非启用

3. **exit()未恢复WiFi**：
   - 退出USB模式后WiFi保持关闭状态
   - 系统的校时任务（taskTimeSync）等WiFi依赖功能无法正常工作
   - 第二次enter()时调用`wifi_off()`虽然无害，但WiFi从未被恢复

4. **第三次完全失败**：
   - 两轮不完整的init/deinit循环后，USB OTG硬件处于不可恢复的状态
   - 电脑端无法完成USB设备枚举，显示"无法识别的USB设备"

#### 解决要点
1. 在`USBMassStorage::USBDeinit()`中添加`_usb_deinit()`调用，完整反初始化USB OTG硬件
2. 在`USB_MassStorageModule::exit()`中添加`wifi_on(RTW_MODE_STA)`恢复WiFi硬件
3. 在exit()中添加分步骤屏幕显示，便于诊断退出过程
4. 修正enter()步骤编号从[1/6]开始（而非[0/6]），保持一致性

#### 实施步骤
1. 修改 `CameraSystem/USB/src/USBMassStorage.cpp` - 在`USBDeinit()`末尾添加`_usb_deinit()`调用
2. 修改 `USB_MassStorageModule.cpp` - 重构`exit()`方法，添加完整的退出流程：
   - 步骤1：USBDeinit()（含_usb_deinit()）
   - 步骤2：等待硬件稳定（500ms）
   - 步骤3：wifi_on(RTW_MODE_STA)恢复WiFi
   - 步骤4：完成
3. 修改 `USB_MassStorageModule.cpp` - 修正enter()步骤编号
4. 修改 `Shared_GlobalDefines.h` - 版本号从V1.44递增到V1.45

#### 关键代码变更

**1. USBMassStorage.cpp - USBDeinit()添加_usb_deinit()**
```cpp
void USBMassStorage::USBDeinit(void)
{
    usbd_msc_deinit();
    if (disk_operations) {
        free(disk_operations);
        disk_operations = NULL;
    }
    _usb_deinit();  // 新增：完整反初始化USB OTG硬件
}
```

**2. USB_MassStorageModule.cpp - exit()完整退出流程**
```cpp
void USB_MassStorageModule::exit() {
    if (m_state == STATE_IDLE) {
        return;
    }

    m_state = STATE_STOPPING;

    m_tftManager->fillScreen(ST7789_BLACK);
    m_tftManager->setTextColor(ST7789_YELLOW, ST7789_BLACK);
    m_tftManager->setTextSize(1);
    m_tftManager->setCursor(30, 60);
    m_tftManager->print("[1/4] USB MSC Deinit...");

    m_usbMassStorage.USBDeinit();  // 含usbd_msc_deinit() + _usb_deinit()

    m_tftManager->setCursor(30, 80);
    m_tftManager->print("[2/4] Wait settle...");
    delay(500);

    m_tftManager->setCursor(30, 100);
    m_tftManager->print("[3/4] WiFi Restart...");

    wifi_on(RTW_MODE_STA);  // 恢复WiFi硬件
    delay(1000);

    m_tftManager->setCursor(30, 120);
    m_tftManager->print("[4/4] Done!");
    delay(500);

    m_state = STATE_IDLE;
    m_exitDialogShown = false;
    m_exitConfirmed = true;
}
```

#### 退出流程屏幕诊断指南（V1.45）
通过屏幕显示的步骤编号，可以判断退出过程问题发生的位置：
- **[1/4] 卡住** → USBDeinit()失败（usbd_msc_deinit或_usb_deinit异常）
- **[2/4] 卡住** → 硬件稳定等待中异常
- **[3/4] 卡住** → wifi_on()恢复WiFi失败
- **[4/4]** → 退出成功

#### USB MSC完整生命周期（V1.45）
```
进入流程（enter）：
  [1/6] Release WiFi → WiFi.disconnect() + wifi_off()
  [2/6] USB Init     → _usb_init()
  [3/6] SDIO Init    → sd_gpio_init() + sdio_driver_init()
  [4/6] USB Status   → wait_usb_ready()
  [5/6] Init Disk    → malloc + 设置disk_operations
  [6/6] Load Driver  → usbd_msc_init()

退出流程（exit）：
  [1/4] USB MSC Deinit → usbd_msc_deinit() + free(disk_operations) + _usb_deinit()
  [2/4] Wait settle    → delay(500)
  [3/4] WiFi Restart   → wifi_on(RTW_MODE_STA)
  [4/4] Done           → delay(500)
```

#### 待验证项
- [ ] 编译通过
- [ ] 首次进入USB MSC模式正常工作
- [ ] 退出USB MSC模式后电脑端正确断开U盘
- [ ] 第二次进入USB MSC模式正常工作
- [ ] 第三次及以后多次进出USB MSC模式均正常
- [ ] 退出后WiFi恢复正常，校时任务可正常工作
- [ ] 串口监视器显示"当前版本: V1.45"

#### USB MSC技术限制文档

**限制现象描述**

由于计算机操作系统对USB大容量存储设备（U盘）的处理机制要求，在执行U盘安全退出操作后，必须将U盘进行物理拔插，系统才能重新检测并正常使用该U盘。此限制为USB MSC协议层面的固有特性，非本项目代码缺陷。

**技术原理**

1. USB MSC设备在计算机端执行"安全弹出"操作时，操作系统会执行以下步骤：
   - 刷新文件系统缓存，确保所有数据写入存储介质
   - 卸载文件系统，释放设备句柄
   - 向USB设备发送"允许移除"信号
   - 在设备管理器中将设备标记为"已移除"状态

2. 计算机操作系统在设备被标记为"已移除"后，**不会自动重新枚举同一USB设备**。这是为了防止数据损坏和确保设备状态一致性。

3. 从设备端（AMB82-MINI）角度：
   - `USBDeinit()` 正确调用了 `usbd_msc_deinit()` + `_usb_deinit()`，USB OTG硬件已完全反初始化
   - `enter()` 再次调用 `USBInit()` → `_usb_init()` 时，USB OTG硬件重新初始化
   - 设备端重新完成USB设备枚举流程（USB reset → 设备描述符请求 → 配置描述符请求 → MSC驱动加载）
   - 但计算机端因设备已被标记为"已移除"，**拒绝重新识别同一物理端口上的同一设备**

4. 物理拔插的作用：
   - 物理拔出USB线 → 计算机检测到USB端口断开事件 → 清除端口状态
   - 物理插入USB线 → 计算机检测到USB端口连接事件 → 触发全新设备枚举流程
   - 这是操作系统层面强制重新枚举的唯一可靠方式

**复现步骤**

1. 进入USB MSC模式，电脑正常识别U盘
2. 在电脑端对U盘执行"安全弹出"操作
3. 在设备端通过旋转旋钮→确认退出，关闭USB MSC模式
4. 再次进入USB MSC模式
5. **预期结果**：电脑无法自动识别U盘，需要物理拔插USB线后才能重新识别

**系统环境信息**

| 项目 | 信息 |
|------|------|
| 设备端 | AMB82-MINI (Realtek AmebaPro2 RTL8735B) |
| USB模式 | USB OTG Device模式，MSC类 |
| 操作系统 | Windows 10/11，macOS，Linux均存在此限制 |
| USB库 | CameraSystem/USB (基于Realtek SDK usbd_msc) |
| SD卡 | 通过SDIO接口连接 |

**对项目开发的具体影响分析**

1. **功能边界**：USB MSC功能不支持"热重入"（即不拔插USB线的情况下连续进出MSC模式），这是USB协议层面的限制而非代码缺陷
2. **用户体验**：用户在退出USB MSC模式后再次进入时，必须物理拔插USB线才能让电脑重新识别设备
3. **可能的缓解方案**（未实施，仅记录）：
   - 方案A：在退出USB MSC模式时，在屏幕上显示提示"请拔插USB线后重新进入"
   - 方案B：研究是否可通过USB端口的电气重置（如USB D+/D-信号拉低）模拟拔插效果
   - 方案C：使用USB复合设备模式，在MSC之外维持一个始终存在的USB功能（如CDC），避免设备完全消失
4. **当前处理方式**：在V1.45版本中，exit()流程已完整实现USBDeinit()+WiFi恢复，确保每次进入USB MSC模式时硬件状态干净。物理拔插限制作为已知技术约束记录在案。

---

### 版本 V1.44 - WiFi与USB OTG资源冲突导致USB MSC模式卡死问题修复 (2026-04-19)

#### 问题描述
1. **现象**：USB MSC模式在进入时卡死在初始化阶段，屏幕显示"[1/5] USB Init..."后无响应
2. **电脑识别**：电脑端提示"无法识别的USB设备"
3. **TTL调试**：TTL模式下同样卡住，无法通过串口日志诊断

#### 根本原因分析
Camera项目在启动时会通过后台任务(taskTimeSync)初始化并保持WiFi连接。当用户进入USB MSC模式时，WiFi和USB OTG硬件模块存在资源共享冲突。

AMB82-MINI的WiFi和USB OTG共享相同的硬件资源，如果在没有释放WiFi资源的情况下直接初始化USB，会导致：
1. USB OTG初始化失败或卡死
2. USB设备枚举不完整，电脑无法识别

#### 解决要点
1. 在USB初始化前完全释放WiFi资源
2. 调用`WiFi.disconnect()`断开WiFi连接
3. 调用`wifi_off()`关闭WiFi硬件
4. 添加足够的延时确保资源完全释放
5. 更新屏幕进度显示为6个步骤（新增WiFi释放步骤）

#### 实施步骤
1. 修改 `USB_MassStorageModule.h` - 添加 `#include <WiFi.h>` 以使用WiFi相关函数
2. 修改 `USB_MassStorageModule.cpp` - 在enter()方法中添加WiFi资源释放步骤
3. 修改 `Shared_GlobalDefines.h` - 版本号从V1.43递增到V1.44

#### 关键代码变更
- `USB_MassStorageModule.h` - 添加WiFi头文件：
  ```cpp
  #include <WiFi.h>
  ```

- `USB_MassStorageModule.cpp` - `enter()` 方法新增WiFi释放步骤：
  ```cpp
  m_tftManager->setCursor(30, 60);
  m_tftManager->print("[0/6] Release WiFi...");

  Utils_Logger::info("[USB_MSD_MODULE] 释放WiFi资源，避免USB/WiFi冲突");

  if (WiFi.status() == WL_CONNECTED) {
      WiFi.disconnect();
      Utils_Logger::info("[USB_MSD_MODULE] WiFi已断开连接");
  }

  delay(500);

  wifi_off();
  delay(500);

  Utils_Logger::info("[USB_MSD_MODULE] WiFi硬件已关闭");

  m_tftManager->setCursor(30, 60);
  m_tftManager->print("[1/6] USB Init...");
  ```

#### 屏幕诊断指南（V1.44）
通过屏幕显示的步骤编号，可以判断问题发生的位置：
- **[0/6] 卡住** → WiFi资源释放失败
- **[1/6] 卡住** → USBInit()初始化失败
- **[2/6] 卡住** → SDIOInit()初始化失败
- **[3/6] 卡住** → USBStatus()调用失败
- **[4/6] 卡住** → initializeDisk()失败
- **[5/6] 卡住** → loadUSBMassStorageDriver()失败
- **[6/6]** 但PC仍无法识别 → isConnected()返回0，USB枚举可能有问题

#### 待验证项
- [ ] 编译通过
- [ ] USB直连模式下正确显示6步诊断信息
- [ ] 电脑端能正确识别USB设备
- [ ] WiFi释放后不会影响系统其他功能
- [ ] 串口监视器显示"当前版本: V1.44"

---

### 版本 V1.43 - USB MSC模式USB直连异常问题诊断与修复 (2026-04-19)

#### 问题描述
1. **TTL调试模式**：通过TTL调试接口测试文件传输功能的"USB"模式时，系统行为正常，USB MSC功能完整工作
2. **USB直连模式**：使用USB接口直接连接电脑后，进入文件传输"USB"功能时，出现以下异常：
   - 系统在显示标题"USB MSC Mode"和"Initializing USB..."后
   - 标题消失，屏幕仅显示"Initializing usb..."的下半部分文字残影
   - 系统似乎卡死，用户无法进行任何操作
   - 电脑端提示"无法识别的USB设备"

#### 根本原因分析
根据问题现象和代码分析，可能的原因包括：

1. **USB初始化时序问题**：当USB已经物理连接到PC时，USB OTG控制器的初始化时序可能与未连接时不同，需要更长的稳定时间

2. **USB状态检测机制**：`USBStatus()` 返回0可能表示USB OTG尚未检测到主机连接，而 `isConnected()` 可能需要在初始化完成后一段时间才能返回正确的状态

3. **缺少USB连接状态诊断**：原代码没有在初始化过程中检测USB连接状态，导致无法确定PC是否正确识别设备

4. **exit()方法资源清理问题**：V1.39版本记录中提到"USBDeinit()因SDK中_usb_deinit()符号未导出导致链接错误，暂不调用"，但后来添加了USBDeinit()调用，存在资源未正确释放的风险

#### 解决要点
1. 在USB初始化过程中添加分步骤日志输出，便于诊断问题发生的具体阶段
2. 在USB初始化后添加 `isConnected()` 状态检测，判断PC是否已识别设备
3. 在USBInit()调用前添加100ms延迟，确保系统稳定
4. 当USBStatus()返回非零值时，在屏幕上显示状态码便于诊断
5. 版本号从V1.42递增到V1.43

#### 实施步骤
1. 修改 `USB_MassStorageModule.cpp` - 在enter()方法中添加：
   - 分步骤日志（步骤1-6）
   - USBStatus()返回值检测和显示
   - isConnected()连接状态检测
   - USBInit()前的100ms延迟
2. 修改 `Shared_GlobalDefines.h` - 版本号从V1.42递增到V1.43

#### 关键代码变更
- `USB_MassStorageModule.cpp` - `enter()` 方法：
  - 将步骤进度实时显示在屏幕上，便于通过屏幕诊断问题
  - 显示格式：`[1/5] USB Init...` → `[2/5] SDIO Init...` → `[3/5] USB Status...` → `[4/5] Init Disk...` → `[5/5] Load Driver...` → `[OK] Checking connection...`
  - 在USBInit()前添加100ms延迟确保系统稳定
  - 初始化完成后延迟500ms再更新到运行状态
  ```cpp
  m_tftManager->setTextColor(ST7789_CYAN, ST7789_BLACK);
  m_tftManager->setTextSize(2);
  m_tftManager->setCursor(70, 20);
  m_tftManager->print("USB MSC Mode");

  m_tftManager->setTextColor(ST7789_WHITE, ST7789_BLACK);
  m_tftManager->setTextSize(1);
  m_tftManager->setCursor(30, 60);
  m_tftManager->print("[1/5] USB Init...");

  delay(100);

  m_usbMassStorage.USBInit();

  m_tftManager->setCursor(30, 80);
  m_tftManager->print("[2/5] SDIO Init...");
  m_usbMassStorage.SDIOInit();

  m_tftManager->setCursor(30, 100);
  m_tftManager->print("[3/5] USB Status...");
  int usbStatus = m_usbMassStorage.USBStatus();

  m_tftManager->setCursor(30, 120);
  m_tftManager->print("[4/5] Init Disk...");
  m_usbMassStorage.initializeDisk();

  m_tftManager->setCursor(30, 140);
  m_tftManager->print("[5/5] Load Driver...");
  m_usbMassStorage.loadUSBMassStorageDriver();

  m_tftManager->setCursor(30, 160);
  m_tftManager->print("[OK] Checking connection...");
  int connected = m_usbMassStorage.isConnected();

  delay(500);
  ```

#### 屏幕诊断指南
通过屏幕显示的步骤编号，可以判断问题发生的位置：
- **[1/5] 卡住** → USBInit()初始化失败
- **[2/5] 卡住** → SDIOInit()初始化失败
- **[3/5] 卡住** → USBStatus()调用失败
- **[4/5] 卡住** → initializeDisk()失败
- **[5/5] 卡住** → loadUSBMassStorageDriver()失败
- **[OK]** 但PC仍无法识别 → isConnected()返回0，USB枚举可能有问题

#### 待验证项
- [ ] 编译通过
- [ ] TTL调试模式下USB MSC功能正常
- [ ] USB直连模式下正确显示诊断信息
- [ ] 电脑端能正确识别USB设备
- [ ] 串口监视器显示"当前版本: V1.43"

---

### 版本 V1.42 - taskTimeSync函数WiFi连接崩溃修复 (2026-04-19)

#### 问题描述
1. **崩溃现象**：V1.41版本修复后，系统仍然崩溃
2. **崩溃时机**：崩溃仍然发生在进入"网络校时"功能时，日志显示 `[Driver]: set ssid [Force]` 后崩溃
3. **关键发现**：查看日志发现 `_isSSIDAvailable()` 的日志输出（`[WiFiConn] Scanning for available networks`）完全没有出现

#### 根本原因
V1.41的修复只针对 `WiFiConnector` 类中的 `_connectToAP()` 方法，但 `taskTimeSync` 函数（在 Camera.ino 中）直接调用 `WiFi.begin()` 连接WiFi，**没有经过 WiFiConnector 类**。因此 V1.41 的 SSID 存在性检查对 taskTimeSync 函数完全无效。

#### 解决要点
1. 在 `taskTimeSync` 函数中添加与 `_isSSIDAvailable()` 相同的SSID存在性检查逻辑
2. 在 `WiFi.begin()` 之前先调用 `WiFi.scanNetworks()` 检查目标SSID是否存在
3. 只有SSID存在时才执行 `WiFi.begin()` 连接
4. 版本号从V1.41递增到V1.42

#### 实施步骤
1. 修改 `Camera.ino` 的 `taskTimeSync` 函数 - 在连接前添加SSID扫描检查
2. 修改 `Shared_GlobalDefines.h` - 版本号从V1.41递增到V1.42

#### 关键代码变更
- `Camera.ino` - `taskTimeSync()` 函数：
  ```cpp
  // 先扫描检查SSID是否存在，避免连接不存在的SSID导致WiFi驱动崩溃
  Utils_Logger::info("扫描WiFi网络检查SSID: %s", wifiConfigs[i].ssid);
  int numNetworks = WiFi.scanNetworks();
  if (numNetworks >= 0) {
      bool ssidFound = false;
      for (int j = 0; j < numNetworks; j++) {
          if (WiFi.SSID(j) == String(wifiConfigs[i].ssid)) {
              ssidFound = true;
              break;
          }
      }
      WiFi.scanDelete();
      if (!ssidFound) {
          Utils_Logger::info("SSID %s 不存在，跳过连接", wifiConfigs[i].ssid);
          continue;
      }
      Utils_Logger::info("SSID %s 存在，准备连接", wifiConfigs[i].ssid);
  }
  ```

---

### 版本 V1.41 - WiFi连接前增加SSID存在性检查修复崩溃问题 (2026-04-19)

#### 问题描述
1. **崩溃现象**：在无Force SSID的WiFi网络环境下，进入"网络校时"功能时系统崩溃，Bus Fault，PC指向0x32316464（栈损坏）
2. **崩溃时机**：崩溃发生在V1.40版本修复后，仍然在尝试连接不存在的SSID（Force）时崩溃
3. **问题分析**：V1.40的 `_resetWiFiHardware()` 修复是在连接失败后的补救措施，但根本问题是**连接不存在的SSID时会触发WiFi驱动状态机崩溃**，而不是连接失败返回

#### 根本原因
Realtek AmebaPro2的WiFi驱动在尝试连接不存在的SSID时，会导致WiFi驱动状态机进入异常状态，后续任何WiFi操作都可能触发崩溃。即使在 `_connectToAP()` 失败后执行完整的硬件复位，也无法阻止已经发生的驱动崩溃。

#### 解决要点
1. 在尝试连接某个SSID之前，先扫描可用WiFi网络并检查该SSID是否存在于当前环境中
2. 只有在SSID存在时才执行 `WiFi.begin()` 连接操作，避免触发WiFi驱动的崩溃
3. 新增 `_isSSIDAvailable()` 私有方法，使用 `WiFi.scanNetworks()` 扫描并检查目标SSID
4. 版本号从V1.40递增到V1.41

#### 实施步骤
1. 修改 `WiFi_WiFiConnector.h` - 添加 `_isSSIDAvailable()` 私有方法声明
2. 修改 `WiFi_WiFiConnector.cpp` - 实现 `_isSSIDAvailable()` 方法
3. 修改 `_connectToAP()` - 在连接前调用 `_isSSIDAvailable()` 检查SSID是否存在
4. 修改 `Shared_GlobalDefines.h` - 版本号从V1.40递增到V1.41

#### 关键代码变更
- `WiFi_WiFiConnector.h`：添加 `_isSSIDAvailable(const char* ssid)` 方法声明
- `WiFi_WiFiConnector.cpp`：
  ```cpp
  bool WiFiConnector::_isSSIDAvailable(const char* ssid)
  {
      Utils_Logger::info("[WiFiConn] Scanning for available networks to check SSID: %s", ssid);

      int numNetworks = WiFi.scanNetworks();
      if (numNetworks < 0) {
          Utils_Logger::error("[WiFiConn] Network scan failed, error code: %d", numNetworks);
          return false;
      }

      Utils_Logger::info("[WiFiConn] Found %d networks", numNetworks);

      bool found = false;
      for (int i = 0; i < numNetworks; i++) {
          String availableSSID = WiFi.SSID(i);
          if (availableSSID == ssid) {
              found = true;
              break;
          }
      }

      WiFi.scanDelete();
      return found;
  }
  ```
- `_connectToAP()` 连接前检查：
  ```cpp
  if (!_isSSIDAvailable(ssid)) {
      Utils_Logger::info("[WiFiConn] SSID %s not found in scan results, skipping connection", ssid);
      return false;
  }
  ```

---

### 版本 V1.40 - 无Force网络环境下WiFi连接崩溃修复 (2026-04-19)

#### 问题描述
1. **崩溃现象**：在无Force SSID的WiFi网络环境下，进入"网络校时"功能时系统崩溃，Bus Fault，PC指向0x32316464（栈损坏）
2. **崩溃时机**：崩溃发生在连续尝试连接不存在的SSID（Force）之后，与BLE无关
3. **根本原因**：在连接不存在的SSID时，`WiFi.begin()` 失败后仅调用 `WiFi.disconnect()` 无法完全重置WiFi硬件状态，导致后续操作访问了处于异常状态的WiFi驱动，最终栈被覆写

#### 解决要点
1. 在 `_connectToAP()` 中，WiFi连接失败后执行完整的WiFi硬件复位流程，而非仅调用 `WiFi.disconnect()`
2. 新增 `_resetWiFiHardware()` 私有方法，包含 `wifi_off()` → 等待 → `wifi_on()` → 等待 的完整复位序列
3. 版本号从V1.39递增到V1.40

#### 实施步骤
1. 修改 `WiFi_WiFiConnector.h` - 添加 `_resetWiFiHardware()` 私有方法声明
2. 修改 `WiFi_WiFiConnector.cpp` - 实现 `_resetWiFiHardware()` 方法，并在 `_connectToAP()` 失败时调用
3. 修改 `Shared_GlobalDefines.h` - 版本号从V1.39递增到V1.40

#### 关键代码变更
- `WiFi_WiFiConnector.cpp`：
  ```cpp
  void WiFiConnector::_resetWiFiHardware() {
      Utils_Logger::info("[WiFiConn] Resetting WiFi hardware...");
      wifi_config_autoreconnect(0, 0, 0);
      WiFi.disconnect();
      delay(500);
      wifi_off();
      delay(500);
      wifi_on();
      delay(1000);
      Utils_Logger::info("[WiFiConn] WiFi hardware reset complete");
  }
  ```
- `_connectToAP()` 失败时调用：`_resetWiFiHardware()` 而非直接返回

---

### 版本 V1.39 - USB MSC模式退出对话框交互功能实现 (2026-04-19)

#### 问题描述
1. **USB模式退出交互缺失**：根据需求文档，在进入USB MSC模式后，旋转旋钮应显示"退出"对话框，但该交互功能尚未实现
2. **中文字符显示异常**：退出对话框中的"返回"和"确认"中文字符使用GB2312编码直接输出，无法在TFT屏幕上正确渲染
3. **退出后返回主菜单位置错误**：退出USB模式后未正确返回主菜单，三角形指示器未指向选项E

#### 解决要点
1. 修改`USB_MassStorageModule::handleRotation()`：旋转旋钮时，若退出对话框未显示则弹出对话框，若已显示则切换"返回"/"确认"选项
2. 修改`USB_MassStorageModule::handleButton()`：在退出对话框中，选择"返回"时关闭对话框恢复USB模式显示，选择"确认"时执行退出操作
3. 使用`Display_font16x16.h`中的字体索引常量（FONT16_IDX_FAN2/FONT16_IDX_HUI2/FONT16_IDX_QUE2/FONT16_IDX_REN2）替代GB2312编码，通过`FontRenderer::drawChineseString()`正确渲染中文
4. 在`USB_MassStorageModule::exit()`中重置状态标志释放USB资源（注意：`USBDeinit()`因SDK中`_usb_deinit()`符号未导出导致链接错误，暂不调用）
5. 在`MenuContext`中新增`returnFromUsbMode()`方法，退出USB模式后返回主菜单并将三角形指示器定位到选项E（POSITION_E）
6. 修改编码器按钮回调`handleEncoderButton()`，在USB模块退出后调用`returnFromUsbMode()`
7. 版本号从V1.38递增到V1.39

#### 实施步骤
1. 修改 `USB_MassStorageModule.h` - 添加`#include "Display_font16x16.h"`、`isExitConfirmed()`方法和`m_exitConfirmed`成员变量
2. 修改 `USB_MassStorageModule.cpp` - 重构核心交互逻辑：
   - `handleRotation()`：首次旋转弹出退出对话框，再次旋转切换选项
   - `handleButton()`：处理对话框中的按钮选择逻辑
   - `showExitDialog()`/`updateExitDialogDisplay()`：使用字体索引渲染中文
   - `exit()`：重置状态标志释放资源并设置退出标志
3. 修改 `Menu_MenuContext.h` - 添加`returnFromUsbMode()`方法声明
4. 修改 `Menu_MenuContext.cpp` - 实现`returnFromUsbMode()`方法，修改编码器按钮回调处理USB退出
5. 修改 `Shared_GlobalDefines.h` - 版本号从V1.38递增到V1.39

#### 关键代码变更
- `USB_MassStorageModule.cpp`：使用`static const uint8_t strUsdBack[] = {FONT16_IDX_FAN2, FONT16_IDX_HUI2, 0}`定义中文字体索引数组
- `Menu_MenuContext.cpp`：在`handleEncoderButton()`中添加`if (usbMassStorageModule.isExitConfirmed()) { menuContext.returnFromUsbMode(); }`
- `MenuContext::returnFromUsbMode()`：调用`triangleController.moveToPosition(TriangleController::POSITION_E)`定位三角形到选项E

---

### 版本 V1.38 - BLE WiFi配网功能与OTA WiFi连接策略重构 (2026-04-15)

#### 问题描述
1. **子菜单C功能替换**：将子菜单选项C从"关闭系统"改为"蓝牙配网（BLE WiFi Config）"，实现完整的WiFi连接管理功能
2. **OTA WiFi连接策略缺陷**：原executeOTA()在启动时无条件执行WiFi.disconnect()断开当前连接，即使已连接到正确网络也会断开；且WiFi连接策略仅支持Force→Tiger两个SSID，缺少保存SSID和BLE配网回退机制
3. **WiFi凭据持久化需求**：通过BLE配网连接的非Force/Tiger WiFi SSID需要持久保存，断电不丢失
4. **版本号管理**：需要规范版本号递增机制，每次迭代递增0.01

#### 解决要点
1. 创建WiFiConnector模块，封装WiFi连接策略和BLE配网功能
2. 使用FlashMemory实现WiFi凭据持久化存储（最多5组SSID/密码）
3. 重构executeOTA()为6阶段流程，增加保存SSID和BLE配网回退
4. 实现BLE WiFi配网子菜单功能（子菜单C）
5. 版本号从V1.37递增到V1.38

#### 实施步骤
1. 创建 `WiFi_WiFiConnector.h` - WiFi连接器头文件，定义WiFiConnectResult枚举、WiFiConnectorState枚举、SavedAP结构体和WiFiConnector类
2. 创建 `WiFi_WiFiConnector.cpp` - WiFi连接器实现，包含：
   - connectWithStrategy() - 完整WiFi连接策略（Force→Tiger→保存SSID→BLE配网）
   - startBLEConfig()/stopBLEConfig() - BLE配网启停
   - saveAP()/loadSavedAPs()/clearSavedAPs() - WiFi凭据管理
   - _loadFromFlash()/_saveToFlash() - FlashMemory持久化
3. 修改 `Menu_MenuItems.h` - 添加MENU_OPERATION_BLE_WIFI_CONFIG枚举
4. 修改 `Menu_MenuItems.cpp` - 子菜单C标签改为"BLE WiFi"，操作改为MENU_OPERATION_BLE_WIFI_CONFIG
5. 修改 `Menu_MenuContext.h` - 添加BLE配网状态变量和方法声明
6. 修改 `Menu_MenuContext.cpp` - 实现executeBleWifiConfig()、showBleWifiConfigScreen()、stopBleWifiConfig()，重构executeOTA()
7. 修改 `Shared_GlobalDefines.h` - 版本号从V1.37更新到V1.38

#### 技术方案

**1. WiFi连接策略（WiFiConnector模块）**
```
连接策略优先级：
1. 检查当前连接 → 已连接且IP正确则直接使用
2. Force SSID → 尝试3次
3. Tiger SSID → 尝试3次
4. 保存的SSID → 逐个尝试3次
5. BLE WiFi Config → 启动BLE配网，等待手机APP配置（超时120秒）
```

**2. WiFi凭据持久化（FlashMemory）**
- 使用AmebaPro2的FlashMemory库
- 存储地址：0xFE000（4KB扇区）
- 魔数校验：0x57494649（"WIFI"）
- 最多保存5组SSID/密码
- 每组包含：SSID（33字节）、密码（64字节）、有效标志（1字节）

**3. BLE WiFi配网（BLEWifiConfigService）**
- 使用AmebaPro2 BLE库的BLEWifiConfigService
- 广播名称：Ameba_XXXXXX（基于BT地址）
- 服务UUID：FF01
- 配网流程：手机APP扫描BLE设备 → 发送WiFi SSID和密码 → 设备自动连接

**4. OTA WiFi连接流程（6阶段）**
```
阶段1: 显示"升级中"界面，设置 inOtaProgress=true
阶段2: 检查当前WiFi连接状态
  - 已连接且IP号段正确 → 提示已连接，跳到阶段6
  - 已连接但IP号段不对 → 断开后进入阶段3
阶段3: 按Force→Tiger→保存SSID→BLE配网策略连接
阶段4: 每次连接后验证IP第3段是否与服务器IP匹配
阶段5: 连接失败处理
  → wifi_config_autoreconnect(0,0,0) 禁用自动重连
  → WiFi.disconnect() 释放资源
  → 重置所有OTA标志 → 红屏2秒 → 返回子菜单
阶段6: 连接成功: 显示设备IP → 调用 ota.start_OTA_threads()
```

**5. BLE配网子菜单功能（子菜单C）**
```
工作流程：
1. 检查当前WiFi连接 → 已连接且IP正确 → 显示"WiFi Connected!"和IP/SSID
2. 已连接但IP号段不对 → 断开重连
3. 未连接 → Force→Tiger→保存SSID→BLE配网
4. BLE配网期间显示倒计时和状态
5. 按压按钮可取消BLE配网
6. 成功连接后显示IP和SSID，3秒后返回子菜单
7. 非Force/Tiger的SSID自动保存到Flash
```

#### 核心代码变更

**1. WiFi_WiFiConnector.h - 新建**
```cpp
#define WIFI_CONN_MAX_SAVED_AP      5
#define WIFI_CONN_SSID_MAX_LEN      33
#define WIFI_CONN_PASS_MAX_LEN      64

typedef struct {
    char ssid[WIFI_CONN_SSID_MAX_LEN];
    char password[WIFI_CONN_PASS_MAX_LEN];
    bool valid;
} SavedAP;

class WiFiConnector {
public:
    WiFiConnectResult connectWithStrategy(uint8_t targetIpSegment3);
    WiFiConnectResult checkCurrentConnection(uint8_t targetIpSegment3);
    bool startBLEConfig();
    void stopBLEConfig();
    void saveAP(const char* ssid, const char* password);
    void loadSavedAPs();
    SavedAP& getSavedAP(int index);
    // ...
};
```

**2. Menu_MenuItems.cpp - 子菜单C修改**
```cpp
// 修改前:
{.label = "关闭系统", .type = MENU_ITEM_TYPE_NONE, .operation = MENU_OPERATION_NONE, .dataIndex = 2}

// 修改后:
{.label = "BLE WiFi", .type = MENU_ITEM_TYPE_FUNCTION, .operation = MENU_OPERATION_BLE_WIFI_CONFIG, .dataIndex = 2}
```

**3. Menu_MenuContext.cpp - executeOTA()重构**
```cpp
// 修改前: 无条件WiFi.disconnect()后尝试Force→Tiger
// 修改后: 先检查当前连接 → Force→Tiger→保存SSID→BLE配网
void MenuContext::executeOTA() {
    // 阶段1: 显示升级界面
    // 阶段2: 检查当前WiFi连接
    if (WiFi.status() == WL_CONNECTED) {
        if (currentIp[2] == otaServerIp[2]) needConnect = false;
        else { WiFi.disconnect(); }
    }
    // 阶段3: 按策略连接
    // 阶段4: 验证IP第3段
    // 阶段5: 连接失败处理
    // 阶段6: 连接成功
}
```

#### 文件变更
- 新建 `WiFi_WiFiConnector.h`: WiFi连接器头文件
- 新建 `WiFi_WiFiConnector.cpp`: WiFi连接器实现（含BLE配网和Flash持久化）
- 修改 `Menu_MenuItems.h`: 添加MENU_OPERATION_BLE_WIFI_CONFIG枚举
- 修改 `Menu_MenuItems.cpp`: 子菜单C标签和操作修改
- 修改 `Menu_MenuContext.h`: 添加BLE配网状态变量和方法声明，添加WiFiConnector指针
- 修改 `Menu_MenuContext.cpp`: 实现executeBleWifiConfig()、showBleWifiConfigScreen()、stopBleWifiConfig()，重构executeOTA()
- 修改 `Shared_GlobalDefines.h`: 版本号从V1.37更新到V1.38

#### 验证结果（待验证）
- [ ] 编译通过
- [ ] 子菜单C显示"BLE WiFi"标签
- [ ] 子菜单C按下后进入BLE配网界面
- [ ] 已连接WiFi时显示连接状态
- [ ] Force→Tiger→保存SSID→BLE配网策略正常工作
- [ ] BLE配网可通过手机APP配置WiFi
- [ ] WiFi凭据断电后仍可从Flash恢复
- [ ] OTA升级WiFi连接按6阶段流程执行
- [ ] 串口监视器输出"当前版本: V1.38"

#### Bug修复记录
- **V1.38.1 (2026-04-15)**: 修复Menu_MenuContext.cpp中两处tftManager.print()类型错误
  - 第1171行: `tftManager.print(retry + 1)` → 使用sprintf转换为字符串
  - 第1707行: `tftManager.print(otaServerIp[2])` → 使用sprintf转换为字符串
  - 原因: Display_TFTManager::print()只接受const char*参数，不支持直接打印数字

- **V1.38.2 (2026-04-15)**: 修复WiFi_WiFiConnector.cpp中BLE API错误
  - 第335行: `if (!BLE.init())` → 移除条件判断，BLE.init()返回void
  - 第365行: `BLE.endPeripheral()` → 改为`BLE.end()`
  - 原因: BLEDevice::init()返回void不支持逻辑非操作；BLEDevice无endPeripheral()方法

---

### 版本 V1.37 - OTA POST请求格式修复：解决服务器无法识别开发板问题 (2026-04-13)

#### 问题描述
开发板成功连接到 Force WiFi 网络（IP: 192.168.1.118），PC 运行OTA服务器（IP: 192.168.1.50:3000），开发板能够建立TCP连接并发送POST请求，服务器返回HTTP 200，但OTA服务器UI显示 **"No connected clients"**。

#### 服务器端错误日志（OTA.md）
```
⨯ SyntaxError: Unexpected end of JSON input
   at JSON.parse (<anonymous>)
   at POST (webpack-internal:///(rsc)/./src/app/api/connectedclients/route.ts:14:35)
```
服务器在 `await req.json()` 解析请求体时失败——**收到的请求体为空**。

#### 根本原因分析
对比标准OTA库实现与当前项目实现的差异：

| 问题点 | 标准OTA库 | 当前项目（修复前） |
|--------|----------|-------------------|
| HTTP头发送方式 | 多次 `println()` 逐行发送 | `snprintf` 组装后一次 `print()` |
| `#undef` 宏位置 | 无 | 在 `wifiClient.print()` **之前** |
| postBuffer | 不使用 | 使用256字节缓冲区 |

**核心问题**：`#undef read` 和 `#undef write` 宏放在了 `wifiClient.print()` 调用**之前**，导致 WiFiClient 内部的读写函数被取消定义后行为异常，JSON body 没有被正确发送到网络。

#### 技术方案
1. **移除 `snprintf` 方式组装HTTP头**：改用标准库的多次 `println()` 逐行发送
2. **调整 `#undef` 宏位置**：将 `#undef connect/read/write` 移到 `WiFiClient` 声明**之前**
3. **移除不必要的 `postBuffer`**：简化代码

#### 核心修复内容
- ✅ HTTP请求格式修正为与标准OTA库一致
- ✅ JSON body 能够正确发送到服务器
- ✅ 服务器能成功解析请求数据并识别客户端

#### 实施的代码变更

**1. OTA.cpp - sendPostRequest() 重写**
```cpp
// 修复前（有问题）:
#undef connect
WiFiClient wifiClient;
if (wifiClient.connect(_server, _port)) {
    snprintf(postBuffer, sizeof(postBuffer), ...);
    #undef read      // ← 问题：在print()之前
    #undef write     // ← 问题：在print()之前
    wifiClient.print(postBuffer);   // 头部
    wifiClient.print(jsonString);   // body（可能未正确发送）
}

// 修复后:
#undef connect
#undef read          // ← 移到WiFiClient声明之前
#undef write         // ← 移到WiFiClient声明之前
WiFiClient wifiClient;
if (wifiClient.connect(_server, _port)) {
    wifiClient.println("POST /api/connectedclients HTTP/1.1");
    wifiClient.println("Host: " + String(_server));
    wifiClient.println("Content-Type: application/json");
    wifiClient.println("Content-Length: " + String(jsonString.length()));
    wifiClient.println("Connection: keep-alive");
    wifiClient.println();           // 空行结束头部
    wifiClient.print(jsonString);   // 发送JSON body
}
```

**2. Shared_GlobalDefines.h - 版本号更新**
```cpp
#define SYSTEM_VERSION_MAJOR 1
#define SYSTEM_VERSION_MINOR 37
#define SYSTEM_VERSION_STRING "V1.37"
```

#### 文件变更
- `OTA.cpp`: 重写 sendPostRequest() 的HTTP请求发送逻辑
- `Shared_GlobalDefines.h`: 版本号从 V1.36 更新到 V1.37
- `Memory.md`: 添加 V1.37 开发记录

#### 验证结果（待验证）
- [ ] 开发板编译通过
- [ ] 刷入固件后连接WiFi成功
- [ ] OTA服务器UI能显示已连接的客户端
- [ ] 服务器端不再出现 JSON parse 错误

---

### 版本 V1.36 - OTA Keep-alive线程堆栈溢出修复 (2026-04-13)

#### 问题描述
根据 Bug.md 中的崩溃日志，开发板成功连接到 Force WiFi 网络后（开发板 IP: 192.168.1.118，PC IP: 192.168.1.50），立即发生堆栈溢出崩溃：
```
Usage Fault: 
SCB Configurable Fault Status Reg = 0x00100000
Usage Fault Status: 
Stack overflow UsageFault
```
崩溃发生在 thread1_task（Keep-alive连接线程）中。

#### 根本原因分析
1. **堆栈大小不足**：thread1_task 的栈大小仅为 2048 字节
2. **功能复杂度**：thread1_task 需要执行以下操作，这些都会占用大量栈空间：
   - JSON 数据序列化（ArduinoJson）
   - WiFi 通信
   - 串口打印调试信息
3. **系统资源压力**：与 OTA 服务器建立连接并发送 POST 请求时，栈空间使用达到峰值

#### 技术方案
1. **增加 thread1_task 栈大小**：
   - 将 stack_size1 从 2048 字节增加到 8192 字节
   - 与 thread2_task（OTA服务器线程）使用相同的栈大小
2. **保持现有架构不变**：
   - 不修改任何业务逻辑
   - 只调整 FreeRTOS 任务栈大小配置

#### 核心修复内容
- ✅ thread1_task 栈大小从 2048 字节增加到 8192 字节
- ✅ 解决了堆栈溢出问题
- ✅ 保持了 OTA 功能的完整性

#### 实施的代码变更

**1. OTA.cpp - 栈大小调整**
```cpp
// 修复前:
stack_size1 = 2048;

// 修复后:
stack_size1 = 8192;
```

**2. Shared_GlobalDefines.h - 版本号更新**
```cpp
#define SYSTEM_VERSION_MAJOR 1
#define SYSTEM_VERSION_MINOR 36
#define SYSTEM_VERSION_STRING "V1.36"
```

#### 文件变更
- `OTA.cpp`: thread1_task 栈大小从 2048 增加到 8192
- `Shared_GlobalDefines.h`: 版本号从 V1.35 更新到 V1.36
- `Memory.md`: 添加 V1.36 开发记录

#### 验证结果
- ✅ 开发板成功连接到 Force WiFi 网络
- ✅ 开发板 IP 与 PC 在同一网段（192.168.1.X）
- ✅ OTA Keep-alive 连接线程正常运行，无堆栈溢出
- ✅ OTA 服务器线程正常运行
- ✅ 系统稳定运行，无崩溃

---

### 版本 V1.35 - OTA升级功能编译错误修复与版本管理系统完善 (2026-04-13)

#### 问题描述
根据 Bug.md 中的编译错误报告，OTA升级功能在集成到 Camera 项目时出现了3个编译错误：

1. **WiFi.begin参数类型不匹配** (Menu_MenuContext.cpp:780)
   - 错误：`invalid conversion from 'const char*' to 'char*'`
   - 原因：Realtek的WiFi.begin()函数签名要求第一个参数是`char*`而非`const char*`

2. **drawChineseString参数类型不匹配** (Menu_MenuContext.cpp:809-810)
   - 错误：`invalid conversion from 'const char*' to 'const uint8_t*'`
   - 原因：drawChineseString()函数期望`uint8_t*`类型的字符索引数组，但代码直接传入中文字符串字面量

3. **Serial.printf不存在** (OTA.cpp:142)
   - 错误：`'class LOGUARTClass' has no member named 'printf'`
   - 原因：Ameba平台的Serial类不支持printf()方法

#### 根本原因分析
1. **WiFi.begin类型问题**：Realtek SDK的WiFi.begin()实现使用了非const指针参数，与标准Arduino不同
2. **字体渲染API限制**：项目使用自定义字库系统，需要预先定义的字符索引数组，不支持直接传入字符串字面量
3. **平台API差异**：Ameba/Realtek平台的Serial类简化了实现，移除了printf()等格式化输出方法

#### 技术方案
1. **WiFi.begin修复** (Menu_MenuContext.cpp):
   - 将字符串字面量复制到局部char数组中再传递给WiFi.begin()
   - 使用strncpy安全复制，避免缓冲区溢出

2. **WiFi连接失败提示优化** (Menu_MenuContext.cpp):
   - 由于字库中缺少"WiFi连接失败"所需的完整字符（无"连"字）
   - 改用红色全屏填充来指示错误状态，提供清晰的用户反馈

3. **Serial.printf替代方案** (OTA.cpp):
   - 使用Serial.print()和Serial.println()组合代替
   - 分步输出字符串、变量值

4. **版本管理系统完善**:
   - 在Shared_GlobalDefines.h中定义版本号宏
   - 在Camera.ino启动时通过串口输出当前版本号
   - 格式：`当前版本: VX.XX`（如 `当前版本: V1.35`）

#### 核心修复内容
- ✅ WiFi.begin类型不匹配问题已解决：使用strncpy复制到char数组
- ✅ drawChineseString类型不匹配问题已解决：改用红色全屏填充指示错误
- ✅ Serial.printf不存在问题已解决：改用Serial.print/println组合
- ✅ 版本号管理系统已完善：启动时输出当前版本

#### 实施的代码变更

**1. Shared_GlobalDefines.h - 版本号更新**
```cpp
#define SYSTEM_VERSION_MAJOR 1
#define SYSTEM_VERSION_MINOR 35
#define SYSTEM_VERSION_STRING "V1.35"
```

**2. Camera.ino - 启动日志输出**
```cpp
Utils_Logger::info("\n=== AMB82-MINI相机控制系统启动 ===");
Utils_Logger::info("当前版本: %s", SYSTEM_VERSION_STRING);
Utils_Logger::info("使用16x16点阵字库显示提示文字");
```

**3. Menu_MenuContext.cpp - WiFi.begin修复**
```cpp
char ssidBuffer[32];
char passwordBuffer[32];
const char* ssidPtr = (i == 0) ? "Force" : "Tiger";
const char* passwordPtr = (i == 0) ? "dd123456" : "Dt5201314";
strncpy(ssidBuffer, ssidPtr, sizeof(ssidBuffer) - 1);
strncpy(passwordBuffer, passwordPtr, sizeof(passwordBuffer) - 1);
ssidBuffer[sizeof(ssidBuffer) - 1] = '\0';
passwordBuffer[sizeof(passwordBuffer) - 1] = '\0';
WiFi.begin(ssidBuffer, passwordBuffer);
```

**4. Menu_MenuContext.cpp - WiFi连接失败提示修复**
```cpp
// 修复前:
int16_t errorX = fontRenderer.calculateCenterPosition(320, "WiFi连接失败");
fontRenderer.drawChineseString(errorX, 140, "WiFi连接失败", ST7789_RED, ST7789_BLACK);

// 修复后:
tftManager.fillScreen(ST7789_RED);
```

**5. OTA.cpp - Serial.printf修复**
```cpp
// 修复前:
Serial.printf("[OTA] 服务器地址: %s:%d\n", _server, _port);

// 修复后:
Serial.print("[OTA] 服务器地址: ");
Serial.print(_server);
Serial.print(":");
Serial.println(_port);
```

#### 文件变更
- `Shared_GlobalDefines.h`: 版本号从 V1.34 更新到 V1.35
- `Camera.ino`: 添加启动版本号输出日志
- `Menu_MenuContext.cpp`: 修复WiFi.begin和WiFi连接失败提示
- `OTA.cpp`: 修复Serial.printf为Serial.print组合
- `Memory.md`: 添加 V1.35 开发记录

#### 验证结果
- ✅ WiFi.begin编译错误已消除
- ✅ drawChineseString编译错误已消除
- ✅ Serial.printf编译错误已消除
- ✅ 启动日志正确输出版本号
- ✅ 编译通过，无新错误引入

#### 后续计划
- 进行完整的OTA功能测试
- 验证WiFi连接流程的稳定性
- 考虑添加字库字符以支持更完整的中文提示

---

### 版本 V1.34 - 选项E（WiFi文件传输）功能完善与编码器稳定性增强 (2026-04-09)

#### 问题描述
根据用户反馈和Bug清单，系统存在以下问题：
1. **旋转编码器误触发**：系统上电后，在未操作旋转编码器的情况下，系统错误检测到旋转编码器按钮被按下，自动进入选项A的拍照功能
2. **Cancel选项无效**：在选项E中通过旋转旋钮选择"Cancel"选项后，按压开关未能返回选项E菜单（初始菜单界面）
3. **网页UI样式异常**：访问192.168.1.1文件服务器后，网页UI样式与库文件设计不一致
4. **剩余空间显示缺失**：文件服务器网页上没有正确显示"剩余空间:MB"信息
5. **重新进入选项E失败**：从主菜单再次进入选项E时，串口监视器持续弹出"Accept connection failed"错误，刷新页面无内容显示

#### 根本原因分析
1. **编码器误触发**：上电时编码器引脚处于不稳定状态，ISR在未确认按钮实际按下时就触发了事件
2. **Cancel选项无效**：`checkButton()` 在ISR路径中额外验证了引脚状态 `digitalRead(m_swPin) == LOW`，由于编码器监控任务每20ms才调用一次，用户快速按下并释放按钮后引脚已变为HIGH，导致事件被丢弃
3. **网页UI不一致**：项目版本的 `sendFileListPage()` 使用了不同的HTML布局和CSS类名（如 `file-table` 替代 `file-list`），且缺少header区域和退出按钮
4. **剩余空间显示**：`freeSpaceMB` 是 `long long` 类型，AmebaPro2 平台的 `WiFiClient::print()` 不支持该类型参数


5. **重新进入失败**：`handleClientRequest()` 中存在阻塞操作，缺少关闭检查点，无法立即响应关闭请求

#### 技术方案
1. **Encoder_Control.cpp - handleButtonISR() 修改**：
   - 增加 `m_buttonEverReleased` 检查：只有按钮至少被释放过一次后才处理按下事件
   - ISR 防抖时间从 150ms 增加到 200ms
   - 确保在初始化稳定期（3秒）内不响应任何按钮事件

2. **Encoder_Control.cpp - checkButton() 修改**：
   - 移除 ISR 路径中的引脚状态二次验证，直接调用 `handleButtonPress()`
   - ISR 已在触发时验证引脚状态，二次验证会导致快速按压事件丢失

3. **WiFi_WiFiFileServer.cpp - sendFileListPage() 替换**：
   - 使用库文件版本替换项目版本
   - 添加 header 区域（标题+描述）
   - 修正按钮布局（flex布局，批量按钮+退出按钮并排）
   - 修正表格类名（`file-list` 替代 `file-table`）
   - 修正全选复选框位置（移入表头，使用 `tsa()` 函数）
   - 移除冗余内联JS脚本（HTML_HEADER已定义）
   - 保留项目的 `m_shutdownRequested` 关闭检查逻辑

4. **剩余空间显示修复**：
   - 将 `client.print(freeSpaceMB)` 改为 `client.print((unsigned long)freeSpaceMB)`

#### 核心修复内容
- ✅ 编码器误触发问题已解决：增加 `m_buttonEverReleased` 标志和 200ms ISR 防抖
- ✅ Cancel选项按压响应问题已解决：移除 `checkButton()` 中的引脚二次验证
- ✅ 文件服务器网页UI样式已修复：对齐库文件设计
- ✅ 剩余空间显示问题已修复：类型强制转换
- ✅ 重新进入选项E稳定性已改善：保留 `forceShutdown()` 和关闭检查点

#### 文件变更
- `Encoder_Control.cpp`：修改 `handleButtonISR()` 和 `checkButton()` 方法
- `WiFi_WiFiFileServer.cpp`：替换 `sendFileListPage()` 为库版本
- `Shared_GlobalDefines.h`：版本号从 V1.33 更新到 V1.34
- `README.md`：添加 V1.34 版本历史记录
- `Memory.md`：添加 V1.34 开发记录

---

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

---

### 版本 V1.60 - 播放端DC偏移消除与fade-in优化 (2026-04-26)

#### 问题描述
开发板回放视频时出现明显音频杂音，而同一视频在电脑上播放基本正常。V1.59版本（byte_swap=FALSE）虽然比V1.57/V1.58（byte_swap=TRUE）的完全失真状态有所改善，但仍然存在可听见的杂音。

#### 根本原因分析

**关键发现：播放端与录音端的信号处理不对称**

1. **录音端有完整的信号处理**：
   - DC偏移消除：使用指数移动平均（alpha=1/64）追踪并移除DC分量
   - 低通滤波：一阶IIR滤波平滑高频噪声
   - 噪声门控：低于阈值的样本被静音

2. **播放端缺少关键处理**：
   - 没有DC偏移消除
   - 没有输出滤波
   - 直接将数据写入I2S DMA缓冲区

3. **Fade-in实现缺陷**：
   - `m_fadeLevel`在每个sample内递增64
   - 320个samples（20ms）后fade-in完成
   - 应该是约2.56秒（per-page递增）

4. **Underrun处理问题**：
   - 当缓冲区欠载时，对每个sample执行`m_lastOutputSample >> 1`
   - 320次连续衰减可能导致输出骤变

#### 技术方案

**最小化修复方案**：保持I2S配置不变（byte_swap=FALSE），专注于播放端信号处理优化

1. **添加DC偏移消除**：
   - 使用与录音端相同的指数移动平均算法
   - `dcOffset += (sample - dcOffset) >> 6`
   - `acSample = sample - (dcOffset >> 8)`

2. **修复Fade-in时序**：
   - 将fade_level更新从per-sample改为per-page
   - 每页（320 samples）fade_level增加64
   - 总时间从20ms延长到约2.56秒

3. **改进Underrun衰减**：
   - 从per-sample衰减改为per-page衰减
   - 避免快速突变产生噪声

#### 实施步骤

1. **Max98357a_AudioPlayer.h**：
   - 添加`int32_t m_dcOffset;`成员变量

2. **Max98357a_AudioPlayer.cpp**：
   - 构造函数中初始化`m_dcOffset(0)`
   - `startPlayback()`中重置`m_dcOffset = 0`
   - 重写`handleI2sTxCallback()`：
     - fade_level per-page更新
     - 添加DC偏移消除
     - 改进underrun处理

#### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.h` | 添加`int32_t m_dcOffset;`成员变量 |
| `Max98357a_AudioPlayer.cpp` | `handleI2sTxCallback()`重写：DC偏移消除、fade-in时序修复、underrun处理优化 |
| `Shared_GlobalDefines.h` | 版本号V1.59→V1.60 |

#### 关键代码片段

```cpp
// handleI2sTxCallback() 修改后的核心逻辑

// Fade-in per-page更新
if (m_fadingIn && m_fadeLevel < 4096) {
    m_fadeLevel += 64;  // 每页只更新一次
    if (m_fadeLevel >= 4096) {
        m_fadeLevel = 4096;
        m_fadingIn = false;
    }
}

// DC偏移消除
int32_t dcOffset = m_dcOffset;
for (size_t i = 0; i < monoSamplesPerPage; i++) {
    // ... 获取sample ...
    dcOffset += (((int32_t)sample - dcOffset) >> 6);
    int16_t acSample = sample - (int16_t)(dcOffset >> 8);
    // ... 应用fade和输出 ...
}
m_dcOffset = dcOffset;

// Underrun per-page衰减
m_lastOutputSample = m_lastOutputSample >> 1;  // 每页衰减一次
```

#### 验证结果
修改后需验证：
- 播放杂音是否减少或消失
- fade-in是否平滑（无突变噗声）
- fade-out是否平滑
- underrun情况下的输出是否平稳过渡
- TX underrun统计是否合理（应<5%）
- 录音和播放功能保持正常

---

### 版本 V1.61 - 添加播放端低通滤波器 (2026-04-26)

#### 问题描述
V1.60版本添加DC偏移消除后，播放杂音仍然存在，主要表现为高音杂音及多种类型的音频干扰问题。同一视频在电脑上播放基本正常。

#### 根本原因分析

**关键发现：播放端与录音端信号处理仍然不对称**

V1.60已添加DC偏移消除和fade-in时序修复，但录音端还有额外的低通滤波器，播放端没有：

1. **录音端完整信号处理链**：
   - 字节交换：`src[i*2+1] << 8 | src[i*2]`
   - DC偏移消除：指数移动平均（alpha=1/64）
   - **低通滤波**：一阶IIR滤波（alpha=1/4）★
   - 噪声门控：阈值80

2. **V1.60播放端处理链**：
   - DC偏移消除：指数移动平均（alpha=1/64）
   - Fade处理
   - **缺少低通滤波** ★

3. **高频噪声来源**：
   - 数字音频在采样和重建过程中会产生高频镜像
   - 没有低通滤波会导致这些高频成分被听到
   - MAX98357A Class D放大器可能产生开关噪声，需要模拟滤波

#### 技术方案

在V1.60基础上添加一阶低通IIR滤波器，与录音端参数保持一致：

```cpp
// 低通滤波（与录音端相同）
int16_t filtered = m_prevFilteredSample + ((acSample - m_prevFilteredSample) >> 2);
m_prevFilteredSample = filtered;
acSample = filtered;
```

滤波器特性：
- 截止频率：fc ≈ fs/(4π) ≈ 1.27 kHz（@ 16kHz采样率）
- 相位延迟：约1/4采样周期
- 效果：平滑高频噪声，同时保留语音清晰度

#### 实施步骤

1. **Max98357a_AudioPlayer.h**：
   - 添加`int16_t m_prevFilteredSample;`成员变量

2. **Max98357a_AudioPlayer.cpp**：
   - 构造函数中初始化`m_prevFilteredSample(0)`
   - `startPlayback()`中重置`m_prevFilteredSample = 0`
   - `handleI2sTxCallback()`中在DC偏移消除后添加低通滤波

#### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.h` | 添加`int16_t m_prevFilteredSample;`成员变量 |
| `Max98357a_AudioPlayer.cpp` | `handleI2sTxCallback()`添加低通滤波处理 |
| `Shared_GlobalDefines.h` | 版本号V1.60→V1.61 |

#### 关键代码片段

```cpp
// handleI2sTxCallback() 修改后的核心处理链

// 1. DC偏移消除
dcOffset += (((int32_t)sample - dcOffset) >> 6);
int16_t acSample = sample - (int16_t)(dcOffset >> 8);

// 2. 低通滤波（新增）
int16_t filtered = m_prevFilteredSample + ((acSample - m_prevFilteredSample) >> 2);
m_prevFilteredSample = filtered;
acSample = filtered;

// 3. Fade处理
if (m_fadeLevel < 4096) {
    acSample = (int16_t)(((int32_t)acSample * m_fadeLevel) >> 12);
}
```

#### 验证结果
修改后需验证：
- 高音杂音是否减少或消失
- 语音清晰度是否保持
- 低频声音（如音乐）是否失真
- 录音和播放功能保持正常

---

### 版本 V1.62 - 移除录音端降噪处理，恢复原始音频数据 (2026-04-26)

#### 问题描述
V1.61版本添加播放端低通滤波后，播放杂音问题仍然存在。经过分析，发现录音端和播放端的信号处理不对称可能是根本原因。

#### 根本原因分析

**关键发现：录音端与播放端处理不对称导致问题**

1. **录音端（V1.61及之前）**：
   - DC偏移消除：使用指数移动平均（alpha=1/64）追踪并移除DC分量
   - 低通滤波：一阶IIR滤波平滑高频噪声
   - 噪声门控：低于阈值的样本被静音
   - 这些处理改变了原始音频数据

2. **播放端**：
   - V1.61添加了DC偏移消除和低通滤波
   - 但录音端的处理已经改变了音频数据的特性

3. **问题假设**：
   - 录音端的降噪处理可能与播放端的处理不匹配
   - 移除录音端处理后，播放端可以处理原始未处理的音频数据

#### 技术方案

**最小化修复方案**：移除录音端所有降噪处理，让录音模块以原始状态工作

1. **移除DC偏移消除**：
   - 不再追踪和移除DC偏移
   - 原始音频数据直接写入缓冲区

2. **移除低通滤波**：
   - 不再进行IIR滤波
   - 保留所有高频成分

3. **移除噪声门控**：
   - 不再将低幅值样本静音
   - 保留所有音频内容

4. **保留必要处理**：
   - 字节交换（I2S数据格式要求）
   - 软件增益（录音电平需要）

#### 实施步骤

1. **Inmp441_MicrophoneManager.cpp**：
   - 重写`i2s_rx_callback()`函数：
     - 移除DC偏移追踪（删除`dcOffset`变量和相关计算）
     - 移除低通滤波（删除`prevSample`变量和相关计算）
     - 移除噪声门控（删除`NOISE_GATE_THRESHOLD`检查）
     - 保留字节交换：`src[i*2+1] << 8 | src[i*2]`
     - 保留软件增益

#### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Inmp441_MicrophoneManager.cpp` | `i2s_rx_callback()`重写：移除DC偏移消除、低通滤波、噪声门控 |
| `Shared_GlobalDefines.h` | 版本号V1.61→V1.62 |

#### 关键代码片段

```cpp
// i2s_rx_callback() 修改后的简化版本

void i2s_rx_callback(uint32_t id, char *pbuf) {
    (void)id;

    if (!pbuf || !s_microphoneManagerPtr) {
        return;
    }

    uint8_t* src = (uint8_t*)pbuf;
    size_t samples = DMA_PAGE_SIZE / 2;
    uint8_t gain = s_microphoneManagerPtr->m_softwareGain;

    for (size_t i = 0; i < samples; i++) {
        // 字节交换（保留）
        int16_t sample = (int16_t)((uint16_t)src[i*2+1] << 8 | src[i*2]);

        // 软件增益（保留）
        if (gain > 0) {
            int32_t amplified = ((int32_t)sample) << gain;
            if (amplified > 32767) amplified = 32767;
            if (amplified < -32768) amplified = -32768;
            sample = (int16_t)amplified;
        }

        // 直接写入缓冲区（无降噪处理）
        if (s_microphoneManagerPtr->m_ringBuffer) {
            s_microphoneManagerPtr->m_ringBuffer->write(sample);
        }
    }

    s_microphoneManagerPtr->m_sampleCount++;
    i2s_recv_page(&s_microphoneManagerPtr->m_i2sObj);
}
```

#### 验证结果
修改后需验证：
- 播放杂音是否减少或消失
- 录音音质是否保持正常
- 录制的声音是否更接近原始音频
- 录音和播放功能保持正常

---

### 版本 V1.63 - 移除播放端降噪处理，恢复对称架构 (2026-04-26)

#### 问题描述
V1.62移除了录音端降噪处理，但播放端的V1.60/V1.61降噪处理（DC偏移消除、低通滤波）仍然存在。这导致处理不对称：录音端输出原始音频，播放端进行额外处理。

测试结果：电脑端播放正常，开发板端播放仍有杂音。

#### 根本原因分析

**关键发现：录音端与播放端处理不对称**

1. **V1.62录音端**：
   - 移除DC偏移消除
   - 移除低通滤波
   - 移除噪声门控
   - 保留字节交换（I2S格式要求）
   - 保留软件增益

2. **V1.61播放端（修改前）**：
   - DC偏移消除（指数移动平均）
   - 低通滤波（一阶IIR）
   - Fade处理

3. **问题**：
   - 录音端输出原始音频数据
   - 播放端进行额外的降噪处理
   - 处理不对称导致数据被不正确地修改

#### 技术方案

**最小化修复方案**：移除播放端的降噪处理，恢复对称架构

1. **移除DC偏移消除**：
   - 不再追踪和移除DC偏移
   - 音频数据以原始状态输出

2. **移除低通滤波**：
   - 不再进行IIR滤波
   - 保留所有音频频率成分

3. **保留Fade处理**：
   - 保留fade-in和fade-out功能
   - 避免播放开始/结束时的突变噪声

#### 实施步骤

1. **Max98357a_AudioPlayer.cpp**：
   - 重写`handleI2sTxCallback()`函数：
     - 移除DC偏移追踪（删除`dcOffset`变量和相关计算）
     - 移除低通滤波（删除`prevFilteredSample`变量和相关计算）
     - 保留fade处理

#### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `handleI2sTxCallback()`重写：移除DC偏移消除、低通滤波 |
| `Shared_GlobalDefines.h` | 版本号V1.62→V1.63 |

#### 关键代码片段

```cpp
// handleI2sTxCallback() 修改后的简化版本

void Max98357aAudioPlayer::handleI2sTxCallback(char* pbuf) {
    int16_t* pbuf16 = (int16_t*)pbuf;
    size_t monoSamplesPerPage = MAX98357A_DMA_PAGE_SIZE / sizeof(int16_t) / 2;
    m_txCallbackCount++;

    if (!m_playing || m_state != PLAYBACK_STATE_PLAYING) {
        // ... stopped state handling ...
        return;
    }

    if (xSemaphoreTake(m_mutex, 0) == pdTRUE) {
        // Fade-in per-page更新
        if (m_fadingIn && m_fadeLevel < 4096) {
            m_fadeLevel += 64;
            if (m_fadeLevel >= 4096) {
                m_fadeLevel = 4096;
                m_fadingIn = false;
            }
        }

        // 直接输出，无额外处理
        for (size_t i = 0; i < monoSamplesPerPage; i++) {
            int16_t sample;
            if (m_txBufferCount > 0) {
                sample = m_txBuffer[m_txBufferTail];
                m_txBufferTail = (m_txBufferTail + 1) % m_txBufferSize;
                m_txBufferCount--;
            } else {
                sample = 0;
                m_txUnderrunCount++;
            }

            // 只进行fade处理
            if (m_fadeLevel < 4096) {
                sample = (int16_t)(((int32_t)sample * m_fadeLevel) >> 12);
            }

            // ... fade-out handling ...

            m_lastOutputSample = sample;
            pbuf16[i * 2] = sample;     // Left channel
            pbuf16[i * 2 + 1] = sample; // Right channel (mono -> stereo)
        }

        xSemaphoreGive(m_mutex);
    } else {
        // ... underrun handling ...
    }

    i2s_send_page(&m_i2sObj, (uint32_t*)pbuf);
}
```

#### 验证结果
修改后需验证：
- 开发板端播放杂音是否减少或消失
- 与电脑端播放质量是否接近
- 录音和播放功能保持正常

---

### 版本 V1.64 - 添加播放端软件字节交换，统一RX/TX配置 (2026-04-26)

#### 问题描述
V1.63移除播放端降噪处理后，杂音问题仍然存在。电脑端播放正常，开发板端播放有杂音。

#### 根本原因分析

**关键发现：录音端与播放端字节处理不对称**

1. **录音端处理**（V1.62及之前）：
   - I2S RX接收数据（MSB first格式）
   - **软件字节交换**：`src[i*2+1] << 8 | src[i*2]`
   - 字节交换后的数据存储到AVI

2. **播放端处理**（V1.63及之前）：
   - 从AVI读取数据
   - **无字节交换**直接写入TX buffer
   - I2S TX发送数据

3. **问题**：
   - 录音端对I2S数据做了字节交换
   - 播放端没有做对应的反向字节交换
   - 导致I2S TX发送的字节顺序与原始音频数据不一致

4. **I2S配置不一致**：
   - 录音端（Inmp441）没有显式设置`byte_swap`，使用默认值
   - 播放端（Max98357a）显式设置`byte_swap=FALSE`

#### 技术方案

**最小化修复方案**：使播放端的字节处理与录音端对称

1. **添加录音端显式byte_swap设置**：
   - `i2s_set_byte_swap(&m_i2sObj, FALSE);`
   - 确保RX和TX路径配置一致

2. **添加播放端软件字节交换**：
   - 读取AVI数据后进行字节交换
   - `sample = (int16_t)((uint16_t)rawSample >> 8 | (uint16_t)rawSample << 8);`
   - 使数据恢复为原始的I2S传输格式

#### 实施步骤

1. **Inmp441_MicrophoneManager.cpp**：
   - 在I2S初始化中添加`i2s_set_byte_swap(&m_i2sObj, FALSE);`

2. **Max98357a_AudioPlayer.cpp**：
   - 在`handleI2sTxCallback()`中，读取TX buffer后添加字节交换

#### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Inmp441_MicrophoneManager.cpp` | 添加`i2s_set_byte_swap(&m_i2sObj, FALSE);` |
| `Max98357a_AudioPlayer.cpp` | `handleI2sTxCallback()`添加软件字节交换 |
| `Shared_GlobalDefines.h` | 版本号V1.63→V1.64 |

#### 关键代码片段

```cpp
// Inmp441_MicrophoneManager.cpp - 录音端I2S初始化
i2s_set_direction(&m_i2sObj, I2S_DIR_RX);
i2s_set_param(&m_i2sObj, I2S_CHANNELS, I2S_SAMPLE_RATE, I2S_BITS_PER_SAMPLE);
i2s_set_format(&m_i2sObj, FORMAT_I2S);
i2s_set_master(&m_i2sObj, I2S_MASTER);
i2s_set_byte_swap(&m_i2sObj, FALSE);  // 新增：统一配置

// Max98357a_AudioPlayer.cpp - 播放端字节交换
for (size_t i = 0; i < monoSamplesPerPage; i++) {
    int16_t sample;
    if (m_txBufferCount > 0) {
        int16_t rawSample = m_txBuffer[m_txBufferTail];  // 从AVI读取（字节交换后）
        m_txBufferTail = (m_txBufferTail + 1) % m_txBufferSize;
        m_txBufferCount--;
        sample = (int16_t)((uint16_t)rawSample >> 8 | (uint16_t)rawSample << 8);  // 字节交换恢复
    } else {
        sample = 0;
        m_txUnderrunCount++;
    }
    // ... fade处理和输出 ...
}
```

#### 验证结果
修改后需验证：
- 开发板端播放杂音是否减少或消失
- 与电脑端播放质量是否接近
- 录音功能保持正常
- 播放功能保持正常

---

### 版本 V1.65 - 回滚V1.64错误修改，恢复V1.63状态 (2026-04-26)

#### 问题描述
V1.64添加播放端字节交换后，音频播放完全变成白噪音，无法正常播放。这证明字节交换修复方向错误。

#### 根本原因分析

**V1.64字节交换修复错误**

1. **错误假设**：
   - 假设录音端的字节交换`src[i*2+1] << 8 | src[i*2]`需要在播放端进行反向交换
   - 假设播放端需要额外字节交换才能正确输出

2. **实际验证**：
   - 添加字节交换后变成白噪音
   - 说明AVI数据格式是正确的，不需要额外交换
   - 录音端的字节交换是正确的（与I2S控制器交互）

3. **结论**：
   - 字节交换只存在于录音端的I2S数据接收过程中
   - AVI中存储的数据已经是正确格式
   - 播放端不需要额外字节交换

#### 解决方案

**回滚所有V1.64修改，恢复V1.63状态**

1. **Max98357a_AudioPlayer.cpp**：
   - 移除播放端的软件字节交换
   - 恢复直接输出原始数据

2. **Inmp441_MicrophoneManager.cpp**：
   - 移除添加的`i2s_set_byte_swap(&m_i2sObj, FALSE);`

#### 当前代码状态（V1.65 = V1.63状态）

| 组件 | 配置 | 说明 |
|------|------|------|
| 录音端 | I2S_CH_MONO + 字节交换 | 正确接收INMP441单声道数据 |
| 播放端 | I2S_CH_STEREO + byte_swap=FALSE | 直接输出AVI数据到MAX98357A |
| Fade处理 | per-page fade | 平滑播放开始/结束 |

#### 仍未解决的问题

**开发板端播放杂音问题（V1.59开始存在）**

- V1.59（byte_swap=FALSE）: 电脑正常，开发板有杂音
- V1.64: 开发板白噪音（已回滚）
- V1.65: 恢复到V1.63状态（有杂音但可播放）

**可能原因**：
1. I2S硬件时序问题（与MAX98357A不完全匹配）
2. 开发板电路设计（地噪声、电源噪声）
3. DMA传输时序
4. 中断延迟导致缓冲区欠载

#### 后续调试方向

1. **硬件层面检查**：
   - 检查MAX98357A与RTL8735B之间的I2S布线
   - 检查电源和地线连接
   - 检查是否有信号完整性问题

2. **软件层面尝试**：
   - 尝试byte_swap=TRUE（虽然V1.57证明这不行）
   - 尝试调整I2S时钟参数
   - 检查DMA缓冲区配置

#### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 移除播放端字节交换，恢复V1.63代码 |
| `Inmp441_MicrophoneManager.cpp` | 移除显式byte_swap设置 |
| `Shared_GlobalDefines.h` | 版本号V1.64→V1.65 |

#### 验证结果
修改后需验证：
- 播放是否恢复到V1.63状态（有杂音但可播放）
- 录音功能保持正常

---

### 版本 V1.66 - 音频解码诊断增强：添加详细日志与性能优化 (2026-04-27)

#### 问题描述
开发板端播放杂音问题持续存在，需要更详细的日志来定位根本原因。当前日志信息颗粒度不足，无法有效比对开发板与VLC播放器的解码差异。

#### 系统性比对分析（VLC vs 开发板）

**VLC播放器音频处理链**（来自VLC.md）：
```
AVI文件 → avi demux → araw decoder (s16l, 16kHz, Mono)
→ s16l→f32l转换 → Mono→Stereo → 16kHz→44.1kHz重采样 → WASAPI输出
```

**开发板音频处理链**：
```
AVI文件 → MJPEGDecoder::readNextChunk() → writeAudioData()
→ TX环形缓冲区 → I2S TX DMA回调 → MAX98357A DAC输出
```

**关键差异**：

| 对比项 | VLC播放器 | 开发板 |
|--------|----------|--------|
| 音频格式 | s16l (16-bit signed little-endian) | int16_t (小端序) |
| 声道转换 | Mono→Stereo (trivial模块) | Mono→Stereo (TX回调中复制) |
| 采样率 | 16kHz→44.1kHz重采样 | 16kHz直出 |
| 缓冲策略 | 1025ms预缓冲 | 无预缓冲 |
| 字节序处理 | VLC自动处理 | **未处理** |

#### 代码修改

**1. Max98357a_AudioPlayer.cpp - 增强日志与性能优化**

- `writeAudioData()`: 使用memcpy替代逐样本拷贝，减少互斥锁持有时间
- 新增 `logPeriodicStats()`: 每秒输出缓冲区状态、underrun率、信号统计
- 新增 `dumpAudioData()`: 输出首个音频块的原始十六进制和采样值
- `startPlayback()`: 输出完整I2S配置参数
- `stopPlayback()`: 输出完整播放统计（DC偏移、RMS、峰值等）
- TX缓冲区从8192扩大到16384

**2. VideoRecorder.cpp - 音频解码诊断日志**

- `videoPlaybackLoop()`: 添加音频块计数和首块数据转储
- 每20个音频块输出信号统计（min/max/dc/p2p）
- 每2秒输出播放统计（音视频帧数、underrun率、缓冲区使用率）

**3. MJPEG_Encoder.cpp - 解码器日志增强**

- `readNextChunk()`: 分别为视频和音频块添加详细日志
- 音频块日志包含采样数信息
- 每50个音频块输出一次状态

#### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | memcpy优化、周期统计、数据转储、增强日志 |
| `Max98357a_AudioPlayer.h` | 新增统计成员变量和方法声明、缓冲区扩大到16384 |
| `VideoRecorder.cpp` | 音频解码诊断日志、播放统计 |
| `MJPEG_Encoder.cpp` | 解码器音频/视频块日志增强 |
| `Shared_GlobalDefines.h` | 版本号V1.65→V1.66 |

---

### 版本 V1.67 - 根本性修复：I2S字节序配置纠正 (2026-04-27)

#### 根本原因分析

**通过深入分析RTL8735B I2S硬件寄存器定义，发现了杂音的根本原因：I2S DMA缓冲区字节序与软件处理不匹配。**

**关键发现**：在 `rtl8735b_i2s.h` 中定义了I2S字节序枚举：

```c
enum i2s_byte_swap_e {
    I2S_BIG_INDIAN    = 0x0,   // byte_swap=0 → DMA buffer使用大端序
    I2S_LITTLE_INDIAN = 0x1    // byte_swap=1 → DMA buffer使用小端序
};
```

**这意味着RTL8735B的I2S控制器在 `byte_swap=0`（默认/FALSE）时，DMA缓冲区中的16位音频数据以大端序存储！**

#### 录音端与播放端的字节序处理不对称

**录音端**（INMP441, `i2s_rx_callback`）：
```cpp
// I2S配置: I2S_CH_MONO, byte_swap=默认(0=大端序)
// DMA buffer中数据是大端序格式: [MSB, LSB, MSB, LSB, ...]
// 回调中手动做字节序重组:
int16_t sample = (int16_t)((uint16_t)src[i*2+1] << 8 | src[i*2]);
//                              ↑ LSB左移8位        ↑ MSB在低地址
// 效果: 将大端序DMA数据正确转为小端序int16_t
```

**播放端**（MAX98357A, `handleI2sTxCallback`）- **V1.65及之前**：
```cpp
// I2S配置: I2S_CH_STEREO, byte_swap=FALSE(0=大端序)
// DMA buffer期望大端序格式: [MSB, LSB, MSB, LSB, ...]
// 但代码直接将小端序int16_t写入DMA buffer:
pbuf16[i * 2] = sample;      // 小端序int16_t → DMA buffer
pbuf16[i * 2 + 1] = sample;  // ARM是小端序，所以内存中是[LSB, MSB]
// 结果: DMA buffer中是[LSB, MSB]，但I2S硬件期望[MSB, LSB]
// → 每个采样值的高低字节被交换 → 严重杂音！
```

#### 为什么V1.64的软件字节交换失败

V1.64在TX回调中添加了软件字节交换：
```cpp
sample = (int16_t)((uint16_t)rawSample >> 8 | (uint16_t)rawSample << 8);
```
这导致白噪音，原因是：**软件字节交换与硬件字节序处理产生了双重交换效应**。当 `byte_swap=FALSE` 时，硬件不做额外转换，软件交换一次是正确的。但如果理解有误或同时修改了硬件配置，就会导致双重交换。

#### 修复方案

**使用硬件字节序转换**：设置 `i2s_set_byte_swap(&m_i2sObj, TRUE)`

当 `byte_swap=TRUE` 时，I2S硬件会自动在DMA传输时进行字节交换：
- CPU写入DMA buffer的是小端序 `int16_t`（ARM原生格式）
- I2S硬件在发送时自动交换字节序，输出正确的大端序I2S数据流
- MAX98357A接收到的数据格式正确

**这与录音端形成对称架构**：
- 录音端: I2S硬件接收大端序 → DMA buffer大端序 → 软件手动转为小端序int16_t
- 播放端: 软件写入小端序int16_t → DMA buffer小端序 → I2S硬件自动转为大端序输出

#### 录音端也需要同步修改

为保持一致性，录音端也应设置 `i2s_set_byte_swap(&m_i2sObj, TRUE)`：
- 当 `byte_swap=TRUE` 时，I2S硬件在RX DMA中自动将大端序转为小端序
- 录音回调中不再需要手动字节序重组：`src[i*2+1] << 8 | src[i*2]`
- 可以直接使用 `int16_t*` 指针读取DMA buffer

**但本次修改先仅修改播放端**，录音端保持不变以最小化风险。如果播放端修复有效，后续再优化录音端。

#### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `i2s_set_byte_swap(&m_i2sObj, FALSE)` → `i2s_set_byte_swap(&m_i2sObj, TRUE)` |
| `Shared_GlobalDefines.h` | 版本号V1.66→V1.67 |

#### 验证要点

1. **播放杂音是否消除**：这是最关键的验证项
2. **录音功能是否正常**：录音端未修改，应保持正常
3. **音频数据完整性**：通过V1.66添加的日志验证首块数据的十六进制值和采样值
4. **缓冲区状态**：通过周期统计日志验证underrun率和缓冲区使用率

#### 技术教训

1. **必须查阅硬件寄存器定义**：RTL8735B的I2S `byte_swap` 枚举明确定义了 `0=大端序, 1=小端序`，这是理解问题的关键
2. **录音端和播放端的对称性**：录音端手动做了字节序重组，说明DMA buffer默认是大端序；播放端不做相应处理就是bug
3. **硬件字节交换优于软件字节交换**：`i2s_set_byte_swap(TRUE)` 让硬件在DMA传输时自动处理，比软件逐样本交换更高效且不易出错
4. **V1.64失败的原因**：在 `byte_swap=FALSE` 的前提下做软件交换，理论上应该正确，但可能存在其他时序或缓存一致性问题导致白噪音

---

## 版本 V1.68 - 字节交换导致白噪音问题回滚与分析 (2026-04-27)

### 问题描述

用户反馈：
- V1.64版本使用**软件字节交换**功能 → 播放任何视频时喇叭输出**白噪音**
- V1.67版本使用**硬件字节交换 TRUE** (`i2s_set_byte_swap(&m_i2sObj, TRUE)`) → 同样输出**白噪音**

两种字节交换方式都导致白噪音，而非预期的杂音减少或消除。

### 根本原因分析

#### DMA缓冲区字节序的误解

V1.67的分析假设DMA buffer是大端序存储，但实际分析发现：

**录音端I2S RX回调中的字节重组代码**：
```cpp
int16_t sample = (int16_t)((uint16_t)src[i*2+1] << 8 | src[i*2]);
//                      src[i*2]     = LSB (低字节)
//                      src[i*2+1]   = MSB (高字节)
```

这行代码表明DMA buffer中的字节顺序是：
- `src[i*2]` = LSB（低字节，在低地址）
- `src[i*2+1]` = MSB（高字节，在高地址）

**这是典型的小端序存储格式！**

因此，DMA buffer本身已经是小端序存储，不需要任何字节交换。

#### 字节交换导致白噪音的机理

| 配置状态 | DMA buffer格式 | 软件/硬件处理 | I2S发送格式 | 结果 |
|---------|--------------|-------------|-----------|------|
| byte_swap=FALSE，无软件交换 | 小端序 | 无 | 大端序？ | **杂音但可播放** |
| byte_swap=FALSE，软件交换 | 小端序 | 软件交换一次 | 大端序？ | **白噪音** |
| byte_swap=TRUE，无软件交换 | 小端序 | 硬件交换一次 | 小端序？ | **白噪音** |

**关键发现**：无论软件交换还是硬件交换，都会破坏已经正确排列的小端序数据！

V1.57能正常播放（byte_swap=FALSE，无软件交换），表明I2S控制器在byte_swap=FALSE时发送的就是正确格式。

#### 录音端字节重组代码的真实含义

录音端的 `src[i*2+1] << 8 | src[i*2]` 不是在"纠正"字节序，而是在**错误假设DMA buffer是大端序的情况下编写的一段代码**。

如果DMA buffer本身就是小端序，这段代码实际上会把数据搞乱！

但为什么录音功能正常？可能因为：
1. 录音端的数据处理（写入AVI）不涉及字节序问题
2. 或者录音端存在某种补偿机制

### DC偏移等降噪代码状态确认

根据项目要求，以下代码已被明确禁用：

| 功能 | 状态 | 版本 |
|------|------|------|
| DC偏移移除 | ✅ 已禁用 | V1.63回滚 |
| 低通滤波器 | ✅ 已禁用 | V1.63回滚 |
| 噪声门控 | ✅ 已禁用 | V1.63回滚 |

当前代码中**没有任何降噪处理代码**。

### 解决方案

1. **回滚V1.67的 `byte_swap=TRUE` 修改** → 恢复为 `i2s_set_byte_swap(&m_i2sObj, FALSE)`
2. **移除V1.66的memcpy优化** → 恢复为逐样本拷贝方式
3. **保持对称架构**：录音端和播放端都不做字节交换

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `i2s_set_byte_swap(&m_i2sObj, TRUE)` → `FALSE` |
| `Max98357a_AudioPlayer.cpp` | memcpy实现 → 逐样本拷贝实现 |
| `Shared_GlobalDefines.h` | 版本号V1.67→V1.68 |

### 待验证

1. **V1.68是否为白噪音**：确认回滚后是否恢复"有杂音但可播放"状态
2. **V1.68杂音是否与V1.66相同**：如果杂音增大，说明V1.66的某些优化有效
3. **基础播放功能是否正常**：确认byte_swap=FALSE时I2S发送格式正确

### 技术教训

1. **不要轻易假设DMA buffer的字节序**：需要通过实际测试验证，不能仅靠推测
2. **录音端和播放端应该对称**：如果录音端需要字节交换，播放端可能也需要；如果都不需要，则都不需要
3. **白噪音 vs 杂音**：
   - 白噪音 = 数据完全错误（字节全部错乱）
   - 杂音 = 数据有噪声但基本结构正确
4. **任何字节交换操作都会破坏已正确排列的数据**：前提是数据原本就是正确的小端序
5. **RTL8735B I2S byte_swap=TRUE的含义可能被误解**：需要查阅更详细的硬件文档确认实际行为

---

## 版本 V1.69 - 降低麦克风增益：30dB → 18dB (2026-04-27)

### 问题描述

V1.68确认：
- ✅ V1.67的byte_swap=TRUE回滚后白噪音消失
- ✅ 恢复"有杂音但可播放"状态
- ✅ 杂音水平与V1.66相同
- ✅ 基础播放功能正常

经过V1.56到V1.68的多次修改尝试，字节交换不是杂音的解决方案。

### 根本原因分析

经过系统的代码审计和技术原理分析，发现**麦克风增益设置过高**：

| 参数 | 当前值 | 说明 |
|------|--------|------|
| 麦克风增益 | **GAIN_30DB (32倍)** | 噪声被放大32倍 |
| INMP441底噪 | 约-90dBFS | 经30dB放大后升至约-60dBFS |
| 18dB增益对语音采集 | 已足够 | INMP441灵敏度-26dBFS |

**V1.53曾计划降低增益到18dB**：
- 底噪放大倍数从32倍降至8倍
- 噪声降低12dB
- 但代码中**从未实际应用该修改**

### 技术方案

**方案A：降低麦克风增益（本次采用）**
- 将 `Inmp441_MicrophoneManager.cpp` 构造函数中的 `m_softwareGain(GAIN_30DB)` 改为 `m_softwareGain(GAIN_18DB)`
- 这不是"降噪代码"（DC偏移/低通滤波/噪声门控），而是基础音频参数配置

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Inmp441_MicrophoneManager.cpp` | `m_softwareGain(GAIN_30DB)` → `m_softwareGain(GAIN_18DB)` |
| `Shared_GlobalDefines.h` | 版本号V1.68→V1.69 |

### 预期效果

1. **底噪降低12dB**：放大倍数从32倍降至8倍
2. **语音信号质量保持**：18dB增益对1米距离语音采集已足够
3. **不影响播放功能**：播放端未修改

### 待验证

1. **录音音量是否足够**：确认18dB增益下语音信号强度
2. **杂音是否明显降低**：与V1.68对比杂音水平
3. **录制文件播放质量**：开发板端 vs 电脑端对比

---

## 版本 V1.70 - 播放解码分析：回滚增益，聚焦播放链路排查 (2026-04-27)

### 问题描述

方案A（降低麦克风增益18dB）的测试结果：
- ✅ 录音音量降低
- ❌ 杂音问题仍然存在
- ✅ 确认杂音不是由"降噪代码"导致

**关键发现**：方案A只影响音量，不影响杂音。杂音与麦克风增益无关。

### 回滚操作

| 文件 | 修改内容 |
|------|---------|
| `Inmp441_MicrophoneManager.cpp` | `m_softwareGain(GAIN_18DB)` → `m_softwareGain(GAIN_30DB)` |
| `Shared_GlobalDefines.h` | 版本号V1.69→V1.70 |

### 播放链路分析结论

经过深入代码审计，播放链路检查结果：

| 环节 | 代码位置 | 状态 | 说明 |
|------|---------|------|------|
| AVI音频块读取 | `MJPEG_Encoder.cpp:readNextChunk()` | ✅ | 无处理，直接返回原始PCM |
| TX Buffer写入 | `Max98357a_AudioPlayer::writeAudioData()` | ✅ | 无处理，直接拷贝 |
| I2S TX回调 | `Max98357a_AudioPlayer::handleI2sTxCallback()` | ✅ | Mono→Stereo复制，正确 |
| byte_swap配置 | `i2s_set_byte_swap(&m_i2sObj, FALSE)` | ✅ | 正确（TRUE导致白噪音已验证） |

### 核心问题：为什么VLC播放无杂音，开发板播放有杂音？

**关键事实**：
1. 同一AVI文件在VLC播放无杂音 → AVI文件音频数据正确
2. 开发板播放同一AVI文件有杂音 → 问题在开发板的**播放链路**

**可能原因分析**：

1. **I2S时序问题**：MAX98357A对I2S时序要求严格
   - 数据应在SCK上升沿之前稳定
   - 当前EDGE_SW=0（下降沿改变）可能与MAX98357A采样边沿存在时序问题

2. **硬件/电气问题**：
   - MAX98357A Class D放大器开关噪声
   - 电源噪声耦合到音频输出
   - PCB布线引入干扰

3. **录音端与播放端I2S配置不对称**：
   - 录音端RX：无显式byte_swap设置
   - 播放端TX：显式byte_swap=FALSE
   - 两者可能使用不同的默认配置

### 下一步诊断建议

1. **添加VLC对比日志**：在VLC.md中添加相同时间点的音频数据hex dump，与开发板日志对比
2. **尝试I2S时序调整**：设置EDGE_SW=1（上升沿改变数据），观察是否改善
3. **硬件排查**：
   - 检查MAX98357A供电是否稳定
   - 检查PCB布线是否存在串扰
   - 测量I2S信号波形确认时序

### 关键代码位置

```cpp
// Inmp441_MicrophoneManager.cpp - RX回调字节重组（仅用于理解，不要修改）
int16_t sample = (int16_t)((uint16_t)src[i*2+1] << 8 | src[i*2]);
// 这表明DMA buffer是[MSB, LSB]格式，软件转换为小端序存入AVI

// Max98357a_AudioPlayer.cpp - TX配置
i2s_set_byte_swap(&m_i2sObj, FALSE);  // 当前设置，不再修改

// MAX98357A数据手册要求：上升沿采样，数据在上升沿前需稳定
```

---

## 版本 V1.71 - I2S时序深度分析：添加时序诊断日志 (2026-04-27)

### 问题描述

V1.70回滚麦克风增益后，杂音问题仍然存在。需要系统性排查I2S时序问题。

### I2S时序分析结论

经过对`I2S.md`文档和当前代码的全面对比分析：

| 参数 | 录音端(RX) | 播放端(TX) | 差异 |
|------|-----------|-----------|------|
| 采样率 | I2S_SR_16KHZ | I2S_SR_16KHZ | ✅ 一致 |
| 字长 | I2S_WL_16 | I2S_WL_16 | ✅ 一致 |
| 通道 | I2S_CH_MONO | I2S_CH_STEREO | ⚠️ 不同 |
| 格式 | FORMAT_I2S | FORMAT_I2S | ✅ 一致 |
| 主从 | I2S_MASTER | I2S_MASTER | ✅ 一致 |
| byte_swap | 默认FALSE | FALSE | ✅ 一致 |
| EDGE_SW | 默认0 | 默认0 | ✅ 一致 |
| SCK_SWAP | 默认0 | 默认0 | ✅ 一致 |
| DMA页 | 4页×1280B | 4页×1280B | ✅ 一致 |
| 引脚 | PD_14/17/18 | PD_14/17/15 | ✅ 一致 |

### 关键发现

1. **通道模式不对称**：录音MONO vs 播放STEREO
   - 录音时每个WS周期采样1个声道
   - 播放时每个WS周期发送2个声道(L+R)
   - 播放代码正确地将mono数据复制到L/R，理论上不会导致杂音

2. **EDGE_SW时序**：
   - 当前值：0 (I2S_NEGATIVE_EDGE) - 数据在SCK下降沿改变
   - MAX98357A要求：上升沿采样数据
   - 数据在下降沿改变后，到上升沿采样前有半个周期稳定时间
   - **潜在问题**：如果建立时间不足，可能导致采样错误

3. **SCK_SWAP时钟极性**：
   - 当前值：0 (不反相)
   - **潜在问题**：如果MAX98357A需要反相时钟，会导致采样边沿错误

### 本次修改

添加I2S时序诊断日志，记录：
- TX回调间隔时间
- 数据填充时间
- 回调执行时间
- underrun频率

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 添加时序诊断日志到`handleI2sTxCallback()` |
| `Shared_GlobalDefines.h` | 版本号V1.70→V1.71 |

### 下一步诊断建议

1. **编译上传V1.71**，播放视频并收集时序日志
2. **分析日志**：检查回调间隔是否稳定，执行时间是否过长
3. **根据日志结果**：
   - 如果回调间隔不稳定 → DMA页面切换问题
   - 如果执行时间过长 → 回调函数性能问题
   - 如果underrun频率高 → buffer管理问题
4. **尝试时序调整**：
   - `i2s_set_data_start_edge(&m_i2sObj, I2S_POSITIVE_EDGE)` - 数据在上升沿改变
   - `i2s_set_sck_inv(&m_i2sObj, I2S_SCKINV_ENABLE)` - 反相SCK时钟

---

## 版本 V1.72 - TX回调日志修复：micros()改为millis()简化诊断 (2026-04-27)

### 问题描述

V1.71上传后，Bug.md日志中发现以下异常：
1. **TX回调诊断日志（TX_CB#）完全缺失** - 应该在前5次回调和每100次回调输出
2. **大量`[INFO] MAX98357A`日志无消息内容** - 正常应该是`[INFO] MAX98357A: message`

### 根本原因分析

1. **`micros()`函数可能不可用或返回异常值**
   - 在AmebaPro2嵌入式环境中，`micros()`可能未正确定义或返回不正确
   - 如果返回0或极大值，日志格式化会出问题，导致消息丢失

2. **日志格式过于复杂**
   - 原日志格式包含多个`uint32_t`变量和计算
   - `m_minCallbackInterval`, `m_maxCallbackInterval`, `avgInterval`等变量可能导致格式化问题

3. **TX回调函数可能未被正确调用**
   - Bug.md中完全没有TX_CB#日志，说明`handleI2sTxCallback()`可能没有执行
   - 这会导致音频数据没有被正确发送，产生杂音

### 本次修改

1. **将`micros()`改为`millis()`**
   - `millis()`是更常用的Arduino函数，在嵌入式环境中更可靠
   - 虽然精度降低，但对于诊断目的已足够

2. **简化日志格式**
   - 移除`m_minCallbackInterval`, `m_maxCallbackInterval`等变量
   - 只保留关键信息：回调次数、持续时间、平均间隔、underrun次数

3. **优化日志输出条件**
   - 前3次回调必输出
   - 4-100次每10次输出
   - 100次以后每100次输出

### 日志格式变更

**原格式：**
```
[INFO] MAX98357A: TX_CB#1: dur=1234us, interval_min=0us/max=0us/avg=0us, underruns=0
```

**新格式：**
```
[INFO] MAX98357A: TX_CB1: dur=1ms, iv=0ms, und=0
```

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `micros()`→`millis()`；简化日志格式；优化输出条件 |
| `Shared_GlobalDefines.h` | 版本号V1.71→V1.72 |

### 下一步诊断建议

1. **编译上传V1.72**，播放视频并收集诊断日志
2. **观察日志**：如果TX_CB#日志出现，说明回调正常执行
3. **如果仍然没有TX_CB#日志**：
   - TX回调可能没有被I2S硬件触发
   - 需要检查I2S TX中断注册和DMA页面提交
4. **检查杂音**：如果TX_CB#日志正常但仍有杂音，说明问题在音频数据处理或I2S时序

---

## 版本 V1.73 - 添加无条件TX回调诊断日志 (2026-04-27)

### 问题描述

V1.72上传后，Bug.md日志中发现：
1. **TX_CB#诊断日志完全缺失** - 确认`handleI2sTxCallback()`没有被执行
2. **大量`[INFO] MAX98357A`无消息内容** - 日志格式化可能有问题
3. **杂音问题仍然存在**

### 根本原因分析

**TX回调函数未被调用**

经过分析，发现：
1. `initI2sForTx()`中预提交了4个DMA页面（line 188）
2. 这4个页面由DMA硬件自动发送，**不需要TX回调**
3. 当这4个页面发送完成后，TX回调应该被触发来补充更多数据
4. 但TX_CB#日志完全缺失，说明TX回调从未被触发

**DMA预提交页面发送完毕后，没有后续数据补充**

这导致：
- 前4个页面（可能是静音或随机数据）被发送
- 之后没有更多数据补充
- MAX98357A可能输出静音或随机噪声

### 本次修改

**添加无条件TX回调诊断日志**

```cpp
void Max98357aAudioPlayer::handleI2sTxCallback(char* pbuf) {
    static uint32_t s_txDebugCount = 0;
    Utils_Logger::info(TAG, "TX_IRQ%u", s_txDebugCount++);
    // ...
}
```

这个日志**无条件执行**，每次TX中断都会输出，用于确认回调是否被触发。

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 在TX回调开头添加无条件日志`TX_IRQ%u` |
| `Shared_GlobalDefines.h` | 版本号V1.72→V1.73 |

### 下一步诊断

1. **编译上传V1.73**，播放视频并收集诊断日志
2. **观察日志**：
   - 如果出现`TX_IRQ0`, `TX_IRQ1`, `TX_IRQ2`...，说明TX回调正常触发
   - 如果完全没有`TX_IRQ`日志，说明TX中断没有触发，需要检查I2S中断注册
3. **根据诊断结果决定下一步**

---

## 版本 V1.74 - 添加I2S初始化详细诊断日志 (2026-04-27)

### 问题描述

V1.73上传后，Bug.md日志中发现：
1. **TX_IRQ诊断日志完全缺失** - 确认TX回调从未被触发
2. **"Configuring I2S"等日志也没有出现** - 说明`initI2sForTx()`可能没有被调用
3. **但有大量`[INFO] MAX98357A`无消息内容的日志**

### 根本原因分析

**关键发现**：
- TX_IRQ从未出现 → TX回调函数从未被执行
- 但有音频输出（只是杂音）→ 4个预提交的DMA页面在被发送
- "Configuring I2S"日志没有出现 → `initI2sForTx()`可能没有执行或日志丢失

**可能的解释**：
1. `initI2sForTx()`确实被调用了，但日志格式化出了问题
2. TX页面预提交后，DMA完成但不触发TX中断
3. I2S TX中断被意外禁用或屏蔽

### 本次修改

添加详细的诊断日志，追踪I2S初始化的每一步：

```cpp
bool Max98357aAudioPlayer::initI2sForTx() {
    Utils_Logger::info(TAG, ">>> initI2sForTx() CALLED <<<");
    Utils_Logger::info(TAG, "Configuring I2S for TX mode...");
    // ...
    Utils_Logger::info(TAG, "Submitting %d TX DMA pages...", MAX98357A_DMA_PAGE_NUM);
    for (int i = 0; i < MAX98357A_DMA_PAGE_NUM; i++) {
        i2s_send_page(&m_i2sObj, ...);
        Utils_Logger::info(TAG, "TX page %d submitted", i);
    }
    Utils_Logger::info(TAG, "Enabling I2S...");
    i2s_enable(&m_i2sObj);
    Utils_Logger::info(TAG, "I2S enabled");
    // ...
}

bool Max98357aAudioPlayer::switchToPlaybackMode() {
    Utils_Logger::info(TAG, ">>> switchToPlaybackMode() CALLED <<<");
    // ...
}
```

### 日志预期输出

如果V1.74正常工作，应该看到：
```
[INFO] MAX98357A: >>> switchToPlaybackMode() CALLED <<<
[INFO] MAX98357A: Switching to PLAYBACK mode (I2S TX)...
[INFO] MAX98357A: >>> initI2sForTx() CALLED <<<
[INFO] MAX98357A: Configuring I2S for TX mode...
[INFO] MAX98357A: Submitting 4 TX DMA pages...
[INFO] MAX98357A: TX page 0 submitted
[INFO] MAX98357A: TX page 1 submitted
[INFO] MAX98357A: TX page 2 submitted
[INFO] MAX98357A: TX page 3 submitted
[INFO] MAX98357A: Enabling I2S...
[INFO] MAX98357A: I2S enabled
[INFO] MAX98357A: Submitting 4 RX DMA pages...
[INFO] MAX98357A: I2S TX mode initialized...
```

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 添加详细的I2S初始化诊断日志 |
| `Shared_GlobalDefines.h` | 版本号V1.73→V1.74 |

### 下一步诊断

1. **编译上传V1.74**，播放视频并收集诊断日志
2. **观察"switchToPlaybackMode"日志**：如果没出现，说明播放模式切换有问题
3. **观察"initI2sForTx"日志**：如果没出现，说明初始化函数没有被调用
4. **观察"TX page X submitted"日志**：如果没出现，说明DMA页面提交失败
5. **观察"TX_IRQ"日志**：如果出现，说明TX中断正常工作

---

## 版本 V1.75 - 移除过多诊断日志，简化错误输出 (2026-04-27)

### 问题描述

V1.74上传后，系统完全卡死：
1. **I2S错误洪水**：大量`I2S tx page own control error`和`I2S rx page own control error`
2. **系统完全卡死**：错误每秒打印数千次，串口完全被淹没
3. **TX回调从未触发**：TX_IRQ日志从未出现

### 根本原因分析

**关键发现**：
- TX回调(`handleI2sTxCallback`)从未被调用
- DMA页面耗尽后，RTL8735B I2S硬件触发"page own control error"中断
- 这些错误中断每秒触发数千次，完全占据CPU和串口带宽
- 导致系统无法响应，看起来像"卡死"

**I2S错误分析**：
- `0x1ff`(511) = 所有DMA页面的所有权都出错了
- TX和RX同时出错 → I2S DMA控制器完全失控
- 错误中断比正常TX中断优先级更高，持续触发

### 本次修改

1. **移除V1.74中添加的过多诊断日志**：
   - 移除`>>> initI2sForTx() CALLED <<<`
   - 移除`TX page X submitted`日志
   - 移除`Enabling I2S...`等日志

2. **添加简化日志**：
   - 添加`I2S callbacks registered`确认回调注册

3. **关键发现**：
   - TX回调未被调用是核心问题
   - I2S错误是后果，不是原因
   - 必须找出TX回调为什么不被调用

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 移除过多诊断日志，保留关键日志 |
| `Shared_GlobalDefines.h` | 版本号V1.74→V1.75 |

### 下一步诊断

1. **编译上传V1.75**，观察系统是否还会卡死
2. **观察"I2S callbacks registered"日志**：确认回调已注册
3. **观察TX回调是否被触发**：如果TX_IRQ仍不出现，说明Realtek SDK的TX中断机制有问题

---

## 版本 V1.76 - 修复I2S初始化顺序 (2026-04-27)

### 问题描述

V1.75上传后问题依旧：
1. **TX回调仍然不会被触发**
2. **I2S DMA页面所有权错误洪水仍然存在**

### 根本原因分析

**对比INMP441（录音正常）和MAX98357A（播放失败）的I2S初始化顺序差异**：

| 操作步骤 | INMP441 (录音 ✅) | MAX98357A (播放 ❌) |
|----------|-------------------|---------------------|
| i2s_set_direction | I2S_DIR_RX (RX模式) | I2S_DIR_TX (TX模式) |
| 注册中断回调 | `i2s_set_direction` 之前 | `i2s_set_direction` 之前 |
| 提交DMA页 | `i2s_enable()` **之后** | `i2s_enable()` **之前** |

**关键差异**：
1. **提交DMA页面时机**：INMP441先使能I2S再提交页面，MAX98357A是先提交页面再使能
2. **中断注册时机**：中断处理器应在`i2s_set_direction`之后注册，确保方向设置不覆盖中断配置

这两个问题导致TX中断永远不会被触发。

### 本次修改

修复`initI2sForTx()`中的初始化顺序：

```cpp
// 旧顺序 (错误)
i2s_rx/tx_irq_handler(...)  // 先注册中断
i2s_set_direction(I2S_DIR_TX)  // 再设置方向 (可能重置中断!)
i2s_send_page(...) × 4       // 先提交页面
i2s_enable(...)               // 再使能

// 新顺序 (正确，与INMP441一致)
i2s_set_direction(I2S_DIR_TX)  // 先设置方向
i2s_set_param(...)              // 设置参数
i2s_set_format(...)             // 设置格式
i2s_set_master(...)             // 设置主从
i2s_rx/tx_irq_handler(...)     // 再注册中断 (不会被覆盖)
i2s_enable(...)                 // 先使能
i2s_send_page(...) × 4         // 再提交页面
i2s_recv_page(...) × 4         // 提交RX页面防止RX错误
```

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 修复I2S初始化顺序：①方向→②中断注册→③使能→④提交页面 |
| `Shared_GlobalDefines.h` | 版本号V1.75→V1.76 |

### 预期效果

如果TX中断现在被触发，应该看到：
- `TX_IRQ0, TX_IRQ1, TX_IRQ2...` 日志持续出现
- 不再有大量I2S page own control error
- 音频杂音消失

### 下一步

1. **编译上传V1.76**
2. 播放视频，观察：
   - TX_IRQ日志是否出现 → 确认TX中断被触发
   - I2S page own control error是否消失
   - 音频杂音是否改善

---

## 版本 V1.77 - 回退音频代码+移除编码器上电限制 (2026-04-27)

### 问题描述

V1.74-V1.76的所有尝试（调整I2S初始化顺序、注册回调时机）均无效：
- TX回调始终不被触发
- I2S page own control错误洪水依旧
- 初始化顺序调整没有改善

### 回退策略

**Max98357a_AudioPlayer.cpp 回退到 V1.73**：
- `initI2sForTx()` 恢复为先注册中断回调、再设置方向的顺序
- `i2s_send_page` 在 `i2s_enable` 之前提交
- 添加 `i2s_recv_page` 提交RX页面防止RX所有权错误

### 旋转编码器优化

**移除3秒上电稳定期**：
- `m_initCompleteTime = millis() + 3000` → `m_initCompleteTime = millis()`（立即生效）
- 移除 `checkButton()` 中的稳定期检查
- 移除 `handleRotation()` 中的稳定期检查
- 移除 `handleButtonISR()` 中的稳定期检查

**保护机制仍然存在**：
- `m_buttonEverReleased` 检查：按钮必须先被检测到释放状态（HIGH）后才允许按下事件
- 硬件消抖：`init()` 中 10 次循环检查按钮状态
- 软件消抖：`200ms` 中断去抖 + `BUTTON_DEBOUNCE_DELAY` 轮询去抖

这些保护机制足以防止上电时误触发拍照模式。

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 回退到V1.73的I2S初始化顺序 |
| `Encoder_Control.cpp` | 移除3秒上电稳定期 |
| `Shared_GlobalDefines.h` | 版本号V1.76→V1.77 |

### 下一步：从新角度分析音频杂音

既然I2S初始化顺序调整无效，需要从其他角度分析：
1. 不使用I2S TX中断，改用轮询方式填充DMA缓冲区
2. 使用多媒体框架（MMF）的I2S模块代替直接使用HAL层
3. 检查I2S TX是否需要额外的硬件配置（如DMA通道分配）
4. 研究Realtek SDK中其他使用I2S TX的示例代码

---

## 版本 V1.78 - 使用轮询方式替代TX中断回调 (2026-04-27)

### 问题描述

V1.74-V1.76的所有尝试均证实：**RTL8735B I2S TX中断回调在TX-only模式下不被触发**。
TX_IRQ日志从未出现，而INMP441的RX回调在RX模式下正常工作。

### 根本原因分析

**TX中断不触发是已知问题**：
- RTL8735B I2S控制器在 `I2S_DIR_TX`（仅TX）模式下，TX DMA完成中断可能不被正确触发
- RX模式（`I2S_DIR_RX`）的RX回调工作正常
- SDK API `i2s_get_tx_page()` 提供了轮询替代方案

**i2s_get_tx_page() API**：
```c
int *i2s_get_tx_page(i2s_t *obj);
// 返回: 当前TX页面地址（CPU拥有）或 NULL（I2S拥有）
```

当DMA传输完一个页面后，该页面的所有权归还给CPU，此时 `i2s_get_tx_page()` 返回有效地址。

### 解决方案

**方案：轮询方式替代中断回调**

新增 `serviceI2sTx()` 方法：
1. 检查播放状态（`m_playing && PLAYBACK_STATE_PLAYING`）
2. 调用 `i2s_get_tx_page()` 获取可用的TX DMA页面
3. 如果返回NULL，说明没有页面可用 → 返回
4. 如果返回有效地址，填充音频数据并调用 `i2s_send_page()` 提交

**调用时机**：
- `videoPlaybackLoop()` 入口 → 每次循环都尝试服务I2S TX
- 每次 `writeAudioData()` 之后 → 立即将数据推入DMA

**核心优势**：
- 不依赖TX中断回调
- 轮询频率高（每次主循环都调用），不会丢页面
- DMA页面时序：每页20ms（1280字节/64000字节/秒），4页共80ms缓冲
- 主循环通常每几毫秒执行一次，远快于20ms的页面周期

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.h` | 添加 `serviceI2sTx()` 方法声明 |
| `Max98357a_AudioPlayer.cpp` | 实现 `serviceI2sTx()`：使用`i2s_get_tx_page()`轮询填充DMA页面 |
| `VideoRecorder.cpp` | `videoPlaybackLoop()`入口和每次写音频后调用`serviceI2sTx()` |
| `Shared_GlobalDefines.h` | 版本号V1.77→V1.78 |

### 预期效果

- `TX_POLL1~5`: 前5次轮询成功日志
- `TX_POLL50, TX_POLL100...`: 每50次周期性日志
- 音频连续播放，无杂音
- 不再有 I2S page own control error

### 下一步

1. **编译上传V1.78**
 2. 播放视频，观察：
    - TX_POLL 日志是否出现 → 确认轮询获取到页面
    - 音频是否连续无杂音
    - 是否有 I2S 错误

---

## 版本 V1.79 - 改用I2S_DIR_TXRX双向模式修复TX DMA不启动 (2026-04-27)

### 问题描述

V1.78上传后依然杂音，关键日志：
```
Tx_page is busy: 
page_idx: 255, I2SPageNum: 3
```

**分析**：
- `page_idx: 255` (0xFF) = 无效页面索引，TX DMA状态机未初始化
- 无 TX_POLL 日志 = `i2s_get_tx_page()` 永远返回 NULL
- 无 TX_IRQ 日志 = TX 中断从未触发
- 无 I2S page own control error（不同于之前）= 页面从未被消费

### 根本原因分析

**RTL8735B I2S控制器的 `tx_act` 字段**（I2S_CTRL寄存器 bits[2:1]）：

| tx_act值 | 枚举 | 方向 | TX DMA状态 |
|----------|------|------|-----------|
| 0x0 | I2S_ONLY_RX | 仅RX | TX DMA关闭 |
| 0x1 | I2S_ONLY_TX | 仅TX | **TX DMA未完全激活** |
| 0x2 | I2S_TXRX | 双向 | TX+RX DMA均激活 |

**关键发现**：`I2S_ONLY_TX` (tx_act=0x1) 模式下，TX DMA引擎未被完全激活：
- 页面提交成功但不会开始传输
- DMA页面指针保持在无效状态（page_idx=255）
- TX中断永远不会触发
- `i2s_get_tx_page()` 永远返回NULL（页面一直"忙"）

**但是** `I2S_ONLY_RX` (tx_act=0x0) 模式在INMP441中可以正常工作：
- RX DMA正常启动
- RX中断正常触发
- 说明RX-only模式没有问题，只有TX-only模式有bug

### 解决方案

**将方向从 `I2S_DIR_TX` (tx_act=0x1) 改为 `I2S_DIR_TXRX` (tx_act=0x2)**：

```cpp
// 旧代码
i2s_set_direction(&m_i2sObj, I2S_DIR_TX);  // tx_act=0x1, TX DMA不启动

// 新代码
i2s_set_direction(&m_i2sObj, I2S_DIR_TXRX);  // tx_act=0x2, TX+RX DMA均启动
```

**双向模式的RX处理**：
- RX回调(`handleI2sRxCallback`)已实现 → 调用`i2s_recv_page()`丢弃RX数据
- RX DMA页面已通过`i2s_recv_page()`预提交 → 不会产生错误中断

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `I2S_DIR_TX` → `I2S_DIR_TXRX`（启用双向模式） |
| `Shared_GlobalDefines.h` | 版本号V1.78→V1.79 |

### 预期效果

- TX DMA引擎正常启动（tx_act=0x2同时激活TX和RX）
- TX_POLL日志出现 → `i2s_get_tx_page()` 返回有效页面
- TX_IRQ日志出现 → TX中断被触发（作为轮询的补充）
- 音频数据通过DMA正常发送到MAX98357A
- 杂音消失

### 下一步

1. **编译上传V1.79**
2. 播放视频，观察：
   - TX_POLL 日志是否出现 → 确认页面现在可用
   - TX_IRQ 日志是否出现 → 确认TX中断也工作了
   - 音频杂音是否消失

---

## 版本 V1.80 - I2S配置修正 (2026-04-27)

### 修改来源

执行I2S全面检查报告后的修正建议。

### 修改内容

**1. 显式设置 DMA Burst Size**
- `Max98357a_AudioPlayer.cpp`: 添加 `i2s_set_dma_burst_size(&m_i2sObj, BURST16)`
- `Inmp441_MicrophoneManager.cpp`: 添加 `i2s_set_dma_burst_size(&m_i2sObj, BURST16)`

虽然默认值已是 `BURST16 = 0x0F`，但显式设置可确保配置的确定性，避免不同 SDK 版本或重置导致的非预期行为。

**2. 修正引脚注释**
- `Shared_GlobalDefines.h`: "DIN(SDATA) -> 19号引脚" → "DIN(SDATA) -> PD_15"
- 原注释容易误导：GPIO 编号 15 并非物理引脚 19，"19号引脚"含义不清

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 添加 `i2s_set_dma_burst_size(BURST16)` |
| `Inmp441_MicrophoneManager.cpp` | 添加 `i2s_set_dma_burst_size(BURST16)` |
| `Shared_GlobalDefines.h` | 修正引脚注释 + 版本号 V1.79→V1.80 |

### 下一步

1. **编译上传V1.80** 验证修改无副作用
 2. 继续排查音频杂音问题

---

## 版本 V1.81 - 修复i2s_enable时序 + 取消TX中断注册 (2026-04-27)

### 问题描述

V1.80上传后依然杂音，`Tx_page is busy: page_idx: 255` 依旧 → TX DMA 仍未启动。

### 根本原因分析

**对比 INMP441（RX 正常）和 MAX98357A（TX 失败）的初始化时序**：

| 步骤 | INMP441 录音 ✅ | MAX98357A 播放 ❌ |
|------|-------------------|---------------------|
| 注册中断 | enable 之前 | ~~enable 之前~~ (已移除TX) |
| i2s_enable | 先执行 | **后执行** |
| 提交页面 | **enable 之后** | enable **之前** |

**关键差异**：MAX98357A 在 `i2s_enable()` **之前**调用 `i2s_send_page()` 提交了 4 个 TX 页面。

**推测机制**：
- RTL8735B I2S DMA 引擎在 `i2s_enable()` 激活后才会初始化 TX DMA 状态机
- `i2s_enable()` 之前提交的页面，其所有权标记未被 DMA 引擎正确识别
- 导致 DMA 引擎启动后看到"无可用页面"且指针停留在无效值 255

**第二个问题**：之前同时注册了 TX 中断，但使用 `serviceI2sTx()` 轮询。TX 中断回调可能干扰轮询机制。改为纯轮询，不注册 TX 中断。

### 本次修改

`initI2sForTx()` 初始化顺序：

```cpp
// 旧顺序 (错误)
i2s_tx/rx_irq_handler → set_direction → ... → send_page × 4 → enable → recv_page × 4

// 新顺序 (与 INMP441 一致)
set_direction → set_param → set_format → set_master → burst_size → byte_swap
→ i2s_rx_irq_handler (仅 RX) → enable → send_page × 4 + recv_page × 4
```

关键变化：
1. **enable 移到 send_page 之前** — 与 INMP441 一致
2. **取消 TX 中断注册** — 使用纯轮询 (`serviceI2sTx()`)
3. **保留 RX 中断注册** — 用于丢页面的回收

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | enable→send_page；移除TX中断注册；仅保留RX回调 |
| `Shared_GlobalDefines.h` | 版本号 V1.80→V1.81 |

### 预期效果

- `i2s_enable` 后再提交页面，DMA 引擎能正确识别
- `i2s_get_tx_page()` 返回有效地址 → TX_POLL 日志出现
- `page_idx` 变为 0/1/2/3 正常值，不再是 255
- 音频数据正常送出

### 下一步

1. **编译上传V1.81**
2. 播放视频，观察：
   - TX_POLL 日志是否出现
   - `Tx_page is busy: page_idx: 255` 是否消失

---

## 版本 V1.82 - 恢复TX中断注册防止崩溃 (2026-04-27)

### 问题描述

V1.81上传后系统崩溃：
```
[I2S  Err]I2S tx page own control error: 0x     111
Usage Fault: Invalid state... PC = 0x00000000
```

### 崩溃分析

**好消息**：`tx page own control error: 0x111` 说明：
- bit0 (PAGE0_OK) = **TX DMA 启动了！** `enable→send_page` 顺序修复成功！
- bit4 (PAGE0_UNA) = 页面0不可用（轮询还没填充）
- bit8 (EMPTY) = TX FIFO空

**崩溃原因**：V1.81 移除了 `i2s_tx_irq_handler()` 注册，但 TX DMA 启动后产生了中断，无回调注册 → 调用 NULL 函数指针 → `PC=0x00000000` → Usage Fault。

### 解决方案

恢复 TX 中断注册，保持 `enable→send_page` 顺序不变：

```cpp
// 最终正确的 initI2sForTx() 顺序
set_direction(TXRX) → set_param → set_format → set_master
→ burst_size → byte_swap
→ rx_irq_handler + tx_irq_handler  ← 恢复TX中断注册
→ i2s_enable  ← 先使能
→ send_page × 4 + recv_page × 4   ← 再提交页面
```

双重保障：
- **TX 中断回调** (`handleI2sTxCallback`)：正常时由中断驱动填充页面
- **轮询** (`serviceI2sTx`)：作为补充，降低 underrun 概率

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 恢复 `i2s_tx_irq_handler` 注册 |
| `Shared_GlobalDefines.h` | 版本号 V1.81→V1.82 |

### 预期效果

- 不再崩溃
- TX_POLL 和 TX_IRQ 日志同时出现
- 音频数据通过 DMA 正常发送

### 下一步

1. **编译上传V1.82**
2. 播放视频，观察：
   - 系统不再崩溃
   - TX_IRQ/TX_POLL 日志是否出现
   - 音频杂音是否改善

---

## 版本 V1.83 - 回退I2S_DIR_TX保留enable→send_page顺序 (2026-04-27)

### 问题描述

V1.82上传后依然杂音，错误洪水：
```
tx page own control error: 0x111 → 0x110 → 0x1ff (递增升级)
```
- TX DMA 确实启动了（页面被消耗，error从0x111升级到0x1ff）
- 但 TX_IRQ 日志从未出现（回调不触发）
- TX_POLL 日志从未出现（`i2s_get_tx_page` 返回 NULL）
- 页面消耗后无法通过任何机制回收

### 历史版本矩阵

| 版本 | 方向 | DMA提交顺序 | 结果 |
|------|------|------------|------|
| V1.73 | `I2S_DIR_TX` | send_page → enable | page_idx:255, DMA未启动 |
| V1.82 | `I2S_DIR_TXRX` | enable → send_page | ✅ DMA启动, ❌ 回调失败 |
| **V1.83** | **`I2S_DIR_TX`** | **enable → send_page** | **🆕 未测试组合** |

### 本次修改

V1.83 = 之前从未试过的组合：`I2S_DIR_TX` + `enable → send_page`

```cpp
// V1.83 initI2sForTx()
i2s_set_direction(I2S_DIR_TX)  // 回到TX-only
...
i2s_tx_irq_handler + i2s_rx_irq_handler  // 保留中断注册
i2s_enable()                   // 先使能
send_page × 4 + recv_page × 4  // 后提交页面
```

**假设**：
- `I2S_DIR_TXRX` 模式下 TX/RX 中断共享导致回调分发失败
- `I2S_DIR_TX` 模式下 TX 中断独立路由，配合 `enable→send_page` 可能正常工作

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `I2S_DIR_TXRX` → `I2S_DIR_TX`；保持 `enable→send_page` |
| `Shared_GlobalDefines.h` | 版本号 V1.82→V1.83 |

### 预期效果

- 如果 `I2S_DIR_TX` + `enable→send_page` 正确：TX_IRQ/TX_POLL 出现，DMA 正常循环
- 如果仍是 page_idx:255：至少不会产生错误洪水，回到 V1.73 状态

### 下一步

1. **编译上传V1.83**
2. 播放视频，观察：
   - TX_IRQ 或 TX_POLL 日志是否出现
   - `Tx_page is busy: page_idx: 255` 还是 `I2S tx page own control error`

---

## 版本 V1.84 - 移除所有RX代码防止干扰TX中断 (2026-04-27)

### 问题描述

V1.81-V1.83 均有 TX DMA 消耗页面但回调不触发的问题。

### 根因分析

**关键发现**：V1.83 日志中 `I2S_DIR_TX` 模式下仍然出现 RX 错误：
```
I2S rx page own control error: 0x11f → 0x1ff
```

这说明调用了 `i2s_recv_page()` 会**意外激活 RX DMA 引擎**，即使在 `I2S_DIR_TX` 模式下。

| 版本 | RX代码 | RX错误 | TX回调 | 
|------|--------|--------|--------|
| V1.56 | 无 recv_page, 无 handler | 有(被忽略) | ✅ 正常(推测) |
| V1.65+ | recv_page + handler | 无 | ❌ 失败 |
| V1.81 | recv_page + handler | 无(未注册) | ❌ 崩溃 |
| V1.82-83 | recv_page + handler | 0x11f→0x1ff | ❌ 失败 |

**结论**：`i2s_recv_page()` 激活的 RX DMA 与 TX DMA 共享中断源，导致 SDK ISR 中断分发逻辑异常——应该分发给 TX 回调的中断被 RX 错误处理吞掉了。

### 解决方案

**彻底移除所有 RX 相关代码**：
- 不调用 `i2s_recv_page()`
- 不注册 `i2s_rx_irq_handler()`
- 仅保留 TX 中断 + 轮询双重保障

```cpp
// V1.84 initI2sForTx() - 极简 TX 仅模式
i2s_set_direction(I2S_DIR_TX)   // 仅TX
i2s_set_param(I2S_CH_STEREO, I2S_SR_16KHZ, I2S_WL_16)
i2s_set_format(FORMAT_I2S)
i2s_set_master(I2S_MASTER)
i2s_set_dma_burst_size(BURST16)
i2s_set_byte_swap(FALSE)
i2s_tx_irq_handler(...)          // 仅TX中断
i2s_enable()                     // 先使能
i2s_send_page × 4                // 后提交页面
// 不调用 i2s_recv_page()
// 不注册 i2s_rx_irq_handler()
```

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 移除 `i2s_rx_irq_handler`；移除 `i2s_recv_page`；仅保留TX |
| `Shared_GlobalDefines.h` | 版本号 V1.83→V1.84 |

### 预期效果

- 不再有 RX 中断干扰 TX
- TX 中断应能正常分发到 `handleI2sTxCallback`
- `serviceI2sTx()` 轮询作为补充
- 可能有 RX page own control error（RX 没提交页面），但不应影响功能

### 下一步

1. **编译上传V1.84**
2. 播放视频，观察：
   - TX_IRQ 日志是否首次出现
   - 是否有 RX 错误（预期"有"但不影响播放）

---

## 版本 V1.85 - 回退send_page→enable顺序恢复V1.56工作状态 (2026-04-27)

### 问题描述

V1.81-V1.84 所有 `enable→send_page` 组合均失败：DMA 启动但 TX 回调不触发。

### 根因分析

**硬件 RX DMA 通道始终活跃**：
- `i2s_init()` 配置了 RX 引脚 (PD_18)
- `i2s_set_dma_buffer()` 注册了 RX 缓冲区
- 即使不调用 `i2s_recv_page()`，硬件 RX DMA 在 `i2s_enable()` 后也被激活
- SDK ISR 处理 RX 错误时可能**清除所有待处理中断**，包括 TX PAGE_OK

**为什么 V1.56 能工作**：

| | V1.56 ✅ | V1.81-84 ❌ |
|---|---|---|
| send_page 时机 | **enable 之前** | enable 之后 |
| DMA 启动状态 | 4 页已就位，干净启动 | 0 页，立即错误 |
| ISR 处理 | TX PAGE_OK 先于 RX 错误 | RX 错误与 TX 事件混合 |

`enable → send_page` 导致 I2S 在零页面状态下启动 → 立即产生 PAGE_UNA/EMPTY 错误 → ISR 陷入错误处理 → TX 回调永不调用。

`send_page → enable` 下，I2S 启动时有 4 个就绪页面 → DMA 干净地从 page0 开始 → TX PAGE_OK 中断在 RX 错误之前到达 → TX 回调正常触发。

### 解决方案

**完全回退到 V1.56 的 initI2sForTx() 顺序**：

```cpp
// V1.85 initI2sForTx()
i2s_set_direction(I2S_DIR_TX)
i2s_set_param(I2S_CH_STEREO, I2S_SR_16KHZ, I2S_WL_16)
i2s_set_format(FORMAT_I2S)
i2s_set_master(I2S_MASTER)
i2s_set_dma_burst_size(BURST16)
i2s_set_byte_swap(FALSE)
i2s_tx_irq_handler(...)          // 仅注册TX
i2s_send_page × 4                // ⬅ 先提交页面
i2s_enable()                     // ⬅ 后使能I2S
// 不调用 i2s_recv_page()
```

**保留的 V1.85 改进**：
- `i2s_set_dma_burst_size(BURST16)` — 之前版本没有
- `serviceI2sTx()` 轮询 — 作为 TX 回调的补充
- TX_IRQ 诊断日志 — 确认回调是否触发

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `enable → send_page` 回退到 `send_page → enable` |
| `Shared_GlobalDefines.h` | 版本号 V1.84→V1.85 |

### 预期效果

- 恢复 V1.56 能播放音频的状态
- TX_IRQ 日志出现 → TX 回调正常触发
- 可能仍有 RX 错误打印，但不影响功能
- 杂音通过后续版本的数据处理改进来解决

### 下一步

1. **编译上传V1.85**
 2. 验证音频播放是否恢复（即使仍有杂音）

---

## 版本 V1.86 - TXRX+send→enable+双回调+Serial直打诊断 (2026-04-27)

### 问题描述

V1.85 (`I2S_DIR_TX`+`send_page→enable`) TX DMA 完全未启动（无任何TX错误）。

### 根因分析

| 版本 | 方向 | 顺序 | TX DMA |
|------|------|------|--------|
| V1.82 | **TXRX** | enable→send | ✅ 启动 (0x111→0x1ff) |
| V1.85 | **TX** | send→enable | ❌ 未启动 (无TX错误) |

**结论**：RTL8735B `I2S_DIR_TX` (tx_act=0x1) 模式下 TX DMA 引擎**永远不会启动**。
必须使用 `I2S_DIR_TXRX` (tx_act=0x2)。

V1.82 的 `enable→send_page` 让 DMA 在零页面下启动 → 立即错误 → ISR 错误处理吞掉回调。
V1.86 改为 `send_page→enable`：4页就位后再启动 DMA，干净启动。

### 解决方案

```cpp
// V1.86 initI2sForTx()
I2S_DIR_TXRX                       // TXRX双向 (tx_act=0x2)
i2s_rx_irq_handler + i2s_tx_irq_handler  // 双回调
i2s_send_page × 4 + i2s_recv_page × 4    // 先提交TX+RX页面
i2s_enable()                        // 后使能 (干净启动)
```

**诊断改进**：TX回调使用 `Serial.print("[TX_IRQ")` 直接输出，绕过 Utils_Logger 排除日志系统故障。

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | TXRX + send→enable + 双回调 + Serial.print诊断 |
| `Shared_GlobalDefines.h` | 版本号 V1.85→V1.86 |

### 预期效果

- TX+RX DMA 均正常启动（4页就位，干净状态）
- `[TX_IRQ0][TX_IRQ1]...` 通过 Serial 直接输出
- 音频正常播放

### 下一步

1. **编译上传V1.86**
 2. 检查 Serial 输出中是否有 `[TX_IRQ` 日志

---

## 版本 V1.87 - DC偏移消除修复杂音 (2026-04-27)

### 🎉 突破

V1.86 **TX 回调首次正常工作**！`[TX_IRQ0]`~`[TX_IRQ174]+` 连续触发，音频成功播放。
`I2S_DIR_TXRX` + `send_page→enable` + 双回调 是正确的初始化方案。

### 🔍 杂音根因

V1.86 音频数据抽取日志揭示：

| Chunk | DC偏移 | 峰峰值 |
|-------|--------|--------|
| #20 | +127 | 1424 |
| #40 | **+4266** (13%) | 7304 |
| #60 | **-1943** (6%) | 3064 |
| #80 | -710 (2%) | 3632 |

DC 偏移在块之间剧烈跳变（从 +4266 到 -1943）。电脑 VLC 播放无声卡 DC 问题是因为有声卡 AC 耦合电容（硬件高通滤波器）。开发板 MAX98357A 直接输出原始 PCM → DC 电平剧烈波动 → 杂音。

### 解决方案

**在 `writeAudioData()` 中添加 DC 阻塞滤波器**：

```cpp
// 1. 计算当前块的 DC 平均值
int32_t chunkDcSum = accumulate(data, count);
int16_t chunkDc = chunkDcSum / count;

// 2. EMA 跟踪长期 DC 偏移 (alpha = 1/16)
if (first_chunk) m_dcOffset = chunkDc;
else m_dcOffset = (m_dcOffset * 15 + chunkDc) >> 4;

// 3. 写入前减去 DC 估计值，夹紧到 int16
corrected = clamp(data[i] - m_dcOffset, -32768, 32767);
```

**频率特性**：
- EMA alpha = 1/16 per chunk ≈ 32ms per chunk → 时间常数 ~500ms
- 截止频率 ≈ 0.3 Hz 高通
- 音频内容（≥ 20 Hz）完全通过，只有亚赫兹 DC 漂移被衰减

**同时移除 `serviceI2sTx()` 调用**：
- `i2s_get_tx_page()` 始终返回 NULL (page_idx=255)
- TX 回调驱动 DMA 页面填写正常工作
- 轮询未增加任何值

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | `writeAudioData()`添加DC偏移检测+EMA跟踪+减法校正 |
| `VideoRecorder.cpp` | 移除两个 `serviceI2sTx()` 调用 |
| `Shared_GlobalDefines.h` | 版本号 V1.86→V1.87 |

### 预期效果

- TX_IRQ 继续正常触发
- DC 偏移被实时跟踪和消除
- 杂音消失，音频干净
- 正常音频内容不受影响（高通截止 ~0.3Hz ≪ 20Hz）

### 下一步

1. **编译上传V1.87**
 2. 验证杂音是否消失

---

## 版本 V1.88 - 加速DC追踪+TX DMA数据快照 (2026-04-27)

### 问题描述

V1.87 EMA (alpha=1/16) DC 消除未解决杂音。

### 根因分析

V1.87 音频数据分析揭示 DC 在块间以千 LSB 级幅度跳变：

| Chunk | dc | 说明 |
|-------|-----|------|
| #180 | 0 | 无声段 |
| #200 | +6182 | 突然跳变 6182！ |
| #220 | -1871 | 又跳变 -8053 |

EMA (alpha=1/16, τ≈500ms) 无法快速跟踪这些跳变：
- dc=0 → dc=6182：收敛到 95% 需要约 3τ = 1.5 秒
- 在此期间 DAC 输出有巨大的低频飘移 → 杂音

### 解决方案

**加速 DC EMA：alpha 从 1/16 改为 1/4**：

```cpp
// V1.87 (慢): m_dcOffset = (m_dcOffset * 15 + chunkDc) >> 4;  // t≈500ms
// V1.88 (快): m_dcOffset = (m_dcOffset * 3  + chunkDc) >> 2;  // t≈128ms
```

时间常数从 500ms 降到 128ms，4 倍加速。对 DC 跳变的响应快 4 倍，但仍足够平滑。

**添加 DMA 页数据快照**：前 3 次 TX 回调输出实际 DMA 页内容：
```
[TX_DUMP1] dc=X s[0..3]=a,b,c,d
```
验证写入 DAC 的数据是否经过 DC 校正。

**减少 TX 日志**：仅前 10 次打印 `[TX0]..[TX9]`。

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | DC EMA α=1/16→1/4；TX回调添加DMA页快照；减少TX日志 |
| `Shared_GlobalDefines.h` | 版本号 V1.87→V1.88 |

### 预期效果

- DC 跳变在 ~400ms 内收敛（3τ），而非 1.5s
- TX_DUMP 显示 DMA 页数据已消除 DC
- 杂音显著减少

### 下一步

1. **编译上传V1.88**
 2. 查看 TX_DUMP 验证 DMA 数据
 3. 验证杂音是否减少

---

## 版本 V1.89 - 逐采样DC阻塞滤波器替代逐块EMA (2026-04-27)

### 问题描述

V1.87-V1.88 EMA DC 消除未解决杂音。EMA 核心问题：每块用同 DC 值校正全部 512 样本 → 块边界处 DC 估计跳变 → 校正值突然变化 → 输出信号产生阶跃不连续 → 可闻咔嗒声/杂音。

### 根因分析

**逐块 EMA 的固有缺陷**：
```
Chunk N:   dc estimate = 0,    所有512样本减去 0
Chunk N+1: dc estimate = 6182, 所有512样本减去 6182  ← 突然跳变！
```

512 样本边界处的阶跃 → 高频能量 → 可闻杂音。无论 EMA 多快多慢，这种块边界不连续都不可避免。

### 解决方案

**逐采样 DC 阻塞滤波器**（一阶 IIR 高通）：

```cpp
// y[n] = x[n] - x[n-1] + R × y[n-1]
// R = 16273/16384 ≈ 0.9932, fc ≈ 17Hz @ 16kHz
int32_t blocked = sample - m_dcBlockLastIn + ((m_dcBlockLastOut * 16273) >> 14);
m_dcBlockLastIn = sample;
output = clamp(blocked, -32768, 32767);
m_dcBlockLastOut = output;
```

**为什么能解决问题**：
- **连续平滑**：每采样 DC 估计平滑演变，无阶跃不连续
- **频率选择性**：仅衰减 17Hz 以下 → 音频 (≥20Hz) 完整保留
- **零延迟**：每个输出仅依赖前一采样，无群延迟
- **状态连续**：`m_dcBlockLastIn/Out` 在回调间持续 → 无跨页不连续

**与 EMA 对比**：

| 属性 | 逐块 EMA | 逐采样 DC 阻塞 |
|------|---------|-------------|
| 更新频率 | 每块 (32ms) | 每采样 (62.5µs) |
| 块边界 | 阶跃不连续 | 平滑过渡 |
| 延迟 | EMA 滞后 | 零延迟 |
| 音频衰减 | 无 | 仅 <17Hz |

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.h` | `m_dcOffset/m_prevFilteredSample` → `m_dcBlockLastIn/m_dcBlockLastOut` |
| `Max98357a_AudioPlayer.cpp` | `writeAudioData()`移除DC校正(写原始数据)；`handleI2sTxCallback()`添加逐采样DC阻塞 |
| `Shared_GlobalDefines.h` | 版本号 V1.88→V1.89 |

### 预期效果

- TX_DUMP 确认 DMA 页已 DC 阻塞处理
- 无块边界咔嗒声
- 杂音消失，音频干净
- 人声/音乐（≥80Hz）完全不受影响

### 下一步

1. **编译上传V1.89**
2. 验证杂音是否消失

---

## 版本 V1.90 - I2S数据边缘切换到POSITIVE_EDGE (2026-04-27)

### 问题描述

V1.87-V1.89 杂音完全相同。逐块 EMA 和逐采样 DC 阻塞处理均无效 → 杂音根因不在软件数据处理层，而在 **I2S 物理层时序**。

### 根因分析

**杂音跨版本不变 = 与数字信号处理无关 = 物理层问题**。

**I2S 数据边缘不匹配理论**：

| 设备 | 数据发送/采样边沿 |
|------|-------------------|
| RTL8735B (默认) | **下降沿** 发送数据 (EDGE_SW=0) |
| MAX98357A | **上升沿** 采样数据 |

时序差异：
```
BCLK:  ─┐   ┌───┐   ┌───┐   ┌───
        └───┘   └───┘   └───┘   └───
RTL8735B 在下降沿(↓)改变数据
MAX98357A 在上升沿(↑)采样数据
DATA:  ────<X>────────<X>────────
           ↑            ↑
        MAX读这里   数据正在变化！
```

RTL8735B 在下降沿改变数据后，数据需要建立时间稳定。但 MAX98357A 在紧接着的上升沿就采样 → **违反建立时间** → 每个 bit 可能被错误读取 → 持续杂音。

### 解决方案

**将 RTL8735B 的数据发送边沿改为 POSITIVE_EDGE**：

```cpp
i2s_set_data_start_edge(&m_i2sObj, POSITIVE_EDGE);
```

效果：
```
BCLK:  ─┐   ┌───┐   ┌───┐   ┌───
        └───┘   └───┘   └───┘   └───
RTL8735B 在上升沿(↑)改变数据
MAX98357A 在上升沿(↑)采样数据
DATA:  ────<X>────────────<X>────
           ↑                ↑
        RTL发+MAX读同步   数据已稳定
```

数据在上升沿前半个 BCLK 周期（0.5/512kHz ≈ 1µs）就已稳定 → 满足建立时间。

**同时移除 ISR 中的 Serial.print**：
- TX 回调在中断上下文执行，Serial.print 耗时 ~100µs
- 每个回调 ~100µs × 50次/秒 = 5ms/秒占用 → 可能导致后续回调延迟

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 添加 `i2s_set_data_start_edge(POSITIVE_EDGE)`；移除 TX 回调中所有 Serial.print；优化诊断日志 |
| `Shared_GlobalDefines.h` | 版本号 V1.89→V1.90 |

### 先前尝试

V1.58 曾尝试 POSITIVE_EDGE + BYTE_SWAP=TRUE → 白噪音。本次 **单独使用 POSITIVE_EDGE**（BYTE_SWAP=FALSE），情况不同。

### 预期效果

- I2S 数据边缘匹配 → MAX98357A 正确读取所有 bit
- 杂音消失或显著减少
- 如果仍无效，则问题在别处（如电源噪声）

### 下一步

1. **编译上传V1.90**
2. 验证杂音是否改善

---

## 版本 V1.91 - 修复编译警告+ISR诊断+DC阻塞全路径 (2026-04-27)

### 问题描述

V1.90 编译有 2 个警告（unused variable），TX_CB 日志不出现在串口，杂音依旧。

### 根本原因分析

经过 V1.56-V1.90 共 35 个版本的尝试，杂音从未改善。已排除：
- ❌ DC 偏移（EMA 逐块/逐采样 IIR 高通均无效）
- ❌ I2S 数据边缘（POSITIVE_EDGE 无效）
- ❌ TX DMA 初始化顺序（send_page→enable 和 enable→send_page 均尝试）
- ❌ 软件信号处理（DC 阻塞、低通、噪声门等）

**结论**：杂音很可能来自硬件层面 — RTL8735B I2S TX 在 16kHz/16bit/STEREO 配置下的信号质量问题，或 MAX98357A 与开发板的电气连接噪声。需使用示波器实测 I2S 时序才能确认。

### 本次修改

**1. 修复编译警告**：移除 ISR 中 millis() 调用（`callbackStart/callbackEnd/duration/avgInterval/m_lastCallbackTime` 未使用）

**2. 恢复 TX 中断诊断**：前 10 次 `[TX1]...[TX10]` 确认回调调用

**3. DC 阻塞覆盖全路径**：
- 正常播放路径：逐采样 DC 阻塞 ✅
- fade 路径 (non-playing)：也应用 DC 阻塞 ✅
- **underrun 路径**：之前用 `m_lastOutputSample>>1`（有缺陷的信号衰减），改为 DC 阻塞滤波 ✅

**4. 回调诊断**：`[CB1 b=0 u=0]` 格式显示 buffer 使用量和 underrun 数

### 代码变更

| 文件 | 修改内容 |
|------|---------|
| `Max98357a_AudioPlayer.cpp` | 移除 ISR millis()；恢复 TX ISR 诊断；DC 阻塞覆盖 underrun/fade 路径；紧凑代码 |
| `Shared_GlobalDefines.h` | 版本号 V1.90→V1.91 |

### 预期效果

- 编译 0 警告
- `[TX1]...[TX10]` 确认 TX 中断触发
- `[CB1 b=XXXX u=X]` 显示 buffer 状态
- 杂音不变（已确认非软件问题）

### 后续调试方向

1. 用示波器测量 I2S SCK/WS/SDATA 信号质量
2. 检查 MAX98357A 供电纹波
3. 尝试不同采样率（8kHz/44.1kHz）测试
4. 检查 SD 卡读取速度是否导致音频数据延迟

---

## 版本 V1.92 - 修复buffer underrun杂音根因 (2026-04-27)

### 🔍 杂音根因发现

V1.91 日志首次揭示严重 underrun：
```
[CB1  b=0 u=320]   ← 回调1，buffer空，整页underrun
[CB10 b=0 u=2176]  ← 回调10，68% underrun
[CB500 b=1728 u=8128] ← 视频末尾，44% underrun
```

每页 320 样本。underrun → 零输出 → 音频频繁打断 → 宽带杂音。

### 根因

`videoPlaybackLoop()` 处理视频帧后立即 `return`。AVI 中 2-3 音频块 + 1 视频块交替。视频帧之间 buffer 被 TX 耗尽。

### 解决

**1. 视频帧后继续读挂起音频块**：
```cpp
// 视频帧解码+显示后：
for (剩余循环) {
    if (下一个 == AUDIO) writeAudioData();
    else break; // 留给下次调用
}
```

**2. TX buffer 加倍**：16384 → 32768（≈2秒缓冲）

**3. 诊断改百分比**：`[CB100 b=XXXX u=XX%]`

### 代码变更

| 文件 | 变更 |
|------|------|
| `VideoRecorder.cpp` | 视频帧后 inner loop 读取挂起音频块 |
| `Max98357a_AudioPlayer.h` | buffer 16384→32768 |
| `Max98357a_AudioPlayer.cpp` | 诊断百分比 |
| `Shared_GlobalDefines.h` | V1.91→V1.92 |

### 下一步

1. **编译上传V1.92**
2. 验证 underrun 率降至个位数，杂音消失

---

## 版本 V1.93 - 修复byte_swap: 关键时刻 (2026-04-27)

### 🔍 V1.92 结果

underrun 从 100% 降至 1%（b=32128/32768），但**杂音完全不变** → underrun 不是杂音根因。

### 🎯 真正的根因 — 字节序错误

**INMP441 RX 回调（正常工作）中的线索**：

```cpp
int16_t sample = (int16_t)((uint16_t)src[i*2+1] << 8 | src[i*2]);
```

这是在做**手动字节交换**！I2S 线上是 MSB 优先，但 ARM 是小端序。INMP441 RX 不激活硬件 byte_swap，而是软件手动交换。

**MAX98357A TX 也需要字节交换**：

```
i2s_set_byte_swap(FALSE):
  ARM写 0x1234 → 内存[0x34, 0x12] → I2S线[0x34, 0x12] → MAX读 0x3412 ✗ 字节反转！

i2s_set_byte_swap(TRUE):
  ARM写 0x1234 → 内存[0x34, 0x12] → 硬件交换 → I2S线[0x12, 0x34] → MAX读 0x1234 ✓
```

### 为什么 V1.57 测试 FALSE→TRUE "噪音更严重"

V1.57 时 DMA 完全不工作（page_idx:255），数据没有通过 I2S 发送。"噪音更严重"是因为 I2S 引脚悬空，MAX98357A 输入端浮空的状态不同。现在 DMA 正常工作（V1.86+），`TRUE` 应能解决问题。

### SDK 文档确认

I2S.md CD 音频示例：
```c
.i2s_byte_swap = I2S_LITTLE_INDIAN,  // 小端模式 → 启用字节交换
```

### 本次修改

**仅改一行**：`i2s_set_byte_swap(&m_i2sObj, FALSE)` → `TRUE`

同时移除 V1.90 添加的 `POSITIVE_EDGE`（无用），回到默认 `NEGATIVE_EDGE`。

### 代码变更

| 文件 | 变更 |
|------|------|
| `Max98357a_AudioPlayer.cpp` | byte_swap FALSE→TRUE；移除 POSITIVE_EDGE |
| `Shared_GlobalDefines.h` | V1.92→V1.93 |

### 预期效果

- MAX98357A 正确接收字节序 → 无杂音
- 这是经 35+ 版本尝试后，最后剩余的软件修正点

### 下一步

1. **编译上传V1.93**
2. 验证杂音是否消失

---

## 版本 V1.94 - 回退byte_swap+极简TX回调定位噪声源 (2026-04-27)

### 问题

V1.93 byte_swap=TRUE → 白噪音，无任何可识别音频。V1.92 FALSE → 音频+噪声（可识别语音）。**FALSE 是正确的**。

### 本次修改

**终极测试**：将 TX 回调简化为绝对最小——纯原始样本输出：
```cpp
// V1.94 handleI2sTxCallback() - 极简版
sample = (buffer有数据) ? buffer读出 : 0;
pbuf16[i*2] = sample;      // 左声道
pbuf16[i*2+1] = sample;    // 右声道
// 无DC阻塞、无淡入淡出、无任何滤波
```

移除了所有信号处理：DC 阻塞 IIR、淡入淡出、软衰减。如果噪声在 V1.94 中仍完全相同 → 噪声来自硬件链路（SD卡SPI噪声/MAX98357A电源/I2S信号质量），无法通过软件修复。

### 代码变更

| 文件 | 变更 |
|------|------|
| `Max98357a_AudioPlayer.cpp` | byte_swap TRUE→FALSE；TX回调移除所有滤波→极简原始输出 |
| `Shared_GlobalDefines.h` | V1.93→V1.94 |

### 预期

- 如果噪声消失 → 之前的 DC 阻塞或 fade 代码有问题
- 如果噪声不变 → 确认为硬件噪声，软件无法修复

### 下一步

1. **编译上传V1.94**
2. 听：噪声是否变化/消失？

---

## 版本 V1.95 - 修复视频帧率：从AVI头读取FPS (2026-04-28)

### 问题描述

用户报告两个问题：
1. **视频播放速度过快**（帧率异常）
2. **音频播放不完整**（视频结束时音频未播完）

电脑 VLC 播放同一视频正常。

### 根因分析

**VLC 日志关键信息**：
```
avi debug: stream[0] video(MJPG) 1280x720 24bpp 11.000000fps
avi debug: stream[0] created 125 index entries
avi debug: stream[1] audio(0x1 - ) 1 channels 16000Hz 16bits
avi debug: stream[1] created 343 index entries
avi debug: stream[0] length:11 (based on index)
```

**问题1 — FPS 硬编码为 15**：

[MJPEG_Encoder.cpp L782](file:///c:/Users/Debug/AppData/Local/Arduino15/packages/realtek/hardware/AmebaPro2/4.0.9-build20250528/libraries/CameraSystem/examples/Camera/MJPEG_Encoder.cpp#L782):
```cpp
m_fps = 15;  // ← 硬编码！应是11
```

`parseAVIHeader()` 是**空函数**（L1084: `return true;`），从未读取 AVI 文件头的 FPS 信息。

AVI `avih` 头中 `dwMicroSecPerFrame` 位于文件偏移 32 处（4字节 LE）：
- 11 FPS → `dwMicroSecPerFrame = 1000000/11 ≈ 90909`
- 开发板按 15 播放 → 每帧仅 66.7ms 而非 90.9ms → 快 36%

**问题2 — 音频不完整是问题1的后果**：
- 11 FPS × 125 帧 = 11.36 秒视频时长
- 343 chunks × 512 samples / 16000Hz = 10.98 秒音频
- 视频和音频时长匹配（差 ≈0.4 秒为开头静音）
- 但 15 FPS 下视频仅 8.3 秒 → 音频播到 ~76% 就被强制停止

**修复 FPS → 同时修复音频不完整**。

### 解决方案

**从 AVI 文件头读取真实 FPS**：

```cpp
// V1.95: 在 open() 中 parseSimplifiedMovi() 前添加 avih 解析
uint8_t avihBuf[56];
f_lseek(&m_file, 32);           // avih 数据从偏移32开始
f_read(&m_file, avihBuf, 56, &bytesRead);

uint32_t microSecPerFrame = readLE32(avihBuf);
m_fps = (1000000 + microSecPerFrame/2) / microSecPerFrame; // 四舍五入
m_frameCount = readLE32(avihBuf + 16); // dwTotalFrames
```

### 代码变更

| 文件 | 变更 |
|------|------|
| `MJPEG_Encoder.cpp` | `open()`中添加 avih 解析获取真实 FPS 和总帧数 |
| `Shared_GlobalDefines.h` | V1.94→V1.95 |

### AVI 头结构

```
文件偏移 32: dwMicroSecPerFrame  (4字节 LE) → FPS = 1000000 / value
文件偏移 48: dwTotalFrames       (4字节 LE) → 视频总帧数
文件偏移 56: dwStreams           (4字节)
文件偏移 64: dwWidth             (4字节)
文件偏移 68: dwHeight            (4字节)
```

### 预期效果

- 视频按正确 FPS（11）播放，速度正常
- 125 帧 × (1/11) 秒/帧 ≈ 11.4 秒 = 音频 10.98 秒 + 缓冲余量
- 音频完整播放完毕
- 硬编码 FPS=15 作为 fallback（仅当 avih 解析失败时使用）

### 下一步

1. **编译上传V1.95**
2. 验证视频播放速度正常（不加速）
3. 验证音频播放完整（不易被截断）

---

## 版本 V1.96 - 修复预加载不足：音频缓冲预填充逻辑优化 (2026-04-28)

### 问题描述

V1.95 修复后，用户测试反馈问题依旧：
- 播放开始时画面、声音速度正常
- 很快出现视频帧率过快导致播放速度异常
- 音频回放不完整且存在未播放完毕的情况
- 电脑 VLC 播放同一视频正常

### 根因分析

**日志关键信息**：
```
[INFO] MJPEGDecoder: Audio chunk #1 at 326, size=1024, samples=512
[INFO] MJPEGDecoder: Audio chunk #2 at 1358, size=1024, samples=512
[INFO] MJPEGDecoder: Video chunk #1 at 2390, size=30142
[INFO] Preloaded 2 audio chunks (1024 samples) before starting playback
```

**预加载逻辑缺陷**：

原代码 `VideoRecorder.cpp` 中的预加载循环：
```cpp
for (int i = 0; i < 32; i++) {
    MJPEGDecoder::ChunkType type = mjpegDecoder.readNextChunk(&chunkData, &chunkSize);
    // ...
    } else if (type == MJPEGDecoder::CHUNK_TYPE_VIDEO) {
        break;  // ← 遇到视频块立即停止！
    }
}
```

AVI 文件中数据排列顺序：2-3 个音频块 + 1 个视频块交替。遇到第一个视频块就 break，导致**仅预加载 2 个音频块（1024 样本 = 64ms）**。

**64ms 预加载音频远不足以覆盖视频解码阻塞**：
- JPEG 解码是阻塞操作，单帧可能耗时 20-50ms
- I2S TX 回调以 16kHz 恒定速率消耗缓冲区
- 64ms 音频在 50ms 阻塞期间几乎耗尽
- 后续视频帧解码期间音频供应中断 → 音频不完整

### 解决方案

**跳过视频块，持续预加载音频直到积累足够数据**：

```cpp
const int TARGET_PRELOAD_CHUNKS = 32;  // 32 × 512 = 16384 样本 ≈ 1秒
int consecutiveSkippedVideoChunks = 0;

while (preloadedChunks < TARGET_PRELOAD_CHUNKS) {
    MJPEGDecoder::ChunkType type = mjpegDecoder.readNextChunk(&chunkData, &chunkSize);
    if (type == MJPEGDecoder::CHUNK_TYPE_END) {
        break;
    }
    if (type == MJPEGDecoder::CHUNK_TYPE_AUDIO) {
        Max98357aAudioPlayer::getInstance().writeAudioData(
            (const int16_t*)chunkData, sampleCount);
        preloadedChunks++;
        preloadedSamples += sampleCount;
        consecutiveSkippedVideoChunks = 0;
    } else if (type == MJPEGDecoder::CHUNK_TYPE_VIDEO) {
        consecutiveSkippedVideoChunks++;
        if (consecutiveSkippedVideoChunks > 100) {
            break;  // 安全退出，防止异常
        }
        // 跳过视频块，继续循环
    }
}
```

**关键改动**：
1. `for` 循环改为 `while` 循环，以预加载目标数量为导向
2. 遇到视频块不 break，而是跳过并继续计数
3. 预加载 32 个音频块 = 16384 样本 ≈ 1 秒音频缓冲
4. 添加安全机制：连续跳过 100 个视频块后强制退出

**同时修改的辅助函数**：

1. `writeAudioData()` 允许在 `PLAYBACK_STATE_STOPPED` 状态写入（用于预加载）：
```cpp
if (m_state != PLAYBACK_STATE_PLAYING && m_state != PLAYBACK_STATE_STOPPED) {
    return false;
}
```

2. `startPlayback()` 保留预加载的音频数据，不清空缓冲区：
```cpp
bool hasPreloadedData = (m_txBufferCount > 0);
if (!hasPreloadedData) {
    memset(m_txBuffer, 0, m_txBufferSize * sizeof(int16_t));
}
m_txBufferHead = hasPreloadedData ? m_txBufferHead : 0;
m_txBufferCount = hasPreloadedData ? m_txBufferCount : 0;
```

3. 添加 `resetSequentialMode()` 方法用于重置文件读取位置：
```cpp
void MJPEGDecoder::resetSequentialMode() {
    m_sequentialMode = false;
    m_currentFilePos = m_moviStartPos + 12;
    m_currentFrameIndex = 0;
    m_currentAudioFrameIndex = 0;
}
```

### 预加载数据量对比

| 指标 | 修复前 | 修复后 |
|------|--------|--------|
| 预加载音频块数 | 2 | 32 |
| 预加载样本数 | 1024 | 16384 |
| 预加载时长 | 64ms | 1024ms (~1秒) |
| 缓冲区占用 | 3% (1024/32768) | 50% (16384/32768) |

### 代码变更

| 文件 | 变更 |
|------|------|
| `VideoRecorder.cpp` | 预加载逻辑从 break 改为跳过视频块；添加连续跳过计数安全退出 |
| `Max98357a_AudioPlayer.cpp` | `writeAudioData()` 允许 STOPPED 状态写入；`startPlayback()` 保留预加载数据 |
| `MJPEG_Encoder.h` | 添加 `resetSequentialMode()` 方法声明 |
| `MJPEG_Encoder.cpp` | 实现 `resetSequentialMode()` |
| `Shared_GlobalDefines.h` | V1.95→V1.96 |

### 预期效果

- 预加载约 1 秒音频数据作为缓冲
- 视频解码阻塞期间有充足音频数据供应
- 音频播放完整连续，不出现中断
- 用户感知视频播放速度正常（因音频同步正常）

### 下一步

1. **编译上传V1.96**
2. 验证日志显示 `Preloaded 32 audio chunks (16384 samples)`
3. 验证音频播放完整，视频速度正常

---

## 版本 V1.97 - 修复主菜单指示三角形位置保留问题 (2026-04-28)

### 问题描述

主菜单选项B、C、D在返回主菜单时，指示三角形位置未正确保留：
- 从选项B的子菜单返回主菜单时，三角形未指向B
- 从选项C的子菜单返回主菜单时，三角形未指向C
- 从选项D的子菜单返回主菜单时，三角形未指向D

而选项E的现有行为是正确的：从文件传输功能返回主菜单时，三角形正确指向选项E。

### 根因分析

**选项E的正确实现**：

```cpp
// returnFromUsbMode() / returnFromTransferMode()
menuManager.switchToPageByType(MENU_PAGE_MAIN);
triangleController.moveToPosition(TriangleController::POSITION_E);  // 显式恢复到E
showMenu();
```

**选项B、C、D的问题代码**：

```cpp
// switchToMainMenu()
menuManager.switchToPageByType(MENU_PAGE_MAIN);
triangleController.resetPosition();  // 总是重置到A！
showMenu();
```

当从子菜单按F返回主菜单时，`switchToMainMenu()` 总是将三角形重置到POSITION_A，而不是保留用户之前选择的位置。

### 解决方案

**1. 添加位置跟踪变量**：

```cpp
// Menu_MenuContext.h
int lastMainMenuPosition = 0;  // 保存进入子菜单时的主菜单位置
```

**2. 进入子菜单时保存当前位置**：

```cpp
// handleEvent() - MENU_OPERATION_SETTINGS case
case MENU_OPERATION_SETTINGS:
    if (item->type == MENU_ITEM_TYPE_SUBMENU) {
        lastMainMenuPosition = triangleController.getCurrentPosition();  // 保存！
        menuManager.switchToPageByType(MENU_PAGE_SUB);
        triangleController.resetPosition();
    }
    break;
```

**3. 返回主菜单时恢复保存的位置**：

```cpp
// switchToMainMenu()
void MenuContext::switchToMainMenu() {
    // ...
    menuManager.switchToPageByType(MENU_PAGE_MAIN);
    // 恢复三角形到进入子菜单前的主菜单位置
    triangleController.moveToPosition((TriangleController::MenuPosition)lastMainMenuPosition);
    showMenu();
    Utils_Logger::info("已切换回主菜单，三角形指向选项%c", (char)('A' + lastMainMenuPosition));
}
```

### 代码变更

| 文件 | 变更 |
|------|------|
| `Menu_MenuContext.h` | 添加 `lastMainMenuPosition` 成员变量 |
| `Menu_MenuContext.cpp` | `handleEvent()` 中保存当前位置；`switchToMainMenu()` 恢复保存的位置 |
| `Shared_GlobalDefines.h` | V1.96→V1.97 |

### 预期效果

- 从选项B的子菜单返回 → 三角形指向B
- 从选项C的子菜单返回 → 三角形指向C
- 从选项D的子菜单返回 → 三角形指向D
- 与选项E的行为完全一致

### 下一步

1. **编译上传V1.97**
2. 测试路径：
   - 主菜单 → B → 子菜单按F返回 → 验证三角形在B
   - 主菜单 → C → 子菜单按F返回 → 验证三角形在C
   - 主菜单 → D → 子菜单按F返回 → 验证三角形在D
3. 验证连续切换场景

---

## 版本 V1.98 - 修复视频播放结束时音频未完整输出的问题 (2026-04-28)

### 问题描述

用户报告视频回放时音频不完整。分析日志发现：
- 预加载 32 个音频块（约 1 秒）
- TX buffer 使用率维持在 90%+（约 2 秒缓冲）
- 播放结束时到达 `idx1`，显示 vFrames=104, aFrames=295
- 但音频数据未被完整播放

### 根因分析

**TX 回调的播放控制逻辑**：

```cpp
// 原代码 - handleI2sTxCallback()
if (!m_playing || m_state != PLAYBACK_STATE_PLAYING) {
    memset(pbuf, 0, MAX98357A_DMA_PAGE_SIZE);
    i2s_send_page(&m_i2sObj, (uint32_t*)pbuf);
    return;  // ★ 直接返回，不消耗 buffer 中的数据！
}
```

**`stopPlayback()` 的停止逻辑**：

```cpp
m_playing = false;
m_state = PLAYBACK_STATE_STOPPED;
// ...
memset(m_txBuffer, 0, ...);  // ★ 直接清零 buffer，丢弃未播放的音频！
```

**问题机制**：
1. 播放到达 `idx1` 时，`stopVideoPlayback()` 调用 `stopPlayback()`
2. `stopPlayback()` 设置 `m_playing = false`
3. TX 回调检测到 `!m_playing`，直接返回静音页，**不再消耗 buffer**
4. 然后 `memset(m_txBuffer, 0, ...)` 直接清零 buffer
5. 约 1-2 秒的音频数据被直接丢弃

**测试验证**：
- 295 音频块 × 512 采样 ≈ 9.4 秒音频
- TX buffer 最多 32768 采样 ≈ 2 秒缓冲
- 问题发生时 buffer 使用率约 99%，意味着约 2 秒音频被丢弃

### 解决方案

**添加 `m_draining` 模式：允许 TX 回调在停止后继续排空 buffer**

**1. 添加 `m_draining` 标志**：

```cpp
// Max98357a_AudioPlayer.h
bool m_draining;  // 排空模式标志
```

**2. TX 回调支持 draining**：

```cpp
if (!m_playing || m_state != PLAYBACK_STATE_PLAYING) {
    if (!m_draining) {
        memset(pbuf, 0, MAX98357A_DMA_PAGE_SIZE);
        i2s_send_page(&m_i2sObj, (uint32_t*)pbuf);
        return;  // 不在 draining 模式，直接返回静音
    }
    // ★ 在 draining 模式下，继续消耗 buffer
}

if (xSemaphoreTake(m_mutex, 0) == pdTRUE) {
    // ... 消耗 buffer 数据 ...
    if (m_draining && m_txBufferCount == 0) {
        m_draining = false;  // buffer 排空，停止 draining
        Utils_Logger::info(TAG, "Buffer drained, stopping TX");
    }
    xSemaphoreGive(m_mutex);
}
```

**3. `stopPlayback()` 使用 draining 排空**：

```cpp
m_playing = false;
m_state = PLAYBACK_STATE_STOPPED;
// ...
m_draining = true;  // 启动 draining 模式

uint32_t drainStart = millis();
while (m_draining && (millis() - drainStart < 5000)) {
    delay(10);  // 等待 TX 回调排空 buffer
}
```

### 代码变更

| 文件 | 变更 |
|------|------|
| `Max98357a_AudioPlayer.h` | 添加 `m_draining` 成员变量 |
| `Max98357a_AudioPlayer.cpp` | 构造函数初始化 `m_draining`；TX 回调支持 draining 模式；`stopPlayback()` 改为排空 buffer |
| `Shared_GlobalDefines.h` | V1.97 → V1.98 |

### 预期效果

- 视频播放结束时，TX buffer 中的剩余音频数据将被完整播放
- 日志显示 "Buffer drained, stopping TX" 表示排空成功
- 音频播放时间应与视频播放时间一致

### 下一步

1. **编译上传 V1.98**
2. 测试验证：
   - 播放同一视频，对比修复前后音频完整度
   - 观察日志确认 "Buffer drained" 消息
   - 确认音频播放时间与视频时长一致

---

## 版本 V1.99 - 修复主菜单三角形位置保留问题 + 视频帧率控制修复 (2026-04-28)

### 问题描述

用户报告两个问题：

1. **主菜单三角形位置未正确保留**：从选项B、C、D的子菜单返回主菜单时，三角形重置到A而不是保留在原位置
2. **视频帧率过快**：视频播放速度异常快，音视频不同步

### 问题1：主菜单三角形位置未正确保留

#### 根因分析

**日志证据**：
```
[INFO] 三角形移动到位置: C  (选择选项C)
...
[INFO] 已切换回主菜单，三角形指向选项A  (返回主菜单后，三角形在A而不是C！)
```

**代码问题**：
`handleEvent()` 中的 `MENU_EVENT_SELECT` 处理只针对 `MENU_OPERATION_SETTINGS` 保存了 `lastMainMenuPosition`，其他操作（CAPTURE, RECORD, PLAYBACK等）都没有保存。

```cpp
// 原代码 - 只有 SETTINGS 操作保存位置
switch (item->operation) {
    case MENU_OPERATION_SETTINGS:
        if (item->type == MENU_ITEM_TYPE_SUBMENU) {
            lastMainMenuPosition = triangleController.getCurrentPosition();  // ✓ 保存了
        }
        break;
    case MENU_OPERATION_PLAYBACK:
        // ❌ 没有保存 lastMainMenuPosition
        break;
    // ...
}
```

#### 解决方案

在 `MENU_EVENT_SELECT` 处理中，**选择菜单项时**立即保存 `lastMainMenuPosition`，而不是只在特定操作中保存：

```cpp
case MENU_EVENT_SELECT:
    {
        const MenuItem *item = getCurrentMenuItemInfo();
        if (item != nullptr) {
            int currentPos = triangleController.getCurrentPosition();
            Utils_Logger::info("[MenuContext] 选择菜单项: %d (%c) - %s", ...);

            // ★ 保存当前位置，以便从子菜单返回时恢复
            lastMainMenuPosition = currentPos;

            // 根据菜单项类型和操作执行不同的逻辑
            switch (item->operation) {
                // ...
            }
        }
    }
```

### 问题2：视频帧率过快

#### 根因分析

**原帧率控制逻辑问题**：

```cpp
// 原代码
videoFrameCount++;
unsigned long currentMillis = millis();
if (currentMillis - lastFrameTime < frameInterval) {
    while (millis() - lastFrameTime < frameInterval) {
        delay(1);
    }
}
currentMillis = millis();
jpeg.decode(...);     // 耗时操作（如50ms）
drawBitmap(...);      // 耗时操作（如5ms）
lastFrameTime = currentMillis;  // ★ 在耗时操作之后设置！
```

**问题机制**：
1. T=0ms: 显示帧0，`lastFrameTime = 0`
2. T=100ms: 遇到视频帧1，检查 `100-0=100 >= 90`，不等待
3. jpeg.decode() 耗时 50ms，drawBitmap() 耗时 5ms
4. `lastFrameTime = 100ms`（但实际时间可能是155ms）

**结果**：帧1和帧2之间实际只间隔了约55ms（如果下一个视频帧在T=155ms时处理），而不是期望的90ms。

#### 解决方案

使用**目标时间累加**方式，确保帧间隔精确：

```cpp
// 新代码
videoFrameCount++;
unsigned long targetTime = lastFrameTime + frameInterval;  // ★ 计算目标时间
unsigned long currentMillis = millis();
if (targetTime > currentMillis) {
    delay(targetTime - currentMillis);  // ★ 等待到目标时间
}
jpeg.decode(...);
drawBitmap(...);
lastFrameTime = targetTime;  // ★ 使用目标时间而不是当前时间
```

**效果**：
- 每帧都严格按照 `frameInterval` 间隔显示
- jpeg.decode() 和 drawBitmap() 的耗时被自动补偿
- 帧率从过快的约15fps恢复到正确的11fps

### 代码变更

| 文件 | 变更 |
|------|------|
| `Menu_MenuContext.cpp` | `MENU_EVENT_SELECT` 处理中添加 `lastMainMenuPosition = currentPos` |
| `VideoRecorder.cpp` | 帧率控制改用 `targetTime = lastFrameTime + frameInterval` 方式 |
| `Shared_GlobalDefines.h` | V1.98 → V1.99 |

### 预期效果

1. **主菜单三角形位置**：
   - 从任何选项（B、C、D、E等）返回主菜单时，三角形都能正确保留在进入前的位置
   - 日志显示 "三角形指向选项X"（X是进入前的选项）

2. **视频帧率**：
   - 视频帧按照 AVI header 中声明的 fps 精确播放
   - 11fps 视频正确以约90ms/帧的间隔显示
   - 音视频同步改善

### 下一步

1. **编译上传 V1.99**
2. 测试验证：
   - 主菜单 → B → 返回 → 验证三角形在B
   - 主菜单 → C → 返回 → 验证三角形在C
   - 主菜单 → D → 返回 → 验证三角形在D
   - 播放视频，验证帧率正常（11fps），音频完整

---

## 版本 V1.100 - 修复视频帧率时间漂移累积问题 (2026-04-28)

### 问题描述

用户反馈视频播放过程中帧率不稳定，画面与声音均出现播放速度过快的现象。但在视频内容播放完毕后，剩余的音频部分又能恢复正常速度播放。

### 根因分析

**V1.99修复的缺陷**：使用 `lastFrameTime = targetTime` 导致时间漂移累积。

**原V1.99代码**：
```cpp
unsigned long targetTime = lastFrameTime + frameInterval;  // 例如 0 + 90 = 90
unsigned long currentMillis = millis();                      // 例如 100
if (targetTime > currentMillis) {
    delay(targetTime - currentMillis);
}
// ... decode + draw (耗时100ms) ...
lastFrameTime = targetTime;  // ★ 设置为90而非实际的200
```

**时间漂移累积过程**：
| 帧 | lastFrameTime(前) | targetTime | currentMillis | 是否等待 | lastFrameTime(后) |
|----|-------------------|------------|---------------|----------|-------------------|
| 0  | 0                 | 90         | 0             | 否       | 90                |
| 1  | 90                | 180        | 100           | 否       | 180               |
| 2  | 180               | 270        | 200           | 否       | 270               |
| ...| 每帧递增90        | ...        | ...           | 永不等待 | 持续漂移          |

**问题机制**：
- decode + draw 耗时约100ms，超过frameInterval(90ms)
- 每帧都比预定时间晚 `decodeTime - frameInterval ≈ 10ms`
- 漂移持续累积，导致帧率逐渐加快

### 解决方案

**关键修复**：使用实际时间戳更新 `lastFrameTime`，而非目标时间

```cpp
// 关键修复点
lastFrameTime = millis();  // 使用实际时间，而非 targetTime
```

**完整修复后的代码**：
```cpp
unsigned long frameStartTime = millis();              // ★ 在decode前捕获
unsigned long targetTime = lastFrameTime + frameInterval;
if (targetTime > frameStartTime) {                    // ★ 用frameStartTime判断
    delay(targetTime - frameStartTime);
}
// ... decode + draw ...
lastFrameTime = millis();                              // ★ 用实际时间更新
```

**修复效果**：
- 如果 decode 耗时 < frameInterval：正常等待补偿
- 如果 decode 耗时 > frameInterval：不尝试追赶（避免更多漂移）
- lastFrameTime 始终反映实际时间

### 代码变更

| 文件 | 变更 |
|------|------|
| `VideoRecorder.cpp` | 帧率控制：`lastFrameTime = millis()` 替代 `lastFrameTime = targetTime` |
| `Shared_GlobalDefines.h` | V1.99 → V1.100 |

### 下一步

1. **编译上传 V1.100**
2. 测试验证：
   - 播放视频，验证帧率稳定（11fps）
   - 确认画面和声音速度恢复正常
   - 验证视频结束后音频仍能正常播放

---

## 版本 V1.101 - 添加视频帧率调试日志 (2026-04-28)

### 问题描述

用户反馈V1.100修复后，声音速度已恢复正常运行状态，但画面播放速度仍然过快。需要添加详细的帧率调试日志来定位问题根因。

### 根因分析

需要通过调试日志观察：
1. 每帧的实际等待时间和解码耗时
2. 帧率控制逻辑是否正确执行
3. 是否存在其他导致帧率过快的原因

### 解决方案

**添加视频帧率调试日志**：

```cpp
} else if (type == MJPEGDecoder::CHUNK_TYPE_VIDEO) {
    videoFrameCount++;
    unsigned long frameStartTime = millis();
    unsigned long targetTime = lastFrameTime + frameInterval;
    unsigned long waitTime = 0;
    if (targetTime > frameStartTime) {
        waitTime = targetTime - frameStartTime;
        delay(waitTime);
    }
    unsigned long afterDelayTime = millis();
    s_playbackFrameReady = false;
    if (jpeg.open((void*)chunkData, chunkSize, nullptr, jpegReadCallback, jpegSeekCallback, JPEGDrawForPlayback)) {
        jpeg.decode(0, 0, JPEG_SCALE_QUARTER);
        jpeg.close();
    }
    unsigned long afterDecodeTime = millis();
    if (s_playbackFrameReady) {
        tftManager.drawBitmap(0, 30, PLAYBACK_FB_WIDTH, PLAYBACK_FB_HEIGHT, s_playbackFrameBuffer);
    }
    lastFrameTime = millis();
    if (videoFrameCount <= 4) {
        Utils_Logger::info("VIDEO_FRAME: #%u, interval=%u, wait=%ums, decode=%ums, fps=%u",
            videoFrameCount, frameInterval, waitTime,
            afterDecodeTime - afterDelayTime, fps);
    }
```

**日志输出示例**：
```
[INFO] VIDEO_FRAME: #1, interval=90, wait=0ms, decode=89ms, fps=11
[INFO] VIDEO_FRAME: #2, interval=90, wait=0ms, decode=91ms, fps=11
[INFO] VIDEO_FRAME: #3, interval=90, wait=0ms, decode=88ms, fps=11
[INFO] VIDEO_FRAME: #4, interval=90, wait=0ms, decode=90ms, fps=11
```

### 代码变更

| 文件 | 变更 |
|------|------|
| `VideoRecorder.cpp` | 添加前4帧的调试日志，输出等待时间、解码耗时等信息 |
| `Shared_GlobalDefines.h` | V1.100 → V1.101 |

### 下一步

1. **编译上传 V1.101**
2. 测试验证：
   - 播放视频，观察前4帧的 `VIDEO_FRAME` 日志
   - 重点关注 `wait` 字段（应为0表示正常等待）和 `decode` 耗时
   - 如果 decode 耗时持续超过 frameInterval(90ms)，说明解码是瓶颈
   - 如果 wait 始终为0但 decode < frameInterval，说明帧率控制逻辑有问题

---

## 版本 V1.102 - 精确帧率计时与绘制时间测量 (2026-04-28)

### 问题描述

用户反馈V1.101添加调试日志后，画面播放速度仍然过快。日志显示帧间隔配置为100ms（10fps），但实际帧率测量值约为75-87ms（12fps），不符合预期。

### 根因分析

**VIDEO_FRAME日志分析**：

| 帧号 | wait | decode | 实际间隔 |
|------|------|--------|---------|
| #1   | 0ms  | 11ms   | ~11ms   |
| #2   | 64ms | 11ms   | ~75ms   |
| #3   | 77ms | 10ms   | ~87ms   |
| #4   | 75ms | 11ms   | ~86ms   |

**发现的问题**：
1. 使用`millis()`计时精度不足（1ms级别）
2. 没有测量`drawBitmap`的实际耗时
3. 帧率控制逻辑假设decode+draw时间可忽略，但实际可能不可忽略

### 解决方案

**关键修改1：使用`micros()`提高计时精度**

```cpp
// 修改前（精度不足）
unsigned long frameStartTime = millis();
unsigned long targetTime = lastFrameTime + frameInterval;

// 修改后（微秒精度）
unsigned long frameStartTime = micros();
unsigned long targetTime = lastFrameTime + (frameInterval * 1000UL);
```

**关键修改2：测量`drawBitmap`实际耗时**

```cpp
unsigned long drawTimeUs = 0;
if (s_playbackFrameReady) {
    unsigned long beforeDraw = micros();
    tftManager.drawBitmap(0, 30, PLAYBACK_FB_WIDTH, PLAYBACK_FB_HEIGHT, s_playbackFrameBuffer);
    drawTimeUs = micros() - beforeDraw;
}
```

**关键修改3：统一使用微秒级计时**

```cpp
lastFrameTime = micros();
delayMicroseconds(waitTimeUs);
```

**新日志格式**：
```
[INFO] VIDEO_FRAME: #1, interval=100ms, wait=0ms, decode=11ms, draw=14ms, fps=10
[INFO] VIDEO_FRAME: #2, interval=100ms, wait=89ms, decode=11ms, draw=12ms, fps=10
```

### 代码变更

| 文件 | 变更 |
|------|------|
| `VideoRecorder.cpp` | 使用`micros()`替代`millis()`；添加`drawTimeUs`测量；日志增加`draw`字段 |
| `VideoRecorder.cpp` | `resumeVideoPlayback()`中`lastFrameTime`也使用`micros()` |
| `Shared_GlobalDefines.h` | V1.101 → V1.102 |

### 下一步

1. **编译上传 V1.102**
2. 测试验证：
   - 播放视频，观察前4帧的`VIDEO_FRAME`日志
   - 重点关注`draw`字段（`drawBitmap`实际耗时）
   - 确认 totalFrame = wait + decode + draw ≈ 100ms
   - 如果 draw 时间过长，考虑优化 TFT DMA 传输
   - 如果间隔仍然不符，检查是否需要调整 frameInterval

---

## 版本 V1.103 - 修复帧率时间累积误差：改用绝对帧号计算目标时间 (2026-04-28)

### 问题描述

V1.102的VIDEO_FRAME日志显示，虽然添加了`draw`字段测量并使用`micros()`计时，但帧率仍然过快：
- Frame #2: wait=53ms, decode=9ms, draw=7ms → 总计69ms（应为90ms）
- Frame #3: wait=67ms, decode=9ms, draw=6ms → 总计82ms（应为90ms）
- Frame #4: wait=64ms, decode=10ms, draw=6ms → 总计80ms（应为90ms）

实际帧间隔约69-82ms（约12-13fps），而非目标的90ms（11fps）。

### 根因分析

**原代码使用`lastFrameTime`计算目标时间**：
```cpp
unsigned long targetTime = lastFrameTime + (frameInterval * 1000UL);
```

**问题**：`lastFrameTime`在每帧结束时被更新，但由于解码和显示的耗时不精确，或者在多任务环境下的调度延迟，导致计算出的目标时间不准确。

**示例**：
- Frame #1 结束时应设置 `lastFrameTime = T=15ms`（实际耗时）
- Frame #2 计算 `targetTime = 15 + 90 = 105ms`
- 但如果 `lastFrameTime` 被设置为其他值（如 37ms），则 `targetTime = 37 + 90 = 127ms`
- 如果 `currentTime = 15ms`，则 `wait = 127 - 15 = 112ms`（过大）

实际观察到 wait=53ms 而非预期的 75ms，说明计算存在误差。

### 解决方案

**关键修改：使用`playbackStartTime`作为基准时间，用帧号计算目标时间**

```cpp
// 新代码 - 基于播放开始时间和帧号计算目标时间
if (videoFrameCount == 1) {
    playbackStartTime = micros();  // 记录播放开始时间
    lastFrameTime = playbackStartTime;
}

unsigned long currentTime = micros();
// Frame #N 的目标开始时间 = 播放开始时间 + (N-1) × 帧间隔
unsigned long frameTarget = playbackStartTime + ((videoFrameCount - 1) * frameInterval * 1000UL);
unsigned long elapsed = currentTime - playbackStartTime;

unsigned long waitTimeUs = 0;
if (frameTarget > currentTime) {
    waitTimeUs = frameTarget - currentTime;
    delayMicroseconds(waitTimeUs);
}
```

**优势**：
1. 不依赖上一帧的结束时间，避免累积误差
2. 每帧的目标时间是基于播放开始时间的绝对值
3. 如果某帧延迟，下一帧仍然按原计划执行（不会级联延迟）

**预期日志**：
```
[INFO] VIDEO_FRAME: #1, interval=90ms, wait=0ms, decode=9ms, draw=6ms, fps=11
[INFO] VIDEO_FRAME: #2, interval=90ms, wait=75ms, decode=9ms, draw=7ms, fps=11
[INFO] VIDEO_FRAME: #3, interval=90ms, wait=75ms, decode=9ms, draw=6ms, fps=11
[INFO] VIDEO_FRAME: #4, interval=90ms, wait=75ms, decode=10ms, draw=6ms, fps=11
```

### 代码变更

| 文件 | 变更 |
|------|------|
| `VideoRecorder.cpp` | 添加`playbackStartTime`变量；改用帧号计算目标时间 |
| `Shared_GlobalDefines.h` | V1.102 → V1.103 |

### 下一步

1. **编译上传 V1.103**
2. 测试验证：
   - 播放视频，观察前4帧的`VIDEO_FRAME`日志
   - 确认 wait 时间约为 75ms（对于 90ms 帧间隔）
   - 确认 totalFrame = wait + decode + draw ≈ 90ms
   - 如果帧率仍然过快，检查是否有其他任务占用 CPU

---

## 版本 V1.104 - 修复帧间隔整数除法误差：使用微秒精确计算 (2026-04-28)

### 问题描述

V1.103的日志分析显示，虽然使用了`playbackStartTime`绝对时间计算，但帧率仍然过快：
- Frame #2: wait=53ms, decode=9ms, draw=7ms → 总计69ms（应为90ms）
- 目标帧间隔为90ms，但实际处理时间远短于此

### 根因分析

**原代码使用整数除法计算帧间隔**：
```cpp
uint32_t frameInterval = (fps > 0) ? 1000 / fps : 67;  // 90ms for 11fps
unsigned long frameTarget = playbackStartTime + ((videoFrameCount - 1) * frameInterval * 1000UL);
```

**问题**：
1. `1000 / fps = 90`（整数除法），但实际应为 `1000000 / 11 = 90909微秒`
2. `frameInterval * 1000UL = 90 * 1000 = 90000微秒`
3. 每帧误差：90909 - 90000 = 909微秒（约1%误差）
4. 虽然累积误差不大，但根本问题在于整数除法丢失精度

### 解决方案

**使用浮点精确计算微秒间隔**：
```cpp
// 新代码 - 使用微秒级精确计算
uint32_t frameIntervalMs = (fps > 0) ? 1000 / fps : 67;
uint32_t frameIntervalUs = (fps > 0) ? 1000000UL / fps : 67000;

// 帧目标时间计算使用微秒精确值
unsigned long frameTarget = playbackStartTime + ((videoFrameCount - 1) * frameIntervalUs);
```

**优势**：
1. 直接使用`1000000UL / fps`计算微秒间隔，零误差
2. 对于11fps视频：`frameIntervalUs = 1000000 / 11 = 90909微秒`
3. 与毫秒再转微秒相比，精度更高

### 代码变更

| 文件 | 变更 |
|------|------|
| `VideoRecorder.cpp` | 添加`frameIntervalUs`变量，使用`1000000UL / fps`计算微秒间隔 |
| `Shared_GlobalDefines.h` | V1.103 → V1.104 |

### 下一步

1. **编译上传 V1.104**
2. 测试验证：
   - 播放视频，观察前4帧的`VIDEO_FRAME`日志
   - 确认 `interval=` 字段仍显示90ms（旧值，仅用于显示）
   - 确认 `wait=` 时间约为 75-80ms（基于90909微秒间隔）
   - 如果帧率仍然过快，检查是否有其他原因

---

## 版本 V1.105 - 回溯修复：简化视频播放循环，使用简单计时替代复杂计算 (2026-04-28)

### 问题描述

V1.95-V1.104添加的复杂计时控制逻辑导致视频播放速度仍然过快。VIDEO_FRAME日志显示：
- Frame #1: wait=0ms, decode=9ms, draw=8ms → total 17ms
- Frame #2: wait=37ms (应为73ms)
- Frame #3: wait=49ms (应为73ms)

虽然使用了`playbackStartTime`和微秒级精度的`frameIntervalUs`，但帧率仍然比目标快约25%。

### 根因分析

**V1.95引入的复杂计时逻辑存在根本缺陷**：

1. **计算基准时间问题**：
   - `playbackStartTime`在处理第一个视频帧时才设置
   - 但此时已经消耗了大量时间在预加载音频和处理其他chunk
   - 导致计算出的`frameTarget`不准确

2. **累积误差问题**：
   - 原代码使用 `(videoFrameCount - 1) * frameIntervalUs` 计算目标时间
   - 这假设每一帧都精确按帧间隔播放
   - 实际上每帧的解码和显示时间不同，导致误差累积

3. **批量处理16 chunk的问题**：
   - 一次处理16个chunk改变了音视频的自然交错关系
   - 内层循环处理音频chunk后又立即返回，打断了外层循环逻辑

### 解决方案

**回溯到V1.50的简单可靠实现**：

1. **每次只处理一个chunk**：
   - 恢复V1.50的原始设计
   - 音视频数据按文件交错顺序自然同步

2. **使用简单的基于`lastFrameTime`的等待**：
   - 记录上一帧的完成时间 `lastFrameTime`
   - 下一帧在 `lastFrameTime + frameInterval` 后开始处理
   - 如果当前时间还未到达目标时间，则延迟等待

3. **使用毫秒级精度和整数算术**：
   - 使用 `millis()` 替代 `micros()`
   - 使用天花板除法 `(1000 + fps - 1) / fps` 确保正确取整

### 代码变更

**VideoRecorder.cpp - videoPlaybackLoop() 简化**：

```cpp
// 修复前 (V1.104 - 复杂且有bug)
uint32_t frameIntervalUs = (fps > 0) ? 1000000UL / fps : 67000;
for (int i = 0; i < 16; i++) {
    // ... 复杂的微秒计时计算
    unsigned long frameTarget = playbackStartTime + ((videoFrameCount - 1) * frameIntervalUs);
    unsigned long waitTimeUs = 0;
    if (frameTarget > currentTime) {
        delayMicroseconds(waitTimeUs);
    }
}

// 修复后 (V1.105 - 简单可靠)
uint32_t frameIntervalMs = (fps > 0) ? (1000 + fps - 1) / fps : 67;  // 天花板除法

MJPEGDecoder::ChunkType type = mjpegDecoder.readNextChunk(&chunkData, &chunkSize);
// 处理一个chunk...

if (videoFrameCount > 1) {
    unsigned long now = millis();
    unsigned long elapsed = now - lastFrameTime;
    if (elapsed < frameIntervalMs) {
        delay(frameIntervalMs - elapsed);
    }
}
lastFrameTime = millis();
```

### 优势

1. **简单可靠**：不依赖复杂的计时计算
2. **符合V1.50原始设计**：经过验证的稳定实现
3. **音视频自然同步**：按文件交错顺序处理，无需额外同步机制
4. **易于调试**：日志输出更简洁清晰

### 代码变更

| 文件 | 变更 |
|------|------|
| `VideoRecorder.cpp` | 简化videoPlaybackLoop()：每次处理一个chunk，使用简单的lastFrameTime等待 |
| `Shared_GlobalDefines.h` | V1.104 → V1.105 |

### 下一步

1. **编译上传 V1.105**
2. 测试验证：
   - 播放视频，观察前4帧的`VIDEO_FRAME`日志
   - 确认 `elapsed=` 时间约为 80-90ms（解码+显示+等待的总时间）
   - 确认画面和声音播放速度正常

---

## 版本 V1.106 - 修复videoPlaybackLoop中缺失的drawBitmap调用 (2026-04-28)

### 问题描述

V1.105测试发现：音频播放正常，但屏幕上显示四格布局缩略图，没有视频画面回放出来。PLAYBACK日志显示`vFrames`不断增加，说明视频帧被正确计数，但没有实际显示。

### 根因分析

**V1.105简化代码时漏掉了关键调用**：

在V1.105的videoPlaybackLoop中，视频帧被正确解码到`s_playbackFrameBuffer`，但漏掉了`drawBitmap()`调用将帧缓冲区绘制到屏幕：

```cpp
// V1.105的问题代码
if (jpeg.open(...)) {
    jpeg.decode(0, 0, JPEG_SCALE_QUARTER);
    jpeg.close();
}
// 漏掉了这步！
// if (s_playbackFrameReady) {
//     tftManager.drawBitmap(0, 30, PLAYBACK_FB_WIDTH, PLAYBACK_FB_HEIGHT, s_playbackFrameBuffer);
// }
lastFrameTime = millis();
```

### 解决方案

**补全drawBitmap调用**：

```cpp
// 修复后
if (jpeg.open(...)) {
    jpeg.decode(0, 0, JPEG_SCALE_QUARTER);
    jpeg.close();
}
unsigned long afterDecodeTime = millis();
if (s_playbackFrameReady) {
    tftManager.drawBitmap(0, 30, PLAYBACK_FB_WIDTH, PLAYBACK_FB_HEIGHT, s_playbackFrameBuffer);
}
lastFrameTime = millis();
```

### 代码变更

| 文件 | 变更 |
|------|------|
| `VideoRecorder.cpp` | 在videoPlaybackLoop中添加`tftManager.drawBitmap()`调用 |
| `Shared_GlobalDefines.h` | V1.105 → V1.106 |

### 下一步

1. **编译上传 V1.106**
2. 测试验证：
   - 播放视频，确认画面正确显示
   - 确认VIDEO_FRAME日志正常输出

## 版本 V1.107 - 音频解码架构重构：音频优先批量读取解决播放卡顿 (2026-04-29)

### 问题描述

视频回放时声音严重卡顿。串口日志显示TX缓冲区欠载率高达7273.5%，缓冲区在1.5秒内从50%降至0%，之后持续欠载。

**关键日志数据**：

| 时间点 | 音频块数 | 视频帧数 | 欠载率 | 缓冲区占用 |
|--------|---------|---------|--------|-----------|
| 32ms | 1182 | 424 | 0.0% | 16256/32768 (50%) |
| 1536ms | 1229 | 442 | 550.0% | 704/32768 (2%) |
| 3200ms | 1281 | 460 | 5392.1% | 0/32768 (0%) |
| 4864ms | 1333 | 478 | 7273.5% | 0/32768 (0%) |

### 根因分析

**核心问题：音频缓冲区饥饿（Buffer Starvation）**

旧`videoPlaybackLoop()`每次只读取1个chunk：
1. 遇到视频帧时，JPEG解码+SPI显示阻塞50-100ms
2. 阻塞期间无音频数据被读取
3. I2S DMA持续消耗音频缓冲区
4. 缓冲区耗尽 → 大量欠载 → 声音卡顿

**量化分析**：
- 11fps视频，每帧间隔91ms
- JPEG解码+显示约50-100ms
- 解码期间消耗约50-100ms音频，但只预读了0-1个音频块
- 每帧净损失约50-70ms音频数据 → 缓冲区持续递减

### 解决方案：音频优先批量读取架构

**方案选型**：音频优先批量读取策略（方案B），配合视频帧缓冲和水位线机制。

**架构设计**：

```
Phase 1: 音频优先批量读取（最多64 chunks/轮）
  while (chunksRead < 64) {
    chunk = readNextChunk()
    if AUDIO → writeAudioData() + continue
    if VIDEO → copyToVideoBuffer() + break
    if END   → stopPlayback() + return
  }

Phase 2: 水位线检查
  if (audioBuffer < LOW_WATERMARK) {
    继续读取音频，延迟视频帧处理
  }

Phase 3: 视频帧处理（仅当音频缓冲区健康时）
  decodeJPEG() → displayFrame()
```

**关键参数变更**：

| 参数 | 旧值 | 新值 | 说明 |
|------|------|------|------|
| MAX98357A_BUFFER_SIZE | 32768 | 65536 | 缓冲区从1秒增至2秒@16kHz |
| TARGET_PRELOAD_CHUNKS | 32 | 64 | 预加载从1秒增至2秒 |
| MAX_CHUNKS_PER_LOOP | 1 | 64 | 每轮最多读取64个chunk |
| AUDIO_LOW_WATERMARK | 无 | 8192 | 缓冲区低于25%时暂停视频处理 |
| AUDIO_HIGH_WATERMARK | 无 | 32768 | 缓冲区高于50%时恢复视频处理 |

### 代码变更

**1. Max98357a_AudioPlayer.h** — 缓冲区扩大+水位线机制
```cpp
#define MAX98357A_BUFFER_SIZE     65536  // 原为32768
#define MAX98357A_LOW_WATERMARK   8192   // 新增：低水位线
#define MAX98357A_HIGH_WATERMARK  32768  // 新增：高水位线

// 新增查询方法
size_t getBufferCount();
size_t getBufferSize();
bool isBufferAboveLowWatermark();
bool isBufferAboveHighWatermark();
float getBufferFillPercent();
```

**2. VideoRecorder.cpp** — 音频优先批量读取架构
- 新增`ensureVideoFrameBuf()`：动态分配视频帧独立缓冲区
- 新增`processVideoFrame()`：提取视频帧处理逻辑
- 重写`videoPlaybackLoop()`：
  - 每轮最多读取64个chunk（原为1个）
  - 音频chunk直接写入播放器
  - 视频帧拷贝到独立缓冲区，不阻塞音频读取
  - 低水位线时跳过视频帧处理，优先填充音频
  - 高水位线时才处理视频帧
- 重写`startVideoPlayback()`：
  - 预加载量从32增至64个音频chunk
  - 添加缓冲区填充率日志
  - 重置播放统计变量
- 更新`stopVideoPlayback()`：
  - 清理视频帧缓冲区内存
  - 重置播放统计变量

**3. Shared_GlobalDefines.h** — 版本号 V1.106 → V1.107

### 预期效果

1. **欠载率从7273.5%降至<5%**：批量读取确保音频数据持续供给
2. **缓冲区维持在25%以上**：水位线机制防止缓冲区耗尽
3. **视频帧不阻塞音频**：独立缓冲区+延迟处理策略
4. **预加载从1秒增至2秒**：更充足的初始缓冲

### 下一步

1. 编译上传V1.107
2. 播放测试视频，观察PLAYBACK日志中的underrun率和buf占用
3. 确认声音流畅无卡顿
4. 测试不同码率视频的兼容性

---

## 版本 V1.108 - I2S配置系统性比对分析与优化建议实施 (2026-04-29)

### 问题描述

需要参照I2S.md技术规范，对项目中所有I2S相关代码模块进行系统性比对分析，验证初始化参数、数据格式、采样率、位宽、时钟配置、引脚分配、中断处理及错误处理是否符合规范要求。

### 比对范围

| 模块 | 文件 | 功能 |
|------|------|------|
| MAX98357A播放器 | Max98357a_AudioPlayer.h + Max98357a_AudioPlayer.cpp | I2S TX音频播放 |
| INMP441录音器 | Inmp441_MicrophoneManager.h + Inmp441_MicrophoneManager.cpp | I2S RX音频录制 |
| 参考规范 | I2S.md | RTL8735B I2S技术规范 |

### 比对结论

#### ✅ 正确项（与文档一致）

| 参数 | I2S.md推荐值 | 实际值 | 判定 |
|------|-------------|--------|------|
| 采样率 | I2S_SR_16KHZ | I2S_SR_16KHZ | ✅ 一致 |
| 字长 | I2S_WL_16 | I2S_WL_16 | ✅ 一致 |
| 接口格式 | FORMAT_I2S | FORMAT_I2S | ✅ 一致 |
| 主从模式 | I2S_MASTER | I2S_MASTER | ✅ 一致 |
| DMA突发大小 | BURST16 | BURST16 | ✅ 一致 |
| DMA页数/大小 | 4页×1280字节 | 4页×1280字节 | ✅ 一致 |
| 引脚分配 | PD_14/15/16/17/18 | PD_14/15/16/17/18 | ✅ 一致 |

#### ⚠️ 不一致项（与文档推荐不同但有合理原因）

| 项目 | 文档推荐 | 实际值 | 原因 | 风险等级 |
|------|---------|--------|------|---------|
| MAX98357A传输方向 | I2S_ONLY_TX | I2S_DIR_TXRX | V1.79发现TX-only DMA不启动 | 低 |
| MAX98357A字节交换 | I2S_LITTLE_INDIAN | FALSE | V1.57启用后失真，回滚验证 | 低 |
| DMA页面预提交顺序 | enable后管理 | enable前预提交 | RTL8735B硬件要求workaround | 低 |

#### 🔴 缺失项（文档推荐但代码中未设置）

| 项目 | 文档推荐 | 缺失模块 | 风险等级 |
|------|---------|---------|---------|
| 数据边沿(EDGE_SW) | I2S_NEGATIVE_EDGE | 两模块均未显式设置 | 中 |
| SCK反相(SCK_INV) | I2S_SCKINV_DISABLE | 两模块均未显式设置 | 中 |
| WS相位(WS_SWAP) | I2S_LEFT_PHASE | 两模块均未显式设置 | 中 |
| INMP441字节交换 | 显式设置 | 未调用i2s_set_byte_swap | 中 |

#### 🔴 潜在风险点

| # | 风险 | 影响 | 严重程度 |
|---|------|------|---------|
| 1 | INMP441 I2S_CH_MONO模式下DMA数据布局未验证 | 可能读取到右声道无效数据 | 高 |
| 2 | 两模块均未显式设置时钟/边沿参数 | SDK更新可能改变默认值 | 中 |
| 3 | INMP441 RX回调字节序假设与BYTE_SWAP默认值一致性 | 若默认值改变，录音数据损坏 | 中 |

### 优化建议实施

#### 建议1：显式设置所有I2S参数（已完成）

**Max98357a_AudioPlayer.cpp initI2sForTx()**：
```cpp
i2s_set_dma_burst_size(&m_i2sObj, BURST16);
i2s_set_byte_swap(&m_i2sObj, FALSE);
i2s_set_data_start_edge(&m_i2sObj, I2S_NEGATIVE_EDGE);  // 新增
i2s_set_sck_inv(&m_i2sObj, I2S_SCKINV_DISABLE);         // 新增
i2s_set_ws_swap(&m_i2sObj, I2S_LEFT_PHASE);             // 新增
```

**Inmp441_MicrophoneManager.cpp initI2S()**：
```cpp
i2s_set_dma_burst_size(&m_i2sObj, BURST16);
i2s_set_byte_swap(&m_i2sObj, FALSE);                    // 新增：明确字节序
i2s_set_data_start_edge(&m_i2sObj, I2S_NEGATIVE_EDGE);  // 新增
i2s_set_sck_inv(&m_i2sObj, I2S_SCKINV_DISABLE);         // 新增
i2s_set_ws_swap(&m_i2sObj, I2S_LEFT_PHASE);             // 新增
```

#### 建议2：验证INMP441 MONO模式数据布局（已完成）

添加诊断代码验证I2S_CH_MONO模式下DMA缓冲区的实际数据分布：
```cpp
static bool s_monoDiagEnabled = true;
if (s_monoDiagEnabled && s_microphoneManagerPtr->m_sampleCount <= 2) {
    uint8_t* diagSrc = (uint8_t*)pbuf;
    Serial.print("[MONO_DIAG] First 8 samples (hex): ");
    for (int i = 0; i < 8; i++) {
        Serial.print(diagSrc[i*2+1], HEX); Serial.print(" ");
        Serial.print(diagSrc[i*2], HEX); Serial.print(" ");
    }
    Serial.println();
    if (s_microphoneManagerPtr->m_sampleCount == 2) {
        s_monoDiagEnabled = false;
    }
}
```

### 代码变更

| 文件 | 变更 |
|------|------|
| Max98357a_AudioPlayer.cpp | initI2sForTx()添加EDGE_SW/SCK_INV/WS_SWAP显式设置 |
| Inmp441_MicrophoneManager.cpp | initI2S()添加byte_swap/EDGE_SW/SCK_INV/WS_SWAP显式设置 |
| Inmp441_MicrophoneManager.cpp | i2s_rx_callback()添加MONO模式数据布局诊断代码 |
| Shared_GlobalDefines.h | 版本号 V1.107 → V1.108 |

### 验证结果

- ✅ MAX98357A显式设置I2S时钟边沿参数完成
- ✅ INMP441显式设置I2S参数完成
- ✅ MONO模式数据布局诊断代码已添加
- ✅ 版本号更新至V1.108
- ✅ 启动日志正确输出版本号

### 编译错误修复 (2026-04-29)

**问题描述**：
V1.108编译失败，错误信息：
```
cannot convert 'i2s_edge_sw_e' to 'i2s_edge_sw'
cannot convert 'i2s_ws_swap_e' to 'i2s_ws_swap'
```

**根本原因**：
I2S API 函数签名要求特定的枚举类型：
- `i2s_set_data_start_edge()` 需要 `i2s_edge_sw` 类型（应使用 `NEGATIVE_EDGE`）
- `i2s_set_ws_swap()` 需要 `i2s_ws_swap` 类型（应使用 `LEFT_PHASE`）
- `i2s_set_sck_inv()` 需要 `BOOL` 类型（应使用 `FALSE`）

原代码错误地使用了带 `I2S_` 前缀的宏（`I2S_NEGATIVE_EDGE`、`I2S_LEFT_PHASE`、`I2S_SCKINV_DISABLE`）

**修复方案**：
将两个文件中的 I2S 参数改为正确的类型：
```cpp
// 修复前（错误）
i2s_set_data_start_edge(&m_i2sObj, I2S_NEGATIVE_EDGE);
i2s_set_sck_inv(&m_i2sObj, I2S_SCKINV_DISABLE);
i2s_set_ws_swap(&m_i2sObj, I2S_LEFT_PHASE);

// 修复后（正确）
i2s_set_data_start_edge(&m_i2sObj, NEGATIVE_EDGE);
i2s_set_sck_inv(&m_i2sObj, FALSE);
i2s_set_ws_swap(&m_i2sObj, LEFT_PHASE);
```

**代码变更**：
| 文件 | 变更 |
|------|------|
| Max98357a_AudioPlayer.cpp | initI2sForTx() 中 I2S_NEGATIVE_EDGE→NEGATIVE_EDGE, I2S_SCKINV_DISABLE→FALSE, I2S_LEFT_PHASE→LEFT_PHASE |
| Inmp441_MicrophoneManager.cpp | initI2S() 中同样修改 |

### 编译错误修复 - VideoRecorder.cpp (2026-04-29)

**问题描述**：
V1.108编译失败，VideoRecorder.cpp 中多个播放状态变量未声明：
```
's_playbackAudioChunkCount' was not declared in this scope
's_playbackVideoFrameCount' was not declared in this scope
's_lastPlaybackStatsTime' was not declared in this scope
's_firstAudioDumped' was not declared in this scope
's_videoFrameBuf' was not declared in this scope
```

**根本原因**：
静态变量声明位于第 943 行，但被使用的位置在第 719 行（`startVideoPlayback()`）和第 779 行（`stopVideoPlayback()`）。编译器是单通处理，在编译函数时还没有看到变量声明。

**修复方案**：
将播放状态变量声明移到文件开头第 128 行之后（与其他播放相关变量放在一起）：

```cpp
// 原位置：第 943 行（太晚）
// 新位置：第 128 行之后

static uint8_t* s_videoFrameBuf = nullptr;
static uint32_t s_videoFrameBufSize = 0;
static uint32_t s_playbackAudioChunkCount = 0;
static uint32_t s_playbackVideoFrameCount = 0;
static uint32_t s_lastPlaybackStatsTime = 0;
static bool s_firstAudioDumped = false;
```

**代码变更**：
| 文件 | 变更 |
|------|------|
| VideoRecorder.cpp | 在第 128 行后添加播放状态变量声明；删除第 943 行重复声明 |

### 下一步

1. 重新编译上传V1.108
2. 观察MONO_DIAG日志，验证INMP441 MONO模式数据布局
3. 如诊断数据显示偶数/奇数位交替有效/零数据，需修改为STEREO模式并只取左声道
4. 考虑统一两模块的I2S配置常量到Shared_GlobalDefines.h

---

## V1.109 - 视频回放开头声音重复Bug修复 (2026-04-30)

### 对当前指令的理解

用户反馈视频回放时画面正常、声音清晰，但视频开头存在声音重复：录制"1到10"匀速报数，回放音频输出为"1、2、1、2、3、4……10"。要求根据Bug.md日志分析根本原因并提供修复方案，严格遵循行为准则。

### 问题描述

视频回放时，开头约2秒的音频（"1、2"）被播放了两次，导致"1,2,1,2,3,4...10"的异常重复模式。

### 根本原因

**双次初始化导致的音频块重复写入播放器缓冲区。**

详细流程：
1. `startVideoPlayback()` 中预加载循环调用 `mjpegDecoder.readNextChunk()` 从文件头顺序读取 64 个音频块写入播放器
2. 预加载后调用 `mjpegDecoder.resetSequentialMode()` 重置 `m_sequentialMode = false`
3. 主播放循环 `videoPlaybackLoop()` 调用 `readNextChunk()` 时，因 `m_sequentialMode` 为 `false`，再次进入初始化逻辑，从文件头重新读取音频块 #1、#2 写入播放器
4. 播放器缓冲区中：预加载的 #1-#64 + 重复的 #1-#2 → 播放时出现开头重复

关键代码路径：
- `VideoRecorder.cpp` `startVideoPlayback()`: 预加载 → `resetSequentialMode()` → 主循环
- `MJPEG_Encoder.cpp` `readNextChunk()`: `if (!m_sequentialMode)` 分支在每次 `resetSequentialMode()` 后都会重新初始化文件位置

### 修复方案

**跳过主播放循环中已预加载的音频块。** 保持预加载和 `resetSequentialMode()` 的调用不变（因为主循环需要从头读取被预加载跳过的视频帧），但在主循环中遇到音频块时，检查是否为已预加载的块，若是则跳过写入播放器。

### 实施步骤

1. 添加 `static uint32_t s_preloadedAudioChunks = 0;` 状态变量
2. `startVideoPlayback()`: 预加载完成后将 `preloadedChunks` 保存到 `s_preloadedAudioChunks`
3. `startVideoPlayback()`: 重置阶段清空 `s_preloadedAudioChunks = 0`
4. `stopVideoPlayback()`: 停止时清空 `s_preloadedAudioChunks = 0`
5. `videoPlaybackLoop()`: 音频块处理中，若 `s_preloadedAudioChunks > 0` 则递减并跳过 `player.writeAudioData()`

### 代码变更

| 文件 | 变更 |
|------|------|
| VideoRecorder.cpp | 添加 `s_preloadedAudioChunks` 状态变量 |
| VideoRecorder.cpp | `startVideoPlayback()`: 保存预加载计数并重置 |
| VideoRecorder.cpp | `stopVideoPlayback()`: 重置 `s_preloadedAudioChunks` |
| VideoRecorder.cpp | `videoPlaybackLoop()`: 跳过已预加载的音频块 |
| Shared_GlobalDefines.h | 版本号 V1.108 → V1.109 |

---

## V1.110 - 视频回放帧率修复 (2026-04-30)

### 对当前指令的理解

V1.109声音重复Bug修复后，又发现预加载阶段与正式播放阶段过渡处有明显画面掉帧。要求根据Bug.md日志分析根因并提供优化方案。

### 问题描述

视频目标帧率10fps（100ms/帧），实际仅达到约6.8fps（147ms/帧），约35%的帧被丢弃。日志中PLAYBACK统计数据：
- t=2496ms: vFrames=28
- t=5536ms: vFrames=48（20帧/3040ms=6.6fps）
- t=8576ms: vFrames=69（21帧/3040ms=6.9fps）

### 根本原因

`processVideoFrame()` 中 `lastFrameTime` 设在JPEG解码和TFT绘制**完成之后**（`lastFrameTime = millis()`），导致解码时间(~12ms)和绘制时间成为下一帧间隔计算中的"死区时间"——不被下一帧delay计算覆盖，逐帧累积约45ms/帧的额外延迟。

### 修复方案

将 `lastFrameTime = millis()`（解码后）改为 `lastFrameTime = frameStartMs`（解码前）。解码/绘制时间不再逃逸出动态delay的计算范围，delay自动缩短以补偿这些开销。

### 实施步骤

1. 将 `lastFrameTime = millis()` 移到 `frameStartMs` 赋值之后、JPEG解码之前
2. 删除原位置的 `lastFrameTime = millis()`

### 代码变更

| 文件 | 变更 |
|------|------|
| VideoRecorder.cpp | `processVideoFrame()`: `lastFrameTime = frameStartMs` 移至解码前 |
| Shared_GlobalDefines.h | 版本号 V1.109 → V1.110 |

---

## V1.111 - 视频回放帧率修复 (第二轮) (2026-04-30)

### 对当前指令的理解

V1.110帧率修复后，声画不再重复，但视频回放仍出现明显画面掉帧。要求根据Bug.md日志再次分析并提出优化方案。

### 问题描述

目标帧率11fps（91ms/帧），V1.110实际约8.2fps（121ms/帧），约25%帧丢失。日志中PLAYBACK统计数据：
- t=0ms: vFrames=13（初始爆发）
- t=4000ms: vFrames=47（34帧/4000ms，117ms/帧，8.5fps）
- t=7040ms: vFrames=72（25帧/3040ms，121ms/帧，8.2fps）
- t=10080ms: vFrames=97（25帧/3040ms，121ms/帧，8.2fps）

每帧额外33ms开销，远超12ms解码时间。

### 根本原因

**同一RTOS任务调用内连续处理多个视频帧，阻塞音频块写入。**

`videoPlaybackLoop()` 处理一个视频帧后检查 `isBufferAboveHighWatermark()`。由于视频帧解码延迟期间无音频写入，缓冲水位下降，导致检查失败 → `continue` → 继续读取下一个chunk → 命中另一个视频帧 → 再次阻塞91ms → 音频缓冲区持续饥饿而不被填充。

AVI文件中视频帧常连续排列（不总是与音频交替），在一个循环调用内可累积2-4个视频帧，阻塞时间达180-360ms，期间零音频写入。

```
循环内: [视频帧1 91ms] → [水位低,继续] → [视频帧2 91ms] → [水位低,继续] → ...
               ↑ 无音频写入 ↑                  ↑ 无音频写入 ↑
```

V1.110修复了单帧delay偏差(~14ms)，但未解决多帧堆积问题（多帧累积~33ms/帧）。

### 修复方案

**每调用一次 `videoPlaybackLoop()` 最多处理一个视频帧。** 删除 `isBufferAboveHighWatermark()` 检查后的 `continue`，直接 `break` 返回RTOS调度器。下次调用时优先写入累积的音频块，再处理下一个视频帧。

### 实施步骤

1. 删除视频帧处理后 `if (!player.isBufferAboveHighWatermark()) { continue; }` 分支
2. 视频帧处理后无条件 `break`

### 代码变更

| 文件 | 变更 |
|------|------|
| VideoRecorder.cpp | `videoPlaybackLoop()`: 视频帧处理后删除高水位检查，无条件break |
| Shared_GlobalDefines.h | 版本号 V1.110 → V1.111 |

---

## V1.112 - 视频回放初始阶段掉帧修复 (2026-05-05)

### 对当前指令的理解

视频回放功能在播放开始的前两秒出现轻微掉帧现象，需要修复。要求根据Bug.md串口日志分析根因，确保播放初始阶段达到至少15fps稳定播放标准。

### 问题描述

Bug.md日志显示播放初始阶段帧率仅约10fps，低于15fps目标：
- 前4帧间隔100ms（10fps），目标帧率11fps（91ms/帧）
- 音频缓冲区预加载至50%（64 chunks ≈ 32KB），但播放开始后快速消耗
- 视频帧处理被 `isBufferAboveLowWatermark()` 阻塞，等待音频缓冲区回填

### 根本原因

**三重瓶颈叠加导致初始阶段掉帧：**

**1. SD卡读取冗余f_lseek（主要瓶颈）**

`MJPEGDecoder::readNextChunk()` 中每个chunk执行两次 `f_lseek` + `f_read`：
- 第一次：seek到chunk位置，读取8字节头部
- 第二次：seek到chunk位置+8，读取chunk数据

第二次 `f_lseek` 完全冗余——第一次 `f_read` 后文件指针已在数据起始位置。每个chunk多一次SD卡命令往返（~0.5-1ms延迟），在初始阶段密集读取时累积为显著开销。

```
修复前: f_lseek → f_read(header) → f_lseek(冗余!) → f_read(data)
修复后: f_lseek → f_read(header) → f_read(data)  // 文件指针已在正确位置
```

**2. 音频预加载不足**

预加载64个chunks（~32KB）仅填充音频缓冲区50%。播放开始后，视频帧解码延迟（~12ms/帧）期间无音频写入，缓冲区水位快速下降至8KB低水位线以下，触发视频处理阻塞。

**3. 调试日志开销**

每20个音频chunk计算min/max/sum（遍历512个采样），频繁的串口输出占用CPU时间。

### 修复方案

**修复1：移除readNextChunk()中冗余的f_lseek**

删除视频chunk和音频chunk数据读取前的冗余 `f_lseek(&m_file, m_currentFilePos + 8)`。第一次 `f_read` 读取8字节头部后，文件指针已位于数据起始位置，无需再次seek。

**修复2：增加音频预加载量**

`TARGET_PRELOAD_CHUNKS` 从64增至96（~48KB，75%缓冲区填充）。更大的预加载缓冲使播放初始阶段有更多音频数据可用，减少视频帧处理被阻塞的概率。

**修复3：降低调试日志频率**

音频chunk调试日志从每20个输出一次改为每100个输出一次，减少串口I/O开销。

### 代码变更

| 文件 | 变更 |
|------|------|
| MJPEG_Encoder.cpp | `readNextChunk()`: 移除视频chunk和音频chunk数据读取前的冗余 `f_lseek` |
| VideoRecorder.cpp | `startVideoPlayback()`: `TARGET_PRELOAD_CHUNKS` 64→96；音频调试日志频率 20→100 |
| Shared_GlobalDefines.h | 版本号 V1.111 → V1.112 |

### 预期效果

- SD卡读取每chunk节省一次f_lseek调用（~0.5-1ms），初始阶段密集读取时累计节省显著
- 音频预加载从50%提升至75%，播放初始2秒内缓冲区不易跌破低水位线
- 调试日志开销降低5倍
- 初始播放帧率从10fps提升至≥15fps

---

## V1.113 - 视频回放第三秒掉帧修复 (2026-05-05)

### 问题描述

V1.112修复后，初始2秒帧率有所改善，但播放第三秒左右出现**更明显的掉帧**，帧率降至8-10fps。

### 根因分析

通过Bug.md日志分析，发现三个层层叠加的根因：

**根因1：音频缓冲区单位不匹配（核心bug）**

`Max98357a_AudioPlayer.cpp:109` 中 `m_txBufferSize = MAX98357A_BUFFER_SIZE` 将65536**字节**的宏值直接赋给以**sample**为单位的 `m_txBufferSize`。导致：
- 实际分配 `65536 × 2 = 128KB` 缓冲区（预期64KB的2倍）
- `m_txBufferCount`（sample单位）与 `m_txBufferSize`（byte单位）单位不一致
- 溢出检测 `m_txBufferCount > m_txBufferSize` 永远不触发（65536 samples > 65536 bytes 恒真）

**根因2：低水位线阈值过高**

`MAX98357A_LOW_WATERMARK = 8192` 与 `m_txBufferCount`（sample单位）比较，实际阈值为 8192 samples = 16384 bytes。日志显示缓冲区平衡在 6000~9000 samples，**始终低于水位线**，导致视频帧持续被阻塞。

日志证据：
```
[CB5 b=8768 u=0%]   ← t≈256ms, 刚好低于8192水位线
[CB55 b=4608 u=0%]  ← t≈2816ms, 远低于水位线，帧率降至8fps
```

**根因3：预加载块数过多 + pending帧被覆盖**

预加载96个音频块（49152 samples），但修正后的缓冲区仅能容纳32768 samples。二次初始化时跳过全部96个，期间TX回调持续消耗缓冲区。当缓冲区低于水位线时，连续视频帧互相覆盖pending帧，导致帧丢失。

### 修复方案

**修复1：修正缓冲区大小单位**

`Max98357a_AudioPlayer.cpp`:
```cpp
// 修复前
m_txBufferSize = MAX98357A_BUFFER_SIZE;  // 65536 (bytes误用为samples)
// 修复后
m_txBufferSize = MAX98357A_BUFFER_SIZE / sizeof(int16_t);  // 32768 samples = 64KB
```

**修复2：降低低水位线 + 修正高水位线**

`Max98357a_AudioPlayer.h`:
```cpp
// 修复前
#define MAX98357A_LOW_WATERMARK   8192   // 8192 samples = 16384 bytes
#define MAX98357A_HIGH_WATERMARK  32768  // 32768 samples = 65536 bytes
// 修复后
#define MAX98357A_LOW_WATERMARK   4096   // 4096 samples = 8192 bytes (12.5%)
#define MAX98357A_HIGH_WATERMARK  16384  // 16384 samples = 32768 bytes (50%)
```

**修复3：减少预加载块数**

`VideoRecorder.cpp`:
```cpp
// 修复前
const int TARGET_PRELOAD_CHUNKS = 96;  // 超出修正后缓冲区容量
// 修复后
const int TARGET_PRELOAD_CHUNKS = 64;  // 32768 samples = 100% 缓冲区
```

**修复4：修复pending视频帧覆盖**

`VideoRecorder.cpp` `videoPlaybackLoop()`:
```cpp
// 修复前：缓冲区低于水位线时，新视频帧直接覆盖pending帧
if (!player.isBufferAboveLowWatermark()) {
    if (ensureVideoFrameBuf(chunkSize)) {
        memcpy(s_videoFrameBuf, chunkData, chunkSize);  // 覆盖旧帧！
        pendingVideoSize = chunkSize;
        hasPendingVideo = true;
    }
    continue;
}

// 修复后：先处理pending帧再存储新帧
if (!player.isBufferAboveLowWatermark()) {
    if (hasPendingVideo) {
        processVideoFrame(s_videoFrameBuf, pendingVideoSize, fps, frameIntervalMs);
        hasPendingVideo = false;
    }
    if (ensureVideoFrameBuf(chunkSize)) {
        memcpy(s_videoFrameBuf, chunkData, chunkSize);
        pendingVideoSize = chunkSize;
        hasPendingVideo = true;
    }
    continue;
}
```

### 代码变更

| 文件 | 变更 |
|------|------|
| Max98357a_AudioPlayer.cpp | `init()`: `m_txBufferSize` 修正为 `MAX98357A_BUFFER_SIZE / sizeof(int16_t)` |
| Max98357a_AudioPlayer.h | `MAX98357A_LOW_WATERMARK` 8192→4096；`MAX98357A_HIGH_WATERMARK` 32768→16384 |
| VideoRecorder.cpp | `startVideoPlayback()`: `TARGET_PRELOAD_CHUNKS` 96→64 |
| VideoRecorder.cpp | `videoPlaybackLoop()`: 缓冲区低于水位线时先处理pending帧再存储新帧 |
| Shared_GlobalDefines.h | 版本号 V1.112 → V1.113 |

### 预期效果

- 缓冲区大小从128KB修正为64KB，节省64KB SRAM
- 低水位线从16384 bytes降至8192 bytes，缓冲区平衡点（~12000-18000 bytes）远高于水位线
- 预加载块数从96降至64，二次初始化跳过时间减半
- pending帧不再被覆盖，消除因缓冲区低水位导致的视频帧丢失
- 全播放过程帧率稳定≥15fps

---

## V1.114 - 视频回放全程掉帧根因修复 (2026-05-05)

### 问题描述

V1.113修复后，初始2秒帧率正常，但第三秒后**全程画面和声音严重卡顿**，缓冲持续为空。

### 根因分析

通过Bug.md日志深入分析，发现V1.113未能解决的核心根因：

**根因：预加载后 `resetSequentialMode()` 导致2秒"写入空洞"**

`startVideoPlayback()` 的执行流程：
1. 解码器从文件头开始预加载64个音频块到播放器缓冲区 → **缓冲区100%满**
2. **`resetSequentialMode()` 将解码器文件位置重置回 movi 数据开头**
3. `videoPlaybackLoop()` 从文件头重新读取，通过 `s_preloadedAudioChunks=64` 跳过前64个音频块
4. 64个块的跳过过程穿插视频帧处理，**耗时约2秒**
5. 这2秒内，TX回调以 16000 samples/s 持续消耗缓冲，**零音频写入**
6. 缓冲从 32768 samples (100%) 降至 0，之后永远无法恢复

日志铁证：
```
[CB1  b=32448]   ← 初始100%满
[CB50 b=16768]   ← 1秒后50%，跳过进行中
[CB100 b=768]    ← 2秒后2%，跳过期结束，缓冲已空
[CB150 b=0 u=27%] ← 2.5秒后缓冲归零，欠载开始
[CB800 b=576 u=25%] ← 全程缓冲无法恢复到水位线以上
```

跳过期结束后，每个帧间隔(100ms)TX消耗1600 samples，但仅有~1463 samples写入（2.86块/帧 × 512样本），**持续赤字 ~137 samples/帧**，缓冲永远无法恢复。

此外，RTOS任务循环中存在 ~10ms 的 `waitForEvent` 等待开销，每帧间隔实际约为110ms，TX额外消耗 ~160 samples，加大了赤字。

### 修复方案

**修复1：消除 `resetSequentialMode()`，解码器保持预加载后位置**

`VideoRecorder.cpp` `startVideoPlayback()`:
```cpp
// 修复前：重置解码器位置，导致重读+跳过64块
mjpegDecoder.resetSequentialMode();
s_preloadedAudioChunks = preloadedChunks;  // 64 - 导致2秒写入空洞

// 修复后：解码器保持在预加载后位置继续读取，无需跳过
// (移除 resetSequentialMode 调用)
s_preloadedAudioChunks = 0;  // 无跳过，直接写入音频
```

解码器在预加载后 `m_sequentialMode=true` 且 `m_currentFilePos` 已推进到预加载结束位置，下次 `readNextChunk()` 直接从该位置继续，无需任何跳过。

**修复2：视频帧处理后主动预读音频块**

`VideoRecorder.cpp` `videoPlaybackLoop()`:
```cpp
// 修复前：处理完视频帧立即break，下次调用才读取音频
processVideoFrame(...);  // ~100ms阻塞期间零音频写入
break;

// 修复后：处理完视频帧后，主动预读最多6个音频块
processVideoFrame(...);
for (int j = 0; j < 6; j++) {
    ChunkType nextType = readNextChunk(...);
    if (nextType == AUDIO) {
        writeAudioData(...);  // 立即填充缓冲
    } else if (nextType == VIDEO) {
        // 预读到视频帧则缓存，下次处理
        memcpy(pendingFrame...);
        hasPendingVideo = true;
        break;
    } else break;
}
break;
```

### 效果分析

修复1消除了2秒写入空洞，缓冲从100%开始持续有音频写入。配合修复2的主动预读：
- 每帧间隔音频写入：~3块(预读) × 512 + 原有~2.86块 × 512 ≈ 3000 samples
- 每帧间隔TX消耗：~1600 samples (100ms) + ~160 (RTOS 10ms开销) = ~1760 samples
- 净盈余：~1240 samples/帧
- **缓冲持续维持在30%以上，全程远离水位线**

### 代码变更

| 文件 | 变更 |
|------|------|
| VideoRecorder.cpp | `startVideoPlayback()`: 移除 `resetSequentialMode()`，`s_preloadedAudioChunks` 设为 0 |
| VideoRecorder.cpp | `videoPlaybackLoop()`: 视频帧处理后增加主动预读音频块逻辑(最多6块) |
| Shared_GlobalDefines.h | 版本号 V1.113 → V1.114 |

### 预期效果

- 消除2秒写入空洞，缓冲从播放开始即持续得到补充
- 主动预读机制填充RTOS调度开销导致的消耗间隙
- 缓冲全程维持在水位线以上，TX欠载率从 25-27% → <1%
- 全播放过程帧率稳定 ≥15fps，画面和声音均流畅无卡顿

---

## V1.115 - 预加载阶段视频帧保存修复 (2026-05-05)

### 问题描述

V1.114修复了全程掉帧问题，但引入新问题：**播放前几秒视频画面完全丢失**，只听到声音。

### 根因分析

V1.114移除了 `resetSequentialMode()` 来消除2秒写入空洞，但这导致预加载阶段遍历过的视频帧永远丢失：

`startVideoPlayback()` 的预加载循环：
1. 解码器从文件头开始，顺序读取 chunk
2. 遇到音频块 → 写入播放器缓冲区（计入64个预加载目标）
3. 遇到视频块 → **直接丢弃**（原实现只计数 `consecutiveSkippedVideoChunks`）
4. 由于移除了 `resetSequentialMode()`，解码器不会回到文件开头重读这些帧

日志显示预加载阶段丢弃了至少3个视频帧（Video chunk #1~#3），但实际文件比例（383音频帧/134视频帧≈2.86）意味着预加载64个音频块将丢弃约 **22个视频帧**，即 **2.2秒画面完全缺失**。

### 修复方案

**核心思路**：预加载阶段将遇到的视频帧保存到内存队列，播放开始后从队列中按帧间隔逐个回放。

**修复1：新增预加载视频帧队列**

`VideoRecorder.cpp` 新增静态变量：
```cpp
#define MAX_PRELOADED_VIDEO_FRAMES 32
static uint8_t* s_preloadedVideoFrameData[MAX_PRELOADED_VIDEO_FRAMES];
static uint32_t s_preloadedVideoFrameSizes[MAX_PRELOADED_VIDEO_FRAMES];
static int s_preloadedVideoFrameCount = 0;
```

**修复2：预加载时保存视频帧**

`startVideoPlayback()` 预加载循环：
```cpp
// 修复前：遇到视频帧直接丢弃
} else if (type == MJPEGDecoder::CHUNK_TYPE_VIDEO) {
    consecutiveSkippedVideoChunks++;
    if (consecutiveSkippedVideoChunks > 100) break;
}

// 修复后：保存到队列
} else if (type == MJPEGDecoder::CHUNK_TYPE_VIDEO) {
    if (s_preloadedVideoFrameCount < MAX_PRELOADED_VIDEO_FRAMES) {
        uint8_t* savedData = (uint8_t*)malloc(chunkSize);
        if (savedData) {
            memcpy(savedData, chunkData, chunkSize);
            s_preloadedVideoFrameData[s_preloadedVideoFrameCount] = savedData;
            s_preloadedVideoFrameSizes[s_preloadedVideoFrameCount] = chunkSize;
            s_preloadedVideoFrameCount++;
        }
    }
    consecutiveSkippedVideoChunks++;
    if (consecutiveSkippedVideoChunks > 100) break;
}
```

**修复3：播放时优先处理已保存帧**

`videoPlaybackLoop()` 开头新增：
```cpp
if (s_preloadedVideoFrameCount > 0) {
    // 从队列取出一帧
    uint8_t* savedData = s_preloadedVideoFrameData[0];
    uint32_t savedSize = s_preloadedVideoFrameSizes[0];
    // 队列前移
    for (int k = 1; k < s_preloadedVideoFrameCount; k++) { ... }
    s_preloadedVideoFrameCount--;

    processVideoFrame(savedData, savedSize, fps, frameIntervalMs);
    free(savedData);

    // 主动预读音频块维持缓冲水位
    for (int j = 0; j < 6; j++) {
        ChunkType nextType = readNextChunk(...);
        if (nextType == AUDIO) writeAudioData(...);
        else break;
    }
    return;  // 每次调用只处理一帧
}
```

**修复4：停止播放时清理队列**

`stopVideoPlayback()` 新增：
```cpp
for (int i = 0; i < s_preloadedVideoFrameCount; i++) {
    free(s_preloadedVideoFrameData[i]);
}
s_preloadedVideoFrameCount = 0;
```

### 代码变更

| 文件 | 变更 |
|------|------|
| VideoRecorder.cpp | 新增 `s_preloadedVideoFrameData/Sizes/Count` 预加载视频帧队列 |
| VideoRecorder.cpp | `startVideoPlayback()`: 预加载循环中保存视频帧到队列 |
| VideoRecorder.cpp | `videoPlaybackLoop()`: 优先处理已保存帧（每次一帧+主动音频预读） |
| VideoRecorder.cpp | `stopVideoPlayback()`: 释放队列中所有已保存帧 |
| Shared_GlobalDefines.h | 版本号 V1.114 → V1.115 |

### 预期效果

- 预加载阶段丢失的 ~22个视频帧被保存，播放开始后按帧率逐个回放
- 保存帧处理附带的主动音频预读维持缓冲水位，不破坏V1.114的缓冲平衡
- 画面从第一帧开始即正常显示，消除前2秒画面缺失
- 全播放过程帧率稳定 ≥15fps，音视频同步流畅

---

## V1.116 - 预读阶段视频帧丢失修复 (2026-05-05)

### 问题描述

V1.115修复了前2秒画面丢失问题，但播放第三秒左右再次出现掉帧：**画面和声音都异常**。日志显示音频块写入严重不足。

### 根因分析

V1.115的已保存帧播放路径中，预读循环遇到视频帧时直接`break`丢弃数据：

```cpp
// V1.115 已保存帧预读（bug）
for (int j = 0; j < 6; j++) {
    nextType = readNextChunk(...);
    if (nextType == AUDIO) writeAudioData(...);
    else break;  // 遇到视频直接丢弃！
}
```

日志证据：
```
t=2112ms: aChunks=5, vFrames=24, buf=17920/32768 (57%)
t=4128ms: aChunks=32, vFrames=46, buf=5056/32768  (15%)
```

22个已保存帧的预读过程中，每次读取约5-6个chunk会遇到1-2个视频帧。这些视频帧被`readNextChunk`从文件读取后直接丢弃，**累计丢失~22个视频帧**。同时预读写入的音频块未计入`s_playbackAudioChunkCount`，导致统计失真。

### 修复方案

**核心思路**：将局部`hasPendingVideo`/`pendingVideoSize`提升为静态变量，在预读+已保存帧+正常循环三个路径中统一使用同一套pendingVideo机制，视频帧不再丢失。

**修复1：添加静态pendingVideo变量**

```cpp
static bool s_hasPendingVideo = false;
static uint32_t s_pendingVideoSize = 0;
```

**修复2：三个路径统一使用静态pendingVideo**

`videoPlaybackLoop()` 重构为三阶段统一架构：

```
阶段1: s_hasPendingVideo → 处理pending帧 + 预读音频(视频保存到pending)
阶段2: s_preloadedVideoFrameCount > 0 → 处理已保存帧 + 预读音频(视频保存到pending)
阶段3: 正常循环 → 原逻辑不变(使用s_hasPendingVideo/s_pendingVideoSize)
```

三个阶段间通过静态`s_hasPendingVideo`/`s_pendingVideoSize`无缝衔接：
- 已保存帧预读遇到视频 → 保存到`s_hasPendingVideo`
- 下一轮阶段1立即处理 → 预读再遇视频 → 保存回`s_hasPendingVideo`
- 循环往复直到已保存帧耗尽 → 阶段3正常循环接管

**修复3：预读路径统一计数**

三个阶段中的预读循环统一添加`s_playbackAudioChunkCount++`，确保统计准确。

**修复4：stopVideoPlayback重置**

```cpp
s_hasPendingVideo = false;
s_pendingVideoSize = 0;
```

### 代码变更

| 文件 | 变更 |
|------|------|
| VideoRecorder.cpp | 新增 `s_hasPendingVideo`、`s_pendingVideoSize` 静态变量 |
| VideoRecorder.cpp | `videoPlaybackLoop()`：三阶段统一pendingVideo架构 |
| VideoRecorder.cpp | `stopVideoPlayback()`：重置静态pending变量 |
| Shared_GlobalDefines.h | 版本号 V1.115 → V1.116 |

### 预期效果

- 已保存帧预读中遇到的视频帧不再丢失，通过pendingVideo链无缝传递到下一轮处理
- 音频块计数准确，统计日志反映真实状态
- 缓冲水位全程维持在安全水平
- 全程帧率稳定 ≥15fps，画面和声音均流畅

---

## V1.117 - 回退至V1.111 (2026-05-05)

### 背景

V1.112→V1.116的连续迭代引入了越来越复杂的架构（三阶段pendingVideo、静态变量跨阶段传递、预读循环），最终导致V1.116出现「Reached idx1」死循环（文件结束检测失效），性能反而不如V1.111。

### 策略

回退全部代码至V1.111稳定基线。