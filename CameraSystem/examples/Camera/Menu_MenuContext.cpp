/*
 * Menu_MenuContext.cpp - 菜单上下文实现
 * 管理菜单事件和上下文状态
 * 阶段六：菜单模块优化 - 菜单上下文管理
 */

#include "Menu_MenuContext.h"
#include "Camera_CameraManager.h"
#include "Menu_ParamSettings.h"
#include "PowerMode.h"
#include "sys_api.h"
#include "Shared_GlobalDefines.h"

static const uint8_t strConfirmReboot[] = {FONT16_IDX_QUE2, FONT16_IDX_REN2, FONT16_IDX_CHONG, FONT16_IDX_QI3, 0};
static const uint8_t strBack[]          = {FONT16_IDX_FAN2, FONT16_IDX_HUI2, 0};
static const uint8_t strReboot[]        = {FONT16_IDX_CHONG, FONT16_IDX_QI3, 0};
static const uint8_t strRebooting[]     = {FONT16_IDX_CHONG, FONT16_IDX_QI3, FONT16_IDX_ZHONG2, 0};
static const uint8_t strVersion[]       = {FONT16_IDX_BAN2, FONT16_IDX_BEN2, FONT16_IDX_XIN2, FONT16_IDX_XI4, 0};

// 前向声明全局菜单上下文对象
extern MenuContext menuContext;

// 编码器旋转回调函数实现
void handleEncoderRotation(RotationDirection direction) {
    // 如果在拍照中，忽略旋转事件 - 模块化移植：阶段五 - 使用CameraManager
    if (cameraManager.isCapturing()) {
        return;
    }
    
    // 处理旋转事件 - 添加严格的消抖处理
    static unsigned long lastValidRotation = 0;
    unsigned long currentTime = Utils_Timer::getCurrentTime();
    
    // 严格的软件消抖：确保两次旋转事件之间有足够的时间间隔
    if (currentTime - lastValidRotation > ENCODER_ROTATION_DEBOUNCE_DELAY) { // 使用宏定义的消抖时间
        lastValidRotation = currentTime;
        
        // 根据当前系统状态处理旋转事件
        switch (StateManager::getInstance().getCurrentState()) {
            case STATE_MAIN_MENU:
            case STATE_SUB_MENU:
                // 如果在重启确认对话框中，处理对话框选项切换
                if (menuContext.isInRebootConfirm()) {
                    if (direction == ROTATION_CW) {
                        menuContext.setConfirmDefaultBack(false);
                    } else if (direction == ROTATION_CCW) {
                        menuContext.setConfirmDefaultBack(true);
                    }
                    menuContext.showRebootConfirmDialog();
                    break;
                }
                // 菜单模式：使用MenuContext处理旋转事件
                if (direction == ROTATION_CW) {
                    menuContext.handleEvent(MENU_EVENT_DOWN);
                    Utils_Logger::info("菜单：编码器旋转: 顺时针（向下移动菜单项）");
                } else if (direction == ROTATION_CCW) {
                    menuContext.handleEvent(MENU_EVENT_UP);
                    Utils_Logger::info("菜单：编码器旋转: 逆时针（向上移动菜单项）");
                }
                Utils_Logger::info("菜单当前位置: %c", (char)('A' + menuContext.getCurrentMenuItem()));
                break;
                
            case STATE_CAMERA_PREVIEW:
                // 根据当前运行的任务类型区分处理逻辑
                if (TaskManager::getTaskState(TaskManager::TASK_CAMERA_PREVIEW) == TASK_STATE_RUNNING) {
                    // 相机预览模式(A位置)：旋转旋钮返回主菜单
                    Utils_Logger::info("相机预览模式：旋转旋钮 - 返回主菜单");
                    TaskManager::setEvent(EVENT_RETURN_TO_MENU);
                } else if (TaskManager::getTaskState(TaskManager::TASK_FUNCTION_C) == TASK_STATE_RUNNING) {
                    // 回放模式(C位置)：旋钮控制照片导航
                    if (direction == ROTATION_CW) {
                        // 顺时针旋转：下一张照片
                        Utils_Logger::info("回放模式：顺时针旋转 - 显示下一张照片");
                        TaskManager::setEvent(EVENT_NEXT_PHOTO);
                    } else if (direction == ROTATION_CCW) {
                        // 逆时针旋转：上一张照片
                        Utils_Logger::info("回放模式：逆时针旋转 - 显示上一张照片");
                        TaskManager::setEvent(EVENT_PREVIOUS_PHOTO);
                    }
                }
                break;
                
            default:
                Utils_Logger::error("未知系统状态：无法处理旋转事件");
                break;
        }
    } else {
        // 忽略过快的连续旋转（可能是抖动）
        Utils_Logger::info("忽略快速旋转事件（严格消抖处理）");
    }
}

