# I2S（Inter-IC Sound）时序技术文档

> **芯片平台**: Realtek RTL8735B (AmebaPro2)  
> **SDK版本**: 4.0.9-build20250528  
> **文档生成日期**: 2026-04-27

---

## 📁 I2S相关源文件列表

| 文件路径 | 说明 |
|---------|------|
| `system/component/soc/8735b/fwlib/rtl8735b/include/rtl8735b_i2s.h` | I2S HAL层定义和枚举 |
| `system/component/soc/8735b/fwlib/rtl8735b/include/rtl8735b_i2s_type.h` | I2S寄存器定义 |
| `system/component/soc/8735b/fwlib/rtl8735b/include/hal_i2s.h` | I2S HAL API实现 |
| `system/component/mbed/hal_ext/i2s_api.h` | mbed扩展I2S API |
| `system/component/media/mmfv2/module_i2s.h` | 多媒体框架I2S模块 |
| `system/component/media/framework/mmf_source_modules/mmf_source_i2s.h` | I2S源模块 |
| `system/component/media/framework/mmf_sink_modules/mmf_sink_i2s.h` | I2S接收模块 |

---

## 1. 采样率配置（Sample Rate）

**定义位置**: `rtl8735b_i2s.h:L183-L201`

```c
enum i2s_sample_rate_e {
    I2S_SR_8KHZ     =   0x00,   // 电话语音质量
    I2S_SR_12KHZ    =   0x01,
    I2S_SR_16KHZ    =   0x02,   // 宽带语音
    I2S_SR_24KHZ    =   0x03,   // 专业音频
    I2S_SR_32KHZ    =   0x04,   // 广播质量
    I2S_SR_48KHZ    =   0x05,   // DVD质量
    I2S_SR_64KHZ    =   0x06,
    I2S_SR_96KHZ    =   0x07,   // 高解析度
    I2S_SR_192KHZ   =   0x08,   // 超高解析度
    I2S_SR_384KHZ   =   0x09,
    I2S_SR_7p35KHZ  =   0x10,
    I2S_SR_11p025KHZ =  0x11,
    I2S_SR_14p7KHZ  =   0x12,
    I2S_SR_22p05KHZ =   0x13,   // 半CD质量
    I2S_SR_29p4KHZ  =   0x14,
    I2S_SR_44p1KHZ  =   0x15,   // CD质量
    I2S_SR_58p8KHZ  =   0x16,
    I2S_SR_88p2KHZ  =   0x17,   // 高解析度
    I2S_SR_176p4KHZ =   0x18    // 超高解析度
};
```

### 常用采样率速查表

| 采样率 | 枚举值 | 十六进制 | 典型应用 |
|--------|--------|---------|---------|
| 8 kHz | I2S_SR_8KHZ | 0x00 | 电话语音 |
| 16 kHz | I2S_SR_16KHZ | 0x02 | 宽带语音 |
| 22.05 kHz | I2S_SR_22p05KHZ | 0x13 | 半CD质量 |
| 24 kHz | I2S_SR_24KHZ | 0x03 | 专业音频 |
| 32 kHz | I2S_SR_32KHZ | 0x04 | 广播 |
| 44.1 kHz | I2S_SR_44p1KHZ | 0x15 | CD音频 |
| 48 kHz | I2S_SR_48KHZ | 0x05 | DVD/视频 |
| 88.2 kHz | I2S_SR_88p2KHZ | 0x17 | 高解析度 |
| 96 kHz | I2S_SR_96KHZ | 0x07 | 高解析度 |
| 176.4 kHz | I2S_SR_176p4KHZ | 0x18 | 超高解析度 |
| 192 kHz | I2S_SR_192KHZ | 0x08 | 超高解析度 |

---

## 2. 字长配置（Word Length）

**定义位置**: `rtl8735b_i2s.h:L68-L73`

```c
enum i2s_word_len_e {
    I2S_WL_16 = 0x0,    // 16位字长
    I2S_WL_24 = 0x1,    // 24位字长
    I2S_WL_32 = 0x2     // 32位字长
};
```

---

## 3. I2S接口格式（Interface Format）

**定义位置**: `rtl8735b_i2s.h:L128-L133`

```c
enum i2s_format_e {
    I2S_FORMAT_I2S          =   0x0,    // 标准I2S格式
    I2S_FORMAT_LEFT_JUST    =   0x1,    // 左对齐格式
    I2S_FORMAT_RIGHT_JUST   =   0x2     // 右对齐格式
};
```

