# INMP441麦克风驱动技术文档

## 1 概述

本文档详细描述了基于Ameba系列芯片（RTL8735B/AmebaPro2）的INMP441麦克风驱动实现方案。INMP441是一款高性能数字MEMS麦克风，通过I2S接口与主控芯片进行音频数据传输。本文档分析MMF2（Multi-Media Framework 2）框架下的I2S音频初始化示例代码，涵盖I2S接口配置、音频数据采集流程、中断处理机制及系统初始化流程。

**参考文件列表：**

- [mmf2_example_i2s_audio_init.c](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\example\media_framework\mmf2_example_i2s_audio_init.c)
- [example_media_framework.c](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\example\media_framework\example_media_framework.c)
- [example_media_framework.h](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\example\media_framework\example_media_framework.h)
- [readme.txt](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\example\media_framework\readme.txt)
- [module_i2s.c](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\media\mmfv2\module_i2s.c)
- [module_i2s.h](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\media\mmfv2\module_i2s.h)
- [module_audio.c](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\media\mmfv2\module_audio.c)
- [module_audio.h](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\media\mmfv2\module_audio.h)
- [mmf2_siso.h](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\media\mmfv2\mmf2_siso.h)
- [i2s_api.h](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\mbed\hal_ext\i2s_api.h)

---

## 2 系统架构概述

### 2.1 MMF2框架架构

MMF2（Multi-Media Framework 2）是Realtek Ameba系列芯片的多媒体处理框架，采用模块化设计，将音频/视频处理功能封装为独立模块，通过统一接口进行模块间的数据流连接。

```
┌─────────────────────────────────────────────────────────────────┐
│                     MMF2 Framework                              │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐          │
│  │  I2S模块    │───▶│  Audio模块  │───▶│  输出模块   │          │
│  │ (I2S Module)│◀───│(Audio Module)│◀───│(Output Sink)│          │
│  └─────────────┘    └─────────────┘    └─────────────┘          │
│         │                  │                                    │
│    ┌────▼────┐        ┌────▼────┐                              │
│    │ DMA缓冲区 │        │ 音频处理 │                              │
│    │(4页)    │        │(AEC/NS) │                              │
│    └─────────┘        └─────────┘                              │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 核心组件

| 组件 | 文件 | 功能描述 |
|------|------|----------|
| I2S模块 | module_i2s.c/h | I2S接口驱动，负责与INMP441通信 |
| Audio模块 | module_audio.c/h | 音频数据处理，支持AEC/NS/AGC |
| SISO连接器 | mmf2_siso.h | 单输入单输出模块串联 |
| 底层I2S驱动 | i2s_api.h | HAL层I2S接口定义 |

---

## 3 示例代码分析

### 3.1 主函数调用流程

示例代码的入口函数为`mmf2_example_i2s_audio_init()`，位于[mmf2_example_i2s_audio_init.c](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\example\media_framework\mmf2_example_i2s_audio_init.c#L18-L77)。该函数通过系统任务调度器调用，典型调用路径如下：

```
example_media_framework()           // 入口函数
    └─> xTaskCreate()              // 创建MMF2主任务
            └─> example_mmf2_signal_stream_main()
                    └─> example_mmf2_audio_only()
                            └─> mmf2_example_i2s_audio_init()  // I2S音频初始化
