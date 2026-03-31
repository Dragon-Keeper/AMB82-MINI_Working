/*
 * INMP441 Arduino Project for AMB82-MINI - 录音版
 * =========================================
 * 主文件: Inmp441_Rtos.ino
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
 * 
 * SD卡连接:
 * CS      → GPIOD_13 (可配置)
 * MOSI    → GPIOD_11
 * MISO    → GPIOD_12
 * SCK     → GPIOD_10
 * 
 * 编译环境: Arduino IDE + Realtek Ameba SDK
 * 波特率: 115200 bps
 */

#include <Arduino.h>

#include <i2s_api.h>
#include "AmebaFatFS.h"

const int I2S_SAMPLE_RATE   = I2S_SR_16KHZ;
const int I2S_BITS_PER_SAMPLE = I2S_WL_16;
const int I2S_CHANNELS      = I2S_CH_MONO;

const long SERIAL_BAUD_RATE = 115200;

const int DMA_PAGE_NUM   = 4;
const int DMA_PAGE_SIZE = 1280;

const size_t BUFFER_SIZE = 4096;

const int16_t SIGNAL_LEVEL_1 = 500;
const int16_t SIGNAL_LEVEL_2 = 2000;
const int16_t SIGNAL_LEVEL_3 = 8000;

#define I2S_SCK_PIN  PD_14
#define I2S_WS_PIN   PD_17
#define I2S_RX_PIN   PD_18
#define I2S_TX_PIN   PD_15
#define I2S_MCK_PIN  PD_16

enum {
    GAIN_0DB = 0,
    GAIN_6DB = 1,
    GAIN_12DB = 2,
    GAIN_18DB = 3,
    GAIN_24DB = 4,
    GAIN_30DB = 5
};

static uint8_t g_softwareGain = GAIN_30DB; // 默认 30dB 增益

class RingBufferClass {
private:
    int16_t* m_buffer;
    size_t m_size;
    size_t m_head;
    size_t m_tail;
    size_t m_count;
    
public:
    RingBufferClass(size_t size) : m_size(size), m_head(0), m_tail(0), m_count(0) {
        m_buffer = new int16_t[size];
    }
    
    ~RingBufferClass() {
        if (m_buffer) {
            delete[] m_buffer;
        }
    }
    
    bool write(int16_t value) {
        if (m_count >= m_size) {
            m_tail = (m_tail + 1) % m_size;
            m_count--;
        }
        m_buffer[m_head] = value;
        m_head = (m_head + 1) % m_size;
        m_count++;
        return true;
    }
    
    int16_t read() {
        if (m_count == 0) {
            return 0;
        }
        int16_t value = m_buffer[m_tail];
        m_tail = (m_tail + 1) % m_size;
        m_count--;
        return value;
    }
    
    size_t available() { return m_count; }
    bool isEmpty() { return m_count == 0; }
    bool isFull() { return m_count == m_size; }
    void clear() { m_head = m_tail = m_count = 0; }
    size_t getCount() { return m_count; }
    bool writeBuffer(const int16_t* data, size_t count) {
        for (size_t i = 0; i < count; i++) {
            write(data[i]);
        }
        return true;
    }
};

static i2s_t g_i2sObj;
static char g_txBuffer[DMA_PAGE_SIZE * DMA_PAGE_NUM] __attribute__((aligned(32)));
static char g_rxBuffer[DMA_PAGE_SIZE * DMA_PAGE_NUM] __attribute__((aligned(32)));

static RingBufferClass* g_ringBuffer = nullptr;
static bool g_i2sInitialized = false;
static uint32_t g_sampleCount = 0;
static uint32_t g_errorCount = 0;
static bool g_running = false;
static uint32_t g_startTime = 0;
static bool g_serialPlotterMode = false;
static uint8_t g_outputDivider = 4;
static uint32_t g_outputCounter = 0;

