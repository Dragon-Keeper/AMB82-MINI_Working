/*
 * Inmp441_MicrophoneManager.cpp - INMP441麦克风管理器实现
 * 完整迁移自 Inmp441_Rtos.ino
 */

#include "Inmp441_MicrophoneManager.h"

// 全局麦克风管理器实例
Inmp441MicrophoneManager g_microphoneManager;

// 静态指针用于回调访问
static Inmp441MicrophoneManager* s_microphoneManagerPtr = nullptr;

// ===============================================
// RingBufferClass 实现
// ===============================================

RingBufferClass::RingBufferClass(size_t size) : m_size(size), m_head(0), m_tail(0), m_count(0) {
    m_buffer = new int16_t[size];
}

RingBufferClass::~RingBufferClass() {
    if (m_buffer) {
        delete[] m_buffer;
    }
}

bool RingBufferClass::write(int16_t value) {
    if (m_count >= m_size) {
        m_tail = (m_tail + 1) % m_size;
        m_count--;
    }
    m_buffer[m_head] = value;
    m_head = (m_head + 1) % m_size;
    m_count++;
    return true;
}

int16_t RingBufferClass::read() {
    if (m_count == 0) {
        return 0;
    }
    int16_t value = m_buffer[m_tail];
    m_tail = (m_tail + 1) % m_size;
    m_count--;
    return value;
}

size_t RingBufferClass::available() { 
    return m_count; 
}

bool RingBufferClass::isEmpty() { 
    return (m_count == 0); 
}

bool RingBufferClass::isFull() { 
    return (m_count == m_size); 
}

void RingBufferClass::clear() { 
    m_head = m_tail = m_count = 0; 
}

size_t RingBufferClass::getCount() { 
    return m_count; 
}

bool RingBufferClass::writeBuffer(const int16_t* data, size_t count) {
    for (size_t i = 0; i < count; i++) {
        write(data[i]);
    }
    return true;
}

// ===============================================
// Inmp441MicrophoneManager 实现
// ===============================================

Inmp441MicrophoneManager::Inmp441MicrophoneManager()
    : m_ringBuffer(nullptr)
    , m_i2sInitialized(false)
    , m_sampleCount(0)
    , m_errorCount(0)
    , m_running(false)
    , m_startTime(0)
    , m_serialPlotterMode(false)
    , m_outputDivider(4)
    , m_outputCounter(0)
    , m_softwareGain(GAIN_30DB)
    , m_bufferOverflowCount(0)
    , m_maxBufferUsed(0)
    , m_pfs(nullptr)
    , m_sdInitialized(false)
    , m_recording(false)
    , m_recordStartTime(0)
    , m_recordedSamples(0)
    , m_writeBufferIndex(0)
    , m_audioQueue(NULL)
{
    s_microphoneManagerPtr = this;
}

Inmp441MicrophoneManager::~Inmp441MicrophoneManager() {
    if (m_ringBuffer) {
        delete m_ringBuffer;
    }
    if (m_audioQueue) {
        vQueueDelete(m_audioQueue);
        m_audioQueue = NULL;
    }
    s_microphoneManagerPtr = nullptr;
}

bool Inmp441MicrophoneManager::init() {
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
        return false;
    }
    
    m_startTime = millis();
    m_running = true;
    
    Serial.println();
    Serial.println("========== 初始化完成 ==========");
    Serial.println("系统已就绪，可开始采集音频数据");
    Serial.println("----------------------------------------");
    Serial.println("输出格式说明:");
    Serial.println("  文本模式: 时间戳(ms) | 采样值(int16) | 信号等级(0-3)");
    Serial.println("  绘图器模式: 直接输出原始采样值");
    Serial.println("----------------------------------------");
    Serial.println();
    
    return true;
}

void Inmp441MicrophoneManager::loop() {
    if (!m_running || !m_i2sInitialized) {
        return;
    }
    
    processAudioData();
    delayMicroseconds(10);
}

