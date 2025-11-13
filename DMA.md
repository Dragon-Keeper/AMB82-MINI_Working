# Ameba Pro2 DMA功能实现文档

## 概述

本文档详细介绍了Ameba Pro2开发板上的DMA（Direct Memory Access，直接内存访问）功能实现。DMA是一种允许外设直接与内存进行数据传输的技术，无需CPU的干预，可以显著提高数据传输效率并降低CPU负载。

## DMA系统架构

Ameba Pro2使用GDMA（General Purpose DMA）系统，支持多种外设的DMA传输，包括SPI、UART、I2C、ADC、PWM等。

### 核心文件结构

#### 1. GDMA核心定义文件
- **<mcfile name="hal_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_gdma.h"></mcfile>**
  - GDMA核心API头文件，定义了GDMA初始化、通道注册/注销、传输控制等核心函数
  - 包含静态内联函数：`hal_gdma_on/off`、`hal_gdma_chnl_en/dis`、`hal_gdma_isr_en/dis`等

- **<mcfile name="rtl8735b_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/rtl8735b_gdma.h"></mcfile>**
  - GDMA类型定义和结构体声明
  - 定义了GDMA适配器结构体`hal_gdma_adaptor_t`和GDMA函数桩`hal_gdma_func_stubs_t`

- **<mcfile name="rtl8735b_gdma_type.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/rtl8735b_gdma_type.h"></mcfile>**
  - GDMA寄存器定义和位域宏定义
  - 包含中断状态寄存器、中断掩码寄存器、中断清除寄存器等定义

#### 2. GDMA通道类型定义
- **<mcfile name="rtl8735b_gdma_ch_type.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/rtl8735b_gdma_ch_type.h"></mcfile>**
  - GDMA通道相关寄存器定义

#### 3. 外设DMA接口文件
- **<mcfile name="hal_ssi.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_ssi.h"></mcfile>**
  - SPI DMA接口定义，包含SPI与GDMA的握手配置
  - 提供`hal_ssi_tx_gdma_init_setting`、`hal_ssi_rx_gdma_init_setting`等函数

- **<mcfile name="spi_ex_api.h" path="Arduino_package/hardware/system/component/mbed/hal_ext/spi_ex_api.h"></mcfile>**
  - SPI扩展API，包含DMA模式的数据传输函数
  - 提供`spi_master_write_read_stream_dma`、`spi_slave_read_stream_dma`等高级API

- **<mcfile name="hal_sgpio.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_sgpio.h"></mcfile>**
  - SGPIO（Serial GPIO）DMA功能，支持波形生成
  - 提供`hal_sgpio_dma_match_output_init`等函数

## GDMA寄存器映射与访问规范

### GDMA基地址定义

Ameba Pro2的GDMA系统包含两个独立的DMA控制器，每个控制器支持多个通道：

- **GDMA0基地址**: 0x4000_8000 (非安全域)
- **GDMA1基地址**: 0x5000_8000 (安全域)
- **每个GDMA控制器支持8个独立通道**

### GDMA控制寄存器地址偏移