```

### 3.2 核心数据结构

#### 3.2.1 I2S参数结构体 (i2s_params_t)

定义于[module_i2s.h#L55-L75](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\media\mmfv2\module_i2s.h#L55-L75)：

```c
typedef struct i2s_param_s {
    uint32_t            sample_rate;      // 采样率: SR_8KHZ~SR_96KHZ
    uint8_t             i2s_word_length;  // I2S字长: 16/24/32位
    uint8_t             rx_word_length;   // 接收字长
    uint8_t             tx_word_length;   // 发送字长
    i2s_format          i2s_format;       // I2S格式: FORMAT_I2S/FORMAT_LEFT_JUST/FORMAT_RIGHT_JUST
    i2s_ms_mode         i2s_role;         // 主从模式: I2S_MASTER/I2S_SLAVE
    i2s_ws_trig_edge   i2s_ws_edge;      // WS边沿触发
    i2s_edge_sw        i2s_data_edge;    // 数据边沿
    uint8_t             i2s_channel;      // 通道数: CH_MONO/CH_STEREO
    i2s_channel_type   rx_channel;       // 接收通道: I2S_LEFT_CHANNEL/I2S_RIGHT_CHANNEL/I2S_STEREO_CHANNEL
    i2s_channel_type   tx_channel;       // 发送通道
    i2s_direction_type i2s_direction;    // 传输方向: I2S_RX_ONLY/I2S_TX_ONLY/I2S_TRX_BOTH
    uint8_t             rx_byte_swap;    // 接收字节交换
    uint8_t             tx_byte_swap;    // 发送字节交换
    uint8_t             pin_group_num;   // 引脚组编号
} i2s_params_t;
```

#### 3.2.2 音频参数结构体 (audio_params_t)

定义于[module_audio.h#L70-L100](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\media\mmfv2\module_audio.h#L70-L100)：

```c
typedef struct audio_param_s {
    audio_sr            sample_rate;     // 音频采样率: ASR_8KHZ~ASR_48KHZ
    audio_wl            word_length;     // 字长: WL_16BIT
    audio_mic_gain      mic_gain;        // 麦克风增益
    audio_dmic_gain     dmic_l_gain;     // 数字MIC左通道增益
    audio_dmic_gain     dmic_r_gain;     // 数字MIC右通道增益
    int                 channel;          // 通道数
    int                 mix_mode;         // 混音模式
    uint8_t             use_mic_type;     // 麦克风类型: 0=AMIC, 1=DMIC
    int                 mic_bias;          // 麦克风偏置电压
    int                 hpf_set;           // 高通滤波器设置
    // ... EQ/AGC配置
    int                 enable_record;    // 录音使能
    uint8_t             avsync_en;        // 音视频同步使能
} audio_params_t;
```

### 3.3 初始化流程详解

#### 步骤1：I2S模块初始化

```c
i2s_ctx = mm_module_open(&i2s_module);  // 打开I2S模块
```

此步骤完成以下操作：
- 创建I2S模块实例
- 分配I2S上下文结构体（i2s_ctx_t）
- 初始化默认参数

#### 步骤2：配置I2S参数

```c
mm_module_ctrl(i2s_ctx, CMD_I2S_GET_PARAMS, (int)&i2s_params);
i2s_params.sample_rate = SR_16KHZ;              // 16kHz采样率
i2s_params.i2s_direction = I2S_TRX_BOTH;         // 双向传输
mm_module_ctrl(i2s_ctx, CMD_I2S_SET_PARAMS, (int)&i2s_params);
```

**关键配置参数说明：**

| 参数 | 设置值 | 说明 |
|------|--------|------|
| sample_rate | SR_16KHZ | 16kHz采样率，适合语音应用 |
| i2s_direction | I2S_TRX_BOTH | 同时支持收发，用于回声消除场景 |
| i2s_format | FORMAT_I2S | 标准I2S格式（默认） |
| i2s_role | I2S_MASTER | 主模式，Ameba作为主机 |
| i2s_word_length | 16 | 16位字长（默认） |
| rx_channel | I2S_LEFT_CHANNEL | 接收左通道 |

#### 步骤3：配置队列和初始化

```c
mm_module_ctrl(i2s_ctx, MM_CMD_SET_QUEUE_LEN, 120);           // 队列深度120
mm_module_ctrl(i2s_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC); // 静态队列
mm_module_ctrl(i2s_ctx, CMD_I2S_APPLY, 0);                     // 应用配置
```

队列配置说明：
- 队列长度120：用于缓存音频帧
- 静态队列：使用静态内存分配，避免动态分配开销

#### 步骤4：Audio模块初始化

Audio模块的初始化流程与I2S模块类似：

```c
audio_ctx = mm_module_open(&audio_module);      // 打开音频模块
mm_module_ctrl(audio_ctx, CMD_AUDIO_GET_PARAMS, (int)&audio_params);
audio_params.sample_rate = ASR_16KHZ;            // 16kHz采样率
mm_module_ctrl(audio_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_params);
mm_module_ctrl(audio_ctx, MM_CMD_SET_QUEUE_LEN, 6);
mm_module_ctrl(audio_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
mm_module_ctrl(audio_ctx, CMD_AUDIO_APPLY, 0);
```

#### 步骤5：SISO连接建立

创建两个单向数据流连接：

```c
// I2S -> Audio 数据流
siso_i2s_audio = siso_create();
siso_ctrl(siso_i2s_audio, MMIC_CMD_ADD_INPUT, (uint32_t)i2s_ctx, 0);
siso_ctrl(siso_i2s_audio, MMIC_CMD_ADD_OUTPUT, (uint32_t)audio_ctx, 0);
siso_start(siso_i2s_audio);

// Audio -> I2S 数据流（回声消除需要）
siso_audio_i2s = siso_create();
siso_ctrl(siso_audio_i2s, MMIC_CMD_ADD_INPUT, (uint32_t)audio_ctx, 0);
siso_ctrl(siso_audio_i2s, MMIC_CMD_ADD_OUTPUT, (uint32_t)i2s_ctx, 0);
siso_start(siso_audio_i2s);
```

---

## 4 I2S底层驱动分析

### 4.1 DMA缓冲区配置

I2S模块使用4页DMA缓冲区，每页大小为1280字节（640×2）：

```c
#define I2S_DMA_PAGE_NUM   4
#define I2S_DMA_PAGE_SIZE (640 * 2)  // 1280 bytes
```

**引脚配置（AmebaPro2）：**

| 信号 | 引脚组0 | 引脚组1 |
|------|---------|---------|
| SCLK | PD_14 | PF_13 |
| WS | PD_17 | PF_15 |
| TX | PD_15 | PF_14 |
| RX | PD_18 | PF_12 |
| MCK | PD_16 | PF_11 |

### 4.2 中断处理机制

I2S模块使用两个DMA中断处理程序：

#### 4.2.1 接收中断处理 (i2s_rx_complete)

位于[module_i2s.c#L142-L162](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\media\mmfv2\module_i2s.c#L142-L162)：

```c
static void i2s_rx_complete(uint32_t arg, uint8_t *pbuf)
{
    i2s_ctx_t *ctx = (i2s_ctx_t *)arg;
    // 1. 获取硬件时间戳
    uint32_t rx_ts = mm_read_mediatime_ms_fromisr();
    
    // 2. 计算帧起始时间戳
    int frame_bytes = ((ctx->i2s_word_length + 8) / 16) * 2;
    if (ctx->i2s_channel == CH_STEREO) frame_bytes *= 2;
    rx_ts -= 1000 * (I2S_DMA_PAGE_SIZE / frame_bytes) / ctx->sample_rate;
    
    // 3. 复制数据到接收缓冲区
    memcpy(ctx->i2s_rx_cache.rxbuf.data, pbuf, I2S_DMA_PAGE_SIZE);
    ctx->i2s_rx_cache.rxbuf.hw_timestamp = rx_ts;
    ctx->i2s_rx_cache.rxbuf.timestamp = rx_ts + ctx->i2s_timestamp_offset;
    
    // 4. 发送到消息队列
    xQueueSendFromISR(ctx->i2s_rx_cache.queue, &(ctx->i2s_rx_cache.rxbuf), &xHigherPriorityTaskWoken);
    
    // 5. 继续接收下一页
    i2s_recv_page(obj);
}
```

#### 4.2.2 发送中断处理 (i2s_tx_complete)

位于[module_i2s.c#L130-L141](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\media\mmfv2\module_i2s.c#L130-L141)：

```c
static void i2s_tx_complete(uint32_t arg, uint8_t *pbuf)
{
    i2s_ctx_t *ctx = (i2s_ctx_t *)arg;
    uint8_t *ptx_buf = (uint8_t *)i2s_get_tx_page(obj);
    
    // 从队列获取待发送数据，若队列为空则发送静音
    if (xQueueReceiveFromISR(ctx->i2s_tx_cache.queue, 
                              ctx->i2s_tx_cache.txbuf, NULL) != pdPASS) {
        memset(ptx_buf, 0, I2S_DMA_PAGE_SIZE);  // 静音
    } else {
        memcpy(ptx_buf, ctx->i2s_tx_cache.txbuf, I2S_DMA_PAGE_SIZE);
    }
    i2s_send_page(obj, (uint32_t *)ptx_buf);
}
```

### 4.3 接收数据处理线程

```c
static void i2s_rx_handle_thread(void *param)
{
    i2s_ctx_t *ctx = (i2s_ctx_t *)param;
    mm_context_t *mctx = (mm_context_t *)ctx->parent;
    
    while (1) {
        // 1. 等待DMA队列数据
        if (xQueueReceive(ctx->i2s_rx_cache.queue, 
                         &(ctx->i2s_rx_cache.buffer), 40) != pdTRUE) {
            continue;
        }
        
        // 2. 获取输出缓冲区
        if (xQueueReceive(mctx->output_recycle, &output_item, 0xFFFFFFFF) == pdTRUE) {
            // 3. 数据格式转换
            convert_rx_data(...);
            
            // 4. 设置元数据
            output_item->timestamp = ctx->i2s_rx_cache.buffer.timestamp;
            output_item->hw_timestamp = ctx->i2s_rx_cache.buffer.hw_timestamp;
            output_item->type = AV_CODEC_ID_PCM_RAW;
            
            // 5. 发送到输出队列
            xQueueSend(mctx->output_ready, &output_item, 0xFFFFFFFF);
        }
    }
}
```

### 4.4 数据格式转换

I2S模块支持多种字长和通道配置组合，自动进行数据格式转换：

```c
// 支持的转换组合
// I2S字长  -> 输出字长
// 32位    -> 32/24/16位
// 24位    -> 32/24/16位  
// 16位    -> 32/24/16位

// 支持的通道转换
// I2S立体声 -> Mono Left/Right
// I2S Mono  -> Stereo
```

---

## 5 音频数据流向

### 5.1 完整数据流

```
INMP441 MEMS Mic
      │
      ▼ (I2S总线)
┌─────────────────┐
│   I2S DMA       │ ◀── 4页循环缓冲区
│   RX中断        │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ I2S接收处理线程  │ ◀── 数据格式转换(32→16bit)
│ i2s_rx_handle  │
└────────┬────────┘
         │ mm_queue_item (PCM原始数据)
         ▼
┌─────────────────┐
│  SISO连接器     │ ◀── I2S→Audio
│ siso_i2s_audio │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Audio模块      │ ◀── AEC/NS/AGC处理
│  (回声消除)     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  输出队列       │ ◀── RTP/RTSP/MP4等
│ output_ready    │
└─────────────────┘
```

### 5.2 回声消除路径

对于需要回声消除（AEC）的应用，还存在反向数据流：

```
┌─────────────────┐
│  扬声器输出     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  SISO连接器     │ ◀── Audio→I2S
│ siso_audio_i2s │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   I2S DMA       │
│   TX中断        │
└────────┬────────┘
         │
         ▼
      扬声器 (或忽略)
```

---

## 6 关键函数调用关系

### 6.1 函数调用树

```
mmf2_example_i2s_audio_init()
├── mm_module_open(&i2s_module)
│   └── i2s_create()
│       └── vQueueAddToRegistry()
├── mm_module_ctrl(i2s_ctx, CMD_I2S_GET_PARAMS, ...)
├── mm_module_ctrl(i2s_ctx, CMD_I2S_SET_PARAMS, ...)
├── mm_module_ctrl(i2s_ctx, MM_CMD_SET_QUEUE_LEN, 120)
├── mm_module_ctrl(i2s_ctx, MM_CMD_INIT_QUEUE_ITEMS, ...)
├── mm_module_ctrl(i2s_ctx, CMD_I2S_APPLY, 0)
│   └── set_i2s_module_init()
│       ├── i2s_init()
│       ├── i2s_set_dma_buffer()
│       ├── i2s_rx_irq_handler()
│       ├── i2s_tx_irq_handler()
│       └── i2s_enable()
│
├── mm_module_open(&audio_module)
│   └── audio_create()
├── mm_module_ctrl(audio_ctx, CMD_AUDIO_GET_PARAMS, ...)
├── mm_module_ctrl(audio_ctx, CMD_AUDIO_SET_PARAMS, ...)
├── mm_module_ctrl(audio_ctx, MM_CMD_SET_QUEUE_LEN, 6)
├── mm_module_ctrl(audio_ctx, MM_CMD_INIT_QUEUE_ITEMS, ...)
├── mm_module_ctrl(audio_ctx, CMD_AUDIO_APPLY, 0)
│
├── siso_create()
│   └── siso_start()
│       └── xTaskCreate() [i2s_rx_handle_thread]
├── siso_ctrl(siso_i2s_audio, MMIC_CMD_ADD_INPUT, ...)
├── siso_ctrl(siso_i2s_audio, MMIC_CMD_ADD_OUTPUT, ...)
├── siso_start(siso_i2s_audio)
│
├── siso_create()
├── siso_ctrl(siso_audio_i2s, MMIC_CMD_ADD_INPUT, ...)
├── siso_ctrl(siso_audio_i2s, MMIC_CMD_ADD_OUTPUT, ...)
└── siso_start(siso_audio_i2s)
```

### 6.2 I2S模块控制命令

| 命令 | 值 | 功能描述 |
|------|-----|----------|
| CMD_I2S_SET_PARAMS | 0x00 | 设置I2S参数 |
| CMD_I2S_GET_PARAMS | 0x01 | 获取I2S参数 |
| CMD_I2S_SET_SAMPLERATE | 0x02 | 设置采样率 |
| CMD_I2S_SET_TRX | 0x03 | 设置传输方向 |
| CMD_I2S_SET_FORMAT | 0x08 | 设置I2S格式 |
| CMD_I2S_SET_ROLE | 0x09 | 设置主从模式 |
| CMD_I2S_SET_DATA_EDGE | 0x0A | 设置数据边沿 |
| CMD_I2S_SET_WS_EDGE | 0x0B | 设置WS边沿 |
| CMD_I2S_APPLY | 0x20 | 应用配置并初始化硬件 |

---

## 7 INMP441硬件连接

### 7.1 INMP441特性

| 参数 | 规格 |
|------|------|
| 接口 | I2S (Philips标准) |
| 采样率 | 最高48kHz |
| 位深 | 24位 |
| 功耗 | 600µA |
| 灵敏度 | -26 dBFS |
| 声学过载点 | 120 dB SPL |

### 7.2 引脚连接

```
INMP441                  Ameba (RTL8735B)
┌─────────────────┐      ┌─────────────────┐
│ VDD             │─────▶│ 3.3V            │
│ GND             │─────▶│ GND             │
│ SCK (I2S SCK)   │─────▶│ I2S_SCLK        │
│ WS  (I2S L/R)   │─────▶│ I2S_WS          │
│ SD  (I2S Data)  │─────▶│ I2S_RX          │
│ L/R  (Channel)  │─────▶│ GND (Left)      │
└─────────────────┘      └─────────────────┘

注意: INMP441的L/R引脚接地选择左声道，接3.3V选择右声道
```

---

## 8 配置参数汇总

### 8.1 I2S模块配置

| 参数 | 示例值 | 说明 |
|------|--------|------|
| sample_rate | SR_16KHZ | 16kHz采样率 |
| i2s_direction | I2S_TRX_BOTH | 双向传输 |
| i2s_word_length | 16 | 16位字长 |
| rx_word_length | 16 | 接收字长 |
| tx_word_length | 16 | 发送字长 |
| i2s_format | FORMAT_I2S | 标准I2S格式 |
| i2s_role | I2S_MASTER | 主机模式 |
| rx_channel | I2S_LEFT_CHANNEL | 左声道接收 |
| pin_group_num | 0 | 引脚组0 |

### 8.2 Audio模块配置

| 参数 | 示例值 | 说明 |
|------|--------|------|
| sample_rate | ASR_16KHZ | 16kHz采样率 |
| word_length | WL_16BIT | 16位字长 |
| channel | 1 | 单通道 |
| enable_record | 1 | 使能录音 |

---

## 9 使用说明

### 9.1 启用示例

1. 在`platform_opts.h`中设置：
   ```c
   #define CONFIG_EXAMPLE_MEDIA_FRAMEWORK 1
   ```

2. 在[example_media_framework.c](file:///d:\WorkRoom\BW21-CBV-Kit\ameba-rtos-pro2-main\component\example\media_framework\example_media_framework.c#L66)中取消注释：
   ```c
   void example_mmf2_audio_only(void)
   {
       // 取消注释以下行
       mmf2_example_i2s_audio_init();
   }
   ```

3. 编译并下载到开发板

### 9.2 注意事项

- **采样率匹配**：I2S模块和Audio模块的采样率必须一致（均为16kHz）
- **队列长度**：I2S队列深度为120，Audio队列深度为6，需根据应用调整
- **PSRAM需求**：如使用AAC编解码，需要使能PSRAM
- **引脚配置**：确保硬件连接的引脚与代码中的引脚组配置一致

---

## 10 文档版本

| 版本 | 日期 | 作者 | 修改内容 |
|------|------|------|----------|
| 1.0 | 2026-02-28 | MiniMax-M2.5 | 初始版本 |

---

*本文档基于Ameba RTOS Pro2 SDK分析编写，适用于RTL8735B/AmebaPro2系列芯片。*