### 格式说明

| 格式 | 说明 | WS与数据关系 |
|------|------|-------------|
| I2S | 标准I2S | 数据延迟WS一个BCLK周期 |
| LEFT_JUST | 左对齐 | 数据与WS同步开始 |
| RIGHT_JUST | 右对齐 | 数据在WS结束前对齐 |

---

## 4. 主从模式（Master/Slave Mode）

**定义位置**: `rtl8735b_i2s.h:L58-L63`

```c
enum i2s_dev_mode_e {
    I2S_MASTER_MODE =   0x0,    // 主模式：设备生成SCK和WS时钟
    I2S_SLAVE_MODE  =   0x1     // 从模式：设备接收外部时钟
};
```

### 模式对比

| 特性 | 主模式 | 从模式 |
|------|--------|--------|
| SCK生成 | 内部生成 | 外部输入 |
| WS生成 | 内部生成 | 外部输入 |
| 采样率控制 | 自主控制 | 跟随外部设备 |
| 应用场景 | 作为时钟源 | 同步外部音频设备 |

---

## 5. 时钟同步机制

### 5.1 SCK（Serial Clock）配置

**寄存器定义**: `rtl8735b_i2s_type.h:L13`

```c
#define I2S_SHIFT_SCK_SWAP    11
#define I2S_BIT_SCK_SWAP      ((u32)0x00000001 << 11)    // SCK反相控制
```

**边沿选择枚举**: `rtl8735b_i2s.h:L153-L158`

```c
enum i2s_edge_sw_e {
    I2S_NEGATIVE_EDGE   =   0x00,    // 负边沿发送数据
    I2S_POSITIVE_EDGE   =   0x01     // 正边沿发送数据
};
```

### 5.2 时钟切换控制

**定义位置**: `rtl8735b_i2s_type.h:L45`

```c
#define I2S_SHIFT_CLK_SWITCH    18
#define I2S_BIT_CLK_SWITCH      ((u32)0x00000001 << 18)    // 时钟切换
```

### 5.3 时钟频率计算

**位时钟（BCLK）计算公式**:
```
BCLK = 采样率 × 字长 × 通道数
```

**示例计算**:

| 配置 | BCLK计算 | 结果 |
|------|---------|------|
| 48kHz, 16位, 立体声 | 48000 × 16 × 2 | 1.536 MHz |
| 44.1kHz, 24位, 立体声 | 44100 × 24 × 2 | 2.1168 MHz |
| 96kHz, 32位, 立体声 | 96000 × 32 × 2 | 6.144 MHz |

---

## 6. 数据帧结构

### 6.1 控制寄存器（I2S_CTRL）

**定义位置**: `rtl8735b_i2s_type.h:L1-L35`

| 位域 | 名称 | 位移 | 掩码 | 功能说明 |
|------|------|------|------|---------|
| Bit 31 | SW_RSTN | 31 | 0x01 | 软件复位（1=正常，0=复位） |
| Bit 29-30 | WL | 29 | 0x03 | 字长选择（00=16位，01=24位，10=32位） |
| Bit 28 | SLAVE_MODE | 28 | 0x01 | 主从模式选择（0=主，1=从） |
| Bit 27 | MUTE | 27 | 0x01 | 静音控制（1=静音） |
| Bit 18-22 | BURST_SIZE | 18 | 0x1F | DMA突发大小 |
| Bit 15-16 | DEBUG_SWITCH | 15 | 0x03 | 调试切换 |
| Bit 12 | BYTE_SWAP | 12 | 0x01 | 字节交换（大小端转换） |
| Bit 11 | SCK_SWAP | 11 | 0x01 | SCK时钟反相 |
| Bit 10 | LR_SWAP | 10 | 0x01 | 左右声道交换 |
| Bit 8-9 | FORMAT | 8 | 0x03 | 接口格式（00=I2S，01=左对齐，10=右对齐） |
| Bit 7 | LOOP_BACK | 7 | 0x01 | 回环模式 |
| Bit 5 | EDGE_SW | 5 | 0x01 | 数据边沿选择（0=负边沿，1=正边沿） |
| Bit 3-4 | AUDIO_MONO | 3 | 0x03 | 声道配置（00=立体声，01=5.1，10=单声道） |
| Bit 1-2 | TX_ACT | 1 | 0x03 | 发送方向控制 |
| Bit 0 | IIS_EN | 0 | 0x01 | I2S使能（1=使能） |