// 编码器按钮回调函数实现
void handleEncoderButton() {
    Utils_Logger::info("编码器按钮按下");
    
    // 根据当前系统状态处理按钮事件
    switch (StateManager::getInstance().getCurrentState()) {
        case STATE_MAIN_MENU:
        case STATE_SUB_MENU:
            // 菜单模式：由loop()中的buttonPressDetected处理逻辑处理
            StateManager::getInstance().setButtonPressDetected(true);
            break;
            
        case STATE_CAMERA_PREVIEW:
            // 根据当前运行的任务类型区分处理逻辑
            if (TaskManager::getTaskState(TaskManager::TASK_CAMERA_PREVIEW) == TASK_STATE_RUNNING) {
                // 相机预览模式(A位置)：按压开关拍照
                Utils_Logger::info("相机预览模式：按压开关 - 拍照");
                cameraManager.requestCapture();
            } else if (TaskManager::getTaskState(TaskManager::TASK_FUNCTION_C) == TASK_STATE_RUNNING) {
                // 回放模式(C位置)：按压开关返回主菜单
                Utils_Logger::info("回放模式：按压开关 - 返回主菜单");
                TaskManager::setEvent(EVENT_RETURN_TO_MENU);
            }
            break;
            
        default:
            Utils_Logger::error("未知系统状态：无法处理按钮事件");
            break;
    }
}

// 初始化菜单上下文
bool MenuContext::init() {
    if (isInitialized) {
        Utils_Logger::info("[MenuContext] 已经初始化");
        return true;
    }
    
    Utils_Logger::info("[MenuContext] 初始化");
    
    // 初始化菜单管理器
    if (!menuManager.init()) {
        Utils_Logger::error("[MenuContext] 菜单管理器初始化失败");
        return false;
    }
    
    // 初始化三角形控制器
    if (!triangleController.init()) {
        Utils_Logger::error("[MenuContext] 三角形控制器初始化失败");
        return false;
    }
    
    // 初始化参数设置菜单
    paramSettingsMenu = new ParamSettingsMenu(tftManager, fontRenderer);
    paramSettingsMenu->init();
    
    // 显示初始菜单
    showMenu();
    
    isInitialized = true;
    Utils_Logger::info("[MenuContext] 初始化完成");
    return true;
}