| 寄存器名称 | 偏移地址 | 访问权限 | 功能说明 |
|------------|----------|----------|----------|
| GDMA_RAW_TFR | 0x000 | R | 传输完成原始中断状态寄存器 |
| GDMA_RAW_BLOCK | 0x008 | R | 块传输完成原始中断状态寄存器 |
| GDMA_RAW_SRC_TRAN | 0x010 | R | 源端传输完成原始中断状态寄存器 |
| GDMA_RAW_DST_TRAN | 0x018 | R | 目标端传输完成原始中断状态寄存器 |
| GDMA_RAW_ERR_LOW | 0x020 | R | 错误原始中断状态低寄存器 |
| GDMA_RAW_ERR_UP | 0x024 | R | 错误原始中断状态高寄存器 |
| GDMA_STATUS_TFR | 0x028 | R | 传输完成中断状态寄存器 |
| GDMA_STATUS_BLOCK | 0x030 | R | 块传输完成中断状态寄存器 |
| GDMA_STATUS_SRC_TRAN | 0x038 | R | 源端传输完成中断状态寄存器 |
| GDMA_STATUS_DST_TRAN | 0x040 | R | 目标端传输完成中断状态寄存器 |
| GDMA_STATUS_ERR_LOW | 0x048 | R | 错误中断状态低寄存器 |
| GDMA_STATUS_ERR_UP | 0x04C | R | 错误中断状态高寄存器 |
| GDMA_MASK_TFR | 0x050 | R/W | 传输完成中断掩码寄存器 |
| GDMA_MASK_BLOCK | 0x058 | R/W | 块传输完成中断掩码寄存器 |
| GDMA_MASK_SRC_TRAN | 0x060 | R/W | 源端传输完成中断掩码寄存器 |
| GDMA_MASK_DST_TRAN | 0x068 | R/W | 目标端传输完成中断掩码寄存器 |
| GDMA_MASK_ERR_LOW | 0x070 | R/W | 错误中断掩码低寄存器 |
| GDMA_MASK_ERR_UP | 0x074 | R/W | 错误中断掩码高寄存器 |
| GDMA_CLEAR_TFR | 0x078 | W1C | 传输完成中断清除寄存器 |
| GDMA_CLEAR_BLOCK | 0x080 | W1C | 块传输完成中断清除寄存器 |
| GDMA_CLEAR_SRC_TRAN | 0x088 | W1C | 源端传输完成中断清除寄存器 |
| GDMA_CLEAR_DST_TRAN | 0x090 | W1C | 目标端传输完成中断清除寄存器 |
| GDMA_CLEAR_ERR_LOW | 0x098 | W1C | 错误中断清除低寄存器 |
| GDMA_CLEAR_ERR_UP | 0x09C | W1C | 错误中断清除高寄存器 |
| GDMA_STATUS_INT | 0x0A0 | R | 组合中断状态寄存器 |
| GDMA_DMA_CFG_REG | 0x0D8 | R/W | DMA配置寄存器 |
| GDMA_CH_EN_REG | 0x0E0 | R/W | 通道使能寄存器 |
| GDMA_CH_RESET_REG | 0x0F8 | R/W | 通道复位寄存器 |

### GDMA通道寄存器地址偏移

每个通道的寄存器组从基地址开始，按通道号偏移0x100：

| 寄存器名称 | 偏移地址 | 访问权限 | 功能说明 |
|------------|----------|----------|----------|
| GDMA_CH_SAR | 0x000 | R/W | 通道源地址寄存器 |
| GDMA_CH_DAR | 0x008 | R/W | 通道目标地址寄存器 |
| GDMA_CH_LLP | 0x010 | R/W | 通道链表指针寄存器 |
| GDMA_CH_CTL_LOW | 0x018 | R/W | 通道控制寄存器低32位 |
| GDMA_CH_CTL_UP | 0x01C | R/W | 通道控制寄存器高32位 |
| GDMA_CH_CFG_LOW | 0x040 | R/W | 通道配置寄存器低32位 |
| GDMA_CH_CFG_UP | 0x044 | R/W | 通道配置寄存器高32位 |

## GDMA核心API详解

### GDMA适配器结构体

```c
typedef struct _hal_gdma_adaptor_s {
    GDMA_TypeDef                *gdma_dev;          // GDMA设备基地址
    GDMA_CH_TypeDef             *chnl_dev;          // GDMA通道寄存器基地址
    gdma_chnl_num_t             ch_en;              // 通道使能控制
    gdma_ctl_reg_t              gdma_ctl;           // 控制寄存器数据结构
    gdma_cfg_reg_t              gdma_cfg;           // 配置寄存器数据结构
    pgdma_ch_lli_t              pgdma_ch_lli;       // 链表项指针
    gdma_callback_t             gdma_cb_func;       // 回调函数指针
    void                        *gdma_cb_para;      // 回调函数参数
    irq_handler_t               gdma_irq_func;      // 中断处理函数指针
    void                        *gdma_irq_para;     // 中断处理参数
    // ... 其他成员
} hal_gdma_adaptor_t, *phal_gdma_adaptor_t;
```

### 关键API函数

#### 1. GDMA设备控制
- **<mcsymbol name="hal_gdma_on" filename="hal_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_gdma.h" startline="100" type="function"></mcsymbol>** / **<mcsymbol name="hal_gdma_off" filename="hal_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_gdma.h" startline="110" type="function"></mcsymbol>**
  - 功能：开启/关闭GDMA设备
  - 参数：`phal_gdma_adaptor_t` - GDMA适配器指针

