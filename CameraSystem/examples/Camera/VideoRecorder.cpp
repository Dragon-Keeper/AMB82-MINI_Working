/*
 * VideoRecorder.cpp - 视频录制模块实现
 * 实现视频和音频的采集、编码和存储功能
 */

#include "VideoRecorder.h"
#include "Utils_Logger.h"
#include "Display_TFTManager.h"
#include "Display_AmebaST7789_DMA_SPI1.h"
#include "Display_font16x16.h"
#include "JPEGDEC.h"
#include "Encoder_Control.h"
#include "MJPEG_Encoder.h"
#include "Shared_GlobalDefines.h"
#include "Inmp441_MicrophoneManager.h"
#include "RTOS_TaskFactory.h"
#include "RTOS_TaskManager.h"

// 外部对象引用
extern Display_TFTManager tftManager;
extern JPEGDEC jpeg;
extern EncoderControl encoder;

// 媒体文件列表最大数量（定义在文件顶部，确保所有函数可用）
#define MAX_MEDIA_FILES 500

// 前向声明
void cleanupThumbnailCache(void);
void cleanupThumbnailCacheItems(uint32_t count);
void cleanupExcessThumbnailCache(void);
bool initThumbnailCache(uint32_t size);
bool resizeThumbnailCache(uint32_t newSize);

// 全局变量声明
extern uint32_t currentVideoIndex;
extern uint32_t lastSelectedVideoIndex;
extern uint32_t currentGroupIndex;
extern const uint32_t VIDEOS_PER_GROUP;

// 定义全局录制状态变量
RecorderState g_recorderState = REC_IDLE;

// 外部对象引用
extern SDCardManager sdCardManager;

// 全局变量定义


// 全局CameraManager对象声明
extern CameraManager cameraManager;

// 音视频配置和对象
#define VIDEO_CHANNEL_RECORD 0
#define VIDEO_CHANNEL_PREVIEW 1

// 旋转编码器按钮回调函数（视频录制专用）
void handleVideoEncoderButton() {
    // 根据当前录制状态切换视频录制
    if (g_recorderState == REC_IDLE) {
        // 空闲状态：启动视频录制
        startVideoRecording();
    } else if (g_recorderState == REC_RECORDING) {
        // 录制状态：停止视频录制
        stopVideoRecording();
    }
}

// 视频和音频配置
// 配置录制通道为720P、15fps、JPEG编码器
VideoSetting configV(VIDEO_HD, 15, VIDEO_JPEG, 1); // HD=720P, 15fps, JPEG编码, 启用快照
// 配置预览通道为480P(640x480)、15fps、JPEG编码器、启用快照功能（用于实时预览）
VideoSetting configVPreview(VIDEO_VGA, 15, VIDEO_JPEG, 1); // snapshot=1表示启用全尺寸快照

// MJPEG编码器对象
MJPEGEncoder mjpegEncoder;

// 录制状态跟踪
bool updatemodifiedtime = false;
char recordingFileName[128];

// 预览相关变量
typedef struct {
    uint32_t imgAddr;
    uint32_t imgLen;
} PreviewBuffer;

PreviewBuffer g_previewBuffer;

// JPEG解码回调函数
static int32_t jpegReadCallback(JPEGFILE *pFile, uint8_t *pBuf, int32_t lLen) {
    uint8_t* pSrc = (uint8_t*)pFile->fHandle;
    memcpy(pBuf, pSrc + pFile->iPos, lLen);
    pFile->iPos += lLen;
    return lLen;
}

// 内存定位回调函数
static int32_t jpegSeekCallback(JPEGFILE *pFile, int32_t lPos) {
    pFile->iPos = lPos;
    return lPos;
}

