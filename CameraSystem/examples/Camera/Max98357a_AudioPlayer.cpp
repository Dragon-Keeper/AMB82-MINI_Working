/*
 * Max98357a_AudioPlayer.cpp - MAX98357A音频播放管理器实现
 * V1.0: 实现MAX98357A音频芯片驱动，支持I2S音频输出
 *
 * 修复说明:
 * - 使用与Inmp441_MicrophoneManager一致的I2S API
 * - 基于i2s_t结构体进行操作
 * - 支持与INMP441分时复用I2S1接口
 */

#include "Max98357a_AudioPlayer.h"
#include "Utils_Logger.h"
#include "Inmp441_MicrophoneManager.h"
#include <math.h>

#define TAG "MAX98357A"

Max98357aAudioPlayer g_audioPlayer;

static Max98357aAudioPlayer* s_playerPtr = nullptr;

extern "C" {

void i2s_tx_callback_max98357a(uint32_t id, char *pbuf) {
    if (s_playerPtr) {
        s_playerPtr->handleI2sTxCallback(pbuf);
    }
}

void i2s_rx_callback_max98357a(uint32_t id, char *pbuf) {
    if (s_playerPtr) {
        s_playerPtr->handleI2sRxCallback(pbuf);
    }
}

}

Max98357aAudioPlayer::Max98357aAudioPlayer()
    : m_i2sInitialized(false)
    , m_playing(false)
    , m_state(PLAYBACK_STATE_STOPPED)
    , m_syncMode(SYNC_MODE_AUDIO_MASTER)
    , m_playedSamples(0)
    , m_droppedFrames(0)
    , m_lastSyncTime(0)
    , m_speedAdjust(0)
    , m_mutex(NULL)
    , m_txBuffer(NULL)
    , m_txBufferSize(0)
    , m_txBufferHead(0)
    , m_txBufferTail(0)
    , m_txBufferCount(0)
    , m_needSync(true)
    , m_expectedNextFrameTime(0)
    , m_fadeLevel(0)
    , m_lastOutputSample(0)
    , m_fadingIn(false)
    , m_fadingOut(false)
    , m_draining(false)
    , m_dcBlockLastIn(0)
    , m_dcBlockLastOut(0)
    , m_txUnderrunCount(0)
    , m_txCallbackCount(0)
    , m_lastCallbackTime(0)
    , m_maxCallbackInterval(0)
    , m_minCallbackInterval(0xFFFFFFFF)
    , m_maxCallbackDuration(0)
    , m_callbackIntervalCount(0)
    , m_totalCallbackInterval(0)
    , m_audioMinSample(0)
    , m_audioMaxSample(0)
    , m_audioSumSample(0)
    , m_audioSumSquaredSample(0)
    , m_audioSampleCount(0)
    , m_audioChunkCount(0)
    , m_lastStatsTime(0)
    , m_firstChunkDumped(false)
    , m_ringBuffer(nullptr)
    , m_playbackCallback(nullptr)
    , m_callbackUserData(nullptr)
{
    memset(&m_i2sObj, 0, sizeof(m_i2sObj));
    memset(m_txPageBuffer, 0, sizeof(m_txPageBuffer));
    memset(m_rxPageBuffer, 0, sizeof(m_rxPageBuffer));
}

Max98357aAudioPlayer::~Max98357aAudioPlayer() {
    deinit();
}

Max98357aAudioPlayer& Max98357aAudioPlayer::getInstance() {
    return g_audioPlayer;
}

bool Max98357aAudioPlayer::init() {
    if (m_i2sInitialized) {
        Utils_Logger::warn(TAG, "MAX98357A already initialized");
        return true;
    }

    Utils_Logger::info(TAG, "Initializing MAX98357A Audio Player...");

    m_mutex = xSemaphoreCreateMutexStatic(&m_mutexBuffer);
    if (!m_mutex) {
        Utils_Logger::error(TAG, "Failed to create mutex for MAX98357A");
        return false;
    }

    m_txBufferSize = MAX98357A_BUFFER_SIZE;
    m_txBuffer = new int16_t[m_txBufferSize];
    if (!m_txBuffer) {
        Utils_Logger::error(TAG, "Failed to allocate TX buffer for MAX98357A");
        vSemaphoreDelete(m_mutex);
        return false;
    }

    memset(m_txBuffer, 0, m_txBufferSize * sizeof(int16_t));
    m_txBufferHead = 0;
    m_txBufferTail = 0;
    m_txBufferCount = 0;

    s_playerPtr = this;
    m_i2sInitialized = true;
    m_state = PLAYBACK_STATE_STOPPED;

    Utils_Logger::info(TAG, "MAX98357A Audio Player initialized successfully");
    Utils_Logger::info(TAG, "I2S TX pins: BCLK=D20, DIN=D19, WS=D17");
    return true;
}