#### 2. 通道控制
- **<mcsymbol name="hal_gdma_chnl_en" filename="hal_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_gdma.h" startline="120" type="function"></mcsymbol>** / **<mcsymbol name="hal_gdma_chnl_dis" filename="hal_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_gdma.h" startline="130" type="function"></mcsymbol>**
  - 功能：启用/禁用GDMA通道
  - 参数：`phal_gdma_adaptor_t` - GDMA适配器指针

#### 3. 中断控制
- **<mcsymbol name="hal_gdma_isr_en" filename="hal_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_gdma.h" startline="140" type="function"></mcsymbol>** / **<mcsymbol name="hal_gdma_isr_dis" filename="hal_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_gdma.h" startline="150" type="function"></mcsymbol>**
  - 功能：启用/禁用GDMA中断
  - 参数：`phal_gdma_adaptor_t` - GDMA适配器指针

#### 4. 通道设置
- **<mcsymbol name="hal_gdma_chnl_setting" filename="hal_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_gdma.h" startline="200" type="function"></mcsymbol>**
  - 功能：配置GDMA通道参数
  - 参数：`phal_gdma_adaptor_t` - GDMA适配器指针
  - 返回值：`hal_status_t` - 操作状态

#### 5. 数据传输控制
- **<mcsymbol name="hal_gdma_transfer_start" filename="hal_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_gdma.h" startline="300" type="function"></mcsymbol>**
  - 功能：启动GDMA传输
  - 参数：`phal_gdma_adaptor_t` - GDMA适配器指针，`u8 multi_blk_en` - 多块传输使能

## 外设握手接口定义

GDMA支持与多种外设进行硬件握手，相关定义在<mcfile name="rtl8735b_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/rtl8735b_gdma.h"></mcfile>中：

```c
#define GDMA_HANDSHAKE_SPI0_TX              8   // SPI0发送握手
#define GDMA_HANDSHAKE_SPI0_RX              9   // SPI0接收握手
#define GDMA_HANDSHAKE_SPI1_TX              10  // SPI1发送握手
#define GDMA_HANDSHAKE_SPI1_RX              11  // SPI1接收握手
#define GDMA_HANDSHAKE_SGPIO0_TX            19  // SGPIO0发送握手
#define GDMA_HANDSHAKE_PWM0                 20  // PWM0握手
// ... 其他握手接口定义
```

## 8位并口屏幕的DMA实现方法

### GPIO寄存器直接读写 + DMA

对于8位并口屏幕，可以通过GPIO端口直接读写结合DMA实现高效数据传输：

#### 1. GPIO端口寄存器定义

关键寄存器定义在<mcfile name="rtl8735b_gpio_type.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/rtl8735b_gpio_type.h"></mcfile>中：

- **数据输出寄存器**：`GPIOx_OD_STS` - 控制GPIO端口输出数据
- **数据方向寄存器**：`GPIOx_DMD_STS` - 设置GPIO端口方向
- **输出模式使能**：`GPIOx_ODM_EN` - 使能输出模式

#### 2. DMA配置步骤

1. **初始化GPIO端口**
   ```c
   // 设置GPIO端口为输出模式
   hal_gpio_port_dir(GPIO_PORT_A, 0xFF, HAL_GPIO_DIR_OUTPUT);
   ```

2. **配置GDMA适配器**
   ```c
   hal_gdma_adaptor_t gdma_adaptor;
   hal_gdma_chnl_init(&gdma_adaptor);
   ```

3. **设置传输参数**
   ```c
   gdma_adaptor.gdma_ctl.tt_fc = TTFCMemToPeri;  // 内存到外设
   gdma_adaptor.gdma_ctl.dst_tr_width = TrWidthOneByte;  // 目标数据宽度为1字节
   gdma_adaptor.gdma_ctl.block_size = buffer_size;  // 传输块大小
   ```

4. **配置握手接口**
   ```c
   hal_gdma_handshake_init(&gdma_adaptor, custom_handshake_num);
   ```

5. **启动传输**
   ```c
   hal_gdma_transfer_start(&gdma_adaptor, 0);
   ```

## SPI总线连接的DMA实现方法

### SPI DMA传输配置