// 处理菜单事件
bool MenuContext::handleEvent(MenuEventType event) {
    if (!isInitialized) {
        Utils_Logger::error("[MenuContext] 未初始化");
        return false;
    }
    
    Utils_Logger::info("[MenuContext] 处理事件: %d", event);
    
    // 如果在参数设置菜单中，直接委托给参数设置菜单处理
    if (inParamSettings && paramSettingsMenu != nullptr) {
        switch (event) {
            case MENU_EVENT_UP:
                paramSettingsMenu->handleUp();
                break;
            case MENU_EVENT_DOWN:
                paramSettingsMenu->handleDown();
                break;
            case MENU_EVENT_SELECT:
                paramSettingsMenu->handleSelect();
                // 检查是否需要退出参数设置菜单
                if (paramSettingsMenu->getState() == PARAM_STATE_MENU && 
                    paramSettingsMenu->getCurrentMenuItem() == 5) {
                    inParamSettings = false;
                    paramSettingsMenu->reset();
                    menuManager.switchToPageByType(MENU_PAGE_SUB);
                    triangleController.resetPosition();
                    showMenu();
                }
                break;
            case MENU_EVENT_BACK:
                paramSettingsMenu->handleBack();
                // 检查是否需要退出参数设置菜单
                if (paramSettingsMenu->getState() == PARAM_STATE_MENU) {
                    inParamSettings = false;
                    paramSettingsMenu->reset();
                    menuManager.switchToPageByType(MENU_PAGE_SUB);
                    triangleController.resetPosition();
                    showMenu();
                }
                break;
            default:
                break;
        }
        return true;
    }
    
    switch (event) {
        case MENU_EVENT_UP:
            // 向上移动光标
            triangleController.moveUp();
            break;
            
        case MENU_EVENT_DOWN:
            // 向下移动光标
            triangleController.moveDown();
            break;
            
        case MENU_EVENT_SELECT:
            // 选择当前菜单项
            {
                const MenuItem *item = getCurrentMenuItemInfo();
                if (item != nullptr) {
                    int currentPos = triangleController.getCurrentPosition();
                    Utils_Logger::info("[MenuContext] 选择菜单项: %d (%c) - %s", 
                                      currentPos, (char)('A' + currentPos), 
                                      item->label);
                    
                    // 根据菜单项类型和操作执行不同的逻辑
                    switch (item->operation) {
                        case MENU_OPERATION_CAPTURE:
                            Utils_Logger::info("[MenuContext] 执行拍照操作");
                            break;

                        case MENU_OPERATION_FILE_TRANSFER:
                            Utils_Logger::info("[MenuContext] 执行WiFi文件传输操作");
                            break;
                            
                        case MENU_OPERATION_RECORD:
                            Utils_Logger::info("[MenuContext] 执行录像操作");
                            break;
                            
                        case MENU_OPERATION_PLAYBACK:
                            Utils_Logger::info("[MenuContext] 执行回放操作");
                            break;
                            
                        case MENU_OPERATION_SETTINGS:
                            if (item->type == MENU_ITEM_TYPE_SUBMENU) {
                                Utils_Logger::info("[MenuContext] 进入设置子菜单");
                                menuManager.switchToPageByType(MENU_PAGE_SUB);
                                triangleController.resetPosition();
                            }
                            break;
                            
                        case MENU_OPERATION_PARAM_SETTINGS:
                            Utils_Logger::info("[MenuContext] 进入参数设置菜单");
                            inParamSettings = true;
                            paramSettingsMenu->init();
                            paramSettingsMenu->show();
                            break;
                            
                        case MENU_OPERATION_ABOUT:
                            Utils_Logger::info("[MenuContext] 显示关于信息");
                            break;
                            
                        case MENU_OPERATION_EXIT:
                            Utils_Logger::info("[MenuContext] 退出菜单");
                            break;
                            
                        case MENU_OPERATION_BACK:
                            Utils_Logger::info("[MenuContext] 返回上一级菜单");
                            menuManager.switchToPageByType(MENU_PAGE_MAIN);
                            triangleController.resetPosition();
                            break;

                        case MENU_OPERATION_TIME_SYNC:
                            Utils_Logger::info("[MenuContext] 执行校对时间操作");
                            if (TaskManager::getTaskState(TaskManager::TASK_TIME_SYNC) != TASK_STATE_DELETED) {
                                TaskManager::markTaskAsDeleting(TaskManager::TASK_TIME_SYNC);
                            }
                            if (TaskFactory::createDefaultTask(TaskManager::TASK_TIME_SYNC)) {
                                Utils_Logger::info("校对时间任务启动成功");
                            } else {
                                Utils_Logger::error("校对时间任务启动失败");
                            }
                            break;

                        case MENU_OPERATION_SHUTDOWN:
                            Utils_Logger::info("[MenuContext] 关闭系统功能暂未开放");
                            break;

                        case MENU_OPERATION_OTA:
                            Utils_Logger::info("[MenuContext] 系统升级功能暂未开放");
                            break;

                        case MENU_OPERATION_VERSION:
                            Utils_Logger::info("[MenuContext] 显示版本信息");
                            showVersionInfo();
                            break;

                        default:
                            Utils_Logger::info("[MenuContext] 未处理的菜单项操作: %d", item->operation);
                            break;
                    }
                } else {
                    Utils_Logger::info("[MenuContext] 无效的菜单项: %d", triangleController.getCurrentPosition());
                }
            }
            break;
            
        case MENU_EVENT_BACK:
            // 返回上一级菜单
            Utils_Logger::info("[MenuContext] 返回上一级菜单");
            menuManager.switchToPageByType(MENU_PAGE_MAIN);
            triangleController.resetPosition();
            break;
            
        case MENU_EVENT_NEXT_PAGE:
            // 切换到下一页
            menuManager.nextMenu();
            triangleController.resetPosition();
            break;
            
        case MENU_EVENT_PREV_PAGE:
            // 切换到上一页
            menuManager.previousMenu();
            triangleController.resetPosition();
            break;
            
        case MENU_EVENT_RESET:
            // 重置菜单
            reset();
            break;
            
        default:
            Utils_Logger::info("[MenuContext] 未知事件类型: %d", event);
            return false;
    }
    
    // 如果页面切换了，重新显示菜单
    if (event == MENU_EVENT_NEXT_PAGE || event == MENU_EVENT_PREV_PAGE || 
        event == MENU_EVENT_BACK || event == MENU_EVENT_RESET) {
        showMenu();
    }
    
    return true;
}

