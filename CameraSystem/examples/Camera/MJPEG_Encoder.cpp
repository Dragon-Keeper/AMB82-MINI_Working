/*
 * MJPEG_Encoder.cpp - MJPEG视频编码器实现
 * 实现JPEG流编码为AVI格式视频文件
 * 支持双音视频流同步录制
 */

#include "MJPEG_Encoder.h"
#include "Utils_Logger.h"
#include "Shared_GlobalDefines.h"

// 外部SD卡管理器实例
extern SDCardManager sdCardManager;

// 辅助函数：以小端字节序写入32位整数
void writeLE32(uint8_t* buffer, uint32_t value) {
    buffer[0] = value & 0xFF;
    buffer[1] = (value >> 8) & 0xFF;
    buffer[2] = (value >> 16) & 0xFF;
    buffer[3] = (value >> 24) & 0xFF;
}

// 辅助函数：以小端字节序写入16位整数
void writeLE16(uint8_t* buffer, uint16_t value) {
    buffer[0] = value & 0xFF;
    buffer[1] = (value >> 8) & 0xFF;
}

MJPEGEncoder::MJPEGEncoder()
    : m_recording(false)
    , m_width(0)
    , m_height(0)
    , m_fps(0)
    , m_frameCount(0)
    , m_audioFrameCount(0)
    , m_totalAudioSamples(0)
    , m_fileSize(0)
    , m_buffer(nullptr)
    , m_bufferSize(0)
    , m_bufferPos(0)
    , m_maxBufferSize(15 * 1024 * 1024)
    , m_moviStartPos(0)
    , m_moviDataStart(0)
    , m_totalFramesOffset(0)
    , m_videoStrhLengthOffset(0)
    , m_audioStrhLengthOffset(0)
    , m_indexEntries(nullptr)
    , m_indexEntryCount(0)
    , m_indexCapacity(0)
    , m_mutex(nullptr)
{
    memset(m_fileName, 0, sizeof(m_fileName));
    m_mutex = xSemaphoreCreateMutex();
}

MJPEGEncoder::~MJPEGEncoder() {
    if (m_indexEntries) {
        free(m_indexEntries);
        m_indexEntries = nullptr;
    }
    if (m_mutex) {
        vSemaphoreDelete(m_mutex);
        m_mutex = nullptr;
    }
}

bool MJPEGEncoder::begin(const char* fileName, uint32_t width, uint32_t height, uint32_t fps) {
    if (m_recording) {
        Utils_Logger::error("MJPEGEncoder already recording");
        return false;
    }
    
    if (!sdCardManager.isInitialized()) {
        Utils_Logger::error("SDCardManager not initialized");
        return false;
    }
    
    strncpy(m_fileName, fileName, sizeof(m_fileName) - 1);
    m_width = width;
    m_height = height;
    m_fps = fps;
    m_frameCount = 0;
    m_audioFrameCount = 0;
    m_totalAudioSamples = 0;
    m_fileSize = 0;
    m_bufferPos = 0;
    m_recording = true;
    m_indexEntryCount = 0;
    m_indexCapacity = 2000;
    
    m_bufferSize = m_maxBufferSize + 131072;
    
    m_buffer = (uint8_t*)malloc(m_bufferSize);
    if (!m_buffer) {
        Utils_Logger::error("Failed to allocate buffer memory");
        m_recording = false;
        return false;
    }
    
    m_indexEntries = (AVIIndexEntry*)malloc(m_indexCapacity * sizeof(AVIIndexEntry));
    if (!m_indexEntries) {
        Utils_Logger::error("Failed to allocate index memory");
        free(m_buffer);
        m_buffer = nullptr;
        m_recording = false;
        return false;
    }
    
    if (!writeAVIHeader()) {
        Utils_Logger::error("Failed to write AVI header");
        free(m_buffer);
        m_buffer = nullptr;
        if (m_indexEntries) {
            free(m_indexEntries);
            m_indexEntries = nullptr;
        }
        m_recording = false;
        return false;
    }

    Utils_Logger::info("After writeAVIHeader: m_moviStartPos=%d, m_bufferPos=%d", m_moviStartPos, m_bufferPos);
    Utils_Logger::info("Verify movi at %d: '%c%c%c%c', size at %d",
                      m_moviStartPos, m_buffer[m_moviStartPos], m_buffer[m_moviStartPos+1],
                      m_buffer[m_moviStartPos+2], m_buffer[m_moviStartPos+3],
                      m_moviStartPos+4);

    Utils_Logger::info("MJPEGEncoder started: %s, %dx%d, %dfps", fileName, width, height, fps);
    return true;
}