// JPEG解码回调函数（直接绘制模式 - 用于非预览场景如缩略图等）
static int JPEGDraw(JPEGDRAW *pDraw) {
    tftManager.drawBitmap(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
    return 1;
}

// ============================================
// 视频预览帧缓冲区系统（与拍照模式一致的高性能实现）
// 使用帧缓冲区收集完整画面后一次性DMA传输，大幅提升帧率
// ============================================
#define PREVIEW_FB_WIDTH   320
#define PREVIEW_FB_HEIGHT  240

static uint16_t s_previewFrameBuffer[PREVIEW_FB_WIDTH * PREVIEW_FB_HEIGHT];
static bool s_previewFrameReady = false;

// ============================================
// 视频回放帧缓冲区系统（高性能回放，整帧一次DMA传输）
// 1280x720视频以1/4缩放解码为320x180，写入帧缓冲区后一次性绘制
// ============================================
#define PLAYBACK_FB_WIDTH   320
#define PLAYBACK_FB_HEIGHT  180

static uint16_t s_playbackFrameBuffer[PLAYBACK_FB_WIDTH * PLAYBACK_FB_HEIGHT];
static bool s_playbackFrameReady = false;

static int JPEGDrawForPlayback(JPEGDRAW *pDraw) {
    int x = pDraw->x;
    int y = pDraw->y;
    int width = pDraw->iWidth;
    int height = pDraw->iHeight;

    if (x >= PLAYBACK_FB_WIDTH) return 1;
    if (y >= PLAYBACK_FB_HEIGHT) return 1;
    if (x < 0) { width += x; x = 0; }
    if (width <= 0 || height <= 0) return 1;

    int drawWidth = width;
    int drawHeight = height;
    if (x + drawWidth > PLAYBACK_FB_WIDTH) drawWidth = PLAYBACK_FB_WIDTH - x;
    if (y + drawHeight > PLAYBACK_FB_HEIGHT) drawHeight = PLAYBACK_FB_HEIGHT - y;

    for (int row = 0; row < drawHeight; row++) {
        int srcOffset = row * width;
        int dstOffset = (y + row) * PLAYBACK_FB_WIDTH + x;
        for (int col = 0; col < drawWidth; col++) {
            s_playbackFrameBuffer[dstOffset + col] = pDraw->pPixels[srcOffset + col];
        }
    }

    s_playbackFrameReady = true;
    return 1;
}

// 视频预览专用JPEGDraw回调 - 将MCU块写入帧缓冲区
static int JPEGDrawForPreview(JPEGDRAW *pDraw) {
    int x = pDraw->x;
    int y = pDraw->y;
    int width = pDraw->iWidth;
    int height = pDraw->iHeight;
    
    if (x >= PREVIEW_FB_WIDTH) return 1;
    if (y >= PREVIEW_FB_HEIGHT) return 1;
    
    if (x < 0) { width += x; x = 0; }
    if (width <= 0 || height <= 0) return 1;
    
    int drawWidth = width;
    int drawHeight = height;
    if (x + drawWidth > PREVIEW_FB_WIDTH) drawWidth = PREVIEW_FB_WIDTH - x;
    if (y + drawHeight > PREVIEW_FB_HEIGHT) drawHeight = PREVIEW_FB_HEIGHT - y;
    
    for (int row = 0; row < drawHeight; row++) {
        int srcOffset = row * width;
        int dstOffset = (y + row) * PREVIEW_FB_WIDTH + x;
        for (int col = 0; col < drawWidth; col++) {
            s_previewFrameBuffer[dstOffset + col] = pDraw->pPixels[srcOffset + col];
        }
    }
    
    s_previewFrameReady = true;
    return 1;
}

// DS1307时钟读取函数
void readDS1307Time(DS1307_Time& time) {
    static unsigned long lastErrorTime = 0;
    static bool hasReportedError = false;
    
    if (!clockModule.readTime(time)) {
        // 只在第一次失败或间隔10秒后才报告错误
        unsigned long currentTime = millis();
        if (!hasReportedError || (currentTime - lastErrorTime > 10000)) {
            Utils_Logger::error("Failed to read time from DS1307");
            hasReportedError = true;
            lastErrorTime = currentTime;
        }
        // 设置默认时间
        time = {0, 0, 0, 1, 1, 1, 2026};
    } else {
        // 读取成功时重置错误状态
        hasReportedError = false;
    }
}

// 格式化时间戳
void formatTimeStamp(char* buffer, uint32_t bufferSize, const DS1307_Time& time) {
    snprintf(buffer, bufferSize, "%04u-%02u-%02u %02u:%02u:%02u",
             time.year, time.month, time.date,
             time.hours, time.minutes, time.seconds);
}

void videoRecorderInit(void) {
    Utils_Logger::info("Initializing Video Recorder...");
    
    // 串口初始化
    Serial.begin(115200);
    
    // DS1307时钟模块初始化
    // Utils_Logger::info("Initializing DS1307 Clock Module...");
    
    // 系统上电后立即读取一次DS1307时间，确保时间戳可用
    DS1307_Time initialTime;
    readDS1307Time(initialTime);
    
    // 音视频采集引脚初始化
    // Utils_Logger::info("Initializing Audio/Video Pins...");
    
    // 存储初始化 (通过SDCardManager)
    // Utils_Logger::info("Initializing Storage...");
    if (!sdCardManager.isInitialized()) {
        if (!sdCardManager.init()) {
            Utils_Logger::error("SD card initialization failed in VideoRecorder");
        } else {
            // Utils_Logger::info("SD card initialized successfully in VideoRecorder");
        }
    } else {
        // Utils_Logger::info("SD card already initialized");
    }
    
    // 配置相机视频通道
    // Utils_Logger::info("Configuring Camera Video Channels...");
    // 配置录制通道（JPEG编码）
    Camera.configVideoChannel(VIDEO_CHANNEL_RECORD, configV);
    // 配置预览通道
    Camera.configVideoChannel(VIDEO_CHANNEL_PREVIEW, configVPreview);
    Camera.videoInit();
    
    // 等待 VOE 硬件完全初始化（videoInit() 异步，需等待 video_pre_init_procedure 完成）
    Utils_Timer::delayMs(200);
    cameraManager.setVOEReady(true);
    
    // 启动预览通道，确保idle状态下也能显示预览画面
    // Utils_Logger::info("Starting Preview Channel...");
    Camera.channelBegin(VIDEO_CHANNEL_PREVIEW);
    
    // 在 channelBegin() 之后应用 ISP 参数，确保 VOE 完全就绪
    cameraManager.applyISPSettings();
    
    // 重置录制状态
    g_recorderState = REC_IDLE;
    
    // 设置旋转编码器按钮回调函数
    encoder.setButtonCallback(handleVideoEncoderButton);
    
    // 初始化缩略图缓存
    // Utils_Logger::info("Initializing Thumbnail Cache...");
    initThumbnailCache(MAX_MEDIA_FILES);
    
    Utils_Logger::info("Video Recorder Initialized Successfully");

}

void videoRecorderCleanup(void) {
    Utils_Logger::info("Cleaning up Video Recorder...");
    
    // 停止并清理预览通道
    // Utils_Logger::info("Stopping Preview Channel...");
    Camera.channelEnd(VIDEO_CHANNEL_PREVIEW);
    
    // 停止视频初始化（释放视频资源）
    // Utils_Logger::info("Stopping Video Initialization...");
    cameraManager.setVOEReady(false);
    // 注意：Camera.videoEnd()可能不存在，这里我们只停止通道
    
    // 清理缩略图缓存
    // Utils_Logger::info("Cleaning up Thumbnail Cache...");
    cleanupThumbnailCache();
    
    // Utils_Logger::info("Video Recorder Cleaned Up Successfully");
}

void startVideoRecording(void) {
    if (g_recorderState != REC_IDLE) {
        Utils_Logger::error("Video Recorder is not in IDLE state, cannot start recording");
        return;
    }
    
    Utils_Logger::info("Starting Video Recording with Audio...");
    
    // 生成录制文件名（使用.avi扩展名，前缀为Video）
    char fileName[128];
    if (sdCardManager.generateTimestampFileName(fileName, sizeof(fileName), ".avi") == nullptr) {
        Utils_Logger::error("Failed to generate recording filename");
        return;
    }
    
    // 复制文件名用于后续使用
    strncpy(recordingFileName, fileName, sizeof(recordingFileName));
    
    // 启动MJPEG录制（先初始化编码器）
    if (!mjpegEncoder.begin(fileName, 1280, 720, 15)) {
        Utils_Logger::error("Failed to start MJPEG encoder");
        return;
    }
    
    // 先更新录制状态，确保音频处理任务创建后能正确检测状态
    g_recorderState = REC_RECORDING;
    updatemodifiedtime = false;
    
    // 启动视频通道（先启动视频，确保视频帧开始采集）
    Camera.channelBegin(VIDEO_CHANNEL_RECORD);
    
    // 创建视频帧获取RTOS任务（优先级5，高于音频处理任务的优先级4）
    TaskManager::createTask(TaskManager::TASK_VIDEO_FRAME_CAPTURE);
    // Utils_Logger::info("Video frame capture RTOS task created (priority 5)");
    
    // 启动音频采集（在视频通道启动后再启动音频，确保同步开始）
    if (!g_microphoneManager.startAVIRecording()) {
        Utils_Logger::info("Audio recording init failed, continuing with video only");
    } else {
        TaskManager::createTask(TaskManager::TASK_AUDIO_PROCESSING);
        // Utils_Logger::info("Audio processing RTOS task created (priority 4)");
    }
    
    // 读取DS1307获取录制开始时间戳
    DS1307_Time startTime;
    readDS1307Time(startTime);
    
    // 打印开始录制日志
    char timeStamp[32];
    formatTimeStamp(timeStamp, sizeof(timeStamp), startTime);
    Utils_Logger::info("Video recording started at: %s, file: %s", timeStamp, recordingFileName);
}

void stopVideoRecording(void) {
    // 即使状态不是REC_RECORDING，也尝试停止录制，提高用户体验
    if (g_recorderState != REC_RECORDING) {
        // Utils_Logger::info("Video Recorder is not in RECORDING state, attempting to clean up...");
        // 仍然执行停止逻辑，确保资源被正确释放
    }
    
    Utils_Logger::info("Stopping Video Recording with Audio...");
    
    // 先停止视频通道（确保视频采集停止）
    Camera.channelEnd(VIDEO_CHANNEL_RECORD);
    
    // 删除视频帧获取RTOS任务
    TaskManager::deleteTask(TaskManager::TASK_VIDEO_FRAME_CAPTURE);
    // Utils_Logger::info("Video frame capture RTOS task deleted");
    
    // 删除音频处理RTOS任务（停止音频采集）
    TaskManager::deleteTask(TaskManager::TASK_AUDIO_PROCESSING);
    // Utils_Logger::info("Audio processing RTOS task deleted");
    
    // 不再刷新剩余音频数据，避免音画不同步
    // 直接丢弃队列中的剩余数据，确保音视频同步结束
    size_t queueAvailable = g_microphoneManager.getAudioQueueAvailable();
    if (queueAvailable > 0) {
        // Utils_Logger::info("Discarding remaining audio samples from queue: %d blocks", queueAvailable);
    }
    
    // 停止音频采集
    g_microphoneManager.stopAVIRecording();
    
    // 读取DS1307获取录制结束时间戳（在停止MJPEG录制前获取，确保时间戳准确）
    DS1307_Time endTime;
    readDS1307Time(endTime);
    
    // 停止MJPEG录制并传递时间参数（这样在文件写入时会自动设置时间戳）
    mjpegEncoder.end(&endTime);
    
    // 打印停止录制日志
    char timeStamp[32];
    formatTimeStamp(timeStamp, sizeof(timeStamp), endTime);
    Utils_Logger::info("Video recording stopped at: %s, file: %s", timeStamp, recordingFileName);
    
    // 确保更新录制状态
    g_recorderState = REC_IDLE;
    updatemodifiedtime = true;
    
    // Utils_Logger::info("Video Recording with Audio Stopped and File Saved Successfully");
}

void videoRecorderLoop(void) {
    if (g_recorderState != REC_RECORDING) {
        return;
    }

    static uint32_t lastAudioDebugTime = 0;
    static uint32_t audioBlockCounter = 0;
    unsigned long currentMillis = millis();

    AudioDataBlock audioBlock;

    while (g_microphoneManager.receiveAudioDataBlock(&audioBlock, 1 / portTICK_PERIOD_MS)) {
        if (audioBlock.count > 0) {
            size_t audioBytes = audioBlock.count * sizeof(int16_t);
            mjpegEncoder.addAudioFrame((const uint8_t*)audioBlock.samples, audioBytes, audioBlock.timestamp);
            audioBlockCounter++;

            // if (audioBlockCounter % 10 == 0) {
            //     Utils_Logger::info("Audio blocks written: %d", audioBlockCounter);
            // }
        }
    }

    if (currentMillis - lastAudioDebugTime >= 1000) {
        // size_t queueAvailable = g_microphoneManager.getAudioQueueAvailable();
        // Utils_Logger::info("Audio queue: available=%d, blocks=%d", queueAvailable, audioBlockCounter);
        lastAudioDebugTime = currentMillis;
    }
}

void processPreviewFrame(void) {
    if (g_recorderState != REC_RECORDING && g_recorderState != REC_IDLE) {
        return;
    }

    unsigned long currentMillis = millis();
    
    static unsigned long lastBlinkTime = 0;
    static bool dotVisible = false;
    bool shouldDrawDot = false;
    
    if (g_recorderState == REC_RECORDING) {
        if (currentMillis - lastBlinkTime >= 1000) {
            lastBlinkTime = currentMillis;
            dotVisible = !dotVisible;
        }
        shouldDrawDot = dotVisible;
    }
    
    uint32_t imgAddr;
    uint32_t imgLen;
    
    int retryCount = 0;
    const int MAX_RETRIES = 3;
    
    do {
        Camera.getImage(VIDEO_CHANNEL_PREVIEW, &imgAddr, &imgLen);
        if (imgLen > 0) break;
        if (retryCount < MAX_RETRIES - 1) delay(10);
        retryCount++;
    } while (retryCount < MAX_RETRIES);

    if (imgLen > 0) {
        memset(s_previewFrameBuffer, 0, sizeof(s_previewFrameBuffer));
        s_previewFrameReady = false;
        
        if (jpeg.open((void*)imgAddr, imgLen, nullptr, jpegReadCallback, jpegSeekCallback, JPEGDrawForPreview)) {
            jpeg.decode(0, 0, JPEG_SCALE_HALF);
            jpeg.close();
            
            if (s_previewFrameReady) {
                tftManager.drawBitmap(0, 0, PREVIEW_FB_WIDTH, PREVIEW_FB_HEIGHT, s_previewFrameBuffer);
            }
        }
    }
    
    if (shouldDrawDot) {
        const int dotRadius = 14;
        const int dotX = 320 - 20 - dotRadius;
        const int dotY = 20 + dotRadius;
        
        AmebaST7789_DMA_SPI1& tft = tftManager.getTFT();
        
        int x = dotRadius;
        int y = 0;
        int radiusError = 1 - x;
        
        while (x >= y) {
            tft.drawLine(dotX - x, dotY + y, dotX + x, dotY + y, ST7789_RED);
            tft.drawLine(dotX - x, dotY - y, dotX + x, dotY - y, ST7789_RED);
            tft.drawLine(dotX - y, dotY + x, dotX + y, dotY + x, ST7789_RED);
            tft.drawLine(dotX - y, dotY - x, dotX + y, dotY - x, ST7789_RED);
            
            y++;
            if (radiusError < 0) {
                radiusError += 2 * y + 1;
            } else {
                x--;
                radiusError += 2 * (y - x + 1);
            }
        }
    }
}

// 视频播放相关功能

// MJPEG解码器对象
MJPEGDecoder mjpegDecoder;

// 媒体文件列表相关变量
MediaFileInfo mediaFileList[MAX_MEDIA_FILES];
uint32_t mediaFileCount = 0;
bool fileListNeedsRedraw = true; // 文件列表重绘标志
unsigned long lastDrawTime = 0; // 上次绘制时间

// 返回按钮相关变量
bool isBackButtonSelected = true; // 当前是否选中了返回按钮
bool lastBackButtonSelected = false; // 上一次返回按钮的选中状态
uint32_t currentMediaIndex = 0; // 当前选中的媒体索引
uint32_t lastSelectedMediaIndex = UINT32_MAX; // 上一次选中的媒体索引
uint32_t currentGroupIndex = 0; // 当前组索引
const uint32_t MEDIA_PER_GROUP = 4; // 每组显示4个媒体

// 播放状态
bool isPlaying = false;
bool isPaused = false;
unsigned long lastFrameTime = 0;

// 图片查看状态
bool isImageViewing = false;
uint8_t* currentImageBuffer = nullptr;
uint32_t currentImageSize = 0;

// 外部变量声明
extern DS1307_ClockModule clockModule;
extern SDCardManager sdCardManager;
extern Display_TFTManager tftManager;
extern JPEGDEC jpeg;
extern EncoderControl encoder;

// 比较函数：按时间递减排序（最新的在前）
int compareMediaFiles(const void* a, const void* b) {
    const MediaFileInfo* fileA = (const MediaFileInfo*)a;
    const MediaFileInfo* fileB = (const MediaFileInfo*)b;
    
    // 先比较日期
    if (fileA->fdate != fileB->fdate) {
        return fileB->fdate - fileA->fdate;
    }
    // 日期相同，比较时间
    return fileB->ftime - fileA->ftime;
}

// 扫描SD卡上的媒体文件（视频和图片）
void updateFileList(void) {
    mediaFileCount = 0;
    
    if (!sdCardManager.isInitialized()) {
        Utils_Logger::error("SD card not initialized");
        return;
    }
    
    Utils_Logger::info("Scanning for media files...");
    
    // 打开根目录
    DIR dir;
    FILINFO fno;
    
    FRESULT res = f_opendir(&dir, "/");
    if (res != FR_OK) {
        Utils_Logger::error("Failed to open directory: %d", res);
        return;
    }
    
    // 遍历目录
    while (mediaFileCount < MAX_MEDIA_FILES) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0) {
            break;
        }
        
        // 检查是否是文件
        if (!(fno.fattrib & AM_DIR)) {
            const char* ext = strrchr(fno.fname, '.');
            bool isVideo = false;
            bool isImage = false;
            
            if (ext) {
                // 检查是否是视频文件
                if (strcasecmp(ext, ".avi") == 0) {
                    isVideo = true;
                }
                // 检查是否是图片文件
                else if (strcasecmp(ext, ".jpg") == 0 || 
                         strcasecmp(ext, ".jpeg") == 0 ||
                         strcasecmp(ext, ".bmp") == 0) {
                    isImage = true;
                }
            }
            
            if (isVideo || isImage) {
                // 添加到文件列表
                strncpy(mediaFileList[mediaFileCount].fileName, fno.fname, sizeof(mediaFileList[mediaFileCount].fileName));
                mediaFileList[mediaFileCount].fileSize = fno.fsize;
                mediaFileList[mediaFileCount].duration = 0;
                mediaFileList[mediaFileCount].mediaType = isVideo ? MEDIA_TYPE_VIDEO : MEDIA_TYPE_IMAGE;
                mediaFileList[mediaFileCount].fdate = fno.fdate;
                mediaFileList[mediaFileCount].ftime = fno.ftime;
                mediaFileCount++;
                
                // Utils_Logger::info("Found %s file: %s, size: %u bytes", 
                //                   isVideo ? "AVI" : "Image", 
                //                   fno.fname, fno.fsize);
            }
        }
    }
    
    f_closedir(&dir);
    
    // 按时间递减排序（最新的在前）
    if (mediaFileCount > 1) {
        qsort(mediaFileList, mediaFileCount, sizeof(MediaFileInfo), compareMediaFiles);
        // Utils_Logger::info("Media files sorted by time descending");
    }
    
    Utils_Logger::info("Found %u media files", mediaFileCount);
    currentMediaIndex = 0;
}