bool Max98357aAudioPlayer::deinit() {
    if (!m_i2sInitialized) {
        return true;
    }

    stopPlayback();
    deinitI2s();

    if (m_mutex) {
        vSemaphoreDelete(m_mutex);
        m_mutex = NULL;
    }

    if (m_txBuffer) {
        delete[] m_txBuffer;
        m_txBuffer = NULL;
    }

    m_i2sInitialized = false;
    m_state = PLAYBACK_STATE_STOPPED;
    Utils_Logger::info(TAG, "MAX98357A Audio Player deinitialized");

    return true;
}

bool Max98357aAudioPlayer::initI2sForTx() {
    Utils_Logger::info(TAG, "Configuring I2S for TX mode...");
    Utils_Logger::info(TAG, "Sample rate: %d Hz, Word length: 16-bit, Channel: Stereo (Mono->L/R)", MAX98357A_SAMPLE_RATE);

    m_ringBuffer = new RingBufferClass(MAX98357A_BUFFER_SIZE);
    if (!m_ringBuffer) {
        Utils_Logger::error(TAG, "Failed to create ring buffer");
        return false;
    }

    memset(m_txPageBuffer, 0, sizeof(m_txPageBuffer));
    memset(m_rxPageBuffer, 0, sizeof(m_rxPageBuffer));

    i2s_init(&m_i2sObj, I2S_SCK_PIN, I2S_WS_PIN, I2S_TX_PIN, I2S_RX_PIN, I2S_MCK_PIN);

    if (m_i2sObj.i2s_initialized != 1) {
        Utils_Logger::error(TAG, "i2s_init failed");
        return false;
    }

    i2s_set_dma_buffer(&m_i2sObj, m_txPageBuffer, m_rxPageBuffer, MAX98357A_DMA_PAGE_NUM, MAX98357A_DMA_PAGE_SIZE);

    i2s_set_direction(&m_i2sObj, I2S_DIR_TXRX);
    i2s_set_param(&m_i2sObj, I2S_CH_STEREO, I2S_SR_16KHZ, I2S_WL_16);
    i2s_set_format(&m_i2sObj, FORMAT_I2S);
    i2s_set_master(&m_i2sObj, I2S_MASTER);

    i2s_set_dma_burst_size(&m_i2sObj, BURST16);
    i2s_set_byte_swap(&m_i2sObj, FALSE);
    i2s_set_data_start_edge(&m_i2sObj, NEGATIVE_EDGE);
    i2s_set_sck_inv(&m_i2sObj, FALSE);
    i2s_set_ws_swap(&m_i2sObj, LEFT_PHASE);

    i2s_rx_irq_handler(&m_i2sObj, i2s_rx_callback_max98357a, 0);
    i2s_tx_irq_handler(&m_i2sObj, i2s_tx_callback_max98357a, 0);

    for (int i = 0; i < MAX98357A_DMA_PAGE_NUM; i++) {
        i2s_send_page(&m_i2sObj, (uint32_t*)(m_txPageBuffer + i * MAX98357A_DMA_PAGE_SIZE));
        i2s_recv_page(&m_i2sObj);
    }

    i2s_enable(&m_i2sObj);

    Utils_Logger::info(TAG, "I2S TXRX mode init done, %d pages x %d bytes",
        MAX98357A_DMA_PAGE_NUM, MAX98357A_DMA_PAGE_SIZE, MAX98357A_DMA_PAGE_NUM * MAX98357A_DMA_PAGE_SIZE);
    return true;
}

bool Max98357aAudioPlayer::deinitI2s() {
    Utils_Logger::info(TAG, "Deinitializing I2S...");

    if (m_i2sObj.i2s_initialized == 1) {
        i2s_disable(&m_i2sObj);
        i2s_deinit(&m_i2sObj);
        memset(&m_i2sObj, 0, sizeof(m_i2sObj));
    }

    if (m_ringBuffer) {
        delete m_ringBuffer;
        m_ringBuffer = nullptr;
    }

    Utils_Logger::info(TAG, "I2S deinitialized");
    return true;
}

