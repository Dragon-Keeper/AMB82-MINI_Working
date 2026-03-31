#include "RTOS_TaskFactory.h"
#include "Utils_Logger.h"
#include "Camera_ImageConfig.h"
#include "Menu_TriangleController.h"
#include "Menu_MenuContext.h"
#include "Menu_MenuManager.h"
#include "Display_TFTManager.h"
#include "Display_FontRenderer.h"
#include "Camera_CameraManager.h"
#include "System_StateManager.h"
#include "Encoder_Control.h"
#include "Camera_SDCardManager.h"
#include "TJpg_Decoder.h"
#include "Inmp441_MicrophoneManager.h"
#include "VideoRecorder.h"
#include "MJPEG_Encoder.h"

// 外部全局对象声明
extern Display_TFTManager tftManager;
extern MenuManager menuManager;
extern CameraManager cameraManager;
extern MenuContext menuContext;
extern EncoderControl encoder;
extern SDCardManager sdCardManager;
extern Display_FontRenderer fontRenderer;
extern JPEGDEC jpeg;
extern Inmp441MicrophoneManager g_microphoneManager;

// JPEG解码回调函数
bool jpegDrawCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
    // 将解码后的像素数据绘制到屏幕上
    tftManager.drawBitmap(x, y, w, h, bitmap);
    return 1; // 返回1表示继续解码
}

// 系统状态通过StateManager管理，不再使用全局变量



// 任务函数实现

// 拍照功能：编码器旋转处理函数
void capturePhotoHandleEncoderRotation(RotationDirection direction) {
    // 拍照功能中旋转编码器用于返回主菜单
    Utils_Logger::info("拍照模式：检测到编码器旋转，返回主菜单");
    TaskManager::setEvent(EVENT_RETURN_TO_MENU);
}

// 拍照功能：编码器按钮处理函数
void capturePhotoHandleEncoderButton() {
    // 拍照功能中按钮按下用于拍照
    Utils_Logger::info("拍照模式：按钮按下，开始拍照");
    if (!cameraManager.isCapturing()) {
        cameraManager.requestCapture();
    }
}

/**
 * 相机预览任务 (位置A)
 */
