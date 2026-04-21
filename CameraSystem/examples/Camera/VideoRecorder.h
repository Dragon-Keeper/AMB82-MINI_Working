/*
 * VideoRecorder.h - 视频录制模块头文件
 * 实现视频和音频的采集、编码和存储功能
 */

#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "VideoStream.h"
#include "AmebaFatFS.h"
#include "Shared_GlobalDefines.h"
#include "DS3231_ClockModule.h"
#include "Camera_SDCardManager.h"
#include "Camera_CameraManager.h"
#include "Encoder_Control.h"

// 全局CameraManager对象声明
extern CameraManager cameraManager;

// 录制和播放状态枚举
typedef enum {
    REC_IDLE = 0,       // 空闲未录制
    REC_RECORDING = 1,  // 正在录制
    REC_PLAYING = 2,     // 正在播放
    REC_FILE_LIST = 3,   // 文件列表浏览
    REC_MENU = 4         // 功能选择菜单
} RecorderState;

// 媒体类型枚举
typedef enum {
    MEDIA_TYPE_VIDEO = 0,  // 视频文件
    MEDIA_TYPE_IMAGE = 1   // 图片文件
} MediaType;

// 媒体文件信息结构体
typedef struct {
    char fileName[256];   // 文件名
    uint32_t fileSize;    // 文件大小
    uint32_t duration;    // 视频时长（秒），图片为0
    MediaType mediaType;  // 媒体类型
    uint16_t fdate;       // 文件日期 (FAT格式)
    uint16_t ftime;       // 文件时间 (FAT格式)
} MediaFileInfo;

// 全局录制状态变量
extern RecorderState g_recorderState;

// 媒体文件列表相关全局变量
extern bool fileListNeedsRedraw;
extern bool isImageViewing;
extern bool isPlaying;
extern bool isPaused;
extern bool isBackButtonSelected;
extern uint32_t currentMediaIndex;
extern uint32_t currentGroupIndex;

// 缩略图缓存结构体
typedef struct {
    bool valid;            // 缓存是否有效
    uint8_t* jpegFrame;    // JPEG帧数据
    uint32_t frameSize;    // JPEG帧大小
    uint32_t width;        // 缩略图宽度
    uint32_t height;       // 缩略图高度
    uint32_t lastUsed;     // 最后使用时间戳（用于LRU）
    uint32_t priority;     // 优先级（0-10，10最高）
} ThumbnailCache;

// 对外暴露的核心函数声明
void videoRecorderInit(void);
void videoRecorderCleanup(void);
void startVideoRecording(void);
void stopVideoRecording(void);
void videoRecorderLoop(void);
void processPreviewFrame(void);
bool generateThumbnail(const char* fileName, ThumbnailCache& cache, MediaType mediaType);

// 媒体播放相关函数声明
void startVideoPlayback(const char* fileName);
void stopVideoPlayback(void);
void pauseVideoPlayback(void);
void resumeVideoPlayback(void);
void startImageViewer(const char* fileName);
void stopImageViewer(void);
void nextMediaFile(void);
void prevMediaFile(void);
void enterFileListMode(void);
void exitFileListMode(void);
uint32_t getMediaFileCount(void);
const MediaFileInfo* getCurrentMediaFile(void);
void updateFileList(void);
void videoPlaybackLoop(void);
void imageViewerLoop(void);
void drawFileListUI(void);
void updateEncoderButtonCallback(void);
void updateEncoderRotationCallback(void);
void videoHandleEncoderButton(void);
void videoHandleEncoderRotation(RotationDirection direction);

#endif // VIDEO_RECORDER_H