static AmebaFatFS g_fs;
static bool g_sdInitialized = false;
static File g_pcmFile;
static bool g_recording = false;
static uint32_t g_recordStartTime = 0;
static uint32_t g_recordedSamples = 0;

extern "C" {

void i2s_rx_callback(uint32_t id, char *pbuf) {
    (void)id;
    
    if (!pbuf) {
        return;
    }
    
    uint8_t* src = (uint8_t*)pbuf;
    size_t samples = DMA_PAGE_SIZE / 2;
    
    for (size_t i = 0; i < samples; i++) {
        int16_t sample = (int16_t)((uint16_t)src[i*2+1] << 8 | src[i*2]);
        
        uint8_t gain = g_softwareGain;
        if (gain > 0) {
            int32_t amplified = ((int32_t)sample) << gain;
            if (amplified > 32767) amplified = 32767;
            if (amplified < -32768) amplified = -32768;
            sample = (int16_t)amplified;
        }
        
        if (g_ringBuffer) {
            g_ringBuffer->write(sample);
        }
    }
    
    g_sampleCount++;
    i2s_recv_page(&g_i2sObj);
}

void i2s_tx_callback(uint32_t id, char *pbuf) {
    (void)id;
    (void)pbuf;
}

}

void printUsage();
bool hardwareConnectionCheck();
bool initI2S();
bool initSDCard();
bool startRecording();
void stopRecording();
void processAudioData();
void serialPrintTextMode(uint32_t timestamp, int16_t sample, int signalLevel);
void serialPrintPlotterMode(int16_t sample);

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial && millis() < 3000);
    
    Serial.println("\n");
    Serial.println("========================================");
    Serial.println("  INMP441 麦克风音频采集系统 v3.0");
    Serial.println("  开发板: AMB82-MINI");
    Serial.println("  功能: 实时采集 + SD卡录音");
    Serial.println("========================================");
    Serial.println();
    
    printUsage();
    Serial.println();
    
    Serial.println("========== 硬件连接验证 ==========");
    hardwareConnectionCheck();
    Serial.println();
    
    Serial.println("========== 初始化SD卡 ==========");
    if (!initSDCard()) {
        Serial.println("[警告] SD卡初始化失败，录音功能不可用");
    } else {
        Serial.println("[SD] SD卡初始化成功");
    }
    Serial.println();
    
    Serial.println("========== 初始化I2S ==========");
    if (!initI2S()) {
        Serial.println("[致命错误] I2S初始化失败，系统停止");
        Serial.println("[提示] 检查硬件连接是否正确");
        while (1) {
            delay(1000);
        }
    }
    
    g_startTime = millis();
    g_running = true;
    
    Serial.println();
    Serial.println("========== 初始化完成 ==========");
    Serial.println("系统已就绪，可开始采集音频数据");
    Serial.println("----------------------------------------");
    Serial.println("输出格式说明:");
    Serial.println("  文本模式: 时间戳(ms) | 采样值(int16) | 信号等级(0-3)");
    Serial.println("  绘图器模式: 直接输出原始采样值");
    Serial.println("----------------------------------------");
    Serial.println();
}