// 进入文件列表模式
void enterFileListMode(void) {
    if (!sdCardManager.isInitialized()) {
        // Utils_Logger::info("SD card not initialized, attempting to initialize...");
        if (!sdCardManager.init()) {
            Utils_Logger::error("SD card initialization failed, cannot enter file list mode");
            tftManager.fillScreen(ST7789_BLACK);
            tftManager.setTextSize(1);
            tftManager.setCursor(10, 10);
            tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
            tftManager.print("SD card not inserted");
            return;
        }
        // Utils_Logger::info("SD card initialized successfully in enterFileListMode");
    }
    
    updateFileList();
    g_recorderState = REC_FILE_LIST;
    // 重置组索引为0，确保从第一组开始显示
    currentGroupIndex = 0;
    // 默认选中返回按钮
    isBackButtonSelected = true;
    // 重置最后选中状态变量，确保每次进入时状态都是干净的
    lastBackButtonSelected = false;
    lastSelectedMediaIndex = UINT32_MAX;
    // 重置当前媒体索引为0
    currentMediaIndex = 0;
    fileListNeedsRedraw = true; // 设置重绘标志
    
    // Utils_Logger::info("Entered file list mode, %u files available, starting from group 1", mediaFileCount);
}

// 退出文件列表模式
void exitFileListMode(void) {
    // 在Camera项目中，通过RTOS事件机制触发返回主菜单
    g_recorderState = REC_MENU;
    TaskManager::setEvent(EVENT_RETURN_TO_MENU);
    // Utils_Logger::info("Exited file list mode and triggered return to menu event");
}

// 选择下一个媒体文件（仅用于同一组内导航）
void nextMediaFile(void) {
    if (mediaFileCount > 0) {
        // 计算下一个媒体索引
        uint32_t nextIndex = currentMediaIndex + 1;
        
        // 确保不超出当前组范围
        uint32_t currentGroupStart = currentGroupIndex * MEDIA_PER_GROUP;
        uint32_t currentGroupEnd = min(currentGroupStart + MEDIA_PER_GROUP, mediaFileCount);
        
        if (nextIndex < currentGroupEnd) {
            currentMediaIndex = nextIndex;
            Utils_Logger::info("Selected next media: %s", mediaFileList[currentMediaIndex].fileName);
        }
    }
}

// 选择上一个媒体文件（仅用于同一组内导航）
void prevMediaFile(void) {
    if (mediaFileCount > 0) {
        // 计算上一个媒体索引
        uint32_t prevIndex = currentMediaIndex - 1;
        
        // 确保不超出当前组范围
        uint32_t currentGroupStart = currentGroupIndex * MEDIA_PER_GROUP;
        
        if (prevIndex >= currentGroupStart) {
            currentMediaIndex = prevIndex;
            Utils_Logger::info("Selected previous media: %s", mediaFileList[currentMediaIndex].fileName);
        }
    }
}