bool MJPEGEncoder::addVideoFrame(const uint8_t* jpegData, uint32_t jpegSize, uint32_t timestamp) {
    if (!m_recording || !m_buffer || !m_indexEntries) {
        return false;
    }
    
    if (m_mutex && xSemaphoreTake(m_mutex, portMAX_DELAY) != pdTRUE) {
        return false;
    }
    
    uint32_t chunkStartPos = m_bufferPos;
    
    uint32_t requiredSize = jpegSize + 8 + 1;
    if (m_bufferPos + requiredSize > m_bufferSize) {
        Utils_Logger::error("Buffer overflow");
        if (m_mutex) xSemaphoreGive(m_mutex);
        return false;
    }
    
    if (m_indexEntryCount >= m_indexCapacity) {
        uint32_t newCapacity = m_indexCapacity * 2;
        AVIIndexEntry* newEntries = (AVIIndexEntry*)realloc(m_indexEntries, newCapacity * sizeof(AVIIndexEntry));
        if (!newEntries) {
            Utils_Logger::error("Failed to expand index memory");
            if (m_mutex) xSemaphoreGive(m_mutex);
            return false;
        }
        m_indexEntries = newEntries;
        m_indexCapacity = newCapacity;
    }
    
    AVIIndexEntry& entry = m_indexEntries[m_indexEntryCount++];
    memcpy(entry.fourcc, "00db", 4);
    entry.flags = 0x00000010;
    entry.chunkOffset = chunkStartPos - m_moviDataStart;
    entry.chunkSize = jpegSize;
    
    memcpy(m_buffer + m_bufferPos, "00db", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, jpegSize);
    m_bufferPos += 4;
    
    memcpy(m_buffer + m_bufferPos, jpegData, jpegSize);
    m_bufferPos += jpegSize;
    
    if (jpegSize % 2 != 0) {
        m_buffer[m_bufferPos++] = 0;
        entry.chunkSize = jpegSize + 1;
    }
    
    m_frameCount++;
    
    if (m_frameCount % 10 == 0) {
        Utils_Logger::info("Added video frame %d: offset=%d, size=%d", 
                          m_frameCount, entry.chunkOffset, entry.chunkSize);
    }
    
    if (m_mutex) xSemaphoreGive(m_mutex);
    return true;
}