### 6.2 页面大小和采样率寄存器（I2S_PAGE_SIZE_RATE）

**定义位置**: `rtl8735b_i2s_type.h:L44-L52`

| 位域 | 名称 | 位移 | 掩码 | 功能说明 |
|------|------|------|------|---------|
| Bit 18 | CLK_SWITCH | 18 | 0x01 | 时钟切换控制 |
| Bit 14-17 | SR | 14 | 0x0F | 采样率选择（4位，支持19种采样率） |
| Bit 12-13 | PAGE_NUM | 12 | 0x03 | DMA页面数量（0=无用，1=2页，2=3页，3=4页） |
| Bit 0-11 | PAGE_SIZE | 0 | 0xFFF | 页面大小（1-4096字） |

---

## 7. 通道配置

**定义位置**: `rtl8735b_i2s.h:L83-L89`

```c
enum i2s_ch_num_e {
    I2S_CH_STEREO   =   0x0,    // 立体声（双声道）
    I2S_CH_5p1      =   0x1,    // 5.1环绕声
    I2S_CH_MONO     =   0x2     // 单声道
};
```

---

## 8. DMA传输配置

### 8.1 Burst Size（突发传输大小）

**定义位置**: `rtl8735b_i2s.h:L143-L149`

```c
enum i2s_burst_size_e {
    I2S_BURST8      =   0x07,    // 8字突发（32字节）
    I2S_BURST12     =   0x0b,    // 12字突发（48字节）
    I2S_BURST16     =   0x0f     // 16字突发（64字节）
};
```

### 8.2 页面数量配置

**定义位置**: `rtl8735b_i2s.h:L133-L140`

```c
enum i2s_page_num_e {
    I2S_USELESS_PAGE    =   0x1,    // 无用（不使用页面模式）
    I2S_2PAGE           =   0x2,    // 2个DMA页面
    I2S_3PAGE           =   0x3,    // 3个DMA页面
    I2S_4PAGE           =   0x4     // 4个DMA页面
};
```

### 8.3 DMA页面配置示例

**定义位置**: `module_i2s.h:L5-L6`

```c
#define I2S_DMA_PAGE_NUM    4                   // DMA页面数量（2-4）
#define I2S_DMA_PAGE_SIZE   (640 * 2)           // 页面大小：1280字节（64的倍数，最大4095）
```

### 8.4 DMA页面寄存器

| 寄存器 | 地址偏移 | 功能 |
|--------|---------|------|
| I2S_PAGE_PTR_TX | 0x004 | TX页面指针 |
| I2S_PAGE_PTR_RX | 0x008 | RX页面指针 |
| I2S_TX_PAGE0_OWN ~ I2S_TX_PAGE3_OWN | 0x020-0x02C | TX页面所有权 |
| I2S_RX_PAGE0_OWN ~ I2S_RX_PAGE3_OWN | 0x030-0x03C | RX页面所有权 |

---

## 9. WS（Word Select）时序配置

### 9.1 WS触发边沿

**定义位置**: `module_i2s.h:L56-L60`

```c
typedef enum {
    WS_NEGATIVE_EDGE,       // WS（LRCK）在负边沿触发
    WS_POSITIVE_EDGE,       // WS（LRCK）在正边沿触发
} i2s_ws_trig_edge;
```

### 9.2 WS相位控制

**定义位置**: `rtl8735b_i2s.h:L113-L118`

```c
enum i2s_ws_swap_e {
    I2S_LEFT_PHASE      =   0x0,    // 数据出现在WS左相位
    I2S_RIGHT_PHASE     =   0x1     // 数据出现在WS右相位
};
```

### 9.3 WS时序图

```
标准I2S模式:
WS:  ─────┐       ┌───────┐       ┌───────
          │       │       │       │
          └───────┘       └───────┘
         | 左声道  | 右声道 | 左声道

BCLK: ─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─
       └─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └─┘

DATA: ──D0─D1─D2─D3─D4─D5─D6─D7─D8─D9─D10─
      (延迟1个BCLK周期)
```

---

## 10. 引脚定义

### 10.1 标准I2S引脚

**定义位置**: `mmf_source_i2s.h:L28-L30`

```c
#define I2S_SCLK_PIN    PC_1    // 串行时钟（SCK/BCLK）
#define I2S_WS_PIN      PC_0    // 字选择（WS/LRCK）
#define I2S_SD_PIN      PC_2    // 串行数据（SD）
```

