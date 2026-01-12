/*
 * Shared_GlobalDefines.h - 全局定义头文件
 * 定义系统中使用的全局宏和常量
 * 阶段五：相机模块开发 - 全局定义
 */

#ifndef GLOBAL_DEFINES_H
#define GLOBAL_DEFINES_H

// ===============================================
// TFT屏幕引脚定义
// ===============================================
#define TFT_CS   SPI1_SS
#define TFT_DC   4
#define TFT_RST  5
#define TFT_BL   6

// ===============================================
// EC11旋转编码器引脚定义
// ===============================================
#define ENCODER_CLK 7   //这三个引脚与EC11旋转编码器的GND
#define ENCODER_DT  8   //引脚直接都并联上10nF的陶瓷电容，
#define ENCODER_SW  14  //可以减少抖动

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
// DS1307时间读取与计数功能控制开关
// ===============================================
// 1: 启用DS1307时间读取与计数功能
// 0: 禁用DS1307时间读取与计数功能
#define DS1307_TIME_READ_ENABLED 0  

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