bool MJPEGEncoder::addAudioFrame(const uint8_t* audioData, uint32_t audioSize, uint32_t timestamp) {
    if (!m_recording || !m_buffer || !m_indexEntries) {
        return false;
    }
    
    if (m_mutex && xSemaphoreTake(m_mutex, portMAX_DELAY) != pdTRUE) {
        return false;
    }
    
    uint32_t chunkStartPos = m_bufferPos;
    
    uint32_t requiredSize = audioSize + 8 + 1;
    if (m_bufferPos + requiredSize > m_bufferSize) {
        Utils_Logger::error("Buffer overflow for audio");
        if (m_mutex) xSemaphoreGive(m_mutex);
        return false;
    }
    
    if (m_indexEntryCount >= m_indexCapacity) {
        uint32_t newCapacity = m_indexCapacity * 2;
        AVIIndexEntry* newEntries = (AVIIndexEntry*)realloc(m_indexEntries, newCapacity * sizeof(AVIIndexEntry));
        if (!newEntries) {
            Utils_Logger::error("Failed to expand index memory for audio");
            if (m_mutex) xSemaphoreGive(m_mutex);
            return false;
        }
        m_indexEntries = newEntries;
        m_indexCapacity = newCapacity;
    }
    
    AVIIndexEntry& entry = m_indexEntries[m_indexEntryCount++];
    memcpy(entry.fourcc, "01wb", 4);
    entry.flags = 0x00000000;
    entry.chunkOffset = chunkStartPos - m_moviDataStart;
    entry.chunkSize = audioSize;
    
    memcpy(m_buffer + m_bufferPos, "01wb", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, audioSize);
    m_bufferPos += 4;
    
    memcpy(m_buffer + m_bufferPos, audioData, audioSize);
    m_bufferPos += audioSize;
    
    if (audioSize % 2 != 0) {
        m_buffer[m_bufferPos++] = 0;
        entry.chunkSize = audioSize + 1;
    }
    
    m_audioFrameCount++;
    m_totalAudioSamples += (entry.chunkSize / 2);
    
    if (m_audioFrameCount % 20 == 0) {
        Utils_Logger::info("Added audio frame %d: offset=%d, size=%d, totalSamples=%d", 
                          m_audioFrameCount, entry.chunkOffset, entry.chunkSize, m_totalAudioSamples);
    }
    
    if (m_mutex) xSemaphoreGive(m_mutex);
    return true;
}

uint32_t MJPEGEncoder::readLE32(const uint8_t* buffer) {
    return (uint32_t)buffer[0] | 
           (uint32_t)buffer[1] << 8 | 
           (uint32_t)buffer[2] << 16 | 
           (uint32_t)buffer[3] << 24;
}