### 10.2 扩展I2S初始化引脚

**定义位置**: `i2s_api.h:L398-L409`

```c
void i2s_init(i2s_t *obj, PinName sck, PinName ws, PinName sd_tx0, 
              PinName sd_rx, PinName mck, PinName sd_tx1, PinName sd_tx2);
```

| 引脚 | 功能 | 说明 |
|------|------|------|
| sck | 串行时钟 | 位时钟信号 |
| ws | 字选择 | 左右声道选择 |
| sd_tx0 | 发送数据0 | 主数据输出 |
| sd_rx | 接收数据 | 数据输入（可设为NC） |
| mck | 主时钟 | 系统时钟（可设为NC） |
| sd_tx1 | 发送数据1 | 第二通道输出（可设为NC） |
| sd_tx2 | 发送数据2 | 第三通道输出（可设为NC） |

---

## 11. 中断配置

### 11.1 TX中断掩码

**定义位置**: `rtl8735b_i2s.h:L163-L173`

```c
enum i2s_tx_imr_e {
    I2S_TX_INT_PAGE0_OK  = (1 << 0),    // 页面0传输完成
    I2S_TX_INT_PAGE1_OK  = (1 << 1),    // 页面1传输完成
    I2S_TX_INT_PAGE2_OK  = (1 << 2),    // 页面2传输完成
    I2S_TX_INT_PAGE3_OK  = (1 << 3),    // 页面3传输完成
    I2S_TX_INT_PAGE0_UNA = (1 << 4),    // 页面0不可用
    I2S_TX_INT_PAGE1_UNA = (1 << 5),    // 页面1不可用
    I2S_TX_INT_PAGE2_UNA = (1 << 6),    // 页面2不可用
    I2S_TX_INT_PAGE3_UNA = (1 << 7),    // 页面3不可用
    I2S_TX_INT_EMPTY     = (1 << 8)     // FIFO空
};
```

### 11.2 RX中断掩码

**定义位置**: `rtl8735b_i2s.h:L178-L188`

```c
enum i2s_rx_imr_e {
    I2S_RX_INT_PAGE0_OK  = (1 << 0),    // 页面0接收完成
    I2S_RX_INT_PAGE1_OK  = (1 << 1),    // 页面1接收完成
    I2S_RX_INT_PAGE2_OK  = (1 << 2),    // 页面2接收完成
    I2S_RX_INT_PAGE3_OK  = (1 << 3),    // 页面3接收完成
    I2S_RX_INT_PAGE0_UNA = (1 << 4),    // 页面0不可用
    I2S_RX_INT_PAGE1_UNA = (1 << 5),    // 页面1不可用
    I2S_RX_INT_PAGE2_UNA = (1 << 6),    // 页面2不可用
    I2S_RX_INT_PAGE3_UNA = (1 << 7),    // 页面3不可用
    I2s_RX_INT_FULL      = (1 << 8)     // FIFO满
};
```

---

## 12. 多媒体框架I2S参数结构

**定义位置**: `module_i2s.h:L67-L87`

```c
typedef struct i2s_param_s {
    uint32_t                sample_rate;        // 采样率（Hz）
    uint8_t                 i2s_word_length;    // I2S字长（16/24/32）
    uint8_t                 rx_word_length;     // 接收字长
    uint8_t                 tx_word_length;     // 发送字长
    i2s_format              i2s_format;         // 格式（I2S/左对齐/右对齐）
    i2s_ms_mode             i2s_role;           // 主从模式
    i2s_ws_trig_edge        i2s_ws_edge;        // WS触发边沿
    i2s_edge_sw             i2s_data_edge;      // 数据触发边沿
    uint8_t                 i2s_channel;        // 通道数
    i2s_channel_type        rx_channel;         // 接收通道类型
    i2s_channel_type        tx_channel;         // 发送通道类型
    i2s_direction_type      i2s_direction;      // 传输方向
    uint8_t                 rx_byte_swap;       // 接收字节交换
    uint8_t                 tx_byte_swap;       // 发送字节交换
    uint8_t                 pin_group_num;      // 引脚组号
} i2s_params_t;
```

### 12.1 相关枚举类型

**通道类型**:
```c
typedef enum {
    I2S_LEFT_CHANNEL,
    I2S_RIGHT_CHANNEL,
    I2S_STEREO_CHANNEL,
} i2s_channel_type;
```