bool Max98357aAudioPlayer::switchToPlaybackMode() {
    Utils_Logger::info(TAG, "Switching to PLAYBACK mode (I2S TX)...");

    if (m_i2sInitialized) {
        deinitI2s();
    }

    delay(10);

    if (!initI2sForTx()) {
        Utils_Logger::error(TAG, "Failed to switch to playback mode");
        return false;
    }

    Utils_Logger::info(TAG, "Switched to PLAYBACK mode successfully");
    return true;
}

bool Max98357aAudioPlayer::switchToRecordMode() {
    Utils_Logger::info(TAG, "Switching to RECORD mode (I2S RX) - releasing I2S for INMP441...");

    stopPlayback();
    deinitI2s();

    Utils_Logger::info(TAG, "I2S released for INMP441 (record mode)");
    return true;
}

bool Max98357aAudioPlayer::startPlayback() {
    if (m_state == PLAYBACK_STATE_PLAYING) {
        Utils_Logger::warn(TAG, "Already playing");
        return true;
    }

    Utils_Logger::info(TAG, "Starting MAX98357A playback...");

    if (!m_i2sInitialized) {
        if (!init()) {
            Utils_Logger::error(TAG, "Failed to initialize player");
            return false;
        }
    }

    if (!switchToPlaybackMode()) {
        Utils_Logger::error(TAG, "Failed to switch to playback mode");
        return false;
    }

    bool hasPreloadedData = (m_txBufferCount > 0);
    if (!hasPreloadedData) {
        memset(m_txBuffer, 0, m_txBufferSize * sizeof(int16_t));
    }
    m_txBufferHead = hasPreloadedData ? m_txBufferHead : 0;
    m_txBufferTail = 0;
    m_txBufferCount = hasPreloadedData ? m_txBufferCount : 0;
    m_playedSamples = 0;
    m_droppedFrames = 0;
    m_needSync = true;
    m_fadeLevel = 0;
    m_lastOutputSample = 0;
    m_fadingIn = true;
    m_fadingOut = false;
    m_dcBlockLastIn = 0;
    m_dcBlockLastOut = 0;

    m_audioMinSample = 32767;
    m_audioMaxSample = -32768;
    m_audioSumSample = 0;
    m_audioSumSquaredSample = 0;
    m_audioSampleCount = 0;
    m_audioChunkCount = 0;
    m_lastStatsTime = millis();
    m_firstChunkDumped = false;
    m_txUnderrunCount = 0;
    m_txCallbackCount = 0;

    m_playing = true;
    m_state = PLAYBACK_STATE_PLAYING;
    Utils_Logger::info(TAG, "MAX98357A playback started, config: rate=%dHz, bits=%d, ch=STEREO, buf=%u, dmaPages=%d x %dB",
        MAX98357A_SAMPLE_RATE, MAX98357A_BITS_PER_SAMPLE, MAX98357A_BUFFER_SIZE,
        MAX98357A_DMA_PAGE_NUM, MAX98357A_DMA_PAGE_SIZE);

    return true;
}

bool Max98357aAudioPlayer::stopPlayback() {
    if (m_state == PLAYBACK_STATE_STOPPED) {
        return true;
    }

    Utils_Logger::info(TAG, "Stopping MAX98357A playback with fade-out...");

    m_fadingOut = true;

    uint32_t fadeStart = millis();
    while (m_fadingOut && (millis() - fadeStart < 200)) {
        delay(1);
    }

    Utils_Logger::info(TAG, "Fade-out complete, buffer count=%u, draining...", m_txBufferCount);

    m_playing = false;
    m_state = PLAYBACK_STATE_STOPPED;
    m_fadingOut = false;
    m_fadingIn = false;
    m_fadeLevel = 0;
    m_lastOutputSample = 0;
    m_draining = true;

    uint32_t drainStart = millis();
    while (m_draining && (millis() - drainStart < 5000)) {
        delay(10);
    }
    if (m_draining) {
        Utils_Logger::info(TAG, "Drain timeout, remaining buffer=%u", m_txBufferCount);
    }

    Utils_Logger::info(TAG, "TX Stats: callbacks=%u, underruns=%u (%.2f%%)",
        m_txCallbackCount, m_txUnderrunCount,
        m_txCallbackCount > 0 ? (float)m_txUnderrunCount * 100.0f / m_txCallbackCount : 0.0f);

    if (m_audioSampleCount > 0) {
        int16_t dcOffset = (int16_t)(m_audioSumSample / m_audioSampleCount);
        uint32_t meanSq = (uint32_t)(m_audioSumSquaredSample / m_audioSampleCount);
        uint16_t rms = (uint16_t)sqrt((double)meanSq);
        Utils_Logger::info(TAG, "Audio Stats: chunks=%u, samples=%u, min=%d, max=%d, dc=%d, rms=%u, p2p=%d",
            m_audioChunkCount, m_audioSampleCount, m_audioMinSample, m_audioMaxSample,
            dcOffset, rms, m_audioMaxSample - m_audioMinSample);
    }

    Utils_Logger::info(TAG, "Buffer Stats: played=%u, dropped=%u", m_playedSamples, m_droppedFrames);

    if (m_i2sObj.i2s_initialized == 1) {
        if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            memset(m_txBuffer, 0, m_txBufferSize * sizeof(int16_t));
            m_txBufferHead = 0;
            m_txBufferTail = 0;
            m_txBufferCount = 0;
            xSemaphoreGive(m_mutex);
        }

        delay(40);

        i2s_disable(&m_i2sObj);
    }

    Utils_Logger::info(TAG, "MAX98357A playback stopped");
    return true;
}