// 显示菜单
void MenuContext::showMenu() {
    if (!isInitialized) {
        Utils_Logger::error("[MenuContext] 未初始化");
        return;
    }
    
    // 如果在参数设置菜单中，不显示常规菜单
    if (inParamSettings && paramSettingsMenu != nullptr) {
        Utils_Logger::info("[MenuContext] 参数设置菜单模式，不显示常规菜单");
        return;
    }
    
    // 显示当前菜单页面
    menuManager.showCurrentMenu();
    
    // 更新三角形光标显示
    triangleController.updateDisplay();
    
    Utils_Logger::info("[MenuContext] 菜单显示完成");
}

// 获取当前选中的菜单项信息
const MenuItem *MenuContext::getCurrentMenuItemInfo() const {
    const MenuConfig *config = menuManager.getCurrentMenuConfig();
    if (config == nullptr || config->menuItems == nullptr) {
        Utils_Logger::error("[MenuContext] 无效的菜单配置");
        return nullptr;
    }
    
    int currentPos = triangleController.getCurrentPosition();
    if (currentPos >= 0 && currentPos < config->menuItemCount) {
        return &config->menuItems[currentPos];
    }
    
    Utils_Logger::error("[MenuContext] 无效的菜单项索引: %d", currentPos);
    return nullptr;
}

// 重置菜单状态
void MenuContext::reset() {
    if (!isInitialized) {
        Utils_Logger::error("[MenuContext] 未初始化");
        return;
    }
    
    Utils_Logger::info("[MenuContext] 重置菜单状态");
    
    // 重置参数设置菜单状态
    inParamSettings = false;
    if (paramSettingsMenu != nullptr) {
        paramSettingsMenu->reset();
    }
    
    // 切换到主菜单
    menuManager.switchToPageByType(MENU_PAGE_MAIN);
    
    // 重置三角形位置
    triangleController.resetPosition();
    
    // 重新显示菜单
    showMenu();
}