**传输方向**:
```c
typedef enum {
    I2S_RX_ONLY,        // 仅接收
    I2S_TX_ONLY,        // 仅发送
    I2S_TRX_BOTH,       // 收发同时
} i2s_direction_type;
```

---

## 13. HAL API函数列表

### 13.1 初始化与配置

| 函数 | 说明 |
|------|------|
| `hal_i2s_init()` | 初始化I2S设备 |
| `hal_i2s_deinit()` | 反初始化I2S设备 |
| `hal_i2s_func_ctrl()` | 使能/禁用I2S功能 |
| `hal_i2s_set_parameter()` | 设置I2S参数 |
| `hal_i2s_set_rate()` | 设置采样率 |

### 13.2 时序相关配置

| 函数 | 说明 |
|------|------|
| `hal_i2s_set_word_len()` | 设置字长（16/24/32位） |
| `hal_i2s_set_ch_num()` | 设置通道数（单声道/立体声） |
| `hal_i2s_set_format()` | 设置接口格式（I2S/左对齐/右对齐） |
| `hal_i2s_set_master()` | 设置主从模式 |
| `hal_i2s_set_sck_inv()` | 设置SCK反相 |
| `hal_i2s_set_ws_swap()` | 设置WS相位（左/右） |
| `hal_i2s_set_data_start_edge()` | 设置数据发送边沿（正/负） |
| `hal_i2s_set_direction()` | 设置传输方向（TX/RX/TXRX） |

### 13.3 DMA配置

| 函数 | 说明 |
|------|------|
| `hal_i2s_set_dma_buf()` | 设置DMA缓冲区 |
| `hal_i2s_set_lxbus_burst_size()` | 设置DMA突发大小 |
| `hal_i2s_set_page_num()` | 设置页面数量 |
| `hal_i2s_set_page_size()` | 设置页面大小 |
| `hal_i2s_get_tx_page()` | 获取TX页面 |
| `hal_i2s_get_rx_page()` | 获取RX页面 |
| `hal_i2s_page_send()` | 发送页面 |
| `hal_i2s_page_recv()` | 接收页面 |

### 13.4 中断控制

| 函数 | 说明 |
|------|------|
| `hal_i2s_irq_handler()` | 中断处理函数 |
| `hal_i2s_irq_reg()` | 注册中断 |
| `hal_i2s_irq_unreg()` | 注销中断 |
| `hal_i2s_intr_ctrl()` | 控制中断使能 |
| `hal_i2s_clr_intr()` | 清除中断 |

---

## 14. 字节交换配置

**定义位置**: `rtl8735b_i2s.h:L93-L98`

```c
enum i2s_byte_swap_e {
    I2S_BIG_INDIAN      =   0x0,    // 大端模式
    I2S_LITTLE_INDIAN   =   0x1     // 小端模式
};
```

### 24位数据内存布局

**说明位置**: `module_i2s.h:L22-L31`

**正常字节序**:
```
addr0                     addr32
N, N-1, ...., 0, P, P, P, P
```

**反转字节序**:
```
addr0                     addr32
P, P, P, P, 0, ..., N-1, N
```

---

## 15. 回环模式

**定义位置**: `rtl8735b_i2s.h:L123-L128`

```c
enum i2s_loopback_e {
    I2S_LOOPBACK_DISABLE    =   0x0,    // 禁用回环
    I2S_LOOPBACK_ENABLE     =   0x1     // 启用回环
};
```

> **注意**: 启用回环模式时，需要将I2S方向设置为仅TX（I2S_DIR_TX）

---

## 16. 静音模式

**定义位置**: `rtl8735b_i2s.h:L163-L168`

```c
enum i2s_mute_e {
    I2S_MUTE_DISABLE    =   0x0,    // 禁用静音
    I2S_MUTE_ENABLE     =   0x1     // 启用静音
};
```

---

## 17. I2S设备模式结构体

**定义位置**: `rtl8735b_i2s.h:L263-L283`