bool MJPEGEncoder::end(const DS3231_Time* fileTime) {
    if (!m_recording || !m_buffer) {
        return false;
    }
    
    Utils_Logger::info("Ending MJPEG recording: video=%d frames, audio=%d chunks", 
                     m_frameCount, m_audioFrameCount);
    
    bool success = true;
    
    uint32_t moviDataSize = m_bufferPos - m_moviDataStart;
    uint32_t moviListSize = 4 + moviDataSize;
    writeLE32(m_buffer + m_moviStartPos + 4, moviListSize);
    
    Utils_Logger::info("movi debug: startPos=%d, dataStart=%d, dataSize=%d, listSize=%d",
                      m_moviStartPos, m_moviDataStart, moviDataSize, moviListSize);
    Utils_Logger::info("movi header check: '%c%c%c%c' at %d, size at %d, '%c%c%c%c' at %d",
                      m_buffer[m_moviStartPos], m_buffer[m_moviStartPos+1],
                      m_buffer[m_moviStartPos+2], m_buffer[m_moviStartPos+3],
                      m_moviStartPos,
                      m_moviStartPos+4,
                      m_buffer[m_moviStartPos+8], m_buffer[m_moviStartPos+9],
                      m_buffer[m_moviStartPos+10], m_buffer[m_moviStartPos+11],
                      m_moviStartPos+8);

    uint32_t storedMoviSize = readLE32(m_buffer + m_moviStartPos + 4);
    Utils_Logger::info("MOVI size verification: expected=%d, stored=%d", moviListSize, storedMoviSize);

    if (storedMoviSize != moviListSize) {
        Utils_Logger::error("MOVI size mismatch! Fixing...");
        writeLE32(m_buffer + m_moviStartPos + 4, moviListSize);
    }
    
    if (!writeIndex()) {
        Utils_Logger::error("Failed to write index");
        success = false;
    }
    
    m_fileSize = m_bufferPos;
    
    uint32_t riffSize = m_fileSize - 8;
    writeLE32(m_buffer + 4, riffSize);

    uint32_t hdrlSize = m_moviStartPos - 20;
    writeLE32(m_buffer + 16, hdrlSize);
    Utils_Logger::info("hdrl size updated: %d (m_moviStartPos=%d)", hdrlSize, m_moviStartPos);
    
    if (m_totalFramesOffset > 0) {
        writeLE32(m_buffer + m_totalFramesOffset, m_frameCount);
        Utils_Logger::info("Updated total video frames to %d", m_frameCount);
    }
    
    if (m_videoStrhLengthOffset > 0) {
        writeLE32(m_buffer + m_videoStrhLengthOffset, m_frameCount);
        Utils_Logger::info("Updated video strh length to %d", m_frameCount);
    }
    
    if (m_audioStrhLengthOffset > 0) {
        writeLE32(m_buffer + m_audioStrhLengthOffset, m_totalAudioSamples);
        Utils_Logger::info("Updated audio strh length to %d samples (was %d frames)", m_totalAudioSamples, m_audioFrameCount);
    }
    
    Utils_Logger::info("Final file size: %d bytes", m_fileSize);
    Utils_Logger::info("Index entries: %d", m_indexEntryCount);
    
    debugAVIStructure();
    
    if (memcmp(m_buffer, "RIFF", 4) != 0) {
        Utils_Logger::error("RIFF marker missing");
        success = false;
    }
    
    if (memcmp(m_buffer + 8, "AVI ", 4) != 0) {
        Utils_Logger::error("AVI marker missing");
        success = false;
    }
    
    bool idx1Found = false;
    for (uint32_t i = m_moviStartPos; i < m_bufferPos - 4; i++) {
        if (memcmp(m_buffer + i, "idx1", 4) == 0) {
            Utils_Logger::info("idx1 found at offset %d", i);
            idx1Found = true;
            break;
        }
    }
    
    if (!idx1Found) {
        Utils_Logger::error("idx1 index block not found!");
        success = false;
    }
    
    // 写入文件时直接传递时间参数，避免额外的时间戳设置调用
    int32_t writtenBytes = sdCardManager.writeFile(m_fileName, m_buffer, m_fileSize, fileTime);
    if (writtenBytes != (int32_t)m_fileSize) {
        Utils_Logger::error("Failed to write file: expected %d, wrote %d", 
                          m_fileSize, writtenBytes);
        success = false;
    } else {
        Utils_Logger::info("Successfully wrote %d bytes", writtenBytes);
    }
    
    free(m_buffer);
    m_buffer = nullptr;
    
    if (m_indexEntries) {
        free(m_indexEntries);
        m_indexEntries = nullptr;
    }
    
    m_recording = false;
    
    Utils_Logger::info("MJPEGEncoder ended: %s", m_fileName);
    
    return success;
}