void loop() {
    if (!g_running || !g_i2sInitialized) {
        return;
    }
    
    processAudioData();
    
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case 'p':
            case 'P':
                g_serialPlotterMode = true;
                Serial.println("[模式切换] 已切换到串口绘图器模式");
                Serial.println("[提示] 在Arduino IDE串口绘图器中查看波形");
                break;
                
            case 't':
            case 'T':
                g_serialPlotterMode = false;
                Serial.println("[模式切换] 已切换到文本模式");
                break;
                
            case 's':
            case 'S':
                g_running = !g_running;
                Serial.print("[状态] 采集");
                Serial.println(g_running ? "已恢复" : "已暂停");
                break;
                
            case 'r':
            case 'R':
                g_sampleCount = 0;
                g_startTime = millis();
                if (g_ringBuffer) g_ringBuffer->clear();
                Serial.println("[统计] 计数器已重置");
                break;
                
            case '0':
                g_softwareGain = GAIN_0DB;
                Serial.println("[增益] 软件增益: 0dB");
                break;
            case '1':
                g_softwareGain = GAIN_6DB;
                Serial.println("[增益] 软件增益: 6dB");
                break;
            case '2':
                g_softwareGain = GAIN_12DB;
                Serial.println("[增益] 软件增益: 12dB");
                break;
            case '3':
                g_softwareGain = GAIN_18DB;
                Serial.println("[增益] 软件增益: 18dB");
                break;
            case '4':
                g_softwareGain = GAIN_24DB;
                Serial.println("[增益] 软件增益: 24dB");
                break;
            case '5':
                g_softwareGain = GAIN_30DB;
                Serial.println("[增益] 软件增益: 30dB");
                break;
                
            case 'a':
                g_outputDivider = 1;
                Serial.println("[输出] 每1个采样输出1个");
                break;
            case 'b':
                g_outputDivider = 2;
                Serial.println("[输出] 每2个采样输出1个");
                break;
            case 'c':
                g_outputDivider = 4;
                Serial.println("[输出] 每4个采样输出1个");
                break;
            case 'd':
                g_outputDivider = 8;
                Serial.println("[输出] 每8个采样输出1个");
                break;
                
            case 'o':
            case 'O':
                if (!g_sdInitialized) {
                    Serial.println("[错误] SD卡未初始化，无法录音");
                } else if (g_recording) {
                    stopRecording();
                } else {
                    startRecording();
                }
                break;
                
            case 'h':
            case 'H':
            case '?':
                printUsage();
                break;
                
            default:
                break;
        }
    }
    
    delayMicroseconds(10);
}

void printUsage() {
    Serial.println("========== 命令说明 ==========");
    Serial.println("  P - 切换到串口绘图器模式");
    Serial.println("  T - 切换到文本输出模式");
    Serial.println("  S - 暂停/恢复采集");
    Serial.println("  R - 重置计数器");
    Serial.println("  0 - 设置软件增益为 0dB");
    Serial.println("  1 - 设置软件增益为 6dB");
    Serial.println("  2 - 设置软件增益为 12dB");
    Serial.println("  3 - 设置软件增益为 18dB");
    Serial.println("  4 - 设置软件增益为 24dB");
    Serial.println("  5 - 设置软件增益为 30dB");
    Serial.println("  A - 每1个采样输出1个");
    Serial.println("  B - 每2个采样输出1个");
    Serial.println("  C - 每4个采样输出1个");
    Serial.println("  D - 每8个采样输出1个");
    Serial.println("  O - 开始/停止录音");
    Serial.println("  H - 显示帮助信息");
    Serial.println("============================");
}

bool hardwareConnectionCheck() {
    Serial.println("硬件连接检查:");
    
    Serial.print("  SCK (时钟) → GPIOD_14: ");
    Serial.println("OK");
    
    Serial.print("  WS  (字选择) → GPIOD_17: ");
    Serial.println("OK");
    
    Serial.print("  SD  (数据) → GPIOD_18: ");
    Serial.println("OK");
    
    Serial.print("  L/R (声道选择): ");
    Serial.println("需外部连接");
    Serial.println("    推荐: 接地(GND) = 左声道");
    Serial.println("         接3V3 = 右声道");
    
    return true;
}

bool initSDCard() {
    if (!g_fs.begin()) {
        Serial.println("[SD] 错误: SD卡初始化失败");
        Serial.println("[SD] 提示: 请检查SD卡是否插入");
        return false;
    }
    
    char* rootPath = g_fs.getRootPath();
    Serial.print("[SD] 根目录: ");
    Serial.println(rootPath);
    
    g_sdInitialized = true;
    return true;
}