bool Max98357aAudioPlayer::pausePlayback() {
    if (m_state != PLAYBACK_STATE_PLAYING) {
        return false;
    }

    Utils_Logger::info(TAG, "Pausing MAX98357A playback...");
    m_state = PLAYBACK_STATE_PAUSED;
    return true;
}

bool Max98357aAudioPlayer::resumePlayback() {
    if (m_state != PLAYBACK_STATE_PAUSED) {
        return false;
    }

    Utils_Logger::info(TAG, "Resuming MAX98357A playback...");
    m_state = PLAYBACK_STATE_PLAYING;
    return true;
}

bool Max98357aAudioPlayer::writeAudioData(const int16_t* data, size_t count) {
    if (!data || count == 0) {
        return false;
    }

    if (m_state != PLAYBACK_STATE_PLAYING && m_state != PLAYBACK_STATE_STOPPED) {
        return false;
    }

    m_audioChunkCount++;
    for (size_t i = 0; i < count; i++) {
        int16_t s = data[i];
        if (s < m_audioMinSample) m_audioMinSample = s;
        if (s > m_audioMaxSample) m_audioMaxSample = s;
        m_audioSumSample += s;
        m_audioSumSquaredSample += (int64_t)s * s;
    }
    m_audioSampleCount += count;

    if (!m_firstChunkDumped && count > 0) {
        m_firstChunkDumped = true;
        dumpAudioData(data, count, "FIRST_CHUNK");
    }

    if (xSemaphoreTake(m_mutex, portMAX_DELAY) != pdTRUE) {
        return false;
    }

    size_t written = 0;
    while (written < count) {
        if (m_txBufferHead >= m_txBufferSize) {
            m_txBufferHead = 0;
        }

        size_t remainInBuffer = m_txBufferSize - m_txBufferHead;
        size_t remainSamples = count - written;
        size_t toWrite = (remainSamples < remainInBuffer) ? remainSamples : remainInBuffer;

        for (size_t i = 0; i < toWrite; i++) {
            m_txBuffer[m_txBufferHead + i] = data[written + i];
        }

        written += toWrite;
        m_txBufferHead += toWrite;
        m_txBufferCount += toWrite;
    }

    if (m_txBufferCount > m_txBufferSize) {
        size_t overflow = m_txBufferCount - m_txBufferSize;
        m_txBufferTail = (m_txBufferTail + overflow) % m_txBufferSize;
        m_txBufferCount = m_txBufferSize;
        m_droppedFrames += overflow;
    }

    m_playedSamples += count;
    xSemaphoreGive(m_mutex);

    logPeriodicStats();

    return true;
}

bool Max98357aAudioPlayer::writeAudioFrame(const AudioFrame* frame) {
    if (!frame || frame->count == 0) {
        return false;
    }
    return writeAudioData(frame->samples, frame->count);
}

