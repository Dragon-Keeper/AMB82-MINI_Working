/*
 * AVSync_PlaybackManager.cpp - 音视频同步播放管理器实现
 */

#include "AVSync_PlaybackManager.h"
#include "Utils_Logger.h"
#include "Display_TFTManager.h"
#include "Inmp441_MicrophoneManager.h"

#define TAG "AVSync"

extern Display_TFTManager tftManager;

AVSyncPlaybackManager g_avSyncPlayer;

AVSyncPlaybackManager::AVSyncPlaybackManager()
    : m_initialized(false)
    , m_playing(false)
    , m_paused(false)
    , m_syncState(AV_SYNC_STATE_IDLE)
    , m_syncToleranceMs(MAX98357A_SYNC_TOLERANCE)
    , m_mutex(NULL)
    , m_playbackTaskHandle(NULL)
    , m_audioPlayer(NULL)
    , m_videoStartTime(0)
    , m_audioStartTime(0)
    , m_lastFrameTime(0)
    , m_frameIntervalMs(33)
    , m_audioLeadMs(0)
    , m_videoLeadMs(0)
    , m_videoLoaded(false)
{
    memset(&m_stats, 0, sizeof(PlaybackStats));
    memset(m_currentFilePath, 0, sizeof(m_currentFilePath));
}

AVSyncPlaybackManager::~AVSyncPlaybackManager() {
    deinit();
}

AVSyncPlaybackManager& AVSyncPlaybackManager::getInstance() {
    return g_avSyncPlayer;
}

bool AVSyncPlaybackManager::init() {
    if (m_initialized) {
        Utils_Logger::warn(TAG, "Already initialized");
        return true;
    }

    Utils_Logger::info(TAG, "Initializing AV Sync Playback Manager...");

    m_mutex = xSemaphoreCreateMutexStatic(&m_mutexBuffer);
    if (!m_mutex) {
        Utils_Logger::error(TAG, "Failed to create mutex");
        return false;
    }

    m_audioPlayer = &Max98357aAudioPlayer::getInstance();
    if (!m_audioPlayer->init()) {
        Utils_Logger::error(TAG, "Failed to initialize audio player");
        vSemaphoreDelete(m_mutex);
        return false;
    }

    m_frameIntervalMs = 1000 / CAM_FPS;
    m_initialized = true;
    m_syncState = AV_SYNC_STATE_IDLE;

    Utils_Logger::info(TAG, "AV Sync Playback Manager initialized");
    Utils_Logger::info(TAG, "Frame interval: %ums", m_frameIntervalMs);
    Utils_Logger::info(TAG, "Sync tolerance: %ums", m_syncToleranceMs);

    return true;
}

bool AVSyncPlaybackManager::deinit() {
    if (!m_initialized) {
        return true;
    }

    stopPlayback();

    if (m_audioPlayer) {
        m_audioPlayer->deinit();
    }

    if (m_mutex) {
        vSemaphoreDelete(m_mutex);
        m_mutex = NULL;
    }

    m_initialized = false;
    m_syncState = AV_SYNC_STATE_IDLE;

    Utils_Logger::info(TAG, "AV Sync Playback Manager deinitialized");
    return true;
}

bool AVSyncPlaybackManager::startPlayback(const char* filePath) {
    if (!m_initialized) {
        Utils_Logger::error(TAG, "Not initialized");
        return false;
    }

    if (m_playing) {
        Utils_Logger::warn(TAG, "Already playing");
        return true;
    }

    if (xSemaphoreTake(m_mutex, portMAX_DELAY) != pdTRUE) {
        return false;
    }

    Utils_Logger::info(TAG, "Starting playback: %s", filePath);

    strncpy(m_currentFilePath, filePath, sizeof(m_currentFilePath) - 1);

    if (!loadVideoFile(filePath)) {
        Utils_Logger::error(TAG, "Failed to load video file");
        xSemaphoreGive(m_mutex);
        return false;
    }

    resetStats();

    if (!m_audioPlayer->startPlayback()) {
        Utils_Logger::error(TAG, "Failed to start audio playback");
        unloadVideoFile();
        xSemaphoreGive(m_mutex);
        return false;
    }

    m_videoStartTime = millis();
    m_audioStartTime = m_videoStartTime;
    m_lastFrameTime = 0;
    m_audioLeadMs = 0;
    m_videoLeadMs = 0;
    m_syncState = AV_SYNC_STATE_SYNCING;
    m_playing = true;
    m_paused = false;

    xSemaphoreGive(m_mutex);

    Utils_Logger::info(TAG, "Playback started successfully");
    return true;
}

bool AVSyncPlaybackManager::stopPlayback() {
    if (!m_playing) {
        return true;
    }

    Utils_Logger::info(TAG, "Stopping playback...");

    if (xSemaphoreTake(m_mutex, portMAX_DELAY) != pdTRUE) {
        return false;
    }

    m_playing = false;
    m_paused = false;

    m_audioPlayer->stopPlayback();
    unloadVideoFile();

    m_syncState = AV_SYNC_STATE_IDLE;

    Utils_Logger::info(TAG, "Playback stopped");
    Utils_Logger::info(TAG, "Stats - Frames: %u, Dropped: %u, AudioUnderruns: %u, SyncAdjusts: %u",
          m_stats.totalFrames, m_stats.droppedFrames,
          m_stats.audioUnderruns, m_stats.syncAdjustments);

    xSemaphoreGive(m_mutex);

    return true;
}