bool MJPEGEncoder::writeAVIHeader() {
    if (!m_buffer) {
        return false;
    }
    
    memcpy(m_buffer + m_bufferPos, "RIFF", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    memcpy(m_buffer + m_bufferPos, "AVI ", 4);
    m_bufferPos += 4;
    
    memcpy(m_buffer + m_bufferPos, "LIST", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    memcpy(m_buffer + m_bufferPos, "hdrl", 4);
    m_bufferPos += 4;
    
    memcpy(m_buffer + m_bufferPos, "avih", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 56);
    m_bufferPos += 4;
    
    uint32_t microSecPerFrame = 1000000 / m_fps;
    uint32_t maxBytesPerSec = 10000000;
    uint32_t paddingGranularity = 0;
    uint32_t flags = 0x00000010;
    uint32_t totalFrames = 0;
    uint32_t initialFrames = 0;
    uint32_t streams = 2;
    uint32_t suggestedBufferSize = m_width * m_height * 3 + 4096;
    
    writeLE32(m_buffer + m_bufferPos, microSecPerFrame);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, maxBytesPerSec);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, paddingGranularity);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, flags);
    m_bufferPos += 4;
    
    m_totalFramesOffset = m_bufferPos;
    writeLE32(m_buffer + m_bufferPos, totalFrames);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, initialFrames);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, streams);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, suggestedBufferSize);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, m_width);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, m_height);
    m_bufferPos += 4;
    
    for (int i = 0; i < 4; i++) {
        writeLE32(m_buffer + m_bufferPos, 0);
        m_bufferPos += 4;
    }
    
    memcpy(m_buffer + m_bufferPos, "LIST", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 116);
    m_bufferPos += 4;
    memcpy(m_buffer + m_bufferPos, "strl", 4);
    m_bufferPos += 4;
    
    memcpy(m_buffer + m_bufferPos, "strh", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 56);
    m_bufferPos += 4;
    
    memcpy(m_buffer + m_bufferPos, "vids", 4);
    m_bufferPos += 4;
    memcpy(m_buffer + m_bufferPos, "MJPG", 4);
    m_bufferPos += 4;
    
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    writeLE16(m_buffer + m_bufferPos, 0);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, 0);
    m_bufferPos += 2;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 1);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, m_fps);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    
    m_videoStrhLengthOffset = m_bufferPos;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    
    writeLE32(m_buffer + m_bufferPos, m_width * m_height * 3);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    writeLE16(m_buffer + m_bufferPos, 0);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, 0);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, m_width);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, m_height);
    m_bufferPos += 2;
    
    memcpy(m_buffer + m_bufferPos, "strf", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 40);
    m_bufferPos += 4;
    
    uint32_t biSize = 40;
    int32_t biWidth = m_width;
    int32_t biHeight = m_height;
    uint16_t biPlanes = 1;
    uint16_t biBitCount = 24;
    uint32_t biCompression = 0x47504A4D;
    uint32_t biSizeImage = m_width * m_height * 3;
    
    writeLE32(m_buffer + m_bufferPos, biSize);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, biWidth);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, biHeight);
    m_bufferPos += 4;
    writeLE16(m_buffer + m_bufferPos, biPlanes);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, biBitCount);
    m_bufferPos += 2;
    writeLE32(m_buffer + m_bufferPos, biCompression);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, biSizeImage);
    m_bufferPos += 4;
    
    for (int i = 0; i < 4; i++) {
        writeLE32(m_buffer + m_bufferPos, 0);
        m_bufferPos += 4;
    }
    
    memcpy(m_buffer + m_bufferPos, "LIST", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 94);
    m_bufferPos += 4;
    memcpy(m_buffer + m_bufferPos, "strl", 4);
    m_bufferPos += 4;
    
    memcpy(m_buffer + m_bufferPos, "strh", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 56);
    m_bufferPos += 4;
    
    memcpy(m_buffer + m_bufferPos, "auds", 4);
    m_bufferPos += 4;
    memcpy(m_buffer + m_bufferPos, "\x01\x00\x00\x00", 4);
    m_bufferPos += 4;
    
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    writeLE16(m_buffer + m_bufferPos, 0);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, 0);
    m_bufferPos += 2;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 1);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, AUDIO_SAMPLE_RATE);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    
    m_audioStrhLengthOffset = m_bufferPos;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    
    uint32_t bytesPerSec = AUDIO_SAMPLE_RATE * AUDIO_CHANNELS * (AUDIO_BITS_PER_SAMPLE / 8);
    uint16_t blockAlign = AUDIO_CHANNELS * (AUDIO_BITS_PER_SAMPLE / 8);
    
    writeLE32(m_buffer + m_bufferPos, 4096);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    writeLE16(m_buffer + m_bufferPos, 0);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, 0);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, 0);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, 0);
    m_bufferPos += 2;
    
    memcpy(m_buffer + m_bufferPos, "strf", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 18);
    m_bufferPos += 4;
    
    writeLE16(m_buffer + m_bufferPos, 0x0001);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, AUDIO_CHANNELS);
    m_bufferPos += 2;
    writeLE32(m_buffer + m_bufferPos, AUDIO_SAMPLE_RATE);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, bytesPerSec);
    m_bufferPos += 4;
    writeLE16(m_buffer + m_bufferPos, blockAlign);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, AUDIO_BITS_PER_SAMPLE);
    m_bufferPos += 2;
    writeLE16(m_buffer + m_bufferPos, 0);
    m_bufferPos += 2;
    
    m_moviStartPos = m_bufferPos;
    Utils_Logger::info("BEFORE movi write: m_bufferPos=%d", m_bufferPos);
    memcpy(m_buffer + m_bufferPos, "LIST", 4);
    m_bufferPos += 4;
    writeLE32(m_buffer + m_bufferPos, 0);
    m_bufferPos += 4;
    memcpy(m_buffer + m_bufferPos, "movi", 4);
    m_bufferPos += 4;
    Utils_Logger::info("AFTER movi write: m_bufferPos=%d, m_moviStartPos=%d", m_bufferPos, m_moviStartPos);
    Utils_Logger::info("Verify at %d: '%c%c%c%c', at %d: '%c%c%c%c'",
                      m_moviStartPos, m_buffer[m_moviStartPos], m_buffer[m_moviStartPos+1],
                      m_buffer[m_moviStartPos+2], m_buffer[m_moviStartPos+3],
                      m_moviStartPos+8, m_buffer[m_moviStartPos+8], m_buffer[m_moviStartPos+9],
                      m_buffer[m_moviStartPos+10], m_buffer[m_moviStartPos+11]);
    
    m_moviDataStart = m_bufferPos;
    
    return true;
}