void Max98357aAudioPlayer::handleI2sTxCallback(char* pbuf) {
    static uint32_t s_txDebugCount = 0;
    s_txDebugCount++;
    if (s_txDebugCount <= 10) {
        Serial.print("[TX"); Serial.print(s_txDebugCount); Serial.print("]");
    }

    int16_t* pbuf16 = (int16_t*)pbuf;
    size_t totalSlots = MAX98357A_DMA_PAGE_SIZE / sizeof(int16_t);
    size_t monoSamplesPerPage = totalSlots / 2;
    m_txCallbackCount++;

    if (!m_playing || m_state != PLAYBACK_STATE_PLAYING) {
        if (!m_draining) {
            memset(pbuf, 0, MAX98357A_DMA_PAGE_SIZE);
            i2s_send_page(&m_i2sObj, (uint32_t*)pbuf);
            return;
        }
    }

    if (xSemaphoreTake(m_mutex, 0) == pdTRUE) {
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
            pbuf16[i * 2] = sample;
            pbuf16[i * 2 + 1] = sample;
        }
        if (m_draining && m_txBufferCount == 0) {
            m_draining = false;
            Utils_Logger::info(TAG, "Buffer drained, stopping TX");
        }
        xSemaphoreGive(m_mutex);
    } else {
        m_txUnderrunCount++;
        memset(pbuf, 0, MAX98357A_DMA_PAGE_SIZE);
    }

    if (m_txCallbackCount <= 5 || (m_txCallbackCount <= 100 && m_txCallbackCount % 10 == 0) || m_txCallbackCount % 50 == 0) {
        uint32_t urPct = m_txCallbackCount > 0 ? (m_txUnderrunCount * 100 / (m_txCallbackCount * 320)) : 0;
        Serial.print("[CB"); Serial.print(m_txCallbackCount);
        Serial.print(" b="); Serial.print(m_txBufferCount);
        Serial.print(" u="); Serial.print(urPct);
        Serial.println("%]");
    }

    i2s_send_page(&m_i2sObj, (uint32_t*)pbuf);
}

void Max98357aAudioPlayer::serviceI2sTx() {
    if (!m_playing || m_state != PLAYBACK_STATE_PLAYING) return;

    int* pagePtr = i2s_get_tx_page(&m_i2sObj);
    if (pagePtr == NULL) return;

    static uint32_t s_pollCount = 0;
    s_pollCount++;
    if (s_pollCount <= 5 || s_pollCount % 50 == 0) {
        Utils_Logger::info(TAG, "TX_POLL%u: got page", s_pollCount);
    }

    int16_t* pbuf16 = (int16_t*)pagePtr;
    size_t totalSlots = MAX98357A_DMA_PAGE_SIZE / sizeof(int16_t);
    size_t monoSamplesPerPage = totalSlots / 2;

    if (xSemaphoreTake(m_mutex, 0) == pdTRUE) {
        if (m_fadingIn && m_fadeLevel < 4096) {
            m_fadeLevel += 64;
            if (m_fadeLevel >= 4096) {
                m_fadeLevel = 4096;
                m_fadingIn = false;
            }
        }

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

            if (m_fadeLevel < 4096) {
                sample = (int16_t)(((int32_t)sample * m_fadeLevel) >> 12);
            }

            if (m_fadingOut) {
                if (m_fadeLevel > 64) {
                    m_fadeLevel -= 64;
                } else {
                    m_fadeLevel = 0;
                    m_fadingOut = false;
                    m_playing = false;
                    m_state = PLAYBACK_STATE_STOPPED;
                }
                sample = (int16_t)(((int32_t)sample * m_fadeLevel) >> 12);
            }

            m_lastOutputSample = sample;
            pbuf16[i * 2] = sample;
            pbuf16[i * 2 + 1] = sample;
        }
        m_playedSamples += monoSamplesPerPage;

        xSemaphoreGive(m_mutex);
    } else {
        m_lastOutputSample = m_lastOutputSample >> 1;
        for (size_t i = 0; i < monoSamplesPerPage; i++) {
            pbuf16[i * 2] = m_lastOutputSample;
            pbuf16[i * 2 + 1] = m_lastOutputSample;
        }
    }

    i2s_send_page(&m_i2sObj, (uint32_t*)pagePtr);
}

void Max98357aAudioPlayer::handleI2sRxCallback(char* pbuf) {
    i2s_recv_page(&m_i2sObj);
}

bool Max98357aAudioPlayer::syncWithVideo(uint32_t videoTimestamp) {
    if (m_state != PLAYBACK_STATE_PLAYING) {
        return false;
    }

    uint32_t currentAudioTime = (m_playedSamples * 1000) / MAX98357A_SAMPLE_RATE;
    int32_t syncDiff = (int32_t)currentAudioTime - (int32_t)videoTimestamp;

    if (abs(syncDiff) > MAX98357A_SYNC_TOLERANCE) {
        Utils_Logger::warn(TAG, "Audio-Video sync diff: %d ms (tolerance: %d ms)", syncDiff, MAX98357A_SYNC_TOLERANCE);

        if (syncDiff > 0) {
            Utils_Logger::info(TAG, "Audio ahead of video, need to slow down");
        } else {
            Utils_Logger::info(TAG, "Audio behind video, need to speed up");
        }

        m_needSync = true;
        m_lastSyncTime = millis();
    }

    return true;
}