bool AVSyncPlaybackManager::pausePlayback() {
    if (!m_playing || m_paused) {
        return false;
    }

    Utils_Logger::info(TAG, "Pausing playback...");

    m_audioPlayer->pausePlayback();
    m_paused = true;

    return true;
}

bool AVSyncPlaybackManager::resumePlayback() {
    if (!m_playing || !m_paused) {
        return false;
    }

    Utils_Logger::info(TAG, "Resuming playback...");

    m_audioPlayer->resumePlayback();
    m_paused = false;

    return true;
}

bool AVSyncPlaybackManager::loadVideoFile(const char* filePath) {
    Utils_Logger::info(TAG, "Loading video file: %s", filePath);

    m_videoLoaded = true;
    return true;
}

bool AVSyncPlaybackManager::unloadVideoFile() {
    if (!m_videoLoaded) {
        return true;
    }

    Utils_Logger::info(TAG, "Unloading video file");
    m_videoLoaded = false;
    memset(m_currentFilePath, 0, sizeof(m_currentFilePath));

    return true;
}

bool AVSyncPlaybackManager::onVideoFrameReady(VideoFrameInfo* frameInfo) {
    if (!m_playing || m_paused) {
        return false;
    }

    if (!frameInfo) {
        return false;
    }

    m_stats.totalFrames++;
    m_lastFrameTime = millis();

    return true;
}

bool AVSyncPlaybackManager::onAudioFrameReady(AudioFrame* audioFrame) {
    if (!m_playing || m_paused) {
        return false;
    }

    if (!audioFrame) {
        return false;
    }

    if (!m_audioPlayer->writeAudioFrame(audioFrame)) {
        m_stats.audioUnderruns++;
        return false;
    }

    return true;
}

bool AVSyncPlaybackManager::feedAudioToPlayer(const int16_t* samples, size_t count) {
    if (!samples || count == 0) {
        return false;
    }

    if (!m_audioPlayer->isPlaying()) {
        return false;
    }

    if (!m_audioPlayer->writeAudioData(samples, count)) {
        m_stats.audioUnderruns++;
        return false;
    }

    return true;
}

bool AVSyncPlaybackManager::displayVideoFrame(VideoFrameInfo* frameInfo) {
    if (!frameInfo || !frameInfo->frameBuffer) {
        return false;
    }

    tftManager.drawBitmap(0, 0, PLAYBACK_FB_WIDTH, PLAYBACK_FB_HEIGHT, frameInfo->frameBuffer);

    return true;
}

int32_t AVSyncPlaybackManager::calculateSyncError() {
    int32_t audioTimeMs = getCurrentAudioTimeMs();
    int32_t videoTimeMs = getCurrentVideoTimeMs();

    return audioTimeMs - videoTimeMs;
}

bool AVSyncPlaybackManager::adjustPlayback() {
    int32_t syncError = calculateSyncError();

    if (abs(syncError) <= (int32_t)m_syncToleranceMs) {
        if (m_syncState != AV_SYNC_STATE_IN_SYNC) {
            m_syncState = AV_SYNC_STATE_IN_SYNC;
            Utils_Logger::info(TAG, "Audio-Video in sync, error: %ums", syncError);
        }
        return true;
    }

    m_syncState = AV_SYNC_STATE_OUT_OF_SYNC;

    if (syncError > 0) {
        m_audioLeadMs = syncError;
        m_videoLeadMs = 0;
        Utils_Logger::warn(TAG, "Audio ahead by %ums", syncError);

        int32_t adjust = syncError / 2;
        m_audioPlayer->adjustPlaybackSpeed(-adjust);
    } else {
        m_videoLeadMs = -syncError;
        m_audioLeadMs = 0;
        Utils_Logger::warn(TAG, "Video ahead by %ums", -syncError);

        int32_t adjust = -syncError / 2;
        m_audioPlayer->adjustPlaybackSpeed(adjust);
    }

    m_stats.syncAdjustments++;

    if (abs(syncError) > (int32_t)m_stats.maxSyncError) {
        m_stats.maxSyncError = abs(syncError);
    }

    return true;
}

bool AVSyncPlaybackManager::syncTick() {
    if (!m_playing || m_paused) {
        return false;
    }

    int32_t syncError = calculateSyncError();

    if (abs(syncError) > (int32_t)m_syncToleranceMs) {
        adjustPlayback();
    }

    return true;
}

int32_t AVSyncPlaybackManager::getCurrentAudioTimeMs() {
    if (!m_audioPlayer) {
        return 0;
    }

    uint32_t playedSamples = m_audioPlayer->getPlayedSamples();
    return (playedSamples * 1000) / MAX98357A_SAMPLE_RATE;
}

int32_t AVSyncPlaybackManager::getCurrentVideoTimeMs() {
    if (!m_playing) {
        return 0;
    }

    uint32_t currentTime = millis();
    return (int32_t)(currentTime - m_videoStartTime);
}

int32_t AVSyncPlaybackManager::getSyncErrorMs() {
    return calculateSyncError();
}

void AVSyncPlaybackManager::resetStats() {
    memset(&m_stats, 0, sizeof(PlaybackStats));
    m_stats.actualFrameRate = CAM_FPS;
}

void AVSyncPlaybackManager::playbackTaskFunc(void* param) {
    Utils_Logger::info(TAG, "Playback task started");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}