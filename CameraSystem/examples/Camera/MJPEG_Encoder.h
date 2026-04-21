/*
 * MJPEG_Encoder.h - MJPEG视频编码器头文件
 * 实现JPEG流编码为AVI格式视频文件
 * 支持双音视频流同步录制
 * V1.18: 添加互斥锁保护多任务并发访问
 */

#ifndef MJPEG_ENCODER_H
#define MJPEG_ENCODER_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "Camera_SDCardManager.h"
#include "DS3231_ClockModule.h"
#include "AmebaFatFS.h"

// AVI索引条目结构体
struct AVIIndexEntry {
    char fourcc[4];     // 块标识符
    uint32_t flags;     // 标志
    uint32_t chunkOffset; // 块偏移量
    uint32_t chunkSize;   // 块大小
};

class MJPEGEncoder {
public:
    MJPEGEncoder();
    ~MJPEGEncoder();
    
    bool begin(const char* fileName, uint32_t width, uint32_t height, uint32_t fps);
    bool addVideoFrame(const uint8_t* jpegData, uint32_t jpegSize, uint32_t timestamp);
    bool addAudioFrame(const uint8_t* audioData, uint32_t audioSize, uint32_t timestamp);
    bool end(const DS3231_Time* fileTime = nullptr);
    
    bool isRecording() const;
    uint32_t getFrameCount() const;
    uint32_t getFileSize() const;
    void debugAVIStructure();
    
private:
    bool writeAVIHeader();
    bool writeIndex();
    uint32_t readLE32(const uint8_t* buffer);
    
    char m_fileName[256];
    bool m_recording;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_fps;
    uint32_t m_frameCount;
    uint32_t m_audioFrameCount;
    uint32_t m_totalAudioSamples;
    uint32_t m_fileSize;
    
    uint8_t* m_buffer;
    uint32_t m_bufferSize;
    uint32_t m_bufferPos;
    uint32_t m_maxBufferSize;
    uint32_t m_moviStartPos;
    uint32_t m_moviDataStart;
    uint32_t m_totalFramesOffset;
    uint32_t m_videoStrhLengthOffset;
    uint32_t m_audioStrhLengthOffset;
    
    AVIIndexEntry* m_indexEntries;
    uint32_t m_indexEntryCount;
    uint32_t m_indexCapacity;
    
    SemaphoreHandle_t m_mutex;
};

// MJPEG解码器类
class MJPEGDecoder {
public:
    MJPEGDecoder();
    ~MJPEGDecoder();
    
    bool open(const char* fileName);
    bool close();
    bool readNextFrame(uint8_t** frameData, uint32_t* frameSize);
    bool seekToFrame(uint32_t frameIndex);
    bool isOpen() const;
    uint32_t getFrameCount() const;
    uint32_t getWidth() const;
    uint32_t getHeight() const;
    uint32_t getFPS() const;
    uint32_t getFileSize() const;
    uint32_t getCurrentFrameIndex() const;
    
private:
    bool parseAVIHeader();
    bool parseIndex();
    bool parseMoviList();
    bool parseSimplifiedMovi();
    bool parseMoreFrames();
    uint32_t readLE32(const uint8_t* buffer);
    bool readChunkHeader(char* chunkID, uint32_t* chunkSize);
    
    char m_fileName[256];
    bool m_open;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_fps;
    uint32_t m_frameCount;
    uint32_t m_fileSize;
    uint32_t m_currentFrameIndex;
    
    uint8_t* m_buffer;
    uint32_t m_bufferSize;
    
    FIL m_file;
    uint32_t m_moviStartPos;
    uint32_t m_indexStartPos;
    
    struct FrameInfo {
        uint32_t offset;
        uint32_t size;
    };
    
    FrameInfo* m_frameInfos;
    uint32_t m_frameInfoCount;
};

#endif // MJPEG_ENCODER_H