SPI DMA功能通过<mcfile name="hal_ssi.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/hal_ssi.h"></mcfile>提供的API实现：

#### 1. SPI发送DMA初始化

```c
// SPI发送DMA初始化
hal_status_t hal_ssi_tx_gdma_init_setting(phal_ssi_adaptor_t phal_ssi_adaptor, 
                                         phal_gdma_adaptor_t phal_gdma_adaptor);

// SPI发送DMA传输初始化
hal_status_t hal_ssi_dma_send_init(phal_ssi_adaptor_t phal_ssi_adaptor, 
                                  u8 *ptx_data, u32 length);
```

#### 2. SPI接收DMA初始化

```c
// SPI接收DMA初始化
hal_status_t hal_ssi_rx_gdma_init_setting(phal_ssi_adaptor_t phal_ssi_adaptor, 
                                         phal_gdma_adaptor_t phal_gdma_adaptor);

// SPI接收DMA传输初始化
hal_status_t hal_ssi_dma_recv_init(phal_ssi_adaptor_t phal_ssi_adaptor, 
                                  u8 *prx_data, u32 length);
```

#### 3. 高级SPI DMA API

通过<mcfile name="spi_ex_api.h" path="Arduino_package/hardware/system/component/mbed/hal_ext/spi_ex_api.h"></mcfile>提供的高级API：

```c
// 主设备发送接收DMA
int32_t spi_master_write_read_stream_dma(spi_t *obj, char *tx_buffer, 
                                        char *rx_buffer, uint32_t length);

// 主设备发送DMA
int32_t spi_master_write_stream_dma(spi_t *obj, char *tx_buffer, uint32_t length);

// 主设备接收DMA
int32_t spi_master_read_stream_dma(spi_t *obj, char *rx_buffer, uint32_t length);
```

## DMA中断处理机制

### GDMA中断寄存器位域定义

GDMA中断系统包含完整的中断状态、掩码和清除寄存器，定义在<mcfile name="rtl8735b_gdma_type.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/rtl8735b_gdma_type.h"></mcfile>中：

#### 中断状态寄存器位域

```c
// 传输完成中断状态寄存器位域定义
#define GDMA_SHIFT_RAW_TFR                           0
#define GDMA_MASK_RAW_TFR                            ((u32)0x0000003F << 0)

// 块传输完成中断状态寄存器位域定义  
#define GDMA_SHIFT_RAW_BLOCK                         0
#define GDMA_MASK_RAW_BLOCK                          ((u32)0x0000003F << 0)

// 源端传输完成中断状态寄存器位域定义
#define GDMA_SHIFT_RAW_SRC_TRAN                      0
#define GDMA_MASK_RAW_SRC_TRAN                       ((u32)0x0000003F << 0)

// 目标端传输完成中断状态寄存器位域定义
#define GDMA_SHIFT_RAW_DST_TRAN                      0
#define GDMA_MASK_RAW_DST_TRAN                       ((u32)0x0000003F << 0)

// 错误中断状态寄存器位域定义
#define GDMA_SHIFT_RAW_ERR                           0
#define GDMA_MASK_RAW_ERR                            ((u32)0x0000003F << 0)
```

#### 中断掩码寄存器位域

```c
// 中断掩码寄存器支持写使能位，用于原子操作
#define GDMA_SHIFT_MASK_TFR_WE                       8
#define GDMA_MASK_MASK_TFR_WE                        ((u32)0x0000003F << 8)
#define GDMA_SHIFT_MASK_TFR                          0
#define GDMA_MASK_MASK_TFR                           ((u32)0x0000003F << 0)
```

### GDMA中断类型

GDMA支持多种中断类型，定义在<mcfile name="rtl8735b_gdma.h" path="Arduino_package/hardware/system/component/soc/8735b/fwlib/rtl8735b/include/rtl8735b_gdma.h"></mcfile>中：

```c
typedef enum gdma_isr_type_e {
    TransferType        = 0x1,     // 传输完成中断
    BlockType           = 0x2,     // 块传输完成中断
    SrcTransferType     = 0x4,     // 源端传输完成中断
    DstTransferType     = 0x8,     // 目标端传输完成中断
    ErrType             = 0x10     // 错误中断
} gdma_isr_type_t;
```

### 中断服务程序实现

#### 基本中断处理函数

