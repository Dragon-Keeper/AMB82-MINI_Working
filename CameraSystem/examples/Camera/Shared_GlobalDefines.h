/*
 * Shared_GlobalDefines.h - 全局定义头文件
 * 定义系统中使用的全局宏和常量
 * 阶段五：相机模块开发 - 全局定义
 */

#ifndef GLOBAL_DEFINES_H
#define GLOBAL_DEFINES_H

// ===============================================
// 系统版本号定义
// ===============================================
#define SYSTEM_VERSION_MAJOR 1
#define SYSTEM_VERSION_MINOR 111
#define SYSTEM_VERSION_STRING "V1.111"

// ===============================================
// 音频录制配置
// ===============================================
#define AUDIO_ENABLED 1
#define AUDIO_SAMPLE_RATE 16000  // 16kHz采样率
#define AUDIO_CHANNELS 1          // 单声道
#define AUDIO_BITS_PER_SAMPLE 16  // 16位采样
#define AUDIO_FRAME_SIZE 2048     // 音频帧大小(采样数)

// ===============================================
// INMP441 I2S录音模块引脚定义
// ===============================================
// I2S1接口 - 录音时使用(RX模式)
// INMP441 模块 -> AMB82-MINI 开发板
//   SD(DOUT)   -> 16号引脚 (AMB_D16/PD_18/I2S1_SD_RX)  音频数据输出
//   WS(LRC)    -> 17号引脚 (AMB_D17/PD_17/I2S1_WS)      字选择/左右声道时钟
//   SCK(BCLK)  -> 20号引脚 (AMB_D20/PD_14/I2S1_CLK)     位时钟
//   L/R        -> GND                               接地(左声道模式)
//   VCC        -> 3.3V                              电源
// 注意：INMP441只使用I2S1的RX功能，MAX98357A使用TX功能，二者分时复用I2S1接口

// ===============================================
// MAX98357A I2S音频播放模块引脚定义
// ===============================================
// I2S1接口 - 播放时使用(TX模式)
// MAX98357A 模块 -> AMB82-MINI 开发板
//   VCC         -> 3.3V独立稳压电源                   供电
//   GND         -> GND                               地
//   BCLK(SCK)   -> 20号引脚 (AMB_D20/PD_14/I2S1_CLK) 位时钟 (与INMP441共用)
//   DIN(SDATA)  -> PD_15 (AMB_D19/PD_15/I2S1_SD_TX0) 音频数据输入 (MAX98357A专用)
//   LRCK(WS)    -> 17号引脚 (AMB_D17/PD_17/I2S1_WS) 字选择/左右声道时钟 (与INMP441共用)
//   GAIN        -> 10kΩ电阻 → GND                   增益设置(默认9dB)
//   SD          -> 悬空(左声道模式)或接地(右声道)    通道选择
//   OUT+        -> 扬声器正极(+100Ω限流电阻)         音频输出
//   OUT-        -> 扬声器负极(+100Ω限流电阻)         音频输出
// 注意：MAX98357A只使用I2S1的TX功能，与INMP441分时复用I2S1接口

// ===============================================
// I2S1接口引脚复用关系表
// ===============================================
// 引脚    | I2S1功能      | INMP441(录音) | MAX98357A(播放)
// --------|---------------|---------------|-----------------
// D16     | I2S1_SD_RX   | SD(数据输出)  | 未使用
// D17     | I2S1_WS      | WS(字选择)    | LRCK(字选择)
// D19     | I2S1_SD_TX0  | 未使用        | DIN(数据输入)
// D20     | I2S1_CLK     | SCK(位时钟)   | BCLK(位时钟)
// --------|---------------|---------------|-----------------
// 录音与播放不能同时运行，必须分时切换I2S1工作模式

