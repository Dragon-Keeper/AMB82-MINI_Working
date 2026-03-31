/*
 * Inmp441_MicrophoneManager.h - INMP441麦克风管理器
 * V1.28: 移除不必要的DC偏移消除和数字滤波，恢复与原项目一致的简单实现
 * 
 * 功能描述:
 * - 通过AMB82-MINI开发板驱动I2S接口的INMP441麦克风模块
 * - 实现稳定的音频数据实时采集
 * - 支持SD卡PCM格式录音存储
 * - 串口输出包含时间戳、采样值和信号强度等级
 * - 支持串口绘图器可视化音频波形
 * 
 * 硬件连接:
 * INMP441 → AMB82-MINI
 * VDD     → 3v3
 * GND     → GND
 * SCK     → GPIOD_14 (I2S Serial Clock)
 * WS      → GPIOD_17 (I2S Word Select)
 * SD      → GPIOD_18 (I2S Serial Data)
 * L/R     → GND (左声道)
 */

#ifndef INMP441_MICROPHONE_MANAGER_H
#define INMP441_MICROPHONE_MANAGER_H

#include <Arduino.h>
#include <i2s_api.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "AmebaFatFS.h"

// 配置常量
#define I2S_SAMPLE_RATE   I2S_SR_16KHZ
#define I2S_BITS_PER_SAMPLE I2S_WL_16
#define I2S_CHANNELS      I2S_CH_MONO
#define SERIAL_BAUD_RATE  115200
#define DMA_PAGE_NUM   4
#define DMA_PAGE_SIZE 1280
#define BUFFER_SIZE   4096
#define WRITE_BUFFER_SIZE 256
#define SIGNAL_LEVEL_1    500
#define SIGNAL_LEVEL_2    2000
#define SIGNAL_LEVEL_3    8000

// 引脚定义
#define I2S_SCK_PIN  PD_14
#define I2S_WS_PIN   PD_17
#define I2S_RX_PIN   PD_18
#define I2S_TX_PIN   PD_15
#define I2S_MCK_PIN  PD_16

// 增益枚举
enum {
    GAIN_0DB = 0,
    GAIN_6DB = 1,
    GAIN_12DB = 2,
    GAIN_18DB = 3,
    GAIN_24DB = 4,
    GAIN_30DB = 5
};

// 音频数据块结构体（用于队列传递）
typedef struct {
    int16_t samples[512];  // 512个样本（32ms音频数据）
    size_t count;          // 实际样本数
    uint32_t timestamp;    // 时间戳（毫秒）
} AudioDataBlock;

// 音频队列配置
#define AUDIO_QUEUE_SIZE 128  // 队列最多存储128个音频块（约4秒缓冲）

// 回调函数声明
extern "C" {
    void i2s_rx_callback(uint32_t id, char *pbuf);
    void i2s_tx_callback(uint32_t id, char *pbuf);
}

class RingBufferClass {
private:
    int16_t* m_buffer;
    size_t m_size;
    size_t m_head;
    size_t m_tail;
    size_t m_count;
    
public:
    RingBufferClass(size_t size);
    ~RingBufferClass();
    
    bool write(int16_t value);
    int16_t read();
    size_t available();
    bool isEmpty();
    bool isFull();
    void clear();
    size_t getCount();
    bool writeBuffer(const int16_t* data, size_t count);
};

class Inmp441MicrophoneManager {
public:
    Inmp441MicrophoneManager();
    ~Inmp441MicrophoneManager();
    
    // 初始化
    bool init();
    
    // 主循环处理
    void loop();
    
    // 命令处理
    void handleCommand(char cmd);
    
    // 状态查询
    bool isInitialized() { return m_i2sInitialized; }
    bool isRunning() { return m_running; }
    bool isRecording() { return m_recording; }
    uint32_t getSampleCount() { return m_sampleCount; }
    uint32_t getErrorCount() { return m_errorCount; }
    
    // 打印使用说明
    void printUsage();
    
    // 硬件连接检查
    bool hardwareConnectionCheck();

    // 设置外部文件系统（使用sdCardManager的实例）
    void setFileSystem(AmebaFatFS* fs) { m_pfs = fs; if (fs) m_sdInitialized = true; }

    // 录音控制（公开给外部调用）
    bool startRecording();
    void stopRecording();
    
    // 音视频同步录制接口
    bool startAVIRecording();
    void stopAVIRecording();
    size_t getAvailableAudioSamples();
    size_t readAudioSamples(int16_t* buffer, size_t maxSamples);
    
    // RTOS音频队列接口
    bool initAudioQueue();
    void destroyAudioQueue();
    bool sendAudioDataBlock(const AudioDataBlock* block, TickType_t timeout = portMAX_DELAY);
    bool receiveAudioDataBlock(AudioDataBlock* block, TickType_t timeout = portMAX_DELAY);
    size_t getAudioQueueAvailable();
    size_t getAudioQueueFree();
    
    // 让回调函数可以访问私有成员
    friend void i2s_rx_callback(uint32_t id, char *pbuf);
    friend void i2s_tx_callback(uint32_t id, char *pbuf);

private:
    // I2S对象
    i2s_t m_i2sObj;
    char m_txBuffer[DMA_PAGE_SIZE * DMA_PAGE_NUM] __attribute__((aligned(32)));
    char m_rxBuffer[DMA_PAGE_SIZE * DMA_PAGE_NUM] __attribute__((aligned(32)));
    
    // 环形缓冲区
    RingBufferClass* m_ringBuffer;
    
    // 状态变量
    bool m_i2sInitialized;
    uint32_t m_sampleCount;
    uint32_t m_errorCount;
    bool m_running;
    uint32_t m_startTime;
    bool m_serialPlotterMode;
    uint8_t m_outputDivider;
    uint32_t m_outputCounter;
    uint8_t m_softwareGain;
    uint32_t m_bufferOverflowCount;
    size_t m_maxBufferUsed;
    
    // SD卡相关
    AmebaFatFS* m_pfs;  // 指向外部文件系统（由sdCardManager提供）
    bool m_sdInitialized;
    File m_pcmFile;
    bool m_recording;
    uint32_t m_recordStartTime;
    uint32_t m_recordedSamples;
    
    // 录音缓冲区
    uint8_t m_writeBuffer[WRITE_BUFFER_SIZE];
    size_t m_writeBufferIndex;
    
    // RTOS音频队列
    QueueHandle_t m_audioQueue;
    
    // 私有方法
    bool initI2S();
    bool initSDCard();
    void processAudioData();
    void serialPrintTextMode(uint32_t timestamp, int16_t sample, int signalLevel);
    void serialPrintPlotterMode(int16_t sample);
};

// 全局麦克风管理器实例
extern Inmp441MicrophoneManager g_microphoneManager;

#endif // INMP441_MICROPHONE_MANAGER_H