void Inmp441MicrophoneManager::handleCommand(char cmd) {
    switch (cmd) {
        case 'p':
        case 'P':
            m_serialPlotterMode = true;
            Serial.println("[模式切换] 已切换到串口绘图器模式");
            Serial.println("[提示] 在Arduino IDE串口绘图器中查看波形");
            break;
            
        case 't':
        case 'T':
            m_serialPlotterMode = false;
            Serial.println("[模式切换] 已切换到文本模式");
            break;
            
        case 's':
        case 'S':
            m_running = !m_running;
            Serial.print("[状态] 采集");
            Serial.println(m_running ? "已恢复" : "已暂停");
            break;
            
        case 'r':
        case 'R':
            m_sampleCount = 0;
            m_startTime = millis();
            if (m_ringBuffer) m_ringBuffer->clear();
            Serial.println("[统计] 计数器已重置");
            break;
            
        case '0':
            m_softwareGain = GAIN_0DB;
            Serial.println("[增益] 软件增益: 0dB");
            break;
        case '1':
            m_softwareGain = GAIN_6DB;
            Serial.println("[增益] 软件增益: 6dB");
            break;
        case '2':
            m_softwareGain = GAIN_12DB;
            Serial.println("[增益] 软件增益: 12dB");
            break;
        case '3':
            m_softwareGain = GAIN_18DB;
            Serial.println("[增益] 软件增益: 18dB");
            break;
        case '4':
            m_softwareGain = GAIN_24DB;
            Serial.println("[增益] 软件增益: 24dB");
            break;
        case '5':
            m_softwareGain = GAIN_30DB;
            Serial.println("[增益] 软件增益: 30dB");
            break;
            
        case 'a':
        case 'A':
            m_outputDivider = 1;
            Serial.println("[输出] 每1个采样输出1个");
            break;
        case 'b':
        case 'B':
            m_outputDivider = 2;
            Serial.println("[输出] 每2个采样输出1个");
            break;
        case 'c':
        case 'C':
            m_outputDivider = 4;
            Serial.println("[输出] 每4个采样输出1个");
            break;
        case 'd':
        case 'D':
            m_outputDivider = 8;
            Serial.println("[输出] 每8个采样输出1个");
            break;
            
        case 'o':
        case 'O':
            if (!m_sdInitialized) {
                Serial.println("[错误] SD卡未初始化，无法录音");
            } else if (m_recording) {
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
            
        case 'V':
        case 'v':
            Serial.println("========== 缓冲区状态 ==========");
            Serial.print("  当前使用: ");
            if (m_ringBuffer) {
                Serial.print(m_ringBuffer->getCount());
            } else {
                Serial.print("0");
            }
            Serial.print(" / ");
            Serial.println(BUFFER_SIZE);
            Serial.print("  最大使用: ");
            Serial.println(m_maxBufferUsed);
            Serial.print("  溢出次数: ");
            Serial.println(m_bufferOverflowCount);
            Serial.println("================================");
            break;
            
        default:
            break;
    }
}

void Inmp441MicrophoneManager::printUsage() {
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
    Serial.println("  V - 查看缓冲区状态");
    Serial.println("  H - 显示帮助信息");
    Serial.println("============================");
}

bool Inmp441MicrophoneManager::hardwareConnectionCheck() {
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

bool Inmp441MicrophoneManager::initSDCard() {
    if (!m_pfs->begin()) {
        Serial.println("[SD] 错误: SD卡初始化失败");
        Serial.println("[SD] 提示: 请检查SD卡是否插入");
        return false;
    }
    
    char* rootPath = m_pfs->getRootPath();
    Serial.print("[SD] 根目录: ");
    Serial.println(rootPath);
    
    m_sdInitialized = true;
    return true;
}

bool Inmp441MicrophoneManager::startRecording() {
    if (!m_sdInitialized) {
        Serial.println("[录音] 错误: SD卡未初始化");
        return false;
    }
    
    if (m_recording) {
        Serial.println("[录音] 已经在录音中");
        return false;
    }
    
    // 注意：不清空环形缓冲区，让DMA继续写入
    // 与参考项目Inmp441_Rtos.ino保持一致，避免时序问题

    // 重置缓冲区统计
    m_bufferOverflowCount = 0;
    m_maxBufferUsed = 0;
    
    char filename[64];
    uint32_t timestamp = millis();
    sprintf(filename, "/Audio_%lu.pcm", timestamp);
    
    char filepath[128];
    sprintf(filepath, "%s%s", m_pfs->getRootPath(), filename);
    
    m_pcmFile = m_pfs->open(filepath);
    if (!m_pcmFile.isOpen()) {
        Serial.print("[录音] 错误: 无法创建文件 ");
        Serial.println(filepath);
        return false;
    }
    
    m_recording = true;
    m_recordStartTime = millis();
    m_recordedSamples = 0;
    
    Serial.print("[录音] 开始录音: ");
    Serial.println(filename);
    Serial.println("[录音] 发送 'O' 停止录音");
    
    return true;
}

void Inmp441MicrophoneManager::stopRecording() {
    if (!m_recording) {
        return;
    }

    m_pcmFile.close();
    
    uint32_t duration = millis() - m_recordStartTime;
    uint32_t fileSize = m_recordedSamples * 2;
    
    Serial.println("[录音] 录音已停止");
    Serial.print("[录音] 文件: ");
    Serial.print("/Audio_");
    Serial.print(m_recordStartTime);
    Serial.println(".pcm");
    Serial.print("[录音] 时长: ");
    Serial.print(duration / 1000);
    Serial.println(" 秒");
    Serial.print("[录音] 采样数: ");
    Serial.println(m_recordedSamples);
    Serial.print("[录音] 文件大小: ");
    Serial.print(fileSize);
    Serial.println(" 字节");
    
    m_recording = false;
}

bool Inmp441MicrophoneManager::initI2S() {
    Serial.println("[I2S] 开始初始化...");
    
    Serial.println("[I2S] 引脚配置:");
    Serial.print("  SCK  = PD_14\n");
    Serial.print("  WS   = PD_17\n");
    Serial.print("  RX   = PD_18\n");
    Serial.print("  TX   = PD_15\n");
    Serial.print("  MCK  = PD_16\n");
    
    m_ringBuffer = new RingBufferClass(BUFFER_SIZE);
    if (!m_ringBuffer) {
        Serial.println("[I2S] 错误: 缓冲区创建失败");
        m_errorCount++;
        return false;
    }
    
    Serial.println("[I2S] 调用i2s_init...");
    Serial.println("[I2S] 注意: SCK=PD_14, WS=PD_17 匹配I2S_S1组");
    
    i2s_init(&m_i2sObj, I2S_SCK_PIN, I2S_WS_PIN, I2S_TX_PIN, I2S_RX_PIN, I2S_MCK_PIN);
    
    if (m_i2sObj.i2s_initialized != 1) {
        Serial.println("[I2S] 错误: i2s_init失败, i2s_initialized != 1");
        return false;
    }
    Serial.println("[I2S] i2s_init成功");
    
    Serial.println("[I2S] 配置DMA缓冲区...");
    i2s_set_dma_buffer(&m_i2sObj, m_txBuffer, m_rxBuffer, DMA_PAGE_NUM, DMA_PAGE_SIZE);
    
    Serial.println("[I2S] 注册中断处理函数...");
    i2s_rx_irq_handler(&m_i2sObj, i2s_rx_callback, 0);
    i2s_tx_irq_handler(&m_i2sObj, i2s_tx_callback, 0);
    
    Serial.println("[I2S] 设置传输参数...");
    i2s_set_direction(&m_i2sObj, I2S_DIR_RX);
    i2s_set_param(&m_i2sObj, I2S_CHANNELS, I2S_SAMPLE_RATE, I2S_BITS_PER_SAMPLE);
    i2s_set_format(&m_i2sObj, FORMAT_I2S);
    i2s_set_master(&m_i2sObj, I2S_MASTER);
    
    Serial.println("[I2S] 使能I2S...");
    i2s_enable(&m_i2sObj);
    
    for (int i = 0; i < DMA_PAGE_NUM; i++) {
        i2s_recv_page(&m_i2sObj);
    }
    
    m_i2sInitialized = true;
    
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
    Serial.print("  软件增益: "); Serial.print(m_softwareGain * 6); Serial.println("dB");
    
    return true;
}

void Inmp441MicrophoneManager::processAudioData() {
    if (!m_ringBuffer || m_ringBuffer->isEmpty()) {
        return;
    }

    if (m_recording && m_pcmFile.isOpen()) {
        static uint8_t writeBuffer[256];
        static size_t writeBufferIndex = 0;

        while (!m_ringBuffer->isEmpty()) {
            int16_t sample = m_ringBuffer->read();

            writeBuffer[writeBufferIndex++] = (uint8_t)(sample & 0xFF);
            writeBuffer[writeBufferIndex++] = (uint8_t)((sample >> 8) & 0xFF);
            m_recordedSamples++;

            if (writeBufferIndex >= sizeof(writeBuffer)) {
                size_t written = m_pcmFile.write(writeBuffer, writeBufferIndex);
                if (written != writeBufferIndex) {
                    Serial.println("[录音] 警告: 写入数据不完整");
                }
                writeBufferIndex = 0;
            }
        }

        if (writeBufferIndex > 0) {
            m_pcmFile.write(writeBuffer, writeBufferIndex);
        }
    }

    while (!m_ringBuffer->isEmpty()) {
        int16_t sample = m_ringBuffer->read();

        m_outputCounter++;
        if (m_outputCounter >= m_outputDivider) {
            m_outputCounter = 0;

            uint32_t timestamp = millis() - m_startTime;
            int signalLevel = 0;

            int absSample = abs(sample);
            if (absSample >= SIGNAL_LEVEL_3) signalLevel = 3;
            else if (absSample >= SIGNAL_LEVEL_2) signalLevel = 2;
            else if (absSample >= SIGNAL_LEVEL_1) signalLevel = 1;
            else signalLevel = 0;

            if (m_serialPlotterMode) {
                serialPrintPlotterMode(sample);
            } else {
                serialPrintTextMode(timestamp, sample, signalLevel);
            }
        }
    }
}

void Inmp441MicrophoneManager::serialPrintTextMode(uint32_t timestamp, int16_t sample, int signalLevel) {
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

void Inmp441MicrophoneManager::serialPrintPlotterMode(int16_t sample) {
    Serial.println(sample);
}

// ===============================================
// I2S回调函数
// ===============================================

extern "C" {

void i2s_rx_callback(uint32_t id, char *pbuf) {
    (void)id;
    
    if (!pbuf || !s_microphoneManagerPtr) {
        return;
    }
    
    uint8_t* src = (uint8_t*)pbuf;
    size_t samples = DMA_PAGE_SIZE / 2;
    
    for (size_t i = 0; i < samples; i++) {
        int16_t sample = (int16_t)((uint16_t)src[i*2+1] << 8 | src[i*2]);
        
        uint8_t gain = s_microphoneManagerPtr->m_softwareGain;
        if (gain > 0) {
            int32_t amplified = ((int32_t)sample) << gain;
            if (amplified > 32767) amplified = 32767;
            if (amplified < -32768) amplified = -32768;
            sample = (int16_t)amplified;
        }
        
        if (s_microphoneManagerPtr->m_ringBuffer) {
            s_microphoneManagerPtr->m_ringBuffer->write(sample);
        }
    }
    
    s_microphoneManagerPtr->m_sampleCount++;
    i2s_recv_page(&s_microphoneManagerPtr->m_i2sObj);
}

void i2s_tx_callback(uint32_t id, char *pbuf) {
    (void)id;
    (void)pbuf;
}

}

// ===============================================
// 音视频同步录制接口实现
// ===============================================

bool Inmp441MicrophoneManager::startAVIRecording() {
    if (!m_i2sInitialized) {
        return false;
    }
    
    // 注意：不清空环形缓冲区，让DMA继续写入
    // 与参考项目Inmp441_Rtos.ino保持一致，避免时序问题
    
    if (!initAudioQueue()) {
        Serial.println("[AudioQueue] 录音启动时队列初始化失败");
        return false;
    }
    
    return true;
}

void Inmp441MicrophoneManager::stopAVIRecording() {
    destroyAudioQueue();
}

size_t Inmp441MicrophoneManager::getAvailableAudioSamples() {
    if (!m_ringBuffer) {
        return 0;
    }
    return m_ringBuffer->getCount();
}

size_t Inmp441MicrophoneManager::readAudioSamples(int16_t* buffer, size_t maxSamples) {
    if (!m_ringBuffer || !buffer) {
        return 0;
    }
    
    size_t count = 0;
    while (count < maxSamples && !m_ringBuffer->isEmpty()) {
        buffer[count++] = m_ringBuffer->read();
    }
    
    return count;
}

// ===================== RTOS音频队列接口实现 =====================

bool Inmp441MicrophoneManager::initAudioQueue() {
    if (m_audioQueue != NULL) {
        return true; // 队列已初始化
    }
    
    m_audioQueue = xQueueCreate(AUDIO_QUEUE_SIZE, sizeof(AudioDataBlock));
    if (m_audioQueue == NULL) {
        Serial.println("[AudioQueue] 队列创建失败");
        return false;
    }
    
    Serial.println("[AudioQueue] 音频队列初始化成功");
    return true;
}

void Inmp441MicrophoneManager::destroyAudioQueue() {
    if (m_audioQueue) {
        vQueueDelete(m_audioQueue);
        m_audioQueue = NULL;
    }
}

bool Inmp441MicrophoneManager::sendAudioDataBlock(const AudioDataBlock* block, TickType_t timeout) {
    if (m_audioQueue == NULL || block == NULL) {
        return false;
    }
    
    BaseType_t result = xQueueSend(m_audioQueue, block, timeout);
    if (result != pdPASS) {
        Serial.println("[AudioQueue] 发送音频数据块失败");
        return false;
    }
    
    return true;
}

bool Inmp441MicrophoneManager::receiveAudioDataBlock(AudioDataBlock* block, TickType_t timeout) {
    if (m_audioQueue == NULL || block == NULL) {
        return false;
    }
    
    BaseType_t result = xQueueReceive(m_audioQueue, block, timeout);
    if (result != pdPASS) {
        return false;
    }
    
    return true;
}

size_t Inmp441MicrophoneManager::getAudioQueueAvailable() {
    if (m_audioQueue == NULL) {
        return 0;
    }
    
    return uxQueueMessagesWaiting(m_audioQueue);
}

size_t Inmp441MicrophoneManager::getAudioQueueFree() {
    if (m_audioQueue == NULL) {
        return 0;
    }
    
    return uxQueueSpacesAvailable(m_audioQueue);
}
