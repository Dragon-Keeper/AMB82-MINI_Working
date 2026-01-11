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

// 外部全局对象声明
extern Display_TFTManager tftManager;
extern MenuManager menuManager;
extern CameraManager cameraManager;
extern MenuContext menuContext;
extern EncoderControl encoder;
extern SDCardManager sdCardManager;
extern Display_FontRenderer fontRenderer;
extern JPEGDEC jpeg;

// JPEG解码回调函数
bool jpegDrawCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
    // 将解码后的像素数据绘制到屏幕上
    tftManager.drawBitmap(x, y, w, h, bitmap);
    return 1; // 返回1表示继续解码
}

// 系统状态通过StateManager管理，不再使用全局变量



// 任务函数实现

/**
 * 相机预览任务 (位置A)
 */
void taskCameraPreview(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 0;
    
    Utils_Logger::info("相机预览任务 %c 启动", (char)('A' + taskId));
    
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
    cameraManager.startPreview();
    
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
            1 / portTICK_PERIOD_MS  // 减少超时时间，提高响应速度
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            Utils_Logger::info("相机预览任务 %c 收到退出信号", (char)('A' + taskId));
            break;
        }
        
        // 移除不必要的延迟，最大化帧率
    }
    
    // 任务清理 - 使用CameraManager
    cameraManager.stopPreview();
    
    // 立即更新系统状态为STATE_MAIN_MENU，防止编码器事件继续被相机预览模式处理
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 切换回主菜单（在清理资源之前，确保菜单能正常显示）
    menuContext.switchToMainMenu();
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_CAMERA_PREVIEW);
    
    Utils_Logger::info("相机预览任务 %c 退出", (char)('A' + taskId));
    
    // 清理相机资源
    cameraManager.cleanup();
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块B任务 (位置B)
 */
void taskFunctionB(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 1;
    
    Utils_Logger::info("功能模块B任务 %c 启动", (char)('A' + taskId));
    
    // 任务主循环
    while (1) {
        // 检查是否需要退出任务
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            100 / portTICK_PERIOD_MS
        );
        
        // 不再需要直接检查编码器，因为系统状态设置为STATE_CAMERA_PREVIEW后，
        // 编码器旋转事件会通过handleEncoderRotation函数正确处理
        // 移除直接检测避免按钮按下时的微小抖动被误判为旋转事件
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 任务功能实现
        // TODO: 实现功能模块B的具体逻辑
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_B);
    
    Utils_Logger::info("功能模块B任务 %c 退出", (char)('A' + taskId));
    
    // 清理任务参数
    if (params != NULL) {
        delete params;
    }
    
    // 删除任务
    vTaskDelete(NULL);
}

/**
 * 功能模块C任务 (位置C) - 图片回放功能
 */