```c
typedef struct hal_i2s_def_setting_s {
    i2s_burst_size_t    i2s_burst_size;     // DMA突发大小
    i2s_dev_mode_t      i2s_master;         // 主从模式
    i2s_word_len_t      i2s_word_len;       // 字长（16/24位）
    i2s_ch_num_t        i2s_ch_num;         // 通道数（单声道/立体声）
    i2s_page_num_t      i2s_page_num;       // 页面数（2-4）
    u16                 i2s_page_size;      // 页面大小（1-4096字）
    i2s_sample_rate_t   i2s_rate;           // 采样率
    i2s_direction_t     i2s_trx_act;        // 传输方向控制
    i2s_byte_swap_t     i2s_byte_swap;      // 字节交换
    i2s_sck_inv_t       i2s_sck_inv;        // SCK反相
    i2s_ws_swap_t       i2s_ws_swap;        // WS相位控制
    i2s_format_t        i2s_format;         // 接口格式
    i2s_loopback_t      i2s_loop_back;      // 回环模式
    i2s_edge_sw_t       i2s_edge_sw;        // 数据发送边沿
    i2s_tx_imr_t        i2s_tx_intr_msk;    // TX中断掩码
    i2s_rx_imr_t        i2s_rx_intr_msk;    // RX中断掩码
} hal_i2s_def_setting_t;
```

---

## 18. 典型配置示例

### 18.1 CD质量音频配置（44.1kHz, 16位, 立体声）

```c
hal_i2s_def_setting_t cd_audio_config = {
    .i2s_burst_size = I2S_BURST16,          // 16字突发
    .i2s_master = I2S_MASTER_MODE,          // 主模式
    .i2s_word_len = I2S_WL_16,              // 16位字长
    .i2s_ch_num = I2S_CH_STEREO,            // 立体声
    .i2s_page_num = I2S_4PAGE,              // 4个DMA页面
    .i2s_page_size = 1280,                  // 每页1280字
    .i2s_rate = I2S_SR_44p1KHZ,             // 44.1kHz采样率
    .i2s_trx_act = I2S_ONLY_TX,             // 仅发送
    .i2s_byte_swap = I2S_LITTLE_INDIAN,     // 小端模式
    .i2s_sck_inv = I2S_SCKINV_DISABLE,      // 不反相SCK
    .i2s_ws_swap = I2S_LEFT_PHASE,          // 左相位
    .i2s_format = I2S_FORMAT_I2S,           // 标准I2S格式
    .i2s_loop_back = I2S_LOOPBACK_DISABLE,  // 禁用回环
    .i2s_edge_sw = I2S_NEGATIVE_EDGE,       // 负边沿发送
    .i2s_tx_intr_msk = I2S_TX_INT_PAGE0_OK | I2S_TX_INT_PAGE1_OK | 
                       I2S_TX_INT_PAGE2_OK | I2S_TX_INT_PAGE3_OK,
    .i2s_rx_intr_msk = 0
};
```

**时序参数计算**:
- BCLK = 44100 × 16 × 2 = 1,411,200 Hz (1.4112 MHz)
- WS频率 = 44,100 Hz
- 每页数据量 = 1280 × 4 = 5120 字节
- 总缓冲 = 5120 × 4页 = 20,480 字节

### 18.2 高清音频配置（96kHz, 24位, 立体声）

```c
hal_i2s_def_setting_t hd_audio_config = {
    .i2s_burst_size = I2S_BURST16,
    .i2s_master = I2S_MASTER_MODE,
    .i2s_word_len = I2S_WL_24,              // 24位字长
    .i2s_ch_num = I2S_CH_STEREO,
    .i2s_page_num = I2S_4PAGE,
    .i2s_page_size = 640,
    .i2s_rate = I2S_SR_96KHZ,               // 96kHz采样率
    .i2s_trx_act = I2S_ONLY_TX,
    .i2s_byte_swap = I2S_LITTLE_INDIAN,
    .i2s_sck_inv = I2S_SCKINV_DISABLE,
    .i2s_ws_swap = I2S_LEFT_PHASE,
    .i2s_format = I2S_FORMAT_I2S,
    .i2s_loop_back = I2S_LOOPBACK_DISABLE,
    .i2s_edge_sw = I2S_NEGATIVE_EDGE,
    .i2s_tx_intr_msk = I2S_TX_INT_PAGE0_OK | I2S_TX_INT_PAGE1_OK | 
                       I2S_TX_INT_PAGE2_OK | I2S_TX_INT_PAGE3_OK,
    .i2s_rx_intr_msk = 0
};
```

**时序参数计算**:
- BCLK = 96000 × 24 × 2 = 4,608,000 Hz (4.608 MHz)
- WS频率 = 96,000 Hz
- 每页数据量 = 640 × 4 = 2560 字节
- 总缓冲 = 2560 × 4页 = 10,240 字节