// 开始视频播放
void startVideoPlayback(const char* fileName) {
    if (!fileName || strlen(fileName) == 0) {
        Utils_Logger::error("Invalid file name");
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextSize(1);
        tftManager.setCursor(10, 10);
        tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
        tftManager.print("Invalid file name");
        return;
    }
    
    if (!sdCardManager.isInitialized()) {
        Utils_Logger::error("SD card not initialized");
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextSize(1);
        tftManager.setCursor(10, 10);
        tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
        tftManager.print("SD card not inserted");
        return;
    }
    
    // 检查文件是否存在
    FIL file;
    FRESULT res = f_open(&file, fileName, FA_READ);
    if (res != FR_OK) {
        Utils_Logger::error("File not found: %s, error: %d", fileName, res);
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextSize(1);
        tftManager.setCursor(10, 10);
        tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
        tftManager.print("File not found");
        return;
    }
    f_close(&file);
    
    if (!mjpegDecoder.open(fileName)) {
        Utils_Logger::error("Failed to open video file: %s", fileName);
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextSize(1);
        tftManager.setCursor(10, 10);
        tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
        tftManager.print("Unsupported video format");
        return;
    }
    
    g_recorderState = REC_PLAYING;
    isPlaying = true;
    isPaused = false;
    lastFrameTime = 0;
    
    // 编码器回调已在Camera.ino中统一管理，无需在此更新
    
    Utils_Logger::info("Started playing video: %s", fileName);
}

// 停止视频播放
void stopVideoPlayback(void) {
    mjpegDecoder.close();
    isPlaying = false;
    isPaused = false;
    g_recorderState = REC_FILE_LIST;
    
    // 强制重绘文件列表界面
    fileListNeedsRedraw = true;
    
    // 重置lastSelectedMediaIndex，确保下次绘制时强制重绘整个界面
    lastSelectedMediaIndex = UINT32_MAX;
    
    // 立即绘制文件列表界面，确保快速响应
    drawFileListUI();
    
    Utils_Logger::info("Stopped video playback, returning to file list");
    
    // 编码器回调已在Camera.ino中统一管理，无需在此更新
}

// 暂停视频播放
void pauseVideoPlayback(void) {
    if (isPlaying) {
        isPaused = true;
        Utils_Logger::info("Paused video playback");
    }
}

// 恢复视频播放
void resumeVideoPlayback(void) {
    if (isPlaying && isPaused) {
        isPaused = false;
        lastFrameTime = millis();
        Utils_Logger::info("Resumed video playback");
    }
}

// 获取媒体文件数量
uint32_t getMediaFileCount(void) {
    return mediaFileCount;
}

// 获取当前选中的媒体文件
const MediaFileInfo* getCurrentMediaFile(void) {
    if (mediaFileCount > 0 && currentMediaIndex < mediaFileCount) {
        return &mediaFileList[currentMediaIndex];
    }
    return nullptr;
}

// 开始图片查看
void startImageViewer(const char* fileName) {
    if (!fileName || strlen(fileName) == 0) {
        Utils_Logger::error("Invalid file name");
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextSize(1);
        tftManager.setCursor(10, 10);
        tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
        tftManager.print("Invalid file name");
        return;
    }
    
    if (!sdCardManager.isInitialized()) {
        Utils_Logger::error("SD card not initialized");
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextSize(1);
        tftManager.setCursor(10, 10);
        tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
        tftManager.print("SD card not inserted");
        return;
    }
    
    // 读取图片文件到内存
    FIL file;
    FRESULT res = f_open(&file, fileName, FA_READ);
    if (res != FR_OK) {
        Utils_Logger::error("File not found: %s, error: %d", fileName, res);
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextSize(1);
        tftManager.setCursor(10, 10);
        tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
        tftManager.print("File not found");
        return;
    }
    
    // 分配内存
    UINT fileSize = f_size(&file);
    currentImageBuffer = (uint8_t*)malloc(fileSize);
    if (!currentImageBuffer) {
        Utils_Logger::error("Failed to allocate memory for image");
        f_close(&file);
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextSize(1);
        tftManager.setCursor(10, 10);
        tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
        tftManager.print("Memory error");
        return;
    }
    
    // 读取文件
    UINT bytesRead;
    res = f_read(&file, currentImageBuffer, fileSize, &bytesRead);
    f_close(&file);
    
    if (res != FR_OK || bytesRead != fileSize) {
        Utils_Logger::error("Failed to read image file");
        free(currentImageBuffer);
        currentImageBuffer = nullptr;
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextSize(1);
        tftManager.setCursor(10, 10);
        tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
        tftManager.print("Read error");
        return;
    }
    
    currentImageSize = fileSize;
    g_recorderState = REC_PLAYING; // 复用播放状态
    isImageViewing = true;
    
    // 解码并显示图片
    if (jpeg.open((void*)currentImageBuffer, currentImageSize, nullptr, jpegReadCallback, jpegSeekCallback, JPEGDraw)) {
        // 使用1/4缩放，确保宽度不超过240像素，与视频播放保持一致
        jpeg.decode(0, 30, JPEG_SCALE_QUARTER);
        jpeg.close();
    }
    
    Utils_Logger::info("Started viewing image: %s", fileName);
}

// 停止图片查看
void stopImageViewer(void) {
    if (currentImageBuffer) {
        free(currentImageBuffer);
        currentImageBuffer = nullptr;
    }
    currentImageSize = 0;
    isImageViewing = false;
    g_recorderState = REC_FILE_LIST;
    
    // 强制重绘文件列表界面
    fileListNeedsRedraw = true;
    lastSelectedMediaIndex = UINT32_MAX;
    drawFileListUI();
    
    Utils_Logger::info("Stopped image viewer, returning to file list");
}

// 图片查看循环
void imageViewerLoop(void) {
    // 图片查看是静态的，无需循环处理
    // 主要通过编码器按钮来退出
}

// 视频播放循环（使用帧缓冲区实现高性能回放）
void videoPlaybackLoop(void) {
    if (g_recorderState != REC_PLAYING || !isPlaying || isPaused) {
        return;
    }

    unsigned long currentMillis = millis();
    uint32_t fps = mjpegDecoder.getFPS();
    uint32_t frameInterval = (fps > 0) ? 1000 / fps : 67;

    if (currentMillis - lastFrameTime >= frameInterval) {
        lastFrameTime = currentMillis;

        uint8_t* frameData;
        uint32_t frameSize;

        if (mjpegDecoder.readNextFrame(&frameData, &frameSize)) {
            s_playbackFrameReady = false;
            if (jpeg.open((void*)frameData, frameSize, nullptr, jpegReadCallback, jpegSeekCallback, JPEGDrawForPlayback)) {
                jpeg.decode(0, 0, JPEG_SCALE_QUARTER);
                jpeg.close();
            }
            if (s_playbackFrameReady) {
                tftManager.drawBitmap(0, 30, PLAYBACK_FB_WIDTH, PLAYBACK_FB_HEIGHT, s_playbackFrameBuffer);
            }
        } else {
            // 播放结束
            stopVideoPlayback();
        }
    }
}

// 提取文件名主体部分（排除.avi/.jpg后缀）
void extractFileNameBody(const char* fullName, char* body, size_t bodySize) {
    const char* dotPos = strrchr(fullName, '.');
    if (dotPos && (strcasecmp(dotPos, ".avi") == 0 || strcasecmp(dotPos, ".jpg") == 0)) {
        size_t len = dotPos - fullName;
        if (len < bodySize) {
            strncpy(body, fullName, len);
            body[len] = '\0';
        } else {
            strncpy(body, fullName, bodySize - 1);
            body[bodySize - 1] = '\0';
        }
    } else {
        strncpy(body, fullName, bodySize - 1);
        body[bodySize - 1] = '\0';
    }
}

// 根据字符获取字体索引
int getFontIndex(char c) {
    if (c >= 'A' && c <= 'Z') {
        return FONT16_IDX_A + (c - 'A');
    } else if (c >= 'a' && c <= 'z') {
        return FONT16_IDX_a + (c - 'a');
    } else if (c >= '0' && c <= '9') {
        return FONT16_IDX_0 + (c - '0');
    }
    // 其他字符返回默认索引（这里返回下划线）
    return FONT16_IDX_UNDERLINE;
}

// 绘制单个字符
void drawChar(uint16_t x, uint16_t y, char c, uint16_t color) {
    int index = getFontIndex(c);
    if (index >= 0 && index < 148) {
        const uint8_t* fontData = font16x16[index];
        AmebaST7789_DMA_SPI1& tft = tftManager.getTFT();
        for (int row = 0; row < 16; row++) {
            uint16_t lineData = (fontData[row * 2] << 8) | fontData[row * 2 + 1];
            for (int col = 0; col < 16; col++) {
                if (lineData & (0x8000 >> col)) {
                    tft.drawPixel(x + col, y + row, color);
                }
            }
        }
    }
}