bool MJPEGEncoder::writeIndex() {
    if (!m_buffer || !m_indexEntries || m_indexEntryCount == 0) {
        return false;
    }
    
    uint32_t requiredSize = m_indexEntryCount * 16 + 8;
    if (m_bufferPos + requiredSize + 2 > m_bufferSize) {
        Utils_Logger::error("Not enough buffer space for index");
        return false;
    }
    
    memcpy(m_buffer + m_bufferPos, "idx1", 4);
    m_bufferPos += 4;
    
    uint32_t indexSize = m_indexEntryCount * 16;
    writeLE32(m_buffer + m_bufferPos, indexSize);
    m_bufferPos += 4;
    
    for (uint32_t i = 0; i < m_indexEntryCount; i++) {
        const AVIIndexEntry& entry = m_indexEntries[i];
        
        memcpy(m_buffer + m_bufferPos, entry.fourcc, 4);
        m_bufferPos += 4;
        writeLE32(m_buffer + m_bufferPos, entry.flags);
        m_bufferPos += 4;
        writeLE32(m_buffer + m_bufferPos, entry.chunkOffset);
        m_bufferPos += 4;
        writeLE32(m_buffer + m_bufferPos, entry.chunkSize);
        m_bufferPos += 4;
    }
    
    if (m_bufferPos % 2 != 0) {
        m_buffer[m_bufferPos++] = 0;
    }
    
    Utils_Logger::info("idx1 written: %d entries", m_indexEntryCount);
    
    return true;
}

bool MJPEGEncoder::isRecording() const {
    return m_recording;
}

uint32_t MJPEGEncoder::getFrameCount() const {
    return m_frameCount;
}

uint32_t MJPEGEncoder::getFileSize() const {
    return m_fileSize;
}

void MJPEGEncoder::debugAVIStructure() {
    Utils_Logger::info("=== AVI Structure Debug ===");
    
    if (m_bufferPos > 0) {
        Utils_Logger::info("RIFF at 0: %c%c%c%c", 
                          m_buffer[0], m_buffer[1], m_buffer[2], m_buffer[3]);
        Utils_Logger::info("File size: %u", readLE32(m_buffer + 4));
        Utils_Logger::info("AVI at 8: %c%c%c%c", 
                          m_buffer[8], m_buffer[9], m_buffer[10], m_buffer[11]);
        Utils_Logger::info("movi LIST at %u", m_moviStartPos);
        Utils_Logger::info("Total buffer: %u bytes", m_bufferPos);
    }
    Utils_Logger::info("=== End AVI Structure Debug ===");
}