---

## 19. I2S时序关键要点总结

### 19.1 时钟关系

| 时钟信号 | 说明 | 计算公式 |
|---------|------|---------|
| MCLK | 主时钟（可选） | 通常为采样率的256倍或512倍 |
| BCLK | 位时钟（SCK） | 采样率 × 字长 × 通道数 |
| WS | 字选择（LRCK） | 等于采样率 |

### 19.2 时序配置检查清单

- [ ] 选择正确的主从模式
- [ ] 配置采样率（影响WS频率）
- [ ] 设置字长（影响BCLK频率）
- [ ] 选择I2S格式（标准/左对齐/右对齐）
- [ ] 配置数据边沿（正/负）
- [ ] 设置WS相位（左/右）
- [ ] 配置DMA页面数量和大小
- [ ] 设置传输方向（TX/RX/TXRX）
- [ ] 使能相应中断

### 19.3 支持的采样率范围

- **最低**: 7.35 kHz
- **最高**: 384 kHz
- **标准音频**: 8k, 16k, 22.05k, 24k, 32k, 44.1k, 48k, 88.2k, 96k, 176.4k, 192k

### 19.4 数据宽度支持

- 16位：适用于CD质量音频
- 24位：适用于专业音频/高清音频
- 32位：适用于需要额外精度的应用

### 19.5 DMA传输机制

- 使用循环页面缓冲（2-4页）
- 页面大小范围：1-4096字
- 支持8/12/16字突发传输
- 页面完成中断通知CPU填充/读取数据

---

## 20. 故障排查指南

### 20.1 常见问题

| 问题 | 可能原因 | 解决方案 |
|------|---------|---------|
| 无声音输出 | I2S未使能 | 检查IIS_EN位是否置1 |
| 声音失真 | 采样率不匹配 | 确认发送数据采样率与配置一致 |
| 左右声道反了 | WS相位错误 | 调整LR_SWAP位 |
| 数据错位 | 格式不匹配 | 检查I2S格式（标准/左对齐/右对齐） |
| 中断不触发 | 中断掩码未设置 | 配置正确的中断掩码 |
| DMA传输停止 | 页面未正确交接 | 确保在页面完成中断中提交新页面 |

### 20.2 调试建议

1. 使用回环模式测试硬件连接
2. 检查引脚复用配置是否正确
3. 使用示波器测量SCK和WS频率
4. 验证数据边沿与外部设备匹配
5. 检查DMA页面所有权位设置

---

## 附录：寄存器地址映射

| 寄存器名称 | 地址偏移 | 复位值 | 说明 |
|-----------|---------|--------|------|
| I2S_CTRL | 0x000 | 0x00000000 | 控制寄存器 |
| I2S_PAGE_PTR_TX | 0x004 | 0x00000000 | TX页面指针 |
| I2S_PAGE_PTR_RX | 0x008 | 0x00000000 | RX页面指针 |
| I2S_PAGE_SIZE_RATE | 0x00C | 0x00005000 | 页面大小和采样率 |
| I2S_TX_ISR_EN | 0x010 | 0x00000000 | TX中断使能 |
| I2S_TX_ISR_STATUS | 0x014 | 0x00000000 | TX中断状态 |
| I2S_RX_ISR_EN | 0x018 | 0x00000000 | RX中断使能 |
| I2S_RX_ISR_STATUS | 0x01C | 0x00000000 | RX中断状态 |
| I2S_TX_PAGE0_OWN | 0x020 | 0x00000000 | TX页面0所有权 |
| I2S_TX_PAGE1_OWN | 0x024 | 0x00000000 | TX页面1所有权 |
| I2S_TX_PAGE2_OWN | 0x028 | 0x00000000 | TX页面2所有权 |
| I2S_TX_PAGE3_OWN | 0x02C | 0x00000000 | TX页面3所有权 |
| I2S_RX_PAGE0_OWN | 0x030 | 0x00000000 | RX页面0所有权 |
| I2S_RX_PAGE1_OWN | 0x034 | 0x00000000 | RX页面1所有权 |
| I2S_RX_PAGE2_OWN | 0x038 | 0x00000000 | RX页面2所有权 |
| I2S_RX_PAGE3_OWN | 0x03C | 0x00000000 | RX页面3所有权 |
| I2S_VERSION_ID | 0x040 | - | 版本ID |

---

**文档结束**