```c
// GDMA0中断处理函数
void GDMA0_IRQHandler(void);

// GDMA1中断处理函数
void GDMA1_IRQHandler(void);

// 注册中断处理函数
void hal_gdma_irq_reg(phal_gdma_adaptor_t phal_gdma_adaptor, 
                     irq_handler_t irq_handler, void *irq_data);
```

#### 完整的中断服务程序示例

```c
#include "hal_gdma.h"
#include "rtl8735b_gdma_type.h"

// GDMA中断服务程序
void GDMA_IRQHandler(phal_gdma_adaptor_t phal_gdma_adaptor) {
    // 读取中断状态寄存器
    uint32_t int_status = phal_gdma_adaptor->gdma_dev->GDMA_STATUS_INT;
    
    // 检查传输完成中断
    if (int_status & TransferType) {
        // 处理传输完成事件
        handleTransferComplete(phal_gdma_adaptor);
        
        // 清除中断标志
        phal_gdma_adaptor->gdma_dev->GDMA_CLEAR_TFR = TransferType;
    }
    
    // 检查块传输完成中断
    if (int_status & BlockType) {
        // 处理块传输完成事件
        handleBlockTransferComplete(phal_gdma_adaptor);
        
        // 清除中断标志
        phal_gdma_adaptor->gdma_dev->GDMA_CLEAR_BLOCK = BlockType;
    }
    
    // 检查错误中断
    if (int_status & ErrType) {
        // 处理错误事件
        handleDMAError(phal_gdma_adaptor);
        
        // 清除中断标志
        phal_gdma_adaptor->gdma_dev->GDMA_CLEAR_ERR_LOW = ErrType;
    }
}

// 中断优先级配置示例
void setupDMAInterruptPriority(void) {
    // 设置GDMA0中断优先级为2（中等优先级）
    NVIC_SetPriority(GDMA0_IRQn, 2);
    
    // 设置GDMA1中断优先级为3（较低优先级）
    NVIC_SetPriority(GDMA1_IRQn, 3);
    
    // 使能GDMA中断
    NVIC_EnableIRQ(GDMA0_IRQn);
    NVIC_EnableIRQ(GDMA1_IRQn);
}

// 中断处理辅助函数定义
void handleTransferComplete(phal_gdma_adaptor_t phal_gdma_adaptor) {
    printf("DMA传输完成，通道：%d\n", phal_gdma_adaptor->gdma_index);
    
    // 执行传输完成后的操作
    // 例如：通知应用程序、准备下一次传输等
}

void handleBlockTransferComplete(phal_gdma_adaptor_t phal_gdma_adaptor) {
    printf("DMA块传输完成，通道：%d\n", phal_gdma_adaptor->gdma_index);
    
    // 处理块传输完成事件
    // 例如：更新块指针、处理下一个块等
}

void handleDMAError(phal_gdma_adaptor_t phal_gdma_adaptor) {
    printf("DMA错误发生，通道：%d\n", phal_gdma_adaptor->gdma_index);
    
    // 读取错误状态寄存器
    uint32_t error_status = phal_gdma_adaptor->gdma_dev->GDMA_STATUS_ERR_LOW;
    
    // 根据错误类型进行处理
    if (error_status & BUS_ERROR) {
        printf("总线错误发生\n");
    }
    if (error_status & TRANSFER_ERROR) {
        printf("传输错误发生\n");
    }
    
    // 执行错误恢复操作
    // 例如：重置DMA通道、重新配置参数等
}
```

### 中断优先级与嵌套处理

GDMA中断支持优先级配置和嵌套处理，确保关键数据传输的及时响应：

```c
// 中断优先级配置
void configureDMAInterruptPriority(void) {
    // 设置传输完成中断为最高优先级
    hal_gdma_irq_priority_set(GDMA0_IRQn, 0);
    
    // 设置错误中断为中等优先级
    hal_gdma_irq_priority_set(GDMA1_IRQn, 1);
    
    // 使能中断嵌套
    __enable_irq();
}

// 临界区保护
void dmaCriticalSection(void) {
    // 进入临界区，禁用中断
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    
    // 执行关键DMA操作
    performCriticalDMAOperation();
    
    // 退出临界区，恢复中断状态
    if (!primask) {
        __enable_irq();
    }
}
```

## 多块传输模式