// 处理系统设置子菜单
void MenuContext::handleSubMenu() {
    // 子菜单中的旋转和按钮事件现在通过EncoderControl回调函数处理
    // MenuContext会自动协调菜单管理器和三角形控制器
    
    // 如果在重启确认对话框中，处理对话框交互
    if (inRebootConfirm) {
        if (StateManager::getInstance().isButtonPressDetected()) {
            StateManager::getInstance().setButtonPressDetected(false);
            
            if (confirmDefaultBack) {
                // 默认选中"返回"，退出确认对话框
                hideRebootConfirmDialog();
                triangleController.moveToPosition(TriangleController::POSITION_B);
            } else {
                // 选中"重启"，执行重启
                executeReboot();
            }
        }
        return;
    }
    
    // 处理按钮事件（只在F位置返回主菜单）
    if (StateManager::getInstance().isButtonPressDetected()) {
        StateManager::getInstance().setButtonPressDetected(false);
        
        if (getCurrentMenuItem() == POS_F) {
            // F位置按下：返回主菜单
            Utils_Logger::info("子菜单F位置：返回主菜单");
            switchToMainMenu();
        } else if (getCurrentMenuItem() == POS_B) {
            // B位置按下：显示重启确认对话框
            Utils_Logger::info("子菜单B位置：显示重启确认对话框");
            rebootConfirmPosB = POS_B;
            confirmDefaultBack = true;
            inRebootConfirm = true;
            showRebootConfirmDialog();
        } else if (getCurrentMenuItem() == POS_A) {
            // A位置按下：启动校对时间功能
            Utils_Logger::info("子菜单A位置：启动校对时间功能");

            // 检查任务状态，如果不是删除状态，就标记为删除状态
            if (TaskManager::getTaskState(TaskManager::TASK_TIME_SYNC) != TASK_STATE_DELETED) {
                Utils_Logger::info("将现有任务标记为删除状态");
                TaskManager::markTaskAsDeleting(TaskManager::TASK_TIME_SYNC);
            }

            // 创建新任务
            if (TaskFactory::createDefaultTask(TaskManager::TASK_TIME_SYNC)) {
                Utils_Logger::info("校对时间任务启动成功");
            } else {
                Utils_Logger::error("校对时间任务启动失败");
            }
        } else if (getCurrentMenuItem() == POS_C) {
            Utils_Logger::info("子菜单C位置：功能暂未开放");
        } else if (getCurrentMenuItem() == POS_D) {
            Utils_Logger::info("子菜单D位置：功能暂未开放");
        } else if (getCurrentMenuItem() == POS_E) {
            // E位置按下：版本信息
            Utils_Logger::info("子菜单E位置：版本信息");
            showVersionInfo();
        } else {
            // 其他位置按下：无操作，仅显示位置信息
            Utils_Logger::info("子菜单非F位置按下：位置%c", (char)('A' + getCurrentMenuItem()));
        }
    }
}

// 切换到系统设置子菜单
void MenuContext::switchToSubMenu() {
    Utils_Logger::info("切换到系统设置子菜单");
    StateManager::getInstance().setCurrentState(STATE_SUB_MENU);
    
    // 切换到子菜单页面
    menuManager.switchToPageByType(MENU_PAGE_SUB);
    
    // 重置三角形位置到A并显示菜单
    triangleController.resetPosition();
    showMenu();
    
    Utils_Logger::info("已切换到系统设置子菜单");
}

// 切换回主菜单
void MenuContext::switchToMainMenu() {
    Utils_Logger::info("切换回主菜单");
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    // 清除参数设置菜单标志
    inParamSettings = false;
    
    // 清除可能残留的按钮按下标志，防止错误触发任务创建
    StateManager::getInstance().setButtonPressDetected(false);
    
    // 切换回主菜单页面
    menuManager.switchToPageByType(MENU_PAGE_MAIN);
    
    // 重置三角形位置到A并显示菜单
    triangleController.resetPosition();
    showMenu();
    
    Utils_Logger::info("已切换回主菜单");
}