bool Max98357aAudioPlayer::adjustPlaybackSpeed(int32_t speedAdjust) {
    if (m_state != PLAYBACK_STATE_PLAYING) {
        return false;
    }

    m_speedAdjust = speedAdjust;
    Utils_Logger::info(TAG, "Playback speed adjusted by %d", speedAdjust);
    return true;
}

bool Max98357aAudioPlayer::feedAudioData(const int16_t* samples, size_t count) {
    return writeAudioData(samples, count);
}

void Max98357aAudioPlayer::setPlaybackCallback(PlaybackCallback callback, void* userData) {
    m_playbackCallback = callback;
    m_callbackUserData = userData;
}

bool Max98357aAudioPlayer::isI2sInitialized() const {
    return m_i2sObj.i2s_initialized == 1;
}

bool Max98357aAudioPlayer::acquireI2S() {
    if (m_playing) {
        Utils_Logger::warn(TAG, "Cannot acquire I2S - playback is active");
        return false;
    }

    if (m_i2sObj.i2s_initialized == 1) {
        Utils_Logger::info(TAG, "I2S already initialized");
        return true;
    }

    Utils_Logger::info(TAG, "Acquiring I2S for TX (MAX98357A)...");
    return initI2sForTx();
}

bool Max98357aAudioPlayer::releaseI2S() {
    Utils_Logger::info(TAG, "Releasing I2S for TX (MAX98357A)...");

    if (m_playing) {
        stopPlayback();
    }

    deinitI2s();
    return true;
}

void Max98357aAudioPlayer::logPeriodicStats() {
    uint32_t now = millis();
    if (now - m_lastStatsTime < 1000) {
        return;
    }
    m_lastStatsTime = now;

    uint32_t audioTimeMs = (m_playedSamples * 1000) / MAX98357A_SAMPLE_RATE;
    float underrunRate = m_txCallbackCount > 0 ? (float)m_txUnderrunCount * 100.0f / m_txCallbackCount : 0.0f;
    float bufferFillPct = (float)m_txBufferCount * 100.0f / m_txBufferSize;

    Utils_Logger::info(TAG, "[1s] t=%ums, buf=%.0f%% (%u/%u), underruns=%u/%u (%.1f%%), chunks=%u",
        audioTimeMs, bufferFillPct, m_txBufferCount, m_txBufferSize,
        m_txUnderrunCount, m_txCallbackCount, underrunRate, m_audioChunkCount);

    if (m_audioSampleCount > 0) {
        int16_t dcOffset = (int16_t)(m_audioSumSample / (int64_t)m_audioSampleCount);
        uint32_t meanSq = (uint32_t)(m_audioSumSquaredSample / (int64_t)m_audioSampleCount);
        uint16_t rms = (uint16_t)sqrt((double)meanSq);
        Utils_Logger::info(TAG, "[1s] signal: min=%d, max=%d, dc=%d, rms=%u, p2p=%d",
            m_audioMinSample, m_audioMaxSample, dcOffset, rms,
            m_audioMaxSample - m_audioMinSample);
    }

    m_audioMinSample = 32767;
    m_audioMaxSample = -32768;
    m_audioSumSample = 0;
    m_audioSumSquaredSample = 0;
    m_audioSampleCount = 0;
}

void Max98357aAudioPlayer::dumpAudioData(const int16_t* data, size_t count, const char* context) {
    const uint8_t* raw = (const uint8_t*)data;
    size_t hexLen = count * sizeof(int16_t);
    if (hexLen > 32) hexLen = 32;

    char hexBuf[128];
    int pos = 0;
    for (size_t i = 0; i < hexLen && pos < 120; i++) {
        pos += snprintf(hexBuf + pos, sizeof(hexBuf) - pos, "%02X ", raw[i]);
    }

    Utils_Logger::info(TAG, "DUMP[%s]: %u samples, raw[%uB]: %s",
        context, count, hexLen, hexBuf);

    if (count >= 8) {
        Utils_Logger::info(TAG, "DUMP[%s]: s[0..7]=%d %d %d %d %d %d %d %d",
            context, data[0], data[1], data[2], data[3],
            data[4], data[5], data[6], data[7]);
    }
}