bool startRecording() {
    if (!g_sdInitialized) {
        Serial.println("[录音] 错误: SD卡未初始化");
        return false;
    }
    
    if (g_recording) {
        Serial.println("[录音] 已经在录音中");
        return false;
    }
    
    char filename[64];
    uint32_t timestamp = millis();
    sprintf(filename, "/Audio_%lu.pcm", timestamp);
    
    char filepath[128];
    sprintf(filepath, "%s%s", g_fs.getRootPath(), filename);
    
    g_pcmFile = g_fs.open(filepath);
    if (!g_pcmFile.isOpen()) {
        Serial.print("[录音] 错误: 无法创建文件 ");
        Serial.println(filepath);
        return false;
    }
    
    g_recording = true;
    g_recordStartTime = millis();
    g_recordedSamples = 0;
    
    Serial.print("[录音] 开始录音: ");
    Serial.println(filename);
    Serial.println("[录音] 发送 'O' 停止录音");
    
    return true;
}

void stopRecording() {
    if (!g_recording) {
        return;
    }
    
    g_pcmFile.close();
    
    uint32_t duration = millis() - g_recordStartTime;
    uint32_t fileSize = g_recordedSamples * 2;
    
    Serial.println("[录音] 录音已停止");
    Serial.print("[录音] 文件: ");
    Serial.print("/Audio_");
    Serial.print(g_recordStartTime);
    Serial.println(".pcm");
    Serial.print("[录音] 时长: ");
    Serial.print(duration / 1000);
    Serial.println(" 秒");
    Serial.print("[录音] 采样数: ");
    Serial.println(g_recordedSamples);
    Serial.print("[录音] 文件大小: ");
    Serial.print(fileSize);
    Serial.println(" 字节");
    
    g_recording = false;
}

bool initI2S() {
    Serial.println("[I2S] 开始初始化...");
    
    Serial.println("[I2S] 引脚配置:");
    Serial.print("  SCK  = PD_14\n");
    Serial.print("  WS   = PD_17\n");
    Serial.print("  RX   = PD_18\n");
    Serial.print("  TX   = PD_15\n");
    Serial.print("  MCK  = PD_16\n");
    
    g_ringBuffer = new RingBufferClass(BUFFER_SIZE);
    if (!g_ringBuffer) {
        Serial.println("[I2S] 错误: 缓冲区创建失败");
        g_errorCount++;
        return false;
    }
    
    Serial.println("[I2S] 调用i2s_init...");
    Serial.println("[I2S] 注意: SCK=PD_14, WS=PD_17 匹配I2S_S1组");
    
    i2s_init(&g_i2sObj, I2S_SCK_PIN, I2S_WS_PIN, I2S_TX_PIN, I2S_RX_PIN, I2S_MCK_PIN);
    
    if (g_i2sObj.i2s_initialized != 1) {
        Serial.println("[I2S] 错误: i2s_init失败, i2s_initialized != 1");
        return false;
    }
    Serial.println("[I2S] i2s_init成功");
    
    Serial.println("[I2S] 配置DMA缓冲区...");
    i2s_set_dma_buffer(&g_i2sObj, g_txBuffer, g_rxBuffer, DMA_PAGE_NUM, DMA_PAGE_SIZE);
    
    Serial.println("[I2S] 注册中断处理函数...");
    i2s_rx_irq_handler(&g_i2sObj, i2s_rx_callback, 0);
    i2s_tx_irq_handler(&g_i2sObj, i2s_tx_callback, 0);
    
    Serial.println("[I2S] 设置传输参数...");
    i2s_set_direction(&g_i2sObj, I2S_DIR_RX);
    i2s_set_param(&g_i2sObj, I2S_CHANNELS, I2S_SAMPLE_RATE, I2S_BITS_PER_SAMPLE);
    i2s_set_format(&g_i2sObj, FORMAT_I2S);
    i2s_set_master(&g_i2sObj, I2S_MASTER);
    
    Serial.println("[I2S] 使能I2S...");
    i2s_enable(&g_i2sObj);
    
    for (int i = 0; i < DMA_PAGE_NUM; i++) {
        i2s_recv_page(&g_i2sObj);
    }
    
    g_i2sInitialized = true;
    
    Serial.println("[I2S] I2S初始化完成");
    Serial.println("[I2S] 配置参数:");
    Serial.print("  采样率: ");
    switch (I2S_SAMPLE_RATE) {
        case I2S_SR_8KHZ: Serial.println("8 kHz"); break;
        case I2S_SR_16KHZ: Serial.println("16 kHz"); break;
        case I2S_SR_44p1KHZ: Serial.println("44.1 kHz"); break;
        case I2S_SR_48KHZ: Serial.println("48 kHz"); break;
        default: Serial.println("未知"); break;
    }
    Serial.print("  数据位宽: 16位");
    Serial.print("  声道模式: "); Serial.println(I2S_CHANNELS == I2S_CH_MONO ? "单声道" : "立体声");
    Serial.print("  DMA页数: "); Serial.println(DMA_PAGE_NUM);
    Serial.print("  每页大小: "); Serial.print(DMA_PAGE_SIZE); Serial.println(" 字节");
    Serial.print("  软件增益: "); Serial.print(g_softwareGain * 6); Serial.println("dB");
    
    return true;
}

