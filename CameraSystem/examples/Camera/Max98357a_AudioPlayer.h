/*
 * Max98357a_AudioPlayer.h - MAX98357A音频播放管理器
 * V1.0: 实现MAX98357A音频芯片驱动，支持I2S音频输出
 *
 * 功能描述:
 * - 通过AMB82-MINI开发板驱动I2S接口的MAX98357A音频放大器模块
 * - 实现音频数据发送和播放功能
 * - 支持与INMP441录音模块分时复用I2S1接口
 * - 支持音视频同步播放，误差控制在可接受范围内
 *
 * 硬件连接:
 * MAX98357A → AMB82-MINI
 * VCC        → 3.3V独立稳压电源
 * GND        → GND
 * BCLK       → GPIOD_14 (I2S Serial Clock)
 * DIN        → GPIOD_15 (I2S Serial Data)
 * LRCK       → GPIOD_17 (I2S Word Select)
 * GAIN       → 10kΩ → GND (默认9dB增益)
 * SD         → 悬空(左声道模式)
 * OUT+       → 扬声器+(建议串联100Ω限流)
 * OUT-       → 扬声器-(建议串联100Ω限流)
 *
 * 注意: 录音与播放不能同时运行，共用同一组I2S1接口(D14/D15/D17)
 */

#ifndef MAX98357A_AUDIO_PLAYER_H
#define MAX98357A_AUDIO_PLAYER_H

#include <Arduino.h>
#include <i2s_api.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "Shared_GlobalDefines.h"

class RingBufferClass;

#define MAX98357A_SAMPLE_RATE     16000
#define MAX98357A_BITS_PER_SAMPLE 16
#define MAX98357A_CHANNELS        2
#define MAX98357A_DMA_PAGE_SIZE   1280
#define MAX98357A_DMA_PAGE_NUM    4
#define MAX98357A_BUFFER_SIZE     65536
#define MAX98357A_SYNC_TOLERANCE  50
#define MAX98357A_LOW_WATERMARK   8192
#define MAX98357A_HIGH_WATERMARK  32768

#define I2S_SCK_PIN  PD_14
#define I2S_WS_PIN   PD_17
#define I2S_TX_PIN   PD_15
#define I2S_RX_PIN   PD_18
#define I2S_MCK_PIN  PD_16

typedef struct {
    int16_t samples[512];
    size_t count;
    uint32_t timestamp;
} AudioFrame;

typedef enum {
    PLAYBACK_STATE_STOPPED = 0,
    PLAYBACK_STATE_PLAYING,
    PLAYBACK_STATE_PAUSED,
    PLAYBACK_STATE_BUFFERING
} PlaybackState;

typedef enum {
    SYNC_MODE_AUDIO_MASTER = 0,
    SYNC_MODE_VIDEO_MASTER,
    SYNC_MODE_EXTERNAL_SYNC
} SyncMode;

typedef void (*PlaybackCallback)(void* userData);

extern "C" {
    void i2s_tx_callback_max98357a(uint32_t id, char *pbuf);
    void i2s_rx_callback_max98357a(uint32_t id, char *pbuf);
}

class Max98357aAudioPlayer {
friend void i2s_tx_callback_max98357a(uint32_t id, char *pbuf);
friend void i2s_rx_callback_max98357a(uint32_t id, char *pbuf);

public:
    static Max98357aAudioPlayer& getInstance();

    bool init();
    bool deinit();

    bool startPlayback();
    bool stopPlayback();
    bool pausePlayback();
    bool resumePlayback();

    bool writeAudioData(const int16_t* data, size_t count);
    bool writeAudioFrame(const AudioFrame* frame);

    PlaybackState getState() { return m_state; }
    bool isPlaying() { return m_state == PLAYBACK_STATE_PLAYING; }
    bool isInitialized() { return m_i2sInitialized; }

    uint32_t getPlayedSamples() { return m_playedSamples; }
    uint32_t getDroppedFrames() { return m_droppedFrames; }
    size_t getBufferCount() { return m_txBufferCount; }
    size_t getBufferSize() { return m_txBufferSize; }
    bool isBufferAboveLowWatermark() { return m_txBufferCount >= MAX98357A_LOW_WATERMARK; }
    bool isBufferAboveHighWatermark() { return m_txBufferCount >= MAX98357A_HIGH_WATERMARK; }
    float getBufferFillPercent() { return m_txBufferSize > 0 ? (float)m_txBufferCount * 100.0f / m_txBufferSize : 0.0f; }

    void setSyncMode(SyncMode mode) { m_syncMode = mode; }
    SyncMode getSyncMode() { return m_syncMode; }

    bool syncWithVideo(uint32_t videoTimestamp);
    bool adjustPlaybackSpeed(int32_t speedAdjust);

    void handleI2sTxCallback(char* pbuf);
    void handleI2sRxCallback(char* pbuf);
    void serviceI2sTx();

    bool feedAudioData(const int16_t* samples, size_t count);

    void setPlaybackCallback(PlaybackCallback callback, void* userData);

    bool isI2sInitialized() const;

    bool acquireI2S();
    bool releaseI2S();

public:
    Max98357aAudioPlayer();
    ~Max98357aAudioPlayer();

    bool initI2sForTx();
    bool deinitI2s();
    bool switchToPlaybackMode();
    bool switchToRecordMode();

    bool m_i2sInitialized;
    bool m_playing;
    PlaybackState m_state;
    SyncMode m_syncMode;

    uint32_t m_playedSamples;
    uint32_t m_droppedFrames;
    uint32_t m_lastSyncTime;
    int32_t m_speedAdjust;

    SemaphoreHandle_t m_mutex;
    StaticSemaphore_t m_mutexBuffer;

    int16_t* m_txBuffer;
    size_t m_txBufferSize;
    size_t m_txBufferHead;
    size_t m_txBufferTail;
    size_t m_txBufferCount;

    bool m_needSync;
    uint32_t m_expectedNextFrameTime;

    int16_t m_fadeLevel;
    int16_t m_lastOutputSample;
    bool m_fadingIn;
    bool m_fadingOut;
    bool m_draining;

    int32_t m_dcBlockLastIn;
    int32_t m_dcBlockLastOut;

    uint32_t m_txUnderrunCount;
    uint32_t m_txCallbackCount;

    uint32_t m_lastCallbackTime;
    uint32_t m_maxCallbackInterval;
    uint32_t m_minCallbackInterval;
    uint32_t m_maxCallbackDuration;
    uint32_t m_callbackIntervalCount;
    uint64_t m_totalCallbackInterval;

    int16_t m_audioMinSample;
    int16_t m_audioMaxSample;
    int64_t m_audioSumSample;
    int64_t m_audioSumSquaredSample;
    uint32_t m_audioSampleCount;
    uint32_t m_audioChunkCount;
    uint32_t m_lastStatsTime;
    bool m_firstChunkDumped;

    void logPeriodicStats();
    void dumpAudioData(const int16_t* data, size_t count, const char* context);

    i2s_t m_i2sObj;
    char m_txPageBuffer[MAX98357A_DMA_PAGE_SIZE * MAX98357A_DMA_PAGE_NUM] __attribute__((aligned(32)));
    char m_rxPageBuffer[MAX98357A_DMA_PAGE_SIZE * MAX98357A_DMA_PAGE_NUM] __attribute__((aligned(32)));

    RingBufferClass* m_ringBuffer;

    PlaybackCallback m_playbackCallback;
    void* m_callbackUserData;
};

#endif