// 绘制字符串
void drawString(uint16_t x, uint16_t y, const char* str, uint16_t color) {
    int i = 0;
    while (str[i] != '\0') {
        drawChar(x + i * 16, y, str[i], color);
        i++;
    }
}

// 前向声明
void cleanupThumbnailCache(void);
void cleanupThumbnailCacheItems(uint32_t count);
void cleanupExcessThumbnailCache(void);
bool initThumbnailCache(uint32_t size);
bool resizeThumbnailCache(uint32_t newSize);

// 缩略图缓存
ThumbnailCache* thumbnailCache = nullptr;
uint32_t thumbnailCacheSize = 0;     // 当前缓存大小
uint32_t thumbnailCacheMaxSize = 0; // 最大缓存大小（0=无限制，由init设置）
uint32_t thumbnailCacheUsed = 0;     // 已使用的缓存项数

// 初始化缩略图缓存
bool initThumbnailCache(uint32_t size) {
    // 清理现有缓存
    cleanupThumbnailCache();
    
    // 分配新的缓存空间
    thumbnailCache = (ThumbnailCache*)malloc(size * sizeof(ThumbnailCache));
    if (!thumbnailCache) {
        Utils_Logger::error("Failed to allocate thumbnail cache memory");
        return false;
    }
    
    // 初始化缓存项
    for (uint32_t i = 0; i < size; i++) {
        thumbnailCache[i].valid = false;
        thumbnailCache[i].jpegFrame = nullptr;
        thumbnailCache[i].frameSize = 0;
        thumbnailCache[i].width = 0;
        thumbnailCache[i].height = 0;
        thumbnailCache[i].lastUsed = 0;
        thumbnailCache[i].priority = 5; // 默认中等优先级
    }
    
    thumbnailCacheSize = size;
    thumbnailCacheUsed = 0;
    thumbnailCacheMaxSize = size;
    
    Utils_Logger::info("Thumbnail cache initialized with size: %d", size);
    return true;
}

// 调整缩略图缓存大小
bool resizeThumbnailCache(uint32_t newSize) {
    if (newSize == thumbnailCacheSize) {
        return true; // 大小不变，无需调整
    }
    
    // 分配新的缓存空间
    ThumbnailCache* newCache = (ThumbnailCache*)malloc(newSize * sizeof(ThumbnailCache));
    if (!newCache) {
        Utils_Logger::error("Failed to allocate new thumbnail cache memory");
        return false;
    }
    
    // 初始化新缓存
    for (uint32_t i = 0; i < newSize; i++) {
        newCache[i].valid = false;
        newCache[i].jpegFrame = nullptr;
        newCache[i].frameSize = 0;
        newCache[i].width = 0;
        newCache[i].height = 0;
        newCache[i].lastUsed = 0;
        newCache[i].priority = 5;
    }
    
    // 复制现有有效缓存项（如果新大小大于0）
    if (newSize > 0 && thumbnailCache) {
        uint32_t copyCount = min(thumbnailCacheSize, newSize);
        for (uint32_t i = 0; i < copyCount; i++) {
            if (thumbnailCache[i].valid) {
                newCache[i] = thumbnailCache[i];
            }
        }
    }
    
    // 释放旧缓存
    if (thumbnailCache) {
        free(thumbnailCache);
    }
    
    // 更新缓存信息
    thumbnailCache = newCache;
    thumbnailCacheSize = newSize;
    thumbnailCacheMaxSize = newSize;
    
    // 重新计算已使用的缓存项数
    thumbnailCacheUsed = 0;
    for (uint32_t i = 0; i < thumbnailCacheSize; i++) {
        if (thumbnailCache[i].valid) {
            thumbnailCacheUsed++;
        }
    }
    
    Utils_Logger::info("Thumbnail cache resized to: %d", newSize);
    return true;
}

// 清理缩略图缓存
void cleanupThumbnailCache(void);

// 清理指定数量的缩略图缓存项（使用LRU策略）
void cleanupThumbnailCacheItems(uint32_t count);

// 清理超过缓存上限的项
void cleanupExcessThumbnailCache(void);

// 初始化缩略图缓存
bool initThumbnailCache(uint32_t size);

// 调整缩略图缓存大小
bool resizeThumbnailCache(uint32_t newSize);

// 生成媒体缩略图（支持图片和视频）- 性能优化版
bool generateThumbnail(const char* fileName, ThumbnailCache& cache, MediaType mediaType) {
    FIL file;
    FRESULT res = f_open(&file, fileName, FA_READ);
    if (res != FR_OK) {
        // Utils_Logger::error("Failed to open file for thumbnail: %s, error: %d", fileName, res);
        return false;
    }
    
    if (mediaType == MEDIA_TYPE_IMAGE) {
        // 图片文件：优化为只读取部分数据用于缩略图生成
        uint32_t fileSize = f_size(&file);
        
        // 限制最大读取大小为 64KB（足够生成缩略图，避免读取大文件）
        uint32_t readSize = (fileSize > 64 * 1024) ? 64 * 1024 : fileSize;
        
        uint8_t* tempBuffer = (uint8_t*)malloc(readSize);
        if (!tempBuffer) {
            // Utils_Logger::error("Failed to allocate memory for image thumbnail");
            f_close(&file);
            return false;
        }
        
        UINT bytesRead;
        res = f_read(&file, tempBuffer, readSize, &bytesRead);
        f_close(&file);
        
        if (res != FR_OK || bytesRead == 0) {
            // Utils_Logger::error("Failed to read image file for thumbnail");
            free(tempBuffer);
            return false;
        }
        
        // 分配缓存并复制数据
        cache.jpegFrame = (uint8_t*)malloc(bytesRead);
        if (!cache.jpegFrame) {
            // Utils_Logger::error("Failed to allocate memory for JPEG cache");
            free(tempBuffer);
            return false;
        }
        memcpy(cache.jpegFrame, tempBuffer, bytesRead);
        free(tempBuffer);
        
        cache.frameSize = bytesRead;
        cache.valid = true;
        
        // 解码JPEG获取尺寸信息
        if (jpeg.open((void*)cache.jpegFrame, bytesRead, nullptr, jpegReadCallback, jpegSeekCallback, JPEGDraw)) {
            cache.width = jpeg.getWidth();
            cache.height = jpeg.getHeight();
            jpeg.close();
        } else {
            // 解码失败时标记无效但不报错（可能是截断的 JPEG）
            cache.width = 0;
            cache.height = 0;
        }
        
        return true;
    } else {
        // 视频文件：查找JPEG帧 - 使用堆分配避免栈溢出
        const uint32_t SCAN_BUFFER_SIZE = 8192;  // 8KB 扫描缓冲区（栈安全）
        uint8_t* buffer = (uint8_t*)malloc(SCAN_BUFFER_SIZE);
        if (!buffer) {
            f_close(&file);
            return false;
        }
        
        UINT bytesRead;
        bool foundFrame = false;
        uint32_t frameSize = 0;
        uint32_t totalRead = 0;
        const uint32_t MAX_SCAN_SIZE = 256 * 1024;  // 最大扫描 256KB
        
        // 逐块读取文件，查找JPEG帧
        while (f_read(&file, buffer, SCAN_BUFFER_SIZE, &bytesRead) == FR_OK && bytesRead > 0) {
            totalRead += bytesRead;
            
            // 超过最大扫描大小则停止
            if (totalRead > MAX_SCAN_SIZE) {
                break;
            }
            
            // 查找JPEG帧头 (0xFF 0xD8)
            for (uint32_t j = 0; j < bytesRead - 1; j++) {
                if (buffer[j] == 0xFF && buffer[j+1] == 0xD8) {
                    // 找到JPEG帧头，开始处理
                    
                    // 分配临时缓冲区
                    uint32_t maxFrameSize = 64 * 1024;  // 最大 64KB 的 JPEG 帧
                    uint8_t* tempFrameBuffer = (uint8_t*)malloc(maxFrameSize);
                    if (!tempFrameBuffer) {
                        free(buffer);
                        f_close(&file);
                        return false;
                    }
                    
                    // 复制当前缓冲区中的部分帧数据
                    uint32_t copiedSize = bytesRead - j;
                    if (copiedSize > maxFrameSize) {
                        free(tempFrameBuffer);
                        free(buffer);
                        f_close(&file);
                        return false;
                    }
                    memcpy(tempFrameBuffer, &buffer[j], copiedSize);
                    
                    // 继续读取缓冲区，查找帧尾
                    bool foundEOI = false;
                    UINT nextBytesRead;
                    
                    while (f_read(&file, buffer, SCAN_BUFFER_SIZE, &nextBytesRead) == FR_OK && nextBytesRead > 0) {
                        totalRead += nextBytesRead;
                        
                        // 检查临时缓冲区是否足够大
                        if (copiedSize + nextBytesRead > maxFrameSize) {
                            free(tempFrameBuffer);
                            free(buffer);
                            f_close(&file);
                            return false;
                        }
                        
                        // 复制新读取的数据到临时缓冲区
                        memcpy(&tempFrameBuffer[copiedSize], buffer, nextBytesRead);
                        copiedSize += nextBytesRead;
                        
                        // 查找帧尾 (0xFF 0xD9)
                        for (uint32_t k = 0; k < copiedSize - 1; k++) {
                            if (tempFrameBuffer[k] == 0xFF && tempFrameBuffer[k+1] == 0xD9) {
                                // 计算完整的帧大小
                                frameSize = k + 2;
                                foundFrame = true;
                                foundEOI = true;
                                break;
                            }
                        }
                        
                        if (foundEOI) break;
                    }
                    
                    // 如果找到了完整的JPEG帧
                    if (foundFrame) {
                        // 分配内存保存JPEG帧数据
                        cache.jpegFrame = (uint8_t*)malloc(frameSize);
                        if (!cache.jpegFrame) {
                            // Utils_Logger::error("Failed to allocate memory for JPEG frame");
                            free(tempFrameBuffer);
                            free(buffer);
                            f_close(&file);
                            return false;
                        }
                        
                        // 复制完整的JPEG帧数据
                        memcpy(cache.jpegFrame, tempFrameBuffer, frameSize);
                        free(tempFrameBuffer);
                        
                        // 设置缓存信息
                        cache.frameSize = frameSize;
                        cache.valid = true;
                        
                        // 解码JPEG帧获取尺寸信息
                        if (jpeg.open((void*)cache.jpegFrame, frameSize, nullptr, jpegReadCallback, jpegSeekCallback, JPEGDraw)) {
                            cache.width = jpeg.getWidth();
                            cache.height = jpeg.getHeight();
                            jpeg.close();
                        } else {
                            // Utils_Logger::error("Failed to open JPEG decoder for thumbnail");
                            cache.width = 0;
                            cache.height = 0;
                        }
                        
                        free(buffer);
                        f_close(&file);
                        return true;
                    }
                    
                    // 释放临时缓冲区
                    free(tempFrameBuffer);
                    if (foundFrame) break;
                }
            }
            
            totalRead += bytesRead;
            if (foundFrame) break;
            
            // 限制读取大小，避免处理过大的文件
            if (totalRead > 10 * 1024 * 1024) { // 10MB
                break;
            }
        }
        
        free(buffer);
        f_close(&file);
        
        if (!foundFrame) {
            // Utils_Logger::error("No JPEG frame found in file: %s", fileName);
            return false;
        }
        
        return true;
    }
}