void taskFunctionC(void* pvParameters) {
    // 解析任务参数
    TaskFactory::TaskParams* params = static_cast<TaskFactory::TaskParams*>(pvParameters);
    uint32_t taskId = (params != NULL) ? params->param1 : 2;
    
    Utils_Logger::info("功能模块C任务 %c 启动 - 图片回放功能", (char)('A' + taskId));
    
    // 设置系统状态为相机预览模式，以便编码器旋转事件能正确触发返回主菜单
    StateManager::getInstance().setCurrentState(STATE_CAMERA_PREVIEW);
    
    // 初始化JPEG解码器
    TJpgDec.setCallback(jpegDrawCallback);
    TJpgDec.setJpgScale(4); // 设置缩放比例为4倍，适合将720P缩放到320*180，再通过drawJpg自适应屏幕
    
    // 设置TFT颜色顺序为RGB，与JPEG解码器输出匹配
    tftManager.setColorOrder(true);
    
    // 初始化SD卡文件系统
    AmebaFatFS fs;
    if (!fs.begin()) {
        Utils_Logger::error("无法找到文件系统，正在尝试重新初始化SD卡");
        
        // 清除屏幕并显示错误信息
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setCursor(0, 0);
        // SD卡错误
        const uint8_t errorStr1[] = {0x0A, 0x1C, 0x17, 0x0D, 0x1E, 0x14, 0};
        // 无法找到文件系统
        const uint8_t errorStr2[] = {0x16, 0x1B, 0x17, 0x0F, 0x12, 0x0A, 0x1C, 0x17, 0x1A, 0x10, 0};
        fontRenderer.drawChineseString(10, 10, errorStr1, ST7789_RED, ST7789_BLACK);
        fontRenderer.drawChineseString(10, 40, errorStr2, ST7789_RED, ST7789_BLACK);
        
        // 等待一段时间后退出任务
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        
        // 更新系统状态返回到主菜单
        StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
        
        // 清除所有事件标志，确保系统状态干净
        TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
        
        // 更新任务状态信息
        TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_C);
        
        Utils_Logger::info("功能模块C任务 %c 退出 - 图片回放功能", (char)('A' + taskId));
        
        // 清理任务参数
        if (params != NULL) {
            delete params;
        }
        
        // 删除任务
        vTaskDelete(NULL);
        return;
    }
    
    Utils_Logger::info("SD卡文件系统初始化成功");
    
    // 获取根路径
    String rootPath = String(fs.getRootPath());
    Utils_Logger::info("根路径: %s，长度: %d", rootPath.c_str(), rootPath.length());
    
    // 初始化图片回放所需参数
    uint32_t currentImageIndex = 0;
    // MAX_PHOTOS 定义的照片列表占用的是开发板的RAM内存,对于1000张照片，大约需要：1000 × (32 + 24) = 56,000字节（约54.7KB）
    const uint32_t MAX_PHOTOS = 1000; // 最大照片数量
    String* photoFiles = new String[MAX_PHOTOS]; // 存储照片文件列表（堆空间分配）
    uint32_t photoCount = 0; // 实际照片数量
    
    // 遍历SD卡目录，收集所有照片文件
    // 增加目录缓冲区大小以容纳更多文件（约8KB，足够存储1000个短文件名）
    char dirBuffer[8192]; // 用于存储目录内容的缓冲区
    
    // 记录内存使用信息
    Utils_Logger::info("当前堆空间分配: %d 字节用于照片文件列表", MAX_PHOTOS * sizeof(String));
    Utils_Logger::info("目录缓冲区大小: %d 字节", sizeof(dirBuffer));
    
    // 清空缓冲区，确保起始状态干净
    memset(dirBuffer, 0, sizeof(dirBuffer));
    
    // 打印rootPath信息
    Utils_Logger::info("rootPath: %s", rootPath.c_str());
    
    // 测试直接使用"0:/"作为根路径
    String testPath = "0:/";
    int fileCount = fs.readDir(const_cast<char*>(testPath.c_str()), dirBuffer, sizeof(dirBuffer));
    Utils_Logger::info("使用 testPath '%s' 调用 readDir 返回值: %d", testPath.c_str(), fileCount);
    
    // dirBuffer内容已确认包含文件信息，跳过详细日志
    
    // 不再依赖fileCount返回值，直接检查dirBuffer是否包含有效内容
    // 但要跳过前16个字符，因为它们可能包含系统卷信息
    bool hasValidDirContent = false;
    for (size_t i = 16; i < sizeof(dirBuffer); i++) {
        if (dirBuffer[i] != 0) {
            hasValidDirContent = true;
            break;
        }
    }
    
    if (hasValidDirContent) {
        Utils_Logger::info("找到 %d 个文件", fileCount);
        
        // 解析目录内容，筛选出所有.jpg后缀的文件
        char* p = dirBuffer;
        uint32_t totalFiles = 0;
        
        // 跳过系统卷信息目录
        if (strncmp(p, "System Volume Information", 24) == 0) {
            p += strlen(p) + 1;
            Utils_Logger::info("跳过系统卷信息目录");
        }
        
        while (strlen(p) > 0) {
            totalFiles++;
            String filename = String(p);
            Utils_Logger::info("找到文件: %s", filename.c_str());
            
            // 直接检查文件名是否以.jpg结尾
            if (filename.endsWith(".jpg")) {
                if (photoCount < MAX_PHOTOS) {
                    photoFiles[photoCount++] = filename;
                    Utils_Logger::info("添加照片文件: %s", filename.c_str());
                } else {
                    Utils_Logger::info("照片文件数量已达到最大值 %u", MAX_PHOTOS);
                }
            }
            
            // 移动到下一个文件名（跳过当前文件名和空字符）
            p += strlen(p) + 1;
        }
        
        Utils_Logger::info("总共处理了 %u 个文件，其中 %u 个是照片文件", totalFiles, photoCount);
        
        // 按文件名（时间戳）对照片文件进行排序 - 从最新拍摄到最早拍摄
        if (photoCount > 1) {
            for (uint32_t i = 0; i < photoCount - 1; i++) {
                for (uint32_t j = 0; j < photoCount - i - 1; j++) {
                    if (photoFiles[j] < photoFiles[j + 1]) {
                        // 交换文件
                        String temp = photoFiles[j];
                        photoFiles[j] = photoFiles[j + 1];
                        photoFiles[j + 1] = temp;
                    }
                }
            }
        }
    }
    
    // 检查是否有照片文件
    bool hasPhotos = photoCount > 0;
    
    if (!hasPhotos) {
        Utils_Logger::info("SD卡中没有找到 .jpg 格式的照片");
        
        // 显示提示信息
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setCursor(0, 0);
        // 无图片文件
        const uint8_t noImgStr1[] = {0x18, 0x13, 0x0F, 0x0C, 0x1E, 0x14, 0};
        // SD卡中没有照片
        const uint8_t noImgStr2[] = {0x0A, 0x1C, 0x17, 0x0D, 0x1E, 0x14, 0x16, 0x1B, 0x0F, 0x12, 0x1A, 0x10, 0};
        fontRenderer.drawChineseString(10, 10, noImgStr1, ST7789_RED, ST7789_BLACK);
        fontRenderer.drawChineseString(10, 40, noImgStr2, ST7789_RED, ST7789_BLACK);
    } else {
        // 显示第一张照片
        String firstFilename = photoFiles[0];
        String firstFullPath = rootPath + firstFilename;
        
        File file = fs.open(firstFullPath);
        if (file && file.isOpen()) {
            // 使用File类的readFile方法自动分配内存并读取文件
            unsigned char *file_data;
            uint32_t file_size;
            
            if (file.readFile(file_data, file_size)) {
                Utils_Logger::info("文件读取成功，大小: %d 字节", file_size);
                
                // 清除屏幕
                tftManager.fillScreen(ST7789_BLACK);
                tftManager.setCursor(0, 0);
                
                // 解码并显示JPG图片
                // 注意：getJpgSize的前两个参数是0, 0而不是用于存储宽度和高度的指针
                TJpgDec.getJpgSize(0, 0, (const uint8_t *)file_data, file_size);
                
                // 显示JPG图片，忽略返回值因为实际显示成功但函数可能返回false
                TJpgDec.drawJpg(0, 30, (const uint8_t *)file_data, file_size);
                Utils_Logger::info("正在显示: %s", firstFilename.c_str());
                
                // 释放内存
                free(file_data);
            } else {
                Utils_Logger::error("读取文件失败: %s", firstFilename.c_str());
            }
            // 关闭文件
            file.close();
        }
    }

    // 任务主循环
    while (1) {
        // 检查是否需要退出任务或切换照片
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU | EVENT_NEXT_PHOTO | EVENT_PREVIOUS_PHOTO,
            true,
            100 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            break;
        }
        
        // 处理照片切换事件
        bool needToShowPhoto = false;
        
        // 处理下一张照片事件
        if ((uxBits & EVENT_NEXT_PHOTO) != 0) {
            currentImageIndex++;
            if (currentImageIndex >= photoCount) {
                currentImageIndex = 0; // 循环到第一张
            }
            needToShowPhoto = true;
            Utils_Logger::info("切换到下一张照片，索引: %d", currentImageIndex);
        }
        
        // 处理上一张照片事件
        if ((uxBits & EVENT_PREVIOUS_PHOTO) != 0) {
            if (currentImageIndex == 0) {
                currentImageIndex = photoCount - 1; // 循环到最后一张
            } else {
                currentImageIndex--;
            }
            needToShowPhoto = true;
            Utils_Logger::info("切换到上一张照片，索引: %d", currentImageIndex);
        }
        
        // 如果需要显示新照片，并且有照片可显示
        if (needToShowPhoto && photoCount > 0) {
            String currentFilename = photoFiles[currentImageIndex];
            String fullPath = rootPath + currentFilename;
            
            Utils_Logger::info("尝试打开文件: %s", fullPath.c_str());
            
            if (fs.exists(fullPath)) {
                // 显示当前照片
                File file = fs.open(fullPath);
                if (file && file.isOpen()) {
                    // 使用File类的readFile方法自动分配内存并读取文件
                    unsigned char *file_data;
                    uint32_t file_size;
                    
                    if (file.readFile(file_data, file_size)) {
                        Utils_Logger::info("文件读取成功，大小: %d 字节", file_size);
                        
                        // 清除屏幕
                        tftManager.fillScreen(ST7789_BLACK);
                        tftManager.setCursor(0, 0);
                        
                        // 解码并显示JPG图片
                        // 注意：getJpgSize的前两个参数是0, 0而不是用于存储宽度和高度的指针
                        TJpgDec.getJpgSize(0, 0, (const uint8_t *)file_data, file_size);
                        
                        // 显示JPG图片，忽略返回值因为实际显示成功但函数可能返回false
                        TJpgDec.drawJpg(0, 30, (const uint8_t *)file_data, file_size);
                        Utils_Logger::info("正在显示: %s", currentFilename.c_str());
                        
                        // 释放内存
                        free(file_data);
                    } else {
                        Utils_Logger::error("读取文件失败: %s", currentFilename.c_str());
                    }
                    // 关闭文件
                    file.close();
                }
            } else {
                Utils_Logger::error("文件不存在: %s", fullPath.c_str());
            }
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // 关闭文件系统
    fs.end();
    
    // 切换回主菜单（在清理资源之前，确保菜单能正常显示）
    menuContext.switchToMainMenu();
    
    // 首先清除退出事件标志，防止残留影响新任务
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    // 更新系统状态返回到主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除所有事件标志，确保系统状态干净
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    // 更新任务状态信息
    TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_C);
    
    // 释放动态分配的内存
    delete[] photoFiles;
    Utils_Logger::info("功能模块C任务 %c 退出 - 图片回放功能", (char)('A' + taskId));
    
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

bool TaskFactory::isValidTaskID(TaskManager::TaskID id) {
    return (id >= 0 && id < TaskManager::TASK_MAX);
}