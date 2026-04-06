/*
 * ISP_ConfigTask.cpp - ISP配置任务实现
 * 实现ISP配置界面的完整交互逻辑
 * 阶段三：ISP配置界面开发 - 任务层
 */

#include "ISP_ConfigTask.h"
#include "Menu_MenuContext.h"

ISPConfigTask g_ispConfigTask;

static ISPConfigTask* g_ispConfigTaskInstance = nullptr;

ISPConfigTask::ISPConfigTask()
    : m_ui(nullptr)
    , m_configManager(nullptr)
    , m_cameraManager(nullptr)
    , m_initialized(false)
    , m_taskRunning(false)
{
}

bool ISPConfigTask::init(ISPConfigUI& ui, ISPConfigManager& configManager, CameraManager& cameraManager)
{
    if (m_initialized) {
        Utils_Logger::info("ISPConfigTask already initialized");
        return true;
    }
    
    m_ui = &ui;
    m_configManager = &configManager;
    m_cameraManager = &cameraManager;
    m_initialized = true;
    g_ispConfigTaskInstance = this;
    
    Utils_Logger::info("ISPConfigTask initialized successfully");
    return true;
}

void ISPConfigTask::run()
{
    if (!m_initialized) {
        Utils_Logger::error("ISPConfigTask not initialized");
        return;
    }
    
    Utils_Logger::info("ISPConfigTask running");
    
    m_ui->showMainMenu();
    applyParametersToCamera();
    
    while (m_taskRunning) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void ISPConfigTask::start()
{
    if (m_taskRunning) {
        Utils_Logger::info("ISPConfigTask already running");
        return;
    }
    
    m_taskRunning = true;
    Utils_Logger::info("ISPConfigTask started");
}

void ISPConfigTask::stop()
{
    m_taskRunning = false;
    Utils_Logger::info("ISPConfigTask stopped");
}

void ISPConfigTask::cleanup()
{
    m_ui = nullptr;
    m_configManager = nullptr;
    m_cameraManager = nullptr;
    m_initialized = false;
    m_taskRunning = false;
    g_ispConfigTaskInstance = nullptr;
}

void ISPConfigTask::handleEncoderRotation(RotationDirection direction)
{
    if (!m_ui) return;
    
    ISPUIState state = m_ui->getUIState();
    
    if (state == ISP_UI_STATE_MENU) {
        processMenuNavigation(direction);
    } else if (state == ISP_UI_STATE_EDITING) {
        processParameterEdit(direction);
    }
}

void ISPConfigTask::handleEncoderButton()
{
    processButtonPress();
}

void ISPConfigTask::processMenuNavigation(RotationDirection direction)
{
    if (!m_ui) return;
    
    int currentIndex = m_ui->getCurrentMenuIndex();
    int newIndex = currentIndex;
    
    if (direction == ROTATION_CW) {
        newIndex = currentIndex + 1;
        if (newIndex >= ISP_MENU_COUNT) {
            newIndex = 0;
        }
    } else if (direction == ROTATION_CCW) {
        newIndex = currentIndex - 1;
        if (newIndex < 0) {
            newIndex = ISP_MENU_COUNT - 1;
        }
    }
    
    m_ui->selectMenuItem(newIndex);
}

void ISPConfigTask::processParameterEdit(RotationDirection direction)
{
    if (!m_ui) return;
    
    int delta = (direction == ROTATION_CW) ? 1 : -1;
    m_ui->updateParameterValue(delta);
    applyParametersToCamera();
}

void ISPConfigTask::processButtonPress()
{
    if (!m_ui) return;
    
    ISPUIState state = m_ui->getUIState();
    int currentIndex = m_ui->getCurrentMenuIndex();
    
    if (state == ISP_UI_STATE_MENU) {
        switch (currentIndex) {
            case ISP_MENU_EXPOSURE:
            case ISP_MENU_BRIGHTNESS:
            case ISP_MENU_CONTRAST:
            case ISP_MENU_SATURATION:
                m_ui->enterEditMode(currentIndex);
                break;
            case ISP_MENU_RESET:
                m_ui->resetParameters();
                applyParametersToCamera();
                break;
            case ISP_MENU_BACK:
                TaskManager::setEvent(EVENT_RETURN_TO_MENU);
                break;
            default:
                break;
        }
    } else if (state == ISP_UI_STATE_EDITING) {
        m_ui->confirmSave();
    }
}

void ISPConfigTask::applyParametersToCamera()
{
    if (!m_configManager || !m_cameraManager) return;
    
    ISPConfig config = m_configManager->getConfig();
    
    m_cameraManager->setExposureMode(config.exposureMode);
    m_cameraManager->setBrightness(config.brightness);
    m_cameraManager->setContrast(config.contrast);
    m_cameraManager->setSaturation(config.saturation);
    m_cameraManager->setAWBMode(config.awbMode);
    m_cameraManager->applyISPSettings();
}

void ISPConfigTask::redrawUI()
{
    if (!m_ui) return;
    
    // 调用ISPConfigUI的redrawUI方法，保持透明背景
    m_ui->redrawUI();
}

static void handleISPRotation(RotationDirection direction)
{
    if (g_ispConfigTaskInstance) {
        g_ispConfigTaskInstance->handleEncoderRotation(direction);
    }
}

static void handleISPButton()
{
    if (g_ispConfigTaskInstance) {
        g_ispConfigTaskInstance->handleEncoderButton();
    }
}

void taskISPConfig(void* pvParameters)
{
    Utils_Logger::info("ISP配置任务启动");
    
    StateManager::getInstance().setCurrentState(STATE_CAMERA_PREVIEW);
    
    extern Display_TFTManager tftManager;
    extern Display_FontRenderer fontRenderer;
    extern JPEGDEC jpeg;
    extern CameraManager cameraManager;
    extern SDCardManager sdCardManager;
    extern EncoderControl encoder;
    extern MenuContext menuContext;
    
    if (!cameraManager.isInitialized()) {
        Utils_Logger::info("相机管理器未初始化，正在重新初始化...");
        if (!cameraManager.init(tftManager, fontRenderer, jpeg)) {
            Utils_Logger::error("相机管理器重新初始化失败");
            vTaskDelete(NULL);
            return;
        }
    }
    
    static ISPConfigManager ispConfigManager;
    static ISPConfigUI ispConfigUI;
    
    if (!ispConfigManager.init(sdCardManager)) {
        Utils_Logger::error("ISPConfigManager init failed");
    }
    
    if (!ispConfigUI.init(tftManager, fontRenderer, ispConfigManager)) {
        Utils_Logger::error("ISPConfigUI init failed");
    }
    
    if (!g_ispConfigTask.init(ispConfigUI, ispConfigManager, cameraManager)) {
        Utils_Logger::error("ISPConfigTask init failed");
    }
    
    encoder.setRotationCallback(handleISPRotation);
    encoder.setButtonCallback(handleISPButton);
    
    cameraManager.startPreview();
    g_ispConfigTask.start();
    
    Utils_Logger::info("ISP配置功能初始化完成，等待用户操作");
    
    while (1) {
        cameraManager.processPreviewFrame();
        
        // 每帧都重绘UI，确保参数设置始终稳定显示在预览画面之上，避免闪烁
        // 提高重绘频率，使界面更流畅
        g_ispConfigTaskInstance->redrawUI();
        
        uint32_t uxBits = TaskManager::waitForEvent(
            EVENT_RETURN_TO_MENU,
            true,
            10 / portTICK_PERIOD_MS
        );
        
        if ((uxBits & EVENT_RETURN_TO_MENU) != 0) {
            Utils_Logger::info("ISP配置任务收到退出信号");
            break;
        }
        
        encoder.checkRotation();
        encoder.checkButton();
    }
    
    g_ispConfigTask.stop();
    cameraManager.stopPreview();
    
    TaskManager::clearEvent(EVENT_RETURN_TO_MENU);
    
    extern void handleEncoderRotation(RotationDirection direction);
    extern void handleEncoderButton();
    encoder.setRotationCallback(handleEncoderRotation);
    encoder.setButtonCallback(handleEncoderButton);
    Utils_Logger::info("已重置编码器回调函数为主菜单回调");
    
    menuContext.switchToMainMenu();
    menuContext.showMenu();
    
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    TaskManager::clearEvent(EVENT_ALL_TASKS_CLEAR);
    
    TaskManager::markTaskAsDeleting(TaskManager::TASK_FUNCTION_D);
    
    Utils_Logger::info("ISP配置任务退出");
    
    vTaskDelete(NULL);
}