// 清理菜单上下文资源
void MenuContext::cleanup() {
    Utils_Logger::info("[MenuContext] 开始清理资源");
    
    // 重置菜单状态
    reset();
    
    // 清理参数设置菜单对象
    if (paramSettingsMenu != nullptr) {
        delete paramSettingsMenu;
        paramSettingsMenu = nullptr;
    }
    
    // 清理菜单管理器资源
    menuManager.cleanup();
    
    // 将系统状态设置为主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    Utils_Logger::info("[MenuContext] 资源清理完成");
}

void MenuContext::showRebootConfirmDialog() {
    Utils_Logger::info("显示重启确认对话框");

    const int screenWidth = 320;
    const int screenHeight = 240;

    const int dialogWidth = 200;
    const int dialogHeight = 90;
    const int dialogX = (screenWidth - dialogWidth) / 2;
    const int dialogY = (screenHeight - dialogHeight) / 2;

    tftManager.fillRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_BLACK);
    tftManager.drawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_WHITE);

    const int titleY = dialogY + (dialogHeight - 16) / 2 - 14;
    const int btnY = dialogY + (dialogHeight - 16) / 2 + 14;

    int16_t titleX = fontRenderer.calculateCenterPosition(dialogWidth, strConfirmReboot) + dialogX;
    fontRenderer.drawChineseString(titleX, titleY, strConfirmReboot, ST7789_WHITE, ST7789_BLACK);

    const int triWidth = 10;
    const int btnSpacing = 70;
    const int totalBtnWidth = triWidth + 8 + fontRenderer.getStringLength(strBack) * 16 + btnSpacing + triWidth + 8 + fontRenderer.getStringLength(strReboot) * 16;
    const int btnStartX = (screenWidth - totalBtnWidth) / 2;
    const int backTextX = btnStartX + triWidth + 8;
    const int rebootTextX = backTextX + btnSpacing + triWidth + 8;

    if (confirmDefaultBack) {
        tftManager.setCursor(btnStartX, btnY);
        tftManager.setTextColor(ST7789_YELLOW);
        tftManager.print(">");
        fontRenderer.drawChineseString(backTextX, btnY, strBack, ST7789_YELLOW, ST7789_BLACK);
        tftManager.setCursor(rebootTextX - triWidth - 8, btnY);
        tftManager.setTextColor(ST7789_WHITE);
        tftManager.print(">");
        fontRenderer.drawChineseString(rebootTextX, btnY, strReboot, ST7789_WHITE, ST7789_BLACK);
    } else {
        tftManager.setCursor(btnStartX, btnY);
        tftManager.setTextColor(ST7789_WHITE);
        tftManager.print(">");
        fontRenderer.drawChineseString(backTextX, btnY, strBack, ST7789_WHITE, ST7789_BLACK);
        tftManager.setCursor(rebootTextX - triWidth - 8, btnY);
        tftManager.setTextColor(ST7789_YELLOW);
        tftManager.print(">");
        fontRenderer.drawChineseString(rebootTextX, btnY, strReboot, ST7789_YELLOW, ST7789_BLACK);
    }
}

void MenuContext::hideRebootConfirmDialog() {
    Utils_Logger::info("隐藏重启确认对话框");

    inRebootConfirm = false;
    confirmDefaultBack = true;
    rebootConfirmPosB = -1;

    showMenu();
}