// MJPEGDecoder类实现（保持原样，仅调整构造函数和析构函数）

MJPEGDecoder::MJPEGDecoder()
    : m_open(false)
    , m_width(0)
    , m_height(0)
    , m_fps(0)
    , m_frameCount(0)
    , m_fileSize(0)
    , m_currentFrameIndex(0)
    , m_buffer(nullptr)
    , m_bufferSize(0)
    , m_moviStartPos(0)
    , m_indexStartPos(0)
    , m_frameInfos(nullptr)
    , m_frameInfoCount(0)
{
    memset(m_fileName, 0, sizeof(m_fileName));
    memset(&m_file, 0, sizeof(m_file));
}

MJPEGDecoder::~MJPEGDecoder() {
    close();
    if (m_buffer) {
        free(m_buffer);
        m_buffer = nullptr;
    }
    if (m_frameInfos) {
        free(m_frameInfos);
        m_frameInfos = nullptr;
    }
}

bool MJPEGDecoder::open(const char* fileName) {
    if (m_open) {
        close();
    }
    
    strncpy(m_fileName, fileName, sizeof(m_fileName) - 1);
    
    FRESULT res = f_open(&m_file, fileName, FA_READ);
    if (res != FR_OK) {
        Utils_Logger::error("Failed to open file: %s", fileName);
        return false;
    }
    
    m_fileSize = f_size(&m_file);
    
    m_bufferSize = 1024 * 1024;
    m_buffer = (uint8_t*)malloc(m_bufferSize);
    if (!m_buffer) {
        Utils_Logger::error("Failed to allocate buffer");
        f_close(&m_file);
        return false;
    }
    
    bool success = false;
    
    m_width = 640;
    m_height = 480;
    m_fps = 15;
    m_frameCount = 100;
    
    if (parseSimplifiedMovi()) {
        success = true;
    }
    
    if (!success) {
        if (parseAVIHeader()) {
            if (parseIndex()) {
                if (parseMoviList()) {
                    success = true;
                }
            }
        }
    }
    
    if (success) {
        m_open = true;
        m_currentFrameIndex = 0;
        Utils_Logger::info("MJPEGDecoder opened: %s", fileName);
        return true;
    } else {
        f_close(&m_file);
        free(m_buffer);
        m_buffer = nullptr;
        return false;
    }
}

bool MJPEGDecoder::close() {
    if (m_open) {
        f_close(&m_file);
        m_open = false;
    }
    
    if (m_frameInfos) {
        free(m_frameInfos);
        m_frameInfos = nullptr;
    }
    
    m_frameInfoCount = 0;
    m_currentFrameIndex = 0;
    m_frameCount = 0;
    
    return true;
}

bool MJPEGDecoder::readNextFrame(uint8_t** frameData, uint32_t* frameSize) {
    if (!m_open || !m_frameInfos) {
        return false;
    }
    
    if (m_currentFrameIndex >= m_frameInfoCount) {
        if (!parseMoreFrames()) {
            return false;
        }
    }
    
    if (m_currentFrameIndex >= m_frameInfoCount) {
        return false;
    }
    
    FrameInfo& frameInfo = m_frameInfos[m_currentFrameIndex];
    
    if (frameInfo.size > m_bufferSize) {
        uint8_t* newBuffer = (uint8_t*)realloc(m_buffer, frameInfo.size);
        if (!newBuffer) {
            return false;
        }
        m_buffer = newBuffer;
        m_bufferSize = frameInfo.size;
    }
    
    f_lseek(&m_file, frameInfo.offset);
    UINT bytesRead;
    f_read(&m_file, m_buffer, frameInfo.size, &bytesRead);
    
    *frameData = m_buffer;
    *frameSize = frameInfo.size;
    m_currentFrameIndex++;
    
    return true;
}