// 绘制文件列表界面
void drawFileListUI(void) {
    unsigned long currentTime = millis();
    
    // 四格布局参数
    const int CELL_WIDTH = 157; // 每个单元格宽度
    const int CELL_HEIGHT = 90; // 每个单元格高度
    
    // 硬编码的单元格位置（左上角坐标）
    const int CELL_POSITIONS[4][2] = {
        {0, 20},   // 左上角单元格
        {160, 20},  // 右上角单元格
        {0, 130},  // 左下角单元格
        {160, 130}  // 右下角单元格
    };
    
    // 硬编码的缩略图位置（左上角坐标）
    const int THUMBNAIL_POSITIONS[4][2] = {
        {0, 20},   // 左上角缩略图
        {160, 20},  // 右上角缩略图
        {0, 130},  // 左下角缩略图
        {160, 130}  // 右下角缩略图
    };
    
    // 返回按钮参数
    const int BACK_BUTTON_WIDTH = 60; // 返回按钮宽度（调整为更合适的大小）
    const int BACK_BUTTON_HEIGHT = 25; // 返回按钮高度（调整为更合适的大小）
    const int BACK_BUTTON_X = 260; // 返回按钮X坐标（右上角，调整位置）
    const int BACK_BUTTON_Y = 12; // 返回按钮Y坐标（右上角，调整位置）
    
    if (mediaFileCount == 0) {
        // 清空屏幕并显示无文件信息
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextSize(1);
        tftManager.setCursor(10, 10);
        tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
        tftManager.print("Media Files");
        tftManager.setCursor(10, 30);
        tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
        tftManager.print("No media files found");
        
        // 绘制返回按钮
        if (isBackButtonSelected) {
            // 选中状态显示红色边框
            tftManager.drawRectangle(BACK_BUTTON_X - 2, BACK_BUTTON_Y - 2, BACK_BUTTON_WIDTH + 4, BACK_BUTTON_HEIGHT + 4, ST7789_RED);
            tftManager.drawRectangle(BACK_BUTTON_X - 4, BACK_BUTTON_Y - 4, BACK_BUTTON_WIDTH + 8, BACK_BUTTON_HEIGHT + 8, ST7789_RED);
        } else {
            // 未选中状态显示白色边框
            tftManager.drawRectangle(BACK_BUTTON_X - 2, BACK_BUTTON_Y - 2, BACK_BUTTON_WIDTH + 4, BACK_BUTTON_HEIGHT + 4, ST7789_WHITE);
        }
        tftManager.setCursor(BACK_BUTTON_X + 10, BACK_BUTTON_Y + 5);
        tftManager.setTextSize(1);
        tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
        tftManager.print("Back");
    } else if (lastSelectedMediaIndex == UINT32_MAX || fileListNeedsRedraw) {
        // 首次绘制或强制重绘时，清空整个屏幕
        tftManager.fillScreen(ST7789_BLACK);
        
        // 标题使用16x16字体
        tftManager.setTextSize(1);
        tftManager.setCursor(10, 10);
        tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
        tftManager.print("Media Files");
        
        // 确保缓存大小足够
        if (mediaFileCount > thumbnailCacheSize) {
            uint32_t newSize = mediaFileCount * 1.5;
            resizeThumbnailCache(newSize);
        }
        
        // 清理过多的缓存项
        cleanupExcessThumbnailCache();
        
        // 计算当前组的起始和结束索引
        uint32_t startIndex = currentGroupIndex * MEDIA_PER_GROUP;
        uint32_t endIndex = min(startIndex + MEDIA_PER_GROUP, mediaFileCount);
        
        // 预生成当前组的缩略图，确保绘制时所有缩略图都已准备就绪
        for (uint32_t i = startIndex; i < endIndex; i++) {
            if (i < thumbnailCacheSize) {
                if (!thumbnailCache[i].valid) {
                    generateThumbnail(mediaFileList[i].fileName, thumbnailCache[i], mediaFileList[i].mediaType);
                    if (thumbnailCache[i].valid) {
                        thumbnailCacheUsed++;
                    }
                }
            }
        }
        
        // 显示媒体预览图和缩略图
        for (uint32_t i = startIndex; i < endIndex; i++) {
            // 计算在当前组中的索引（0-3）
            uint32_t groupIndex = i - startIndex;
            
            // 使用硬编码的单元格位置
            int x = CELL_POSITIONS[groupIndex][0];
            int y = CELL_POSITIONS[groupIndex][1];
            
            // 使用硬编码的缩略图位置
            int imgX = THUMBNAIL_POSITIONS[groupIndex][0];
            int imgY = THUMBNAIL_POSITIONS[groupIndex][1];
            
            // 更新最后使用时间
            if (i < thumbnailCacheSize && thumbnailCache[i].valid) {
                thumbnailCache[i].lastUsed = millis();
            } else if (i >= thumbnailCacheSize) {
                // 缓存不足，跳过生成缩略图
                Utils_Logger::info("Thumbnail cache insufficient for file index: %d", i);
            }
            
            // 绘制媒体预览图
            if (i < thumbnailCacheSize && thumbnailCache[i].valid && thumbnailCache[i].jpegFrame != nullptr) {
                // 使用缓存中的JPEG帧数据解码显示
                if (jpeg.open((void*)thumbnailCache[i].jpegFrame, thumbnailCache[i].frameSize, nullptr, jpegReadCallback, jpegSeekCallback, JPEGDraw)) {
                    // 固定缩放比例
                    int scale = JPEG_SCALE_EIGHTH;  // 使用1/8缩放
                    
                    // 解码并显示图像
                    jpeg.decode(imgX, imgY, scale);
                    jpeg.close();
                } else {
                    // 解码失败，显示"Decode Error"文本
                    tftManager.setCursor(x + 5, y + 30);
                    tftManager.setTextSize(1);
                    tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
                    tftManager.print("Decode Error");
                }
            } else {
                // 缓存无效或不足，显示"No Preview"文本
                tftManager.setCursor(x + 5, y + 30);
                tftManager.setTextSize(1);
                tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
                tftManager.print("No Preview");
            }
            
            // 显示文件名（截取部分）
            char fileNameBody[30];
            extractFileNameBody(mediaFileList[i].fileName, fileNameBody, sizeof(fileNameBody));
            // 截取文件名，避免过长（5x7字体，每字符6px宽，152px宽度可显示约25字符）
            if (strlen(fileNameBody) > 24) {
                fileNameBody[21] = '.';
                fileNameBody[22] = '.';
                fileNameBody[23] = '.';
                fileNameBody[24] = '\0';
            }
            tftManager.setCursor(x + 5, y + CELL_HEIGHT - 15);
            tftManager.setTextSize(1);
            tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
            tftManager.print(fileNameBody);
            
            // 显示媒体类型标识
            tftManager.setCursor(x + 5, y + 5);
            tftManager.setTextSize(1);
            if (mediaFileList[i].mediaType == MEDIA_TYPE_VIDEO) {
                tftManager.setTextColor(ST7789_BLUE, ST7789_BLACK);
                tftManager.print("VIDEO");
            } else {
                tftManager.setTextColor(ST7789_GREEN, ST7789_BLACK);
                tftManager.print("IMAGE");
            }
            
            // 绘制边框（在图像和文本之后绘制，确保边框显示在最前面）
            if (i == currentMediaIndex) {
                // 选中的媒体显示红色边框
                tftManager.drawRectangle(x - 2, y - 2, CELL_WIDTH + 4, CELL_HEIGHT + 4, ST7789_RED);
                tftManager.drawRectangle(x - 4, y - 4, CELL_WIDTH + 8, CELL_HEIGHT + 8, ST7789_RED);
            } else {
                // 未选中的媒体显示白色边框
                tftManager.drawRectangle(x - 2, y - 2, CELL_WIDTH + 4, CELL_HEIGHT + 4, ST7789_WHITE);
            }
        }
        
        // 预加载下一组的缩略图（性能优化：提前准备，用户切换时无需等待）
        uint32_t nextGroupStart = (currentGroupIndex + 1) * MEDIA_PER_GROUP;
        if (nextGroupStart < mediaFileCount) {
            uint32_t nextGroupEnd = min(nextGroupStart + MEDIA_PER_GROUP, mediaFileCount);
            for (uint32_t i = nextGroupStart; i < nextGroupEnd; i++) {
                if (i < thumbnailCacheSize && !thumbnailCache[i].valid) {
                    generateThumbnail(mediaFileList[i].fileName, thumbnailCache[i], mediaFileList[i].mediaType);
                    if (thumbnailCache[i].valid) {
                        thumbnailCacheUsed++;
                    }
                }
            }
        }
        
        // 绘制返回按钮
        if (isBackButtonSelected) {
            // 选中状态显示红色边框
            tftManager.drawRectangle(BACK_BUTTON_X - 2, BACK_BUTTON_Y - 2, BACK_BUTTON_WIDTH + 4, BACK_BUTTON_HEIGHT + 4, ST7789_RED);
            tftManager.drawRectangle(BACK_BUTTON_X - 4, BACK_BUTTON_Y - 4, BACK_BUTTON_WIDTH + 8, BACK_BUTTON_HEIGHT + 8, ST7789_RED);
        } else {
            // 未选中状态显示白色边框
            tftManager.drawRectangle(BACK_BUTTON_X - 2, BACK_BUTTON_Y - 2, BACK_BUTTON_WIDTH + 4, BACK_BUTTON_HEIGHT + 4, ST7789_WHITE);
        }
        tftManager.setCursor(BACK_BUTTON_X + 10, BACK_BUTTON_Y + 5);
        tftManager.setTextSize(1);
        tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
        tftManager.print("Back");
        
        // 计算总组数
        uint32_t totalGroups = (mediaFileCount + MEDIA_PER_GROUP - 1) / MEDIA_PER_GROUP;
        
        // 如果文件数量超过当前组，显示翻页提示
        if (totalGroups > 1) {
            tftManager.setCursor(10, 260);
            tftManager.setTextSize(1);
            tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
            // 使用sprintf格式化字符串
            char buffer[64];
            sprintf(buffer, "Group %lu/%lu - Rotate to navigate", currentGroupIndex + 1, totalGroups);
            tftManager.print(buffer);
        }
    } else if (lastSelectedMediaIndex != currentMediaIndex || isBackButtonSelected != lastBackButtonSelected) {
        // 计算当前组的起始索引
        uint32_t startIndex = currentGroupIndex * MEDIA_PER_GROUP;
        
        // 检查是否需要整体刷新（如果选中的媒体不在当前组中）
        if (!isBackButtonSelected && (currentMediaIndex < startIndex || currentMediaIndex >= startIndex + MEDIA_PER_GROUP)) {
            // 选中的媒体不在当前组中，需要更新组索引并整体刷新
            currentGroupIndex = currentMediaIndex / MEDIA_PER_GROUP;
            fileListNeedsRedraw = true;
            drawFileListUI();
            return;
        }
        
        // 只更新选中状态，无需整体刷新
        // 清除上一次选中的媒体边框
        if (lastSelectedMediaIndex < mediaFileCount && !lastBackButtonSelected) {
            // 检查上一次选中的媒体是否在当前组中
            if (lastSelectedMediaIndex >= startIndex && lastSelectedMediaIndex < startIndex + MEDIA_PER_GROUP) {
                uint32_t groupIndex = lastSelectedMediaIndex - startIndex;
                
                // 使用硬编码的单元格位置
                int x = CELL_POSITIONS[groupIndex][0];
                int y = CELL_POSITIONS[groupIndex][1];
                
                // 绘制白色边框
                tftManager.drawRectangle(x - 2, y - 2, CELL_WIDTH + 4, CELL_HEIGHT + 4, ST7789_WHITE);
                tftManager.drawRectangle(x - 4, y - 4, CELL_WIDTH + 8, CELL_HEIGHT + 8, ST7789_BLACK);
            }
        }
        
        // 清除返回按钮的选中状态
        if (lastBackButtonSelected) {
            tftManager.drawRectangle(BACK_BUTTON_X - 2, BACK_BUTTON_Y - 2, BACK_BUTTON_WIDTH + 4, BACK_BUTTON_HEIGHT + 4, ST7789_WHITE);
        }
        
        // 绘制返回按钮的选中状态
        if (isBackButtonSelected) {
            // 绘制红色边框
            tftManager.drawRectangle(BACK_BUTTON_X - 2, BACK_BUTTON_Y - 2, BACK_BUTTON_WIDTH + 4, BACK_BUTTON_HEIGHT + 4, ST7789_RED);
            tftManager.drawRectangle(BACK_BUTTON_X - 4, BACK_BUTTON_Y - 4, BACK_BUTTON_WIDTH + 8, BACK_BUTTON_HEIGHT + 8, ST7789_RED);
        } else if (currentMediaIndex < mediaFileCount) {
            // 绘制当前选中的媒体边框
            uint32_t groupIndex = currentMediaIndex - startIndex;
            
            // 使用硬编码的单元格位置
            int x = CELL_POSITIONS[groupIndex][0];
            int y = CELL_POSITIONS[groupIndex][1];
            
            // 绘制红色边框
            tftManager.drawRectangle(x - 2, y - 2, CELL_WIDTH + 4, CELL_HEIGHT + 4, ST7789_RED);
            tftManager.drawRectangle(x - 4, y - 4, CELL_WIDTH + 8, CELL_HEIGHT + 8, ST7789_RED);
        }
        
        // 更新最后返回按钮选中状态
        lastBackButtonSelected = isBackButtonSelected;
    }
    
    // 更新最后选中的媒体索引
    lastSelectedMediaIndex = currentMediaIndex;
    // 更新最后绘制时间
    lastDrawTime = currentTime;
    // 重置重绘标志
    fileListNeedsRedraw = false;
}