// ===============================================
// TFT屏幕引脚定义
// ===============================================
// SPI1接口引脚分配（与INMP441录音模块共享，需分时复用）
// ST7789 TFT屏幕 -> AMB82-MINI 开发板
//   SCK         -> 1号引脚 (AMB_D1/PF_6/SPI1_SCLK)    SPI时钟
//   SD(MOSI)    -> 2号引脚 (AMB_D2/PF_7/SPI1_MOSI)    SPI数据输出
//   TFT_CS      -> SPI1_SS (定义于variant.h)           片选
//   TFT_DC      -> 4号引脚 (AMB_D4/PF_11)              数据/命令选择
//   TFT_RST     -> 5号引脚 (AMB_D5/PF_12)              复位
//   TFT_BL      -> 6号引脚 (AMB_D6/PF_13)              背光控制
// 注意：录音与播放不能同时运行，共用同一组SPI1/I2S1接口
#define TFT_CS   SPI1_SS
#define TFT_DC   4
#define TFT_RST  5
#define TFT_BL   6

// ===============================================
// EC11旋转编码器引脚定义
// ===============================================
#define ENCODER_CLK 7   //引脚A 这三个引脚与EC11旋转编码器的GND引脚直接都并联上10nF的陶瓷电容，可以减少抖动
#define ENCODER_DT  8   //引脚B 
#define ENCODER_SW  14  //引脚C 000

// ===============================================
// DS3231 RTC时钟模块引脚定义
// ===============================================
// I2C1接口引脚分配
// DS3231 模块 -> AMB82-MINI 开发板
//   SCL        -> 10号引脚 (AMB_D10/PF_1/I2C1_SCL)     I2C时钟
//   SDA        -> 9号引脚 (AMB_D9/PF_2/I2C1_SDA)      I2C数据
//   VCC        -> 3.3V                                  电源
//   GND        -> GND                                   地
// 注意：DS3231使用I2C1接口，与LOG_UART的CTS/RTS引脚复用

// ===============================================
// 相机通道与配置定义
// ===============================================
#define PREVIEW_CH  0  // 预览通道：VGA
#define STILL_CH    1  // 拍照通道：720p

// ===============================================
// FreeRTOS事件组定义
// ===============================================
#define EVENT_TASK_A_REQUESTED    (1 << 0)
#define EVENT_TASK_B_REQUESTED    (1 << 1)
#define EVENT_TASK_C_REQUESTED    (1 << 2)
#define EVENT_TASK_D_REQUESTED    (1 << 3)
#define EVENT_TASK_E_REQUESTED    (1 << 4)
#define EVENT_TASK_F_REQUESTED    (1 << 5)
#define EVENT_RETURN_TO_MENU      (1 << 6)
#define EVENT_NEXT_PHOTO          (1 << 7)  // 显示下一张照片
#define EVENT_PREVIOUS_PHOTO      (1 << 8)  // 显示上一张照片
#define EVENT_ALL_TASKS_CLEAR     (0x1FF) // 清除所有任务事件（更新为包含新的照片导航事件）

// ===============================================
// 系统常量定义
// ===============================================
#define BUTTON_DEBOUNCE_DELAY 50  // 按钮消抖延迟（毫秒）
#define ENCODER_ROTATION_DEBOUNCE_DELAY 100  // 旋转编码器旋转消抖延迟（毫秒）
#define CAM_FPS 30                // 相机帧率设置（帧/秒）

// ===============================================
// DS3231时间读取与计数功能控制开关
// ===============================================
// 1: 启用DS3231时间读取与计数功能
// 0: 禁用DS3231时间读取与计数功能
#define DS3231_TIME_READ_ENABLED 1  

// ===============================================
// ST7789颜色常量定义
// ===============================================
#define ST7789_BLACK       0x0000
#define ST7789_WHITE       0xFFFF
#define ST7789_RED         0xF800
#define ST7789_GREEN       0x07E0
#define ST7789_BLUE        0x001F
#define ST7789_CYAN        0x07FF
#define ST7789_MAGENTA     0xF81F
#define ST7789_YELLOW      0xFFE0

// ===============================================
// 屏幕分辨率定义
// ===============================================
#define ST7789_TFTWIDTH  240
#define ST7789_TFTHEIGHT 320

#endif // GLOBAL_DEFINES_H