void MenuContext::executeReboot() {
    Utils_Logger::info("执行系统重启...");

    inRebootConfirm = false;
    confirmDefaultBack = true;
    rebootConfirmPosB = -1;

    tftManager.fillScreen(ST7789_BLACK);

    int16_t rebootX = fontRenderer.calculateCenterPosition(320, strRebooting);
    fontRenderer.drawChineseString(rebootX, 112, strRebooting, ST7789_YELLOW, ST7789_BLACK);

    Utils_Logger::info("系统即将重启...");
    delay(1000);

    sys_reset();
}

void MenuContext::executeShutdown() {
    Utils_Logger::info("执行关闭系统（深度睡眠）...");

    tftManager.fillScreen(ST7789_BLACK);
    tftManager.setCursor(50, 150);
    tftManager.print("entering deep sleep...");

    delay(500);

    PowerMode.begin(DEEPSLEEP_MODE, -1, 0, 0);
    PowerMode.start();
}

void MenuContext::executeOTA() {
    Utils_Logger::info("执行系统升级（OTA）...");

    tftManager.fillScreen(ST7789_BLACK);
    tftManager.setCursor(30, 150);
    tftManager.print("OTA update starting...");

    delay(1000);

    // OTA更新逻辑将通过WiFiFileServer或其他OTA模块处理
    // 这里仅显示提示信息
}

static void formatDate(const char* srcDate, char* dstBuf) {
    char monthStr[4] = {0};
    int day = 0, year = 0;
    sscanf(srcDate, "%3s %d %d", monthStr, &day, &year);

    int month = 1;
    if (!strcmp(monthStr, "Jan")) month = 1;
    else if (!strcmp(monthStr, "Feb")) month = 2;
    else if (!strcmp(monthStr, "Mar")) month = 3;
    else if (!strcmp(monthStr, "Apr")) month = 4;
    else if (!strcmp(monthStr, "May")) month = 5;
    else if (!strcmp(monthStr, "Jun")) month = 6;
    else if (!strcmp(monthStr, "Jul")) month = 7;
    else if (!strcmp(monthStr, "Aug")) month = 8;
    else if (!strcmp(monthStr, "Sep")) month = 9;
    else if (!strcmp(monthStr, "Oct")) month = 10;
    else if (!strcmp(monthStr, "Nov")) month = 11;
    else if (!strcmp(monthStr, "Dec")) month = 12;

    sprintf(dstBuf, "%04d-%02d-%02d", year, month, day);
}

void MenuContext::showVersionInfo() {
    Utils_Logger::info("显示版本信息...");

    const int screenWidth = 320;
    const int screenHeight = 240;

    const int dialogWidth = 210;
    const int dialogHeight = 160;
    const int dialogX = (screenWidth - dialogWidth) / 2;
    const int dialogY = (screenHeight - dialogHeight) / 2;

    tftManager.fillRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_BLACK);
    tftManager.drawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_WHITE);

    int16_t titleX = fontRenderer.calculateCenterPosition(dialogWidth, strVersion) + dialogX;
    fontRenderer.drawChineseString(titleX, dialogY + 14, strVersion, ST7789_WHITE, ST7789_BLACK);

    const int textLeft = dialogX + 20;
    const int lineHeight = 22;
    int lineY = dialogY + 40;

    tftManager.setCursor(textLeft, lineY);
    tftManager.print("Ver: ");
    tftManager.print(SYSTEM_VERSION_STRING);

    lineY += lineHeight;
    tftManager.setCursor(textLeft, lineY);
    tftManager.print("FW: v4.0.9");

    lineY += lineHeight;
    char dateBuf[12];
    formatDate(__DATE__, dateBuf);
    tftManager.setCursor(textLeft, lineY);
    tftManager.print("Build: ");
    tftManager.print(dateBuf);

    lineY += lineHeight;
    tftManager.setCursor(textLeft, lineY);
    tftManager.print("Cam: GC2053");

    lineY += lineHeight;
    tftManager.setCursor(textLeft, lineY);
    tftManager.print("Board: AMB82-MINI");

    delay(4000);

    showMenu();
}