// 清理缩略图缓存
void cleanupThumbnailCache(void) {
    if (!thumbnailCache) return;
    
    for (uint32_t i = 0; i < thumbnailCacheSize; i++) {
        if (thumbnailCache[i].valid && thumbnailCache[i].jpegFrame) {
            free(thumbnailCache[i].jpegFrame);
            thumbnailCache[i].jpegFrame = nullptr;
            thumbnailCache[i].valid = false;
            thumbnailCache[i].frameSize = 0;
            thumbnailCache[i].width = 0;
            thumbnailCache[i].height = 0;
            thumbnailCache[i].lastUsed = 0;
            thumbnailCache[i].priority = 5;
        }
    }
    
    thumbnailCacheUsed = 0;
    Utils_Logger::info("Thumbnail cache cleaned up");
}

// 清理指定数量的缩略图缓存项（使用LRU策略）
void cleanupThumbnailCacheItems(uint32_t count) {
    if (!thumbnailCache || thumbnailCacheUsed <= 0) return;
    
    uint32_t cleanedCount = 0;
    
    // 找到最久未使用的缓存项
    while (cleanedCount < count && thumbnailCacheUsed > 0) {
        uint32_t lruIndex = UINT32_MAX;
        uint32_t lruTime = UINT32_MAX;
        uint32_t lowestPriority = 11; // 优先级范围0-10
        
        for (uint32_t i = 0; i < thumbnailCacheSize; i++) {
            if (thumbnailCache[i].valid) {
                // 优先清理低优先级且最久未使用的项
                if (thumbnailCache[i].priority < lowestPriority ||
                    (thumbnailCache[i].priority == lowestPriority && thumbnailCache[i].lastUsed < lruTime)) {
                    lowestPriority = thumbnailCache[i].priority;
                    lruTime = thumbnailCache[i].lastUsed;
                    lruIndex = i;
                }
            }
        }
        
        if (lruIndex != UINT32_MAX) {
            // 清理找到的缓存项
            if (thumbnailCache[lruIndex].jpegFrame) {
                free(thumbnailCache[lruIndex].jpegFrame);
                thumbnailCache[lruIndex].jpegFrame = nullptr;
            }
            thumbnailCache[lruIndex].valid = false;
            thumbnailCache[lruIndex].frameSize = 0;
            thumbnailCache[lruIndex].width = 0;
            thumbnailCache[lruIndex].height = 0;
            thumbnailCache[lruIndex].lastUsed = 0;
            thumbnailCache[lruIndex].priority = 5;
            
            thumbnailCacheUsed--;
            cleanedCount++;
            
            Utils_Logger::info("Cleaned up thumbnail cache item at index: %d, priority: %d, last used: %d", 
                             lruIndex, lowestPriority, lruTime);
        } else {
            break; // 没有更多可清理的缓存项
        }
    }
    
    Utils_Logger::info("Cleaned up %d thumbnail cache items, remaining: %d", cleanedCount, thumbnailCacheUsed);
}