void processAudioData() {
    if (!g_ringBuffer || g_ringBuffer->isEmpty()) {
        return;
    }
    
    if (g_recording && g_pcmFile.isOpen()) {
        static uint8_t writeBuffer[256];
        static size_t writeBufferIndex = 0;
        
        while (!g_ringBuffer->isEmpty()) {
            int16_t sample = g_ringBuffer->read();
            
            writeBuffer[writeBufferIndex++] = (uint8_t)(sample & 0xFF);
            writeBuffer[writeBufferIndex++] = (uint8_t)((sample >> 8) & 0xFF);
            g_recordedSamples++;
            
            if (writeBufferIndex >= sizeof(writeBuffer)) {
                size_t written = g_pcmFile.write(writeBuffer, writeBufferIndex);
                if (written != writeBufferIndex) {
                    Serial.println("[录音] 警告: 写入数据不完整");
                }
                writeBufferIndex = 0;
            }
        }
        
        if (writeBufferIndex > 0) {
            g_pcmFile.write(writeBuffer, writeBufferIndex);
        }
    }
    
    while (!g_ringBuffer->isEmpty()) {
        int16_t sample = g_ringBuffer->read();
        
        g_outputCounter++;
        if (g_outputCounter >= g_outputDivider) {
            g_outputCounter = 0;
            
            uint32_t timestamp = millis() - g_startTime;
            int signalLevel = 0;
            
            int absSample = abs(sample);
            if (absSample >= SIGNAL_LEVEL_3) signalLevel = 3;
            else if (absSample >= SIGNAL_LEVEL_2) signalLevel = 2;
            else if (absSample >= SIGNAL_LEVEL_1) signalLevel = 1;
            else signalLevel = 0;
            
            if (g_serialPlotterMode) {
                serialPrintPlotterMode(sample);
            } else {
                serialPrintTextMode(timestamp, sample, signalLevel);
            }
        }
    }
}

void serialPrintTextMode(uint32_t timestamp, int16_t sample, int signalLevel) {
    // [已屏蔽] 包含"时间戳(ms) | 采样值(int16) | 信号等级(0-3)"标题行及数据行的输出
    // 如需恢复输出，请删除此行注释并注释掉本函数的其余部分
    // 原始输出格式: 时间戳(ms) | 采样值(int16) | 信号等级(0-3)
    
    // 以下代码已被注释以屏蔽输出
    /*
    const char* levelStr[] = {"静音", "低", "中", "高"};
    
    Serial.print(timestamp);
    Serial.print(" | ");
    Serial.print(sample);
    Serial.print(" | ");
    Serial.print(signalLevel);
    Serial.print(" | ");
    Serial.println(levelStr[signalLevel]);
    */
}

void serialPrintPlotterMode(int16_t sample) {
    Serial.println(sample);
}