GDMA支持多块传输模式，可以连续传输多个数据块：

### 链表项结构体

```c
typedef struct _gdma_ch_lli_s {
    u32 sarx;         // 源地址
    u32 darx;         // 目标地址
    u32 llpx;         // 下一个块地址
    u32 ctlx_low;     // 控制寄存器低32位
    u32 ctlx_up;      // 控制寄存器高32位
} gdma_ch_lli_t, *pgdma_ch_lli_t;
```

### 多块传输配置

```c
// 多块传输初始化
void hal_gdma_multi_block_init(phal_gdma_adaptor_t phal_gdma_adaptor);

// 链表块配置
hal_status_t hal_gdma_linked_list_block_config(phal_gdma_adaptor_t phal_gdma_adaptor, 
                                              phal_gdma_block_t phal_gdma_block, u8 block_num);
```

## 实际应用示例

### 示例1：SPI屏幕DMA传输

```c
#include "hal_ssi.h"
#include "hal_gdma.h"

// SPI屏幕DMA传输示例
void spi_screen_dma_transfer(spi_t *spi_obj, uint8_t *frame_buffer, uint32_t frame_size) {
    // 使用高级API进行DMA传输
    int32_t result = spi_master_write_stream_dma(spi_obj, (char*)frame_buffer, frame_size);
    
    if (result == 0) {
        // 传输成功
        printf("SPI DMA传输完成，传输大小：%u字节\n", frame_size);
    } else {
        // 传输失败
        printf("SPI DMA传输失败，错误码：%d\n", result);
    }
}
```

### 示例2：8位并口屏幕DMA传输

```c
#include "hal_gpio.h"
#include "hal_gdma.h"

// 8位并口屏幕DMA传输示例
void parallel_screen_dma_transfer(uint8_t *image_data, uint32_t data_size) {
    hal_gdma_adaptor_t gdma_adaptor;
    
    // 初始化GDMA适配器
    if (hal_gdma_chnl_init(&gdma_adaptor) != HAL_OK) {
        printf("GDMA初始化失败\n");
        return;
    }
    
    // 配置传输参数
    gdma_adaptor.gdma_ctl.tt_fc = TTFCMemToPeri;
    gdma_adaptor.gdma_ctl.dst_tr_width = TrWidthOneByte;
    gdma_adaptor.gdma_ctl.block_size = data_size;
    
    // 设置目标地址为GPIO数据寄存器
    gdma_adaptor.ch_dar = (uint32_t)&GPIOA->OD_STS;
    
    // 设置源地址为图像数据缓冲区
    gdma_adaptor.ch_sar = (uint32_t)image_data;
    
    // 启动传输
    hal_gdma_transfer_start(&gdma_adaptor, 0);
    
    printf("8位并口DMA传输启动，数据大小：%u字节\n", data_size);
}
```

## DMA安全访问规范

### 安全域配置

Ameba Pro2的DMA系统支持安全域和非安全域访问，确保敏感数据传输的安全性：

- **安全域(S)**: 地址以0x5000开头，用于安全敏感操作
- **非安全域(NS)**: 地址以0x4000开头，用于普通操作
- **安全域访问需要特权级别权限**

### 安全访问控制

```c
// 安全域DMA配置示例
void configureSecureDMA(void) {
    // 检查当前特权级别
    if (isPrivilegedMode()) {
        // 配置安全域DMA
        hal_gdma_secure_config(GDMA1_BASE, SECURE_MODE);
        
        // 设置安全传输参数
        setupSecureDMAParameters();
    } else {
        // 使用非安全域DMA
        hal_gdma_secure_config(GDMA0_BASE, NONSECURE_MODE);
    }
}

// 特权级别检查
bool isPrivilegedMode(void) {
    return (__get_CONTROL() & 0x1) == 0;
}

// 安全传输参数设置
void setupSecureDMAParameters(void) {
    // 配置安全传输参数
    hal_gdma_adaptor_t gdma_adaptor;
    
    // 初始化GDMA适配器
    hal_gdma_chnl_init(&gdma_adaptor);
    
    // 设置安全传输模式
    gdma_adaptor.gdma_ctl.secure_mode = SECURE_MODE;
    
    // 配置传输参数
    gdma_adaptor.gdma_ctl.tt_fc = TTFCMemToPeri;
    gdma_adaptor.gdma_ctl.dst_tr_width = TrWidthOneByte;
    
    // 启用安全校验
    gdma_adaptor.gdma_ctl.security_check = 1;
}
```