// 清理超过缓存上限的项
void cleanupExcessThumbnailCache(void) {
    if (!thumbnailCache) return;
    
    uint32_t excessCount = thumbnailCacheUsed - (thumbnailCacheSize * 0.8); // 保持80%的缓存使用率
    if (excessCount > 0) {
        cleanupThumbnailCacheItems(excessCount);
    }
}

// 旋转编码器按钮回调函数（支持播放模式）
void videoHandleEncoderButton() {
    Utils_Logger::info("videoHandleEncoderButton 被调用, g_recorderState=%d", g_recorderState);
    
    switch (g_recorderState) {
        case REC_IDLE:
            // 长按进入文件列表模式
            enterFileListMode();
            break;
        case REC_RECORDING:
            stopVideoRecording();
            break;
        case REC_FILE_LIST:
            Utils_Logger::info("文件列表模式, isBackButtonSelected=%d, mediaFileCount=%u, currentMediaIndex=%u", 
                             isBackButtonSelected, mediaFileCount, currentMediaIndex);
            if (isBackButtonSelected) {
                // 选中了返回按钮，退出文件列表模式
                Utils_Logger::info("选中返回按钮，退出文件列表模式");
                exitFileListMode();
            } else {
                // 选择当前文件，根据媒体类型分别处理
                if (mediaFileCount > 0) {
                    if (mediaFileList[currentMediaIndex].mediaType == MEDIA_TYPE_VIDEO) {
                        // 视频文件，开始播放
                        Utils_Logger::info("播放视频文件: %s", mediaFileList[currentMediaIndex].fileName);
                        startVideoPlayback(mediaFileList[currentMediaIndex].fileName);
                    } else {
                        // 图片文件，查看图片
                        Utils_Logger::info("查看图片文件: %s", mediaFileList[currentMediaIndex].fileName);
                        startImageViewer(mediaFileList[currentMediaIndex].fileName);
                    }
                } else {
                    Utils_Logger::info("没有可播放的媒体文件");
                }
            }
            break;
        case REC_PLAYING:
            Utils_Logger::info("播放模式, isImageViewing=%d, isPaused=%d", isImageViewing, isPaused);
            if (isImageViewing) {
                // 图片查看模式，按下按钮返回文件列表
                Utils_Logger::info("图片查看模式，按下按钮返回文件列表");
                stopImageViewer();
            } else {
                // 视频播放模式，暂停/恢复播放
                if (isPaused) {
                    Utils_Logger::info("视频播放模式，恢复播放");
                    resumeVideoPlayback();
                } else {
                    Utils_Logger::info("视频播放模式，暂停播放");
                    pauseVideoPlayback();
                }
            }
            break;
        case REC_MENU:
            // 菜单状态由Camera.ino中的回调函数处理
            Utils_Logger::info("菜单状态，不处理");
            break;
        default:
            Utils_Logger::info("未知状态: %d", g_recorderState);
            break;
    }
}

// 旋转编码器旋转回调函数（支持文件列表导航和播放控制）
void videoHandleEncoderRotation(RotationDirection direction) {
    if (g_recorderState == REC_FILE_LIST) {
        if (isBackButtonSelected) {
            // 当前选中的是返回按钮
            if (direction == ROTATION_CW) {
                // 顺时针旋转：切换到当前组的第一个媒体
                isBackButtonSelected = false;
                currentMediaIndex = currentGroupIndex * MEDIA_PER_GROUP;
                Utils_Logger::info("Selected first media in group: %s, Group: %d", mediaFileList[currentMediaIndex].fileName, currentGroupIndex + 1);
            } else if (direction == ROTATION_CCW) {
                // 逆时针旋转：返回上一组并选择上一组的最后一个媒体作为焦点
                uint32_t totalGroups = (mediaFileCount + MEDIA_PER_GROUP - 1) / MEDIA_PER_GROUP;
                if (totalGroups > 1) {
                    // 计算上一组索引（支持循环导航）
                    currentGroupIndex = (currentGroupIndex - 1 + totalGroups) % totalGroups;
                    // 选择上一组的最后一个媒体
                    uint32_t groupStart = currentGroupIndex * MEDIA_PER_GROUP;
                    uint32_t groupEnd = min(groupStart + MEDIA_PER_GROUP, mediaFileCount);
                    currentMediaIndex = groupEnd - 1;
                    isBackButtonSelected = false;
                    fileListNeedsRedraw = true;
                    Utils_Logger::info("Navigated to previous group: %d, selected last media: %s", currentGroupIndex + 1, mediaFileList[currentMediaIndex].fileName);
                }
            }
        } else {
            // 当前选中的是媒体
            uint32_t currentGroupStart = currentGroupIndex * MEDIA_PER_GROUP;
            uint32_t currentGroupEnd = min(currentGroupStart + MEDIA_PER_GROUP, mediaFileCount);
            
            if (direction == ROTATION_CW) {
                // 顺时针旋转
                if (currentMediaIndex < currentGroupEnd - 1) {
                    // 不是组的最后一个媒体，选择下一个媒体
                    currentMediaIndex++;
                    Utils_Logger::info("Selected next media: %s", mediaFileList[currentMediaIndex].fileName);
                } else {
                    // 是组的最后一个媒体，进入下一组并保持焦点在返回按钮上
                    uint32_t totalGroups = (mediaFileCount + MEDIA_PER_GROUP - 1) / MEDIA_PER_GROUP;
                    if (totalGroups > 1) {
                        currentGroupIndex = (currentGroupIndex + 1) % totalGroups;
                        // 更新currentMediaIndex为新组的第一个媒体，避免组索引重置
                        currentMediaIndex = currentGroupIndex * MEDIA_PER_GROUP;
                        isBackButtonSelected = false;
                        fileListNeedsRedraw = true;
                        Utils_Logger::info("Navigated to next group: %d, selected first media: %s", currentGroupIndex + 1, mediaFileList[currentMediaIndex].fileName);
                    } else {
                        // 只有一组，循环到返回按钮
                        isBackButtonSelected = true;
                        Utils_Logger::info("Selected back button");
                    }
                }
            } else if (direction == ROTATION_CCW) {
                // 逆时针旋转
                if (currentMediaIndex > currentGroupStart) {
                    // 不是组的第一个媒体，选择上一个媒体
                    currentMediaIndex--;
                    Utils_Logger::info("Selected previous media: %s", mediaFileList[currentMediaIndex].fileName);
                } else {
                    // 是组的第一个媒体，切换到返回按钮
                    isBackButtonSelected = true;
                    Utils_Logger::info("Selected back button");
                }
            }
        }
    } else if (g_recorderState == REC_PLAYING) {
        if (isImageViewing) {
            // 在图片查看过程中检测到旋钮旋转，返回文件列表
            stopImageViewer();
        } else {
            // 在视频回放过程中检测到旋钮旋转，停止播放并返回文件选择界面
            stopVideoPlayback();
        }
        // 返回到文件列表状态
        g_recorderState = REC_FILE_LIST;
        // 确保返回按钮未被选中
        isBackButtonSelected = false;
        // 通知Camera.ino需要重绘文件列表
        extern bool fileListNeedsRedraw;
        fileListNeedsRedraw = true;
        Utils_Logger::info("Stopped media playback due to encoder rotation, returning to file list");
    }
}