bool MJPEGDecoder::parseMoreFrames() {
    return m_frameInfoCount < 999;
}

bool MJPEGDecoder::seekToFrame(uint32_t frameIndex) {
    if (!m_open || !m_frameInfos || frameIndex >= m_frameInfoCount) {
        return false;
    }
    m_currentFrameIndex = frameIndex;
    return true;
}

bool MJPEGDecoder::isOpen() const {
    return m_open;
}

uint32_t MJPEGDecoder::getFrameCount() const {
    return m_frameCount;
}

uint32_t MJPEGDecoder::getWidth() const {
    return m_width;
}

uint32_t MJPEGDecoder::getHeight() const {
    return m_height;
}

uint32_t MJPEGDecoder::getFPS() const {
    return m_fps;
}

uint32_t MJPEGDecoder::getFileSize() const {
    return m_fileSize;
}

uint32_t MJPEGDecoder::getCurrentFrameIndex() const {
    return m_currentFrameIndex;
}

bool MJPEGDecoder::parseAVIHeader() {
    return true;
}

bool MJPEGDecoder::parseIndex() {
    return true;
}

bool MJPEGDecoder::parseMoviList() {
    return true;
}

bool MJPEGDecoder::parseSimplifiedMovi() {
    m_frameInfoCount = 0;
    m_frameInfos = (FrameInfo*)malloc(10000 * sizeof(FrameInfo));
    if (!m_frameInfos) {
        return false;
    }
    
    uint32_t pos = 0;
    uint32_t foundFrames = 0;
    
    // 先寻找 movi 块，跳过文件头部
    while (pos < m_fileSize - 8) {
        UINT bytesRead;
        f_lseek(&m_file, pos);
        f_read(&m_file, m_buffer, 8, &bytesRead);
        
        if (memcmp(m_buffer, "movi", 4) == 0) {
            pos += 8;
            break;
        }
        pos += 1;
    }
    
    // 从 movi 块开始查找所有视频帧
    while (pos < m_fileSize - 8 && foundFrames < 10000) {
        UINT bytesRead;
        f_lseek(&m_file, pos);
        f_read(&m_file, m_buffer, 8, &bytesRead);
        
        if (memcmp(m_buffer, "00db", 4) == 0) {
            // 找到视频帧
            uint32_t frameSize = readLE32(m_buffer + 4);
            if (frameSize > 100 && frameSize < 1000000) {
                if (pos + 8 + frameSize <= m_fileSize) {
                    m_frameInfos[m_frameInfoCount].offset = pos + 8;
                    m_frameInfos[m_frameInfoCount].size = frameSize;
                    m_frameInfoCount++;
                    foundFrames++;
                    pos += 8 + frameSize + (frameSize % 2);
                    continue;
                }
            }
            pos += 8;
        } else if (memcmp(m_buffer, "01wb", 4) == 0) {
            // 找到音频帧，跳过
            uint32_t audioSize = readLE32(m_buffer + 4);
            pos += 8 + audioSize + (audioSize % 2);
        } else if (memcmp(m_buffer, "idx1", 4) == 0) {
            // 找到索引块，停止解析
            break;
        } else {
            // 其他块，向前移动
            pos += 1;
        }
    }
    
    m_frameCount = m_frameInfoCount;
    Utils_Logger::info("MJPEGDecoder found %d video frames", m_frameInfoCount);
    return m_frameInfoCount > 0;
}

uint32_t MJPEGDecoder::readLE32(const uint8_t* buffer) {
    return (uint32_t)buffer[0] | 
           (uint32_t)buffer[1] << 8 | 
           (uint32_t)buffer[2] << 16 | 
           (uint32_t)buffer[3] << 24;
}

bool MJPEGDecoder::readChunkHeader(char* chunkID, uint32_t* chunkSize) {
    return true;
}