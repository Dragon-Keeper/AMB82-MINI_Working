/*
 * AVSync_PlaybackManager.h - 音视频同步播放管理器
 * V1.0: 实现视频回放时的音频同步播放功能
 *
 * 功能描述:
 * - 管理MAX98357A音频播放与视频帧显示的同步
 * - 支持不同视频格式的兼容性处理
 * - 动态调整播放速度以保持音视频同步
 * - 系统资源占用优化
 *
 * 同步机制:
 * - 以视频帧时间戳为参考基准
 * - 音频采样计数为辅助同步点
 * - 动态缓冲区管理，吸收抖动
 * - 同步容差: 50ms (可配置)
 */

#ifndef AVSYNC_PLAYBACK_MANAGER_H
#define AVSYNC_PLAYBACK_MANAGER_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "Max98357a_AudioPlayer.h"
#include "Shared_GlobalDefines.h"

// 同步状态
typedef enum {
    AV_SYNC_STATE_IDLE = 0,
    AV_SYNC_STATE_SYNCING,
    AV_SYNC_STATE_IN_SYNC,
    AV_SYNC_STATE_OUT_OF_SYNC
} AVSyncState;

// 视频帧信息
typedef struct {
    uint32_t frameIndex;       // 帧序号
    uint32_t timestamp;        // 时间戳(ms)
    uint16_t* frameBuffer;    // 帧数据缓冲
    size_t bufferSize;         // 缓冲大小
    bool keyFrame;             // 是否为关键帧
} VideoFrameInfo;

// 播放统计信息
typedef struct {
    uint32_t totalFrames;          // 总播放帧数
    uint32_t droppedFrames;        // 丢帧数
    uint32_t audioUnderruns;       // 音频欠载次数
    uint32_t videoUnderruns;       // 视频欠载次数
    uint32_t syncAdjustments;      // 同步调整次数
    uint32_t maxSyncError;         // 最大同步误差(ms)
    float actualFrameRate;         // 实际帧率
} PlaybackStats;

class AVSyncPlaybackManager {
public:
    static AVSyncPlaybackManager& getInstance();

    bool init();
    bool deinit();

    bool startPlayback(const char* filePath);
    bool stopPlayback();
    bool pausePlayback();
    bool resumePlayback();

    bool onVideoFrameReady(VideoFrameInfo* frameInfo);
    bool onAudioFrameReady(AudioFrame* audioFrame);

    bool syncTick();

    AVSyncState getSyncState() { return m_syncState; }
    PlaybackStats getStats() { return m_stats; }

    void resetStats();

    void setSyncTolerance(uint32_t toleranceMs) {
        m_syncToleranceMs = toleranceMs;
    }
    uint32_t getSyncTolerance() { return m_syncToleranceMs; }

    bool isPlaying() { return m_playing; }

    int32_t getCurrentAudioTimeMs();
    int32_t getCurrentVideoTimeMs();
    int32_t getSyncErrorMs();

    static const uint32_t PLAYBACK_FB_WIDTH = 320;
    static const uint32_t PLAYBACK_FB_HEIGHT = 180;

public:
    AVSyncPlaybackManager();
    ~AVSyncPlaybackManager();
    AVSyncPlaybackManager(const AVSyncPlaybackManager&) = delete;
    AVSyncPlaybackManager& operator=(const AVSyncPlaybackManager&) = delete;

    bool loadVideoFile(const char* filePath);
    bool unloadVideoFile();

    bool feedAudioToPlayer(const int16_t* samples, size_t count);
    bool displayVideoFrame(VideoFrameInfo* frameInfo);

    int32_t calculateSyncError();
    bool adjustPlayback();

    static void playbackTaskFunc(void* param);

private:
    bool m_initialized;
    bool m_playing;
    bool m_paused;

    AVSyncState m_syncState;
    uint32_t m_syncToleranceMs;

    SemaphoreHandle_t m_mutex;
    StaticSemaphore_t m_mutexBuffer;

    TaskHandle_t m_playbackTaskHandle;

    Max98357aAudioPlayer* m_audioPlayer;

    uint32_t m_videoStartTime;
    uint32_t m_audioStartTime;
    uint32_t m_lastFrameTime;
    uint32_t m_frameIntervalMs;

    int32_t m_audioLeadMs;
    int32_t m_videoLeadMs;

    PlaybackStats m_stats;

    bool m_videoLoaded;
    char m_currentFilePath[256];
};

#endif // AVSYNC_PLAYBACK_MANAGER_H