void taskCameraPreview(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 0;
    
    Utils_Logger::info("相机预览任务 %c 启动", (char)('A' + taskId));
    
    // 设置系统状态为相机预览模式
    StateManager::getInstance().setCurrentState(STATE_CAMERA_PREVIEW);
    
    // 任务初始化 - 使用CameraManager
    // 检查相机管理器是否已初始化，如果没有则重新初始化
    if (!cameraManager.isInitialized()) {
        Utils_Logger::info("相机管理器未初始化，正在重新初始化...");
        if (!cameraManager.init(tftManager, fontRenderer, jpeg)) {
            Utils_Logger::error("相机管理器重新初始化失败");
            // 清理任务参数
            if (params != NULL) {
                delete params;
            }
            // 删除任务
            vTaskDelete(NULL);
            return;
        }
    }
    
    // 设置编码器回调函数
    encoder.setRotationCallback(capturePhotoHandleEncoderRotation);
    encoder.setButtonCallback(capturePhotoHandleEncoderButton);
    
    cameraManager.startPreview();
    
    Utils_Logger::info("拍照功能初始化完成，等待用户操作");
    
    // 任务主循环
    while (1) {
        // 处理相机预览 - 使用CameraManager
        cameraManager.processPreviewFrame();
        
        // 检查是否有拍照请求
        if (cameraManager.hasCaptureRequest() && !cameraManager.isCapturing()) {
            // 只在空闲时处理拍照请求，减少对预览的影响
            Utils_Logger::info("Capture request detected, starting capture...");
            cameraManager.clearCaptureRequest();
            cameraManager.capturePhoto();
        }
        
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            10 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            Utils_Logger::info("相机预览任务 %c 收到退出信号", (char)('A' + taskId));
            break;
        }
        
        // 检查编码器状态
        encoder.checkRotation();
        encoder.checkButton();
    }
    
    // 任务清理 - 使用CameraManager
    cameraManager.stopPreview();
    
    // 首先清除退出事件标志
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 重置编码器回调函数为主菜单回调
    extern void handleEncoderRotation(RotationDirection direction);
    extern void handleEncoderButton();
    encoder.setRotationCallback(handleEncoderRotation);
    encoder.setButtonCallback(handleEncoderButton);
    Utils_Logger::info("已重置编码器回调函数为主菜单回调");
    
    // 切换回主菜单并强制重绘整个界面
    menuContext.switchToMainMenu();
    menuContext.showMenu();
    
    // 立即更新系统状态为STATE_MAIN_MENU
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_CAMERA_PREVIEW);
    
    Utils_Logger::info("相机预览任务 %c 退出", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

// 拍视频功能：编码器旋转处理函数
void captureVideoHandleEncoderRotation(RotationDirection direction) {
    // 拍视频功能中旋转编码器用于返回主菜单
    Utils_Logger::info("拍视频模式：检测到编码器旋转，返回主菜单");
    TaskManager::setEvent(EVENT_RETURN_TO_MENU);
}

// 拍视频功能：编码器按钮处理函数
void captureVideoHandleEncoderButton() {
    // 根据当前录制状态切换视频录制
    if (g_recorderState == REC_IDLE) {
        // 空闲状态：启动视频录制
        Utils_Logger::info("拍视频模式：按钮按下，开始录制");
        startVideoRecording();
    } else if (g_recorderState == REC_RECORDING) {
        // 录制状态：停止视频录制
        Utils_Logger::info("拍视频模式：按钮按下，停止录制");
        stopVideoRecording();
    }
}

/**
 * 功能模块B任务 (位置B) - 拍视频功能
 */
void taskFunctionB(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 1;
    
    Utils_Logger::info("功能模块B任务 %c 启动 - 拍视频功能", (char)('A' + taskId));
    
    // 设置系统状态为相机预览模式
    StateManager::getInstance().setCurrentState(STATE_CAMERA_PREVIEW);
    
    // 初始化视频录制器
    Utils_Logger::info("初始化视频录制器...");
    videoRecorderInit();
    
    // 设置编码器回调函数
    encoder.setRotationCallback(captureVideoHandleEncoderRotation);
    encoder.setButtonCallback(captureVideoHandleEncoderButton);
    
    // 进入空闲状态，直接显示预览画面（移除文字提示页面）
    g_recorderState = REC_IDLE;
    
    Utils_Logger::info("拍视频功能初始化完成，等待用户操作");
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            10 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            Utils_Logger::info("拍视频任务收到退出信号");
            break;
        }
        
        // 根据当前状态执行不同的处理
        switch (g_recorderState) {
            case REC_IDLE:
                // 空闲状态：只处理预览帧
                processPreviewFrame();
                break;
            case REC_RECORDING:
                // 录制状态：运行视频录制循环
                videoRecorderLoop();
                // 同时处理预览帧（用于LCD显示）
                processPreviewFrame();
                break;
            default:
                break;
        }
        
        // 优先运行麦克风模块循环（确保音频数据不丢失）
        g_microphoneManager.loop();
        g_microphoneManager.loop();
        
        // 检查编码器状态
        encoder.checkRotation();
        encoder.checkButton();
    }
    
    // 任务清理
    Utils_Logger::info("拍视频功能清理中...");
    
    // 如果正在录制，先停止录制
    if (g_recorderState == REC_RECORDING) {
        stopVideoRecording();
    }
    
    // 清理视频录制器资源（关键修复！）
    videoRecorderCleanup();
    
    // 首先清除退出事件标志
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 重置编码器回调函数为主菜单回调（关键修复！）
    extern void handleEncoderRotation(RotationDirection direction);
    extern void handleEncoderButton();
    encoder.setRotationCallback(handleEncoderRotation);
    encoder.setButtonCallback(handleEncoderButton);
    Utils_Logger::info("已重置编码器回调函数为主菜单回调");
    
    // 切换回主菜单并强制重绘整个界面
    menuContext.switchToMainMenu();
    menuContext.showMenu(); // 关键修复：强制重绘主菜单和三角形，确保界面正确显示
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_B);
    
    Utils_Logger::info("功能模块B任务 %c 退出 - 拍视频功能", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块C任务 (位置C) - 照片、视频回放功能
 */
void taskFunctionC(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 2;
    
    Utils_Logger::info("功能模块C任务 %c 启动 - 照片、视频回放功能", (char)('A' + taskId));
    
    // 设置系统状态为相机预览模式
    StateManager::getInstance().setCurrentState(STATE_CAMERA_PREVIEW);
    
    // 初始化视频录制器（包含回放功能）
    Utils_Logger::info("初始化视频录制器...");
    videoRecorderInit();
    
    // 设置编码器回调函数，用于回放功能的交互（在videoRecorderInit之后设置，避免被覆盖）
    encoder.setRotationCallback(videoHandleEncoderRotation);
    encoder.setButtonCallback(videoHandleEncoderButton);
    
    // 进入文件列表模式
    enterFileListMode();
    
    Utils_Logger::info("照片、视频回放功能初始化完成，等待用户操作");
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            10 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            Utils_Logger::info("照片、视频回放任务收到退出信号");
            break;
        }
        
        // 根据当前状态执行不同的处理
        switch (g_recorderState) {
            case REC_FILE_LIST:
                // 总是调用drawFileListUI()函数，这样选中状态的更新就能被正确处理
                drawFileListUI();
                if (fileListNeedsRedraw) {
                    fileListNeedsRedraw = false;
                }
                break;
            case REC_PLAYING:
                // 运行视频播放或图片查看循环
                if (isImageViewing) {
                    imageViewerLoop();
                } else {
                    videoPlaybackLoop();
                }
                break;
            default:
                break;
        }
        
        // 检查编码器状态
        encoder.checkRotation();
        encoder.checkButton();
    }
    
    // 任务清理
    Utils_Logger::info("照片、视频回放功能清理中...");
    
    // 如果正在播放，先停止播放
    if (g_recorderState == REC_PLAYING) {
        if (isImageViewing) {
            stopImageViewer();
        } else {
            stopVideoPlayback();
        }
    }
    
    // 清理视频录制器资源
    videoRecorderCleanup();
    
    // 首先清除退出事件标志
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 重置编码器回调函数为主菜单回调（关键修复！）
    extern void handleEncoderRotation(RotationDirection direction);
    extern void handleEncoderButton();
    encoder.setRotationCallback(handleEncoderRotation);
    encoder.setButtonCallback(handleEncoderButton);
    Utils_Logger::info("已重置编码器回调函数为主菜单回调");
    
    // 切换回主菜单并强制重绘整个界面
    menuContext.switchToMainMenu();
    menuContext.showMenu(); // 关键修复：强制重绘主菜单和三角形，确保界面正确显示
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_C);
    
    Utils_Logger::info("功能模块C任务 %c 退出 - 照片、视频回放功能", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块D任务 (位置D)
 */
void taskFunctionD(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 3;
    
    Utils_Logger::info("功能模块D任务 %c 启动", (char)('A' + taskId));
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 任务功能实现
        // TODO: 实现功能模块D的具体逻辑
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_D);
    
    Utils_Logger::info("功能模块D任务 %c 退出", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块E任务 (位置E)
 */
void taskFunctionE(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 4;
    
    Utils_Logger::info("功能模块E任务 %c 启动", (char)('A' + taskId));
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 任务功能实现
        // TODO: 实现功能模块E的具体逻辑
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_E);
    
    Utils_Logger::info("功能模块E任务 %c 退出", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 系统设置任务 (位置F)
 */
void taskSystemSettings(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 5;
    
    Utils_Logger::info("系统设置任务 %c 启动", (char)('A' + taskId));
    
    // 任务初始化
    menuContext.switchToSubMenu();
    
    // 任务主循环
    while (1) {
        // 处理子菜单
        menuContext.handleSubMenu();
        
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            Utils_Logger::info("系统设置任务 %c 收到退出信号", (char)('A' + taskId));
            break;
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
    // 任务清理
    menuContext.switchToMainMenu();
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_SYSTEM_SETTINGS);
    
    Utils_Logger::info("系统设置任务 %c 退出", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

bool TaskFactory::init() {
    Utils_Logger::info("TaskFactory initialized successfully");
    return true;
}

bool TaskFactory::registerTask(TaskManager::TaskID id, const char* name, TaskFunction_t func, 
                             uint32_t stackSize, UBaseType_t priority) {
    if (!isValidTaskID(id)) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return false;
    }
    
    if (name == NULL || func == NULL) {
        Utils_Logger::error("Invalid task parameters");
        return false;
    }
    
    // 获取任务信息指针
    TaskManager::TaskInfo* taskInfo = const_cast<TaskManager::TaskInfo*>(TaskManager::getTaskInfo(id));
    if (taskInfo == NULL) {
        Utils_Logger::error("Failed to get task info for ID: %d", id);
        return false;
    }
    
    // 更新任务信息
    taskInfo->name = name;
    taskInfo->function = func;
    taskInfo->stackSize = stackSize;
    taskInfo->priority = priority;
    
    Utils_Logger::info("Registered task %s (ID: %d) with stack size %lu, priority %lu", 
                      name, id, stackSize, priority);
    
    return true;
}

TaskHandle_t TaskFactory::createTask(TaskManager::TaskID id, void* params) {
    if (!isValidTaskID(id)) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return NULL;
    }
    
    // 使用TaskManager的公共接口设置任务参数
    TaskManager::setTaskParams(id, params);
    
    // 使用TaskManager创建任务
    TaskHandle_t handle = TaskManager::createTask(id);
    
    if (handle == NULL) {
        Utils_Logger::error("Failed to create task with ID: %d", id);
        TaskManager::setTaskParams(id, NULL); // 清理参数
    } else {
        Utils_Logger::info("Created task with ID: %d, handle: %p", id, handle);
    }
    
    return handle;
}

TaskHandle_t TaskFactory::createDefaultTask(TaskManager::TaskID id) {
    // 使用默认参数创建任务
    return createTask(id, NULL);
}

TaskHandle_t TaskFactory::createTaskWithParams(TaskManager::TaskID id, const TaskParams& params) {
    // 创建参数副本
    TaskParams* taskParamsCopy = new TaskParams(params);
    if (taskParamsCopy == NULL) {
        Utils_Logger::error("Failed to allocate memory for task parameters");
        return NULL;
    }
    
    // 创建任务
    TaskHandle_t handle = createTask(id, taskParamsCopy);
    
    if (handle == NULL) {
        delete taskParamsCopy; // 清理参数
    }
    
    return handle;
}

bool TaskFactory::unregisterTask(TaskManager::TaskID id) {
    if (!isValidTaskID(id)) {
        Utils_Logger::error("Invalid task ID: %d", id);
        return false;
    }
    
    // 获取任务信息指针
    TaskManager::TaskInfo* taskInfo = const_cast<TaskManager::TaskInfo*>(TaskManager::getTaskInfo(id));
    if (taskInfo == NULL) {
        Utils_Logger::error("Failed to get task info for ID: %d", id);
        return false;
    }
    
    // 清理任务参数
    void* params = TaskManager::getTaskParams(id);
    if (params != NULL) {
        // 将void*转换为TaskParams*后再删除
        delete static_cast<TaskFactory::TaskParams*>(params);
        TaskManager::setTaskParams(id, NULL);
    }
    
    // 重置任务信息
    taskInfo->function = NULL;
    
    Utils_Logger::info("Unregistered task with ID: %d", id);
    
    return true;
}

void TaskFactory::cleanup() {
    // 任务参数由TaskManager统一管理和清理
    Utils_Logger::info("TaskFactory cleaned up successfully");
}

/**
 * 音频处理任务 (TASK_AUDIO_PROCESSING)
 * 优先级: 4 (高于系统设置任务，确保不被视频任务抢占)
 * 功能: 从环形缓冲区读取音频数据，处理后放入队列供videoRecorderLoop使用
 */
void taskAudioProcessing(void* params) {
    TaskFactory::TaskParams* taskParams = static_cast<TaskFactory::TaskParams*>(params);
    uint32_t taskId = (taskParams != NULL) ? taskParams->param1 : 0;
    
    Utils_Logger::info("音频处理任务 %c 启动 - 优先级: %d", 
        (char)('A' + taskId), uxTaskPriorityGet(NULL));
    
    // 初始化音频队列
    if (!g_microphoneManager.initAudioQueue()) {
        Utils_Logger::error("音频队列初始化失败，任务退出");
        if (taskParams != NULL) {
            delete taskParams;
        }
        vTaskDelete(NULL);
        return;
    }
    
    AudioDataBlock audioBlock;
    uint32_t blockCounter = 0;
    size_t accumulatedSamples = 0;
    
    while (1) {
        // 只有在录制状态下才处理音频数据
        if (g_recorderState != REC_RECORDING) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }
        
        // 从环形缓冲区读取音频数据，累积到512样本
        if (accumulatedSamples == 0) {
            audioBlock.timestamp = millis();
        }
        
        size_t samplesToRead = 512 - accumulatedSamples;
        size_t samplesRead = g_microphoneManager.readAudioSamples(
            audioBlock.samples + accumulatedSamples, samplesToRead);
        
        if (samplesRead > 0) {
            accumulatedSamples += samplesRead;
            
            // 当累积满512个样本时，发送到队列
            if (accumulatedSamples >= 512) {
                audioBlock.count = accumulatedSamples;
                
                // 发送到音频队列
                if (g_microphoneManager.sendAudioDataBlock(&audioBlock, 10 / portTICK_PERIOD_MS)) {
                    blockCounter++;
                    if (blockCounter % 10 == 0) {
                        size_t queueAvailable = g_microphoneManager.getAudioQueueAvailable();
                        size_t queueFree = g_microphoneManager.getAudioQueueFree();
                        Utils_Logger::debug("音频队列状态: 已用=%d, 空闲=%d, 块计数=%d", 
                            queueAvailable, queueFree, blockCounter);
                    }
                } else {
                    Utils_Logger::info("音频队列满，丢弃音频块");
                }
                
                // 重置累积计数
                accumulatedSamples = 0;
            }
        }
        
        // 短暂延时，让出CPU
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    
    // 清理任务参数
    if (taskParams != NULL) {
        delete taskParams;
    }
    
    Utils_Logger::info("音频处理任务 %c 退出", (char)('A' + taskId));
    
    // 删除任务
    vTaskDelete(NULL);
}

void taskVideoFrameCapture(void* params) {
    TaskFactory::TaskParams* taskParams = static_cast<TaskFactory::TaskParams*>(params);
    uint32_t taskId = (taskParams != NULL) ? taskParams->param1 : 0;
    
    Utils_Logger::info("视频帧获取任务 %c 启动 - 优先级: %d", 
        (char)('A' + taskId), uxTaskPriorityGet(NULL));
    
    extern MJPEGEncoder mjpegEncoder;
    const uint32_t VIDEO_CHANNEL_RECORD = 0;
    const unsigned long FRAME_INTERVAL = 67;
    unsigned long lastFrameTime = 0;
    uint32_t frameCount = 0;
    
    while (1) {
        if (g_recorderState != REC_RECORDING) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            lastFrameTime = 0;
            frameCount = 0;
            continue;
        }
        
        unsigned long currentTime = millis();
        
        if (currentTime - lastFrameTime >= FRAME_INTERVAL) {
            uint32_t imgAddr;
            uint32_t imgLen;
            
            Camera.getImage(VIDEO_CHANNEL_RECORD, &imgAddr, &imgLen);
            
            lastFrameTime = currentTime;
            
            if (imgLen > 0) {
                mjpegEncoder.addVideoFrame((uint8_t*)imgAddr, imgLen, currentTime);
                frameCount++;
                
                if (frameCount % 15 == 0) {
                    Utils_Logger::info("视频帧获取: %d 帧", frameCount);
                }
            }
        }
        
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    
    if (taskParams != NULL) {
        delete taskParams;
    }
    
    Utils_Logger::info("视频帧获取任务 %c 退出", (char)('A' + taskId));
    vTaskDelete(NULL);
}

bool TaskFactory::isValidTaskID(TaskManager::TaskID id) {
    return (id >= 0 && id < TaskManager::TASK_MAX);
}