## 时序约束与性能优化

### DMA时序要求

- **最小访问间隔**: 至少1个时钟周期
- **总线仲裁延迟**: 最大50ns
- **中断响应时间**: 最大100ns
- **数据传输速率**: 最高可达系统总线频率

### 性能优化策略

1. **使用多块传输模式**：对于大容量数据传输，使用多块传输模式可以减少中断开销
2. **合理设置块大小**：根据实际需求设置合适的块大小，避免频繁中断
3. **使用缓存对齐**：确保DMA缓冲区地址与缓存行对齐，提高传输效率
4. **合理配置中断优先级**：根据系统需求设置合适的中断优先级
5. **使用DMA完成回调**：通过回调函数处理传输完成事件，避免轮询等待

### 高级优化技巧

```c
// 缓存对齐优化
void optimizeDMAPerformance(void) {
    // 确保DMA缓冲区与缓存行对齐
    uint8_t *dma_buffer = (uint8_t*)aligned_alloc(32, BUFFER_SIZE);
    
    // 使用预取技术提高性能
    __builtin_prefetch(dma_buffer, 0, 3);
    
    // 配置DMA传输参数
    setupOptimizedDMAParameters(dma_buffer);
}

// 批量传输优化
void batchDMATransfer(void) {
    // 使用链表模式进行批量传输
    hal_gdma_linked_list_config(&gdma_adaptor, block_list, block_count);
    
    // 启用自动重载功能
    gdma_adaptor.gdma_cfg.reload_en = 1;
    
    // 启动批量传输
    hal_gdma_transfer_start(&gdma_adaptor, 1);
}
```

## 错误处理机制

### DMA错误类型

GDMA系统支持多种错误检测机制：

- **总线错误**: 地址越界或权限错误
- **传输错误**: 数据传输过程中发生的错误
- **配置错误**: DMA参数配置不合理
- **超时错误**: 传输超时检测

### 错误处理实现

```c
// DMA错误处理函数
void handleDMAError(phal_gdma_adaptor_t phal_gdma_adaptor) {
    // 读取错误状态寄存器
    uint32_t error_status = phal_gdma_adaptor->gdma_dev->GDMA_STATUS_ERR_LOW;
    
    // 处理不同类型的错误
    if (error_status & BUS_ERROR) {
        printf("DMA总线错误发生，地址: 0x%08X\n", 
               phal_gdma_adaptor->chnl_dev->GDMA_CH_SAR);
        
        // 停止DMA传输
        hal_gdma_chnl_dis(phal_gdma_adaptor);
        
        // 重置DMA通道
        hal_gdma_chnl_reset(phal_gdma_adaptor);
    }
    
    if (error_status & TRANSFER_ERROR) {
        printf("DMA传输错误发生\n");
        
        // 重新配置DMA参数
        reconfigureDMAParameters(phal_gdma_adaptor);
    }
    
    // 清除错误标志
    phal_gdma_adaptor->gdma_dev->GDMA_CLEAR_ERR_LOW = error_status;
}

// 错误恢复机制
void recoverFromDMAError(phal_gdma_adaptor_t phal_gdma_adaptor) {
    // 保存当前配置
    saveDMAConfiguration(phal_gdma_adaptor);
    
    // 重置DMA通道
    hal_gdma_chnl_reset(phal_gdma_adaptor);
    
    // 重新初始化DMA
    hal_gdma_chnl_init(phal_gdma_adaptor);
    
    // 恢复配置
    restoreDMAConfiguration(phal_gdma_adaptor);
    
    // 重新启动传输
    hal_gdma_transfer_start(phal_gdma_adaptor, 0);
}
```

## 总结

Ameba Pro2的DMA系统提供了强大的数据传输能力，支持多种外设的DMA传输。通过合理使用GDMA API，可以显著提高数据传输效率，降低CPU负载。本文档详细介绍了DMA系统的架构、API使用方法、寄存器映射、中断处理机制、安全访问规范、时序约束、性能优化策略以及错误处理机制，为开发高效的屏幕驱动和其他外设应用提供了完整的技术参考。