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
#include "OTA.h"
#include <WiFi.h>
#include "wifi_conf.h"
#include "DS3231_ClockModule.h"
#include "Shared_SharedResources.h"

extern DS3231_ClockModule clockModule;
extern TimeSyncTaskStatus g_timeSyncStatus;

static const uint8_t strConfirmReboot[] = {FONT16_IDX_QUE2, FONT16_IDX_REN2, FONT16_IDX_CHONG, FONT16_IDX_QI3, 0};
static const uint8_t strBack[]          = {FONT16_IDX_FAN2, FONT16_IDX_HUI2, 0};
static const uint8_t strReboot[]        = {FONT16_IDX_CHONG, FONT16_IDX_QI3, 0};
static const uint8_t strRebooting[]     = {FONT16_IDX_CHONG, FONT16_IDX_QI3, FONT16_IDX_ZHONG2, 0};
static const uint8_t strVersion[]       = {FONT16_IDX_BAN2, FONT16_IDX_BEN2, FONT16_IDX_XIN2, FONT16_IDX_XI4, 0};
static const uint8_t strConfirmOta[]    = {FONT16_IDX_QUE2, FONT16_IDX_REN2, FONT16_IDX_SHENG2, FONT16_IDX_JI2, 0};
static const uint8_t strConfirmExit[]   = {FONT16_IDX_QUE2, FONT16_IDX_REN2, 0};
static const uint8_t strOta[]           = {FONT16_IDX_SHENG2, FONT16_IDX_JI2, 0};
static const uint8_t strOtaProgress[]   = {FONT16_IDX_SHENG2, FONT16_IDX_JI2, FONT16_IDX_ZHONG2, 0};
static const uint8_t strConfirm[]       = {FONT16_IDX_QUE2, FONT16_IDX_REN2, 0};
static const uint8_t strSetIp[]        = {FONT16_IDX_SHE2, FONT16_IDX_ZHI2, 0};
static const uint8_t strTransferMode[]  = {FONT16_IDX_CHUAN2, FONT16_IDX_SHU3, FONT16_IDX_MO2, FONT16_IDX_SHI8, 0};

// 前向声明全局菜单上下文对象
extern MenuContext menuContext;
extern EncoderControl encoder;

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
                // 如果在OTA确认对话框中，处理对话框选项切换
                if (menuContext.isInOtaConfirm()) {
                    if (direction == ROTATION_CW) {
                        menuContext.setOtaConfirmDefaultBack(false);
                    } else if (direction == ROTATION_CCW) {
                        menuContext.setOtaConfirmDefaultBack(true);
                    }
                    menuContext.showOtaConfirmDialog();
                    break;
                }
                // 如果在OTA服务器IP选择界面中，处理选项切换
                if (menuContext.isInOtaIpConfig()) {
                    if (menuContext.isInOtaIpSelectState()) {
                        menuContext.handleOtaIpSelectRotation(direction);
                    } else {
                        menuContext.handleOtaIpRotation(direction);
                    }
                    break;
                }
                // 如果在OTA升级中界面退出对话框已显示，处理对话框选项切换
                if (menuContext.isInOtaProgress() && menuContext.isOtaExitDialogShown()) {
                    if (direction == ROTATION_CW) {
                        menuContext.setOtaProgressDefaultBack(false);
                    } else if (direction == ROTATION_CCW) {
                        menuContext.setOtaProgressDefaultBack(true);
                    }
                    menuContext.showOtaProgressExitDialog();
                    break;
                }
                // 如果在OTA升级中界面但对话框未显示，忽略旋转事件
                if (menuContext.isInOtaProgress()) {
                    break;
                }
                // 如果在BLE WiFi配网界面中，处理退出对话框
                if (menuContext.isInBleWifiConfig()) {
                    menuContext.handleBleExitRotation(direction);
                    break;
                }
                // 如果在USB MSC模式中，委托给USB模块处理
                if (usbMassStorageModule.isRunning()) {
                    usbMassStorageModule.handleRotation(direction);
                    break;
                }
                // 如果在传输模式选择界面中，处理选项切换
                if (menuContext.isInTransferModeSelect()) {
                    menuContext.handleTransferModeSelectRotation(direction);
                    break;
                }
                // 如果在时间同步窗口中，旋转旋钮关闭窗口返回子菜单
                if (menuContext.isInTimeSyncWindow()) {
                    Utils_Logger::info("时间同步窗口：旋转旋钮 - 关闭窗口返回子菜单");
                    menuContext.hideTimeSyncWindow();
                    menuContext.showMenu();
                    break;
                }
                // 菜单模式：使用MenuContext处理旋转事件
                if (direction == ROTATION_CW) {
                    bool wasPosA = (menuContext.getCurrentMenuItem() == POS_A);
                    menuContext.handleEvent(MENU_EVENT_DOWN);
                    Utils_Logger::info("菜单：编码器旋转: 顺时针（向下移动菜单项）");
                    if (wasPosA && menuContext.getCurrentMenuItem() != POS_A && !menuContext.isInTimeSyncWindow()) {
                        menuContext.resetTimeSyncButtonHandled();
                    }
                } else if (direction == ROTATION_CCW) {
                    bool wasPosA = (menuContext.getCurrentMenuItem() == POS_A);
                    menuContext.handleEvent(MENU_EVENT_UP);
                    Utils_Logger::info("菜单：编码器旋转: 逆时针（向上移动菜单项）");
                    if (wasPosA && menuContext.getCurrentMenuItem() != POS_A && !menuContext.isInTimeSyncWindow()) {
                        menuContext.resetTimeSyncButtonHandled();
                    }
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

    // 如果在OTA升级中界面，处理退出确认逻辑
    if (menuContext.isInOtaProgress()) {
        if (menuContext.isOtaExitDialogShown()) {
            Utils_Logger::info("[OTA_EXIT] 对话框已显示，处理按钮选择");
            menuContext.handleOtaProgressExitButton();
        } else {
            Utils_Logger::info("[OTA_EXIT] OTA升级中界面：显示退出确认对话框");
            menuContext.showOtaProgressExitDialog();
        }
        return;
    }

    // 如果在USB MSC模式中，委托给USB模块处理
    if (usbMassStorageModule.isRunning()) {
        usbMassStorageModule.handleButton();
        if (usbMassStorageModule.isExitConfirmed()) {
            menuContext.returnFromUsbMode();
        }
        return;
    }

    // 如果在BLE WiFi配网界面中，处理退出确认逻辑
    if (menuContext.isInBleWifiConfig()) {
        if (menuContext.isBleExitDialogShown()) {
            menuContext.handleBleExitButton();
        } else {
            menuContext.showBleExitDialog();
        }
        return;
    }

    // 如果在传输模式选择界面中，处理选择确认
    if (menuContext.isInTransferModeSelect()) {
        Utils_Logger::info("[TRANSFER] 传输模式选择：处理按钮确认");
        menuContext.handleTransferModeSelectButton();
        return;
    }

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

    // 初始化USB MSC模块
    if (!usbMassStorageModule.init(tftManager, fontRenderer)) {
        Utils_Logger::error("[MenuContext] USB MSC模块初始化失败");
        return false;
    }
    Utils_Logger::info("[MenuContext] USB MSC模块初始化成功");

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

                        case MENU_OPERATION_BLE_WIFI_CONFIG:
                            Utils_Logger::info("[MenuContext] BLE WiFi配网");
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

    // 重置时间同步窗口按钮处理标志，允许下次按压打开窗口
    timeSyncButtonHandled = false;

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
                hideRebootConfirmDialog();
                triangleController.moveToPosition(TriangleController::POSITION_B);
            } else {
                executeReboot();
            }
        }
        return;
    }
    
    // 如果在OTA确认对话框中，处理对话框交互
    if (inOtaConfirm) {
        if (StateManager::getInstance().isButtonPressDetected()) {
            StateManager::getInstance().setButtonPressDetected(false);

            if (otaConfirmDefaultBack) {
                hideOtaConfirmDialog();
                triangleController.moveToPosition(TriangleController::POSITION_D);
            } else {
                inOtaConfirm = false;
                otaConfirmDefaultBack = true;
                otaConfirmPosD = -1;
                showOtaIpConfig();
            }
        }
        return;
    }

    // 如果在OTA升级中界面，只处理按钮事件（旋转由handleEncoderRotation处理）
    if (inOtaProgress) {
        if (StateManager::getInstance().isButtonPressDetected()) {
            StateManager::getInstance().setButtonPressDetected(false);
            if (otaExitDialogShown) {
                Utils_Logger::info("[OTA_EXIT] 检测到按钮按下，调用handleOtaProgressExitButton");
                handleOtaProgressExitButton();
            } else {
                Utils_Logger::info("[OTA_EXIT] 检测到按钮按下，显示退出确认对话框");
                showOtaProgressExitDialog();
            }
        }
        return;
    }

    // 如果在OTA服务器IP配置界面中，处理按钮事件
    if (isInOtaIpConfig()) {
        if (StateManager::getInstance().isButtonPressDetected()) {
            StateManager::getInstance().setButtonPressDetected(false);
            if (isInOtaIpSelectState()) {
                handleOtaIpSelectButton();
            } else {
                handleOtaIpButton();
            }
        }
        return;
    }

    // 如果在BLE WiFi配网界面中，处理按钮事件
    if (inBleWifiConfig) {
        if (StateManager::getInstance().isButtonPressDetected()) {
            StateManager::getInstance().setButtonPressDetected(false);
            if (bleExitDialogShown) {
                handleBleExitButton();
            } else {
                showBleExitDialog();
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
            if (inTimeSyncWindow) {
                Utils_Logger::info("子菜单A位置：关闭时间同步窗口");
                hideTimeSyncWindow();
                showMenu();
            } else if (!timeSyncButtonHandled) {
                timeSyncButtonHandled = true;
                Utils_Logger::info("子菜单A位置：打开时间同步窗口");
                showTimeSyncWindow();
                if (TaskManager::getTaskState(TaskManager::TASK_TIME_SYNC) != TASK_STATE_DELETED) {
                    TaskManager::markTaskAsDeleting(TaskManager::TASK_TIME_SYNC);
                }
                if (TaskFactory::createDefaultTask(TaskManager::TASK_TIME_SYNC)) {
                    Utils_Logger::info("校对时间任务启动成功");
                } else {
                    Utils_Logger::error("校对时间任务启动失败");
                }
            }
        } else if (getCurrentMenuItem() == POS_C) {
            Utils_Logger::info("子菜单C位置：启动BLE WiFi配网");
            executeBleWifiConfig();
        } else if (getCurrentMenuItem() == POS_D) {
            // D位置按下：显示OTA确认对话框
            Utils_Logger::info("子菜单D位置：显示OTA确认对话框");
            otaConfirmPosD = POS_D;
            otaConfirmDefaultBack = true;
            inOtaConfirm = true;
            showOtaConfirmDialog();
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

    const int dialogWidth = 220;
    const int dialogHeight = 100;
    const int dialogX = (screenWidth - dialogWidth) / 2;
    const int dialogY = (screenHeight - dialogHeight) / 2;

    tftManager.fillRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_BLACK);
    tftManager.drawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_WHITE);

    const int titleY = dialogY + 20;
    int16_t titleX = fontRenderer.calculateCenterPosition(dialogWidth, strConfirmReboot) + dialogX;
    fontRenderer.drawChineseString(titleX, titleY, strConfirmReboot, ST7789_WHITE, ST7789_BLACK);

    const int btnY = dialogY + 55;
    const int btnAreaWidth = dialogWidth - 20;
    const int backBtnWidth = fontRenderer.getStringLength(strBack) * 16;
    const int rebootBtnWidth = fontRenderer.getStringLength(strReboot) * 16;
    const int totalBtnWidth = backBtnWidth + 20 + rebootBtnWidth;
    const int btnStartX = dialogX + (btnAreaWidth - totalBtnWidth) / 2;

    const int backTextX = btnStartX + backBtnWidth;
    const int rebootTextX = backTextX + 20 + rebootBtnWidth;

    if (confirmDefaultBack) {
        tftManager.setCursor(btnStartX - 12, btnY);
        tftManager.setTextColor(ST7789_YELLOW);
        tftManager.print(">");
        fontRenderer.drawChineseString(btnStartX, btnY, strBack, ST7789_YELLOW, ST7789_BLACK);

        tftManager.setCursor(backTextX + 8, btnY);
        tftManager.setTextColor(ST7789_WHITE);
        tftManager.print(">");
        fontRenderer.drawChineseString(rebootTextX, btnY, strReboot, ST7789_WHITE, ST7789_BLACK);
    } else {
        tftManager.setCursor(btnStartX - 12, btnY);
        tftManager.setTextColor(ST7789_WHITE);
        tftManager.print(">");
        fontRenderer.drawChineseString(btnStartX, btnY, strBack, ST7789_WHITE, ST7789_BLACK);

        tftManager.setCursor(backTextX + 8, btnY);
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

void MenuContext::showOtaConfirmDialog() {
    Utils_Logger::info("显示OTA确认对话框");

    const int screenWidth = 320;
    const int screenHeight = 240;

    const int dialogWidth = 220;
    const int dialogHeight = 100;
    const int dialogX = (screenWidth - dialogWidth) / 2;
    const int dialogY = (screenHeight - dialogHeight) / 2;

    tftManager.fillRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_BLACK);
    tftManager.drawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_WHITE);

    const int titleY = dialogY + 20;
    int16_t titleX = fontRenderer.calculateCenterPosition(dialogWidth, strConfirmOta) + dialogX;
    fontRenderer.drawChineseString(titleX, titleY, strConfirmOta, ST7789_WHITE, ST7789_BLACK);

    const int btnY = dialogY + 55;
    const int btnAreaWidth = dialogWidth - 20;
    const int backBtnWidth = fontRenderer.getStringLength(strBack) * 16;
    const int otaBtnWidth = fontRenderer.getStringLength(strOta) * 16;
    const int totalBtnWidth = backBtnWidth + 20 + otaBtnWidth;
    const int btnStartX = dialogX + (btnAreaWidth - totalBtnWidth) / 2;

    const int backTextX = btnStartX + backBtnWidth;
    const int otaTextX = backTextX + 20 + otaBtnWidth;

    if (otaConfirmDefaultBack) {
        tftManager.setCursor(btnStartX - 12, btnY);
        tftManager.setTextColor(ST7789_YELLOW);
        tftManager.print(">");
        fontRenderer.drawChineseString(btnStartX, btnY, strBack, ST7789_YELLOW, ST7789_BLACK);

        tftManager.setCursor(backTextX + 8, btnY);
        tftManager.setTextColor(ST7789_WHITE);
        tftManager.print(">");
        fontRenderer.drawChineseString(otaTextX, btnY, strOta, ST7789_WHITE, ST7789_BLACK);
    } else {
        tftManager.setCursor(btnStartX - 12, btnY);
        tftManager.setTextColor(ST7789_WHITE);
        tftManager.print(">");
        fontRenderer.drawChineseString(btnStartX, btnY, strBack, ST7789_WHITE, ST7789_BLACK);

        tftManager.setCursor(backTextX + 8, btnY);
        tftManager.setTextColor(ST7789_YELLOW);
        tftManager.print(">");
        fontRenderer.drawChineseString(otaTextX, btnY, strOta, ST7789_YELLOW, ST7789_BLACK);
    }
}

void MenuContext::hideOtaConfirmDialog() {
    Utils_Logger::info("隐藏OTA确认对话框");

    inOtaConfirm = false;
    otaConfirmDefaultBack = true;
    otaConfirmPosD = -1;

    showMenu();
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

const char* MenuContext::getOtaStateText() {
    extern OTA ota;
    const char* state = ota.getState();
    if (state == OTA::OtaState[0]) return "空闲";
    if (state == OTA::OtaState[1]) return "已接收信号";
    if (state == OTA::OtaState[2]) return "下载中...";
    if (state == OTA::OtaState[3]) return "下载完成";
    if (state == OTA::OtaState[4]) return "准备重启";
    if (state == OTA::OtaState[5]) return "升级失败";
    return "未知";
}

void MenuContext::updateOtaDisplay() {
    if (!otaInProgress) return;

    extern OTA ota;
    const char* state = ota.getState();

    tftManager.setTextColor(ST7789_CYAN, ST7789_BLACK);
    tftManager.setCursor(10, 150);
    tftManager.print("State:");
    tftManager.setCursor(80, 150);
    tftManager.print(getOtaStateText());

    IPAddress deviceIP = WiFi.localIP();
    if (deviceIP != IPAddress(0, 0, 0, 0)) {
        tftManager.setTextColor(ST7789_GREEN, ST7789_BLACK);
        tftManager.setCursor(10, 170);
        tftManager.print("Device IP:");
        tftManager.setCursor(100, 170);
        char ipBuf[16];
        sprintf(ipBuf, "%d.%d.%d.%d", deviceIP[0], deviceIP[1], deviceIP[2], deviceIP[3]);
        tftManager.print(ipBuf);
    }

    if (state == OTA::OtaState[5]) {
        tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
        tftManager.setCursor(40, 200);
        tftManager.print("OTA failed, retrying...");

        otaInProgress = false;

        delay(5000);

        showMenu();
    }
}

void MenuContext::showOtaProgressScreen() {
    Utils_Logger::info("重新显示OTA升级进度界面");

    tftManager.fillScreen(ST7789_BLACK);

    int16_t otaX = fontRenderer.calculateCenterPosition(320, strOtaProgress);
    fontRenderer.drawChineseString(otaX, 90, strOtaProgress, ST7789_YELLOW, ST7789_BLACK);

    tftManager.setTextColor(ST7789_CYAN, ST7789_BLACK);
    tftManager.setCursor(50, 130);
    tftManager.print("Device IP: ");

    if (WiFi.status() == WL_CONNECTED) {
        IPAddress ip = WiFi.localIP();
        char ipBuf[16];
        sprintf(ipBuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        tftManager.setTextColor(ST7789_GREEN, ST7789_BLACK);
        tftManager.print(ipBuf);
    }

    tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
    tftManager.setCursor(50, 180);
    tftManager.print("OTA Server: ");
    tftManager.print(otaServerIpStr);
}

void MenuContext::showOtaProgressExitDialog() {
    Utils_Logger::info("显示OTA升级中退出确认对话框");
    otaExitDialogShown = true;

    const int screenWidth = 320;
    const int screenHeight = 240;

    const int dialogWidth = 220;
    const int dialogHeight = 100;
    const int dialogX = (screenWidth - dialogWidth) / 2;
    const int dialogY = (screenHeight - dialogHeight) / 2;

    tftManager.fillRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_BLACK);
    tftManager.drawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_WHITE);

    const int titleY = dialogY + 20;
    tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
    tftManager.setCursor(dialogX + 60, titleY);
    tftManager.print("Exit OTA");

    const int btnY = dialogY + 55;
    const int btnAreaWidth = dialogWidth - 20;
    const int backBtnWidth = fontRenderer.getStringLength(strBack) * 16;
    const int confirmBtnWidth = fontRenderer.getStringLength(strConfirmExit) * 16;
    const int totalBtnWidth = backBtnWidth + 20 + confirmBtnWidth;
    const int btnStartX = dialogX + (btnAreaWidth - totalBtnWidth) / 2;

    const int backTextX = btnStartX + backBtnWidth;
    const int confirmTextX = backTextX + 20 + confirmBtnWidth;

    if (otaProgressDefaultBack) {
        tftManager.setCursor(btnStartX - 12, btnY);
        tftManager.setTextColor(ST7789_YELLOW);
        tftManager.print(">");
        fontRenderer.drawChineseString(btnStartX, btnY, strBack, ST7789_YELLOW, ST7789_BLACK);

        tftManager.setCursor(backTextX + 8, btnY);
        tftManager.setTextColor(ST7789_WHITE);
        tftManager.print(">");
        fontRenderer.drawChineseString(confirmTextX, btnY, strConfirmExit, ST7789_WHITE, ST7789_BLACK);
    } else {
        tftManager.setCursor(btnStartX - 12, btnY);
        tftManager.setTextColor(ST7789_WHITE);
        tftManager.print(">");
        fontRenderer.drawChineseString(btnStartX, btnY, strBack, ST7789_WHITE, ST7789_BLACK);

        tftManager.setCursor(backTextX + 8, btnY);
        tftManager.setTextColor(ST7789_YELLOW);
        tftManager.print(">");
        fontRenderer.drawChineseString(confirmTextX, btnY, strConfirmExit, ST7789_YELLOW, ST7789_BLACK);
    }
}

void MenuContext::handleOtaProgressExitRotation(RotationDirection direction) {
    if (!inOtaProgress) return;

    otaProgressDefaultBack = !otaProgressDefaultBack;
    showOtaProgressExitDialog();
}

void MenuContext::handleOtaProgressExitButton() {
    if (!inOtaProgress) return;

    Utils_Logger::info("[OTA_EXIT] handleOtaProgressExitButton 被调用");

    if (otaProgressDefaultBack) {
        Utils_Logger::info("[OTA_EXIT] 用户选择返回，继续OTA升级");
        otaExitDialogShown = false;
        otaProgressDefaultBack = true;
        showOtaProgressScreen();
    } else {
        Utils_Logger::info("[OTA_EXIT] 用户选择确认，退出OTA升级，释放资源并返回");

        extern OTA ota;
        ota.stop_OTA_threads();

        Utils_Logger::info("[OTA_EXIT] 正在释放WiFi资源...");
        wifi_config_autoreconnect(0, 0, 0);
        WiFi.disconnect();
        delay(500);

        otaInProgress = false;
        inOtaProgress = false;
        otaExitDialogShown = false;
        otaProgressDefaultBack = true;

        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setCursor(50, 120);
        tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
        tftManager.print("Exiting OTA...");

        delay(1000);

        triangleController.moveToPosition(TriangleController::POSITION_D);
        showMenu();
    }
}

void MenuContext::showOtaIpConfig() {
    Utils_Logger::info("[OTA_IP] 进入服务器IP配置界面");

    otaIpState = OTA_IP_STATE_SELECT;
    otaIpSelectDefaultConfirm = true;

    tftManager.fillScreen(ST7789_BLACK);

    tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
    tftManager.setTextSize(1);
    tftManager.setCursor(30, 30);
    tftManager.print("OTA Server IP Config");

    tftManager.setCursor(40, 60);
    tftManager.print("1. ");
    fontRenderer.drawChineseString(70, 60, strConfirm, ST7789_WHITE, ST7789_BLACK);

    tftManager.setCursor(40, 90);
    tftManager.print("2. ");
    fontRenderer.drawChineseString(70, 90, strSetIp, ST7789_WHITE, ST7789_BLACK);

    tftManager.setCursor(40, 120);
    tftManager.print("192.168.1.50");

    tftManager.setCursor(40, 180);
    tftManager.setTextColor(ST7789_GRAY, ST7789_BLACK);
    tftManager.print("Rotate: Select option");
    tftManager.setCursor(40, 195);
    tftManager.print("Press: Confirm");

    updateOtaIpSelectDisplay();
}

void MenuContext::updateOtaIpDisplay() {
    const int ipY = 120;
    const int fieldWidth = 50;
    const int dotWidth = 10;
    const int totalWidth = fieldWidth * 4 + dotWidth * 3;
    const int startX = (320 - totalWidth) / 2;

    tftManager.fillRectangle(0, ipY - 10, 320, 50, ST7789_BLACK);

    int currentX = startX;

    for (int i = 0; i < 4; i++) {
        bool isActive = (otaIpState == (OtaIpConfigState)(OTA_IP_STATE_FIELD_1 + i));

        if (isActive) {
            tftManager.fillRectangle(currentX - 2, ipY - 2, fieldWidth + 4, 30, ST7789_YELLOW);
            tftManager.setTextColor(ST7789_BLACK, ST7789_YELLOW);
        } else {
            tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
        }

        tftManager.setCursor(currentX + 8, ipY + 4);
        char numBuf[4];
        sprintf(numBuf, "%3d", otaServerIp[i]);
        tftManager.print(numBuf);

        currentX += fieldWidth;

        if (i < 3) {
            tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
            tftManager.setCursor(currentX + 2, ipY + 4);
            tftManager.print(".");
            currentX += dotWidth;
        }
    }

    tftManager.setTextColor(ST7789_GRAY, ST7789_BLACK);
    tftManager.setCursor(60, ipY + 40);
    tftManager.print("Field ");
    char fieldNumBuf[8];
    sprintf(fieldNumBuf, "%d/4", (int)(otaIpState - OTA_IP_STATE_FIELD_1 + 1));
    tftManager.print(fieldNumBuf);
}

void MenuContext::updateOtaIpSelectDisplay() {
    tftManager.fillRectangle(35, 55, 200, 80, ST7789_BLACK);

    if (otaIpSelectDefaultConfirm) {
        tftManager.setCursor(35, 60);
        tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
        tftManager.print(">");
        tftManager.setCursor(70, 60);
        fontRenderer.drawChineseString(70, 60, strConfirm, ST7789_YELLOW, ST7789_BLACK);

        tftManager.setCursor(70, 90);
        tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
        fontRenderer.drawChineseString(70, 90, strSetIp, ST7789_WHITE, ST7789_BLACK);
    } else {
        tftManager.setCursor(35, 60);
        tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
        tftManager.print(">");
        tftManager.setCursor(70, 60);
        fontRenderer.drawChineseString(70, 60, strConfirm, ST7789_WHITE, ST7789_BLACK);

        tftManager.setCursor(35, 90);
        tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
        tftManager.print(">");
        tftManager.setCursor(70, 90);
        fontRenderer.drawChineseString(70, 90, strSetIp, ST7789_YELLOW, ST7789_BLACK);
    }
}

void MenuContext::handleOtaIpSelectRotation(RotationDirection direction) {
    if (otaIpState != OTA_IP_STATE_SELECT) {
        return;
    }

    if (direction == ROTATION_CW) {
        otaIpSelectDefaultConfirm = false;
    } else if (direction == ROTATION_CCW) {
        otaIpSelectDefaultConfirm = true;
    }

    Utils_Logger::info("[OTA_IP] 选择界面切换: %s", otaIpSelectDefaultConfirm ? "确认" : "手动设置");

    updateOtaIpSelectDisplay();
}

void MenuContext::handleOtaIpSelectButton() {
    if (otaIpState != OTA_IP_STATE_SELECT) {
        return;
    }

    Utils_Logger::info("[OTA_IP] 确认选择: %s", otaIpSelectDefaultConfirm ? "确认(默认IP)" : "手动设置");

    if (otaIpSelectDefaultConfirm) {
        sprintf(otaServerIpStr, "%d.%d.%d.%d", otaServerIp[0], otaServerIp[1], otaServerIp[2], otaServerIp[3]);
        Utils_Logger::info("[OTA_IP] 使用默认IP: %s", otaServerIpStr);
        otaIpState = OTA_IP_STATE_SUBMITTED;
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextColor(ST7789_GREEN, ST7789_BLACK);
        tftManager.setCursor(60, 100);
        tftManager.print("Using IP:");
        tftManager.setCursor(60, 130);
        tftManager.print(otaServerIpStr);
        delay(1500);
        executeOTA();
    } else {
        otaIpState = OTA_IP_STATE_FIELD_1;
        tftManager.fillScreen(ST7789_BLACK);
        tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
        tftManager.setTextSize(1);
        tftManager.setCursor(30, 30);
        tftManager.print("OTA Server IP Config");
        tftManager.setCursor(40, 60);
        tftManager.print("Rotate: +/-1");
        tftManager.setCursor(40, 75);
        tftManager.print("Press: Confirm field");
        updateOtaIpDisplay();
    }
}

void MenuContext::handleOtaIpRotation(RotationDirection direction) {
    if (otaIpState < OTA_IP_STATE_FIELD_1 || otaIpState > OTA_IP_STATE_FIELD_4) {
        return;
    }

    int fieldIndex = otaIpState - OTA_IP_STATE_FIELD_1;

    if (direction == ROTATION_CW) {
        otaServerIp[fieldIndex] = (otaServerIp[fieldIndex] == 255) ? 0 : otaServerIp[fieldIndex] + 1;
    } else if (direction == ROTATION_CCW) {
        otaServerIp[fieldIndex] = (otaServerIp[fieldIndex] == 0) ? 255 : otaServerIp[fieldIndex] - 1;
    }

    Utils_Logger::info("[OTA_IP] 字段%d调整为: %d", fieldIndex + 1, otaServerIp[fieldIndex]);

    updateOtaIpDisplay();
}

void MenuContext::handleOtaIpButton() {
    if (otaIpState < OTA_IP_STATE_FIELD_1 || otaIpState > OTA_IP_STATE_FIELD_4) {
        return;
    }

    int fieldIndex = otaIpState - OTA_IP_STATE_FIELD_1;

    Utils_Logger::info("[OTA_IP] 确认字段%d: %d", fieldIndex + 1, otaServerIp[fieldIndex]);

    if (otaIpState == OTA_IP_STATE_FIELD_4) {
        commitOtaServerIp();
    } else {
        otaIpState = (OtaIpConfigState)(otaIpState + 1);
        updateOtaIpDisplay();
    }
}

void MenuContext::commitOtaServerIp() {
    sprintf(otaServerIpStr, "%d.%d.%d.%d",
            otaServerIp[0], otaServerIp[1], otaServerIp[2], otaServerIp[3]);

    Utils_Logger::info("[OTA_IP] IP地址已提交: %s", otaServerIpStr);

    otaIpState = OTA_IP_STATE_SUBMITTED;

    tftManager.fillRectangle(0, 160, 320, 80, ST7789_BLACK);
    tftManager.setTextColor(ST7789_GREEN, ST7789_BLACK);
    tftManager.setCursor(60, 175);
    tftManager.print("IP Saved!");
    tftManager.setCursor(60, 195);
    tftManager.print(otaServerIpStr);

    delay(1500);

    executeOTA();
}

void MenuContext::executeOTA() {
    if (otaInProgress) {
        Utils_Logger::info("OTA升级正在进行中，忽略重复请求");
        return;
    }

    Utils_Logger::info("执行系统升级（OTA）...");
    otaInProgress = true;

    inOtaConfirm = false;
    otaConfirmDefaultBack = true;
    otaConfirmPosD = -1;

    // 阶段1: 显示"升级中"界面，设置 inOtaProgress=true
    tftManager.fillScreen(ST7789_BLACK);

    int16_t otaX = fontRenderer.calculateCenterPosition(320, strOtaProgress);
    fontRenderer.drawChineseString(otaX, 90, strOtaProgress, ST7789_YELLOW, ST7789_BLACK);

    tftManager.setTextColor(ST7789_CYAN, ST7789_BLACK);
    tftManager.setCursor(50, 130);
    tftManager.print("Device IP: ");

    tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
    tftManager.setCursor(50, 180);
    tftManager.print("OTA Server: ");
    tftManager.print(otaServerIpStr);

    Utils_Logger::info("OTA升级启动...");
    Utils_Logger::info("[OTA] OTA服务器: %s", otaServerIpStr);

    extern OTA ota;
    static char server[16] = "192.168.1.50";
    strcpy(server, otaServerIpStr);
    const int otaPort = 3000;

    Utils_Logger::info("[OTA] 检查WiFi连接状态...");
    Utils_Logger::info("[OTA] 按压键可取消OTA升级");

    inOtaProgress = true;

    // 阶段2: 检查当前WiFi连接状态
    bool needConnect = true;

    if (WiFi.status() == WL_CONNECTED) {
        IPAddress currentIp = WiFi.localIP();
        if (currentIp[2] == otaServerIp[2]) {
            // 已连接且IP号段正确 → 提示已连接
            Utils_Logger::info("[OTA] WiFi已连接且IP号段正确: %d.%d.%d.%d",
                              currentIp[0], currentIp[1], currentIp[2], currentIp[3]);
            needConnect = false;
        } else {
            // 连接了但IP号段不对 → 断开后按策略重连
            Utils_Logger::info("[OTA] WiFi已连接但IP号段不对: %d != %d, 断开重连",
                              currentIp[2], otaServerIp[2]);
            wifi_config_autoreconnect(0, 0, 0);
            WiFi.disconnect();
            delay(500);
        }
    }

    // 阶段3: 如果未连接 → 按Force→Tiger→读取保存的SSID→BLE配网策略连接
    if (needConnect) {
        Utils_Logger::info("[OTA] WiFi未连接，按策略连接...");

        bool wifiConnected = false;
        const char* knownSSIDs[] = {"Force", "Tiger"};
        const char* knownPasswords[] = {"dd123456", "Dt5201314"};

        // 步骤1: 尝试Force和Tiger
        for (int i = 0; i < 2; i++) {
            Utils_Logger::info("[OTA] 扫描WiFi网络检查SSID: %s", knownSSIDs[i]);
            int scanCount = WiFi.scanNetworks();
            bool ssidExists = false;
            if (scanCount >= 0) {
                for (int s = 0; s < scanCount; s++) {
                    if (strcmp(WiFi.SSID(s), knownSSIDs[i]) == 0) {
                        ssidExists = true;
                        break;
                    }
                }
            }

            if (!ssidExists) {
                Utils_Logger::info("[OTA] SSID %s 不存在，跳过连接", knownSSIDs[i]);
                tftManager.setTextColor(ST7789_GRAY, ST7789_BLACK);
                tftManager.setCursor(50, 210);
                tftManager.print(knownSSIDs[i]);
                tftManager.print(" not found   ");
                delay(500);
                continue;
            }
            Utils_Logger::info("[OTA] SSID %s 存在，准备连接", knownSSIDs[i]);

            for (int retry = 0; retry < 3; retry++) {
                Utils_Logger::info("[OTA] 尝试连接 %s (%d/3)", knownSSIDs[i], retry + 1);

                tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
                tftManager.setCursor(50, 210);
                tftManager.print("Connecting ");
                tftManager.print(knownSSIDs[i]);
                tftManager.print("  ");
                char retryStr[8];
                sprintf(retryStr, "%d", retry + 1);
                tftManager.print(retryStr);
                tftManager.print("/3");

                char ssidBuf[33], passBuf[65];
                strncpy(ssidBuf, knownSSIDs[i], 32);
                strncpy(passBuf, knownPasswords[i], 64);
                ssidBuf[32] = '\0';
                passBuf[64] = '\0';

                WiFi.begin(ssidBuf, passBuf);

                int connectRetry = 0;
                while (connectRetry < 10 && WiFi.status() != WL_CONNECTED) {
                    delay(500);
                    connectRetry++;
                }

                if (WiFi.status() == WL_CONNECTED) {
                    IPAddress ip = WiFi.localIP();
                    if (ip[2] == otaServerIp[2]) {
                        wifiConnected = true;
                        break;
                    } else {
                        Utils_Logger::info("[OTA] IP号段不匹配: %d != %d", ip[2], otaServerIp[2]);
                        wifi_config_autoreconnect(0, 0, 0);
                        WiFi.disconnect();
                        delay(500);
                    }
                } else {
                    wifi_config_autoreconnect(0, 0, 0);
                    WiFi.disconnect();
                    delay(500);
                }
            }
            if (wifiConnected) break;
        }

        // 步骤2: 尝试保存的SSID
        if (!wifiConnected) {
            if (wifiConnector == nullptr) {
                wifiConnector = new WiFiConnector();
            }
            wifiConnector->loadSavedAPs();

            for (int i = 0; i < WIFI_CONN_MAX_SAVED_AP; i++) {
                if (!wifiConnector->getSavedAP(i).valid) continue;

                bool isKnown = (strcmp(wifiConnector->getSavedAP(i).ssid, "Force") == 0 ||
                               strcmp(wifiConnector->getSavedAP(i).ssid, "Tiger") == 0);
                if (isKnown) continue;

                Utils_Logger::info("[OTA] 尝试保存的SSID: %s", wifiConnector->getSavedAP(i).ssid);

                tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
                tftManager.setCursor(50, 210);
                tftManager.print("Saved: ");
                tftManager.print(wifiConnector->getSavedAP(i).ssid);
                tftManager.print("   ");

                for (int retry = 0; retry < 3; retry++) {
                    WiFi.begin(wifiConnector->getSavedAP(i).ssid, wifiConnector->getSavedAP(i).password);

                    int connectRetry = 0;
                    while (connectRetry < 10 && WiFi.status() != WL_CONNECTED) {
                        delay(500);
                        connectRetry++;
                    }

                    if (WiFi.status() == WL_CONNECTED) {
                        IPAddress ip = WiFi.localIP();
                        if (ip[2] == otaServerIp[2]) {
                            wifiConnected = true;
                            break;
                        } else {
                            wifi_config_autoreconnect(0, 0, 0);
                            WiFi.disconnect();
                            delay(500);
                        }
                    } else {
                        wifi_config_autoreconnect(0, 0, 0);
                        WiFi.disconnect();
                        delay(500);
                    }
                }
                if (wifiConnected) break;
            }
        }

        // 步骤3: 转跳BLE配网子模块
        if (!wifiConnected) {
            Utils_Logger::info("[OTA] 所有已知SSID失败，启动BLE WiFi配网...");

            tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
            tftManager.setCursor(50, 210);
            tftManager.print("BLE WiFi Config...   ");

            if (wifiConnector == nullptr) {
                wifiConnector = new WiFiConnector();
            }

            if (!wifiConnector->startBLEConfig()) {
                Utils_Logger::error("[OTA] BLE配网启动失败");
            } else {
                unsigned long bleStartTime = millis();
                const unsigned long bleTimeout = 120000;

                while (!wifiConnected && (millis() - bleStartTime < bleTimeout)) {
                    if (WiFi.status() == WL_CONNECTED) {
                        IPAddress ip = WiFi.localIP();
                        wifiConnector->stopBLEConfig();

                        if (ip[2] == otaServerIp[2]) {
                            wifiConnected = true;

                            String ssidStr = WiFi.SSID();
                            bool isKnownSSID = (ssidStr == "Force" || ssidStr == "Tiger");
                            if (!isKnownSSID) {
                                Utils_Logger::info("[OTA] 保存新SSID: %s", ssidStr.c_str());
                                wifiConnector->saveAP(ssidStr.c_str(), "");
                            }
                        } else {
                            Utils_Logger::info("[OTA] BLE配网连接但IP号段不对");
                            wifi_config_autoreconnect(0, 0, 0);
                            WiFi.disconnect();
                            delay(500);
                            break;
                        }
                    }

                    if (StateManager::getInstance().isButtonPressDetected()) {
                        StateManager::getInstance().setButtonPressDetected(false);
                        Utils_Logger::info("[OTA] 用户取消BLE配网");
                        wifiConnector->stopBLEConfig();
                        break;
                    }

                    delay(500);
                }

                if (!wifiConnected && wifiConnector->isBLEConfigRunning()) {
                    wifiConnector->stopBLEConfig();
                }
            }
        }

        // 阶段5: 连接失败处理
        if (!wifiConnected) {
            Utils_Logger::error("[OTA] WiFi连接失败!");

            wifi_config_autoreconnect(0, 0, 0);
            WiFi.disconnect();
            delay(500);

            otaInProgress = false;
            inOtaProgress = false;
            otaExitDialogShown = false;

            tftManager.fillScreen(ST7789_RED);
            delay(2000);
            menuContext.showMenu();
            return;
        }
    }

    // 阶段6: 连接成功 - 显示设备IP → 调用 ota.start_OTA_threads()
    IPAddress ip = WiFi.localIP();
    char ipBuf[16];
    sprintf(ipBuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    tftManager.setTextColor(ST7789_GREEN, ST7789_BLACK);
    tftManager.setCursor(140, 130);
    tftManager.print(ipBuf);
    Utils_Logger::info("[OTA] 设备IP地址: %s", ipBuf);

    ota.start_OTA_threads(otaPort, server);

    inOtaProgress = true;

    Utils_Logger::info("[OTA] 线程已启动，等待OTA服务器连接...");
    Utils_Logger::info("[OTA] 按压键可呼出退出确认对话框");
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

void MenuContext::executeBleWifiConfig()
{
    if (inBleWifiConfig) {
        Utils_Logger::info("[BLE_WIFI] BLE配网已在运行中");
        return;
    }

    inBleWifiConfig = true;
    bleWifiConfigActive = true;

    if (wifiConnector == nullptr) {
        wifiConnector = new WiFiConnector();
    }

    WiFiConnectResult checkResult = wifiConnector->checkCurrentConnection(otaServerIp[2]);

    if (checkResult == WIFI_CONN_ALREADY_CONNECTED) {
        IPAddress ip = WiFi.localIP();
        char ipBuf[16];
        sprintf(ipBuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        showBleWifiConfigScreen("WiFi Connected!");
        tftManager.setTextColor(ST7789_GREEN, ST7789_BLACK);
        tftManager.setCursor(50, 150);
        tftManager.print("IP: ");
        tftManager.print(ipBuf);
        tftManager.setCursor(50, 170);
        tftManager.print("SSID: ");
        tftManager.print(WiFi.SSID());
        delay(3000);
        inBleWifiConfig = false;
        bleWifiConfigActive = false;
        showMenu();
        return;
    }

    if (checkResult == WIFI_CONN_WRONG_IP_SEGMENT) {
        showBleWifiConfigScreen("Wrong IP, reconnect...");
        wifi_config_autoreconnect(0, 0, 0);
        WiFi.disconnect();
        delay(500);
    }

    showBleWifiConfigScreen("Connecting Force...");

    const char* knownSSIDs[] = {"Force", "Tiger"};
    const char* knownPasswords[] = {"dd123456", "Dt5201314"};

    for (int i = 0; i < 2; i++) {
        Utils_Logger::info("[BLE_WIFI] 扫描WiFi网络检查SSID: %s", knownSSIDs[i]);
        int scanCount = WiFi.scanNetworks();
        bool ssidExists = false;
        if (scanCount >= 0) {
            for (int s = 0; s < scanCount; s++) {
                if (strcmp(WiFi.SSID(s), knownSSIDs[i]) == 0) {
                    ssidExists = true;
                    break;
                }
            }
        }

        if (!ssidExists) {
            Utils_Logger::info("[BLE_WIFI] SSID %s 不存在，跳过连接", knownSSIDs[i]);
            char skipMsg[32];
            sprintf(skipMsg, "%s not found", knownSSIDs[i]);
            showBleWifiConfigScreen(skipMsg);
            delay(500);
            continue;
        }
        Utils_Logger::info("[BLE_WIFI] SSID %s 存在，准备连接", knownSSIDs[i]);

        for (int retry = 0; retry < 3; retry++) {
            if (!bleWifiConfigActive) {
                inBleWifiConfig = false;
                showMenu();
                return;
            }

            char statusMsg[32];
            sprintf(statusMsg, "Connecting %s %d/3", knownSSIDs[i], retry + 1);
            showBleWifiConfigScreen(statusMsg);

            char ssidBuf[33];
            char passBuf[65];
            strncpy(ssidBuf, knownSSIDs[i], 32);
            strncpy(passBuf, knownPasswords[i], 64);
            ssidBuf[32] = '\0';
            passBuf[64] = '\0';

            WiFi.begin(ssidBuf, passBuf);

            int connectRetry = 0;
            while (connectRetry < 10 && WiFi.status() != WL_CONNECTED) {
                if (!bleWifiConfigActive) {
                    inBleWifiConfig = false;
                    showMenu();
                    return;
                }
                delay(500);
                connectRetry++;
            }

            if (WiFi.status() == WL_CONNECTED) {
                IPAddress ip = WiFi.localIP();
                if (ip[2] == otaServerIp[2]) {
                    char ipBuf[16];
                    sprintf(ipBuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
                    showBleWifiConfigScreen("WiFi Connected!");
                    tftManager.setTextColor(ST7789_GREEN, ST7789_BLACK);
                    tftManager.setCursor(50, 150);
                    tftManager.print("IP: ");
                    tftManager.print(ipBuf);
                    tftManager.setCursor(50, 170);
                    tftManager.print("SSID: ");
                    tftManager.print(knownSSIDs[i]);
                    delay(3000);
                    inBleWifiConfig = false;
                    bleWifiConfigActive = false;
                    showMenu();
                    return;
                }
                Utils_Logger::info("[BLE_WIFI] IP segment mismatch, disconnect");
                wifi_config_autoreconnect(0, 0, 0);
                WiFi.disconnect();
                delay(500);
            } else {
                wifi_config_autoreconnect(0, 0, 0);
                WiFi.disconnect();
                delay(500);
            }
        }
    }

    showBleWifiConfigScreen("Trying saved SSIDs...");
    wifiConnector->loadSavedAPs();

    for (int i = 0; i < WIFI_CONN_MAX_SAVED_AP; i++) {
        if (!bleWifiConfigActive) {
            inBleWifiConfig = false;
            showMenu();
            return;
        }

        if (!wifiConnector->getSavedAP(i).valid) continue;

        bool isKnown = (strcmp(wifiConnector->getSavedAP(i).ssid, "Force") == 0 ||
                       strcmp(wifiConnector->getSavedAP(i).ssid, "Tiger") == 0);
        if (isKnown) continue;

        char statusMsg[48];
        sprintf(statusMsg, "Saved: %s", wifiConnector->getSavedAP(i).ssid);
        showBleWifiConfigScreen(statusMsg);

        for (int retry = 0; retry < 3; retry++) {
            if (!bleWifiConfigActive) {
                inBleWifiConfig = false;
                showMenu();
                return;
            }

            WiFi.begin(wifiConnector->getSavedAP(i).ssid, wifiConnector->getSavedAP(i).password);

            int connectRetry = 0;
            while (connectRetry < 10 && WiFi.status() != WL_CONNECTED) {
                if (!bleWifiConfigActive) {
                    inBleWifiConfig = false;
                    showMenu();
                    return;
                }
                delay(500);
                connectRetry++;
            }

            if (WiFi.status() == WL_CONNECTED) {
                IPAddress ip = WiFi.localIP();
                if (ip[2] == otaServerIp[2]) {
                    char ipBuf[16];
                    sprintf(ipBuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
                    showBleWifiConfigScreen("WiFi Connected!");
                    tftManager.setTextColor(ST7789_GREEN, ST7789_BLACK);
                    tftManager.setCursor(50, 150);
                    tftManager.print("IP: ");
                    tftManager.print(ipBuf);
                    tftManager.setCursor(50, 170);
                    tftManager.print("SSID: ");
                    tftManager.print(wifiConnector->getSavedAP(i).ssid);
                    delay(3000);
                    inBleWifiConfig = false;
                    bleWifiConfigActive = false;
                    showMenu();
                    return;
                }
                wifi_config_autoreconnect(0, 0, 0);
                WiFi.disconnect();
                delay(500);
            } else {
                wifi_config_autoreconnect(0, 0, 0);
                WiFi.disconnect();
                delay(500);
            }
        }
    }

    showBleWifiConfigScreen("BLE Config...");

    if (!wifiConnector->startBLEConfig()) {
        showBleWifiConfigScreen("BLE Start Failed!");
        delay(2000);
        inBleWifiConfig = false;
        bleWifiConfigActive = false;
        showMenu();
        return;
    }

    unsigned long bleStartTime = millis();
    const unsigned long bleTimeout = 120000;

    while (bleWifiConfigActive && (millis() - bleStartTime < bleTimeout)) {
        if (WiFi.status() == WL_CONNECTED) {
            IPAddress ip = WiFi.localIP();
            char ipBuf[16];
            sprintf(ipBuf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

            wifiConnector->stopBLEConfig();

            if (ip[2] == otaServerIp[2]) {
                String ssidStr = WiFi.SSID();
                bool isKnownSSID = (ssidStr == "Force" || ssidStr == "Tiger");

                if (!isKnownSSID) {
                    Utils_Logger::info("[BLE_WIFI] Saving new SSID: %s", ssidStr.c_str());
                    wifiConnector->saveAP(ssidStr.c_str(), "");
                }

                showBleWifiConfigScreen("WiFi Connected!");
                tftManager.setTextColor(ST7789_GREEN, ST7789_BLACK);
                tftManager.setCursor(50, 150);
                tftManager.print("IP: ");
                tftManager.print(ipBuf);
                tftManager.setCursor(50, 170);
                tftManager.print("SSID: ");
                tftManager.print(ssidStr.c_str());
                delay(3000);
                inBleWifiConfig = false;
                bleWifiConfigActive = false;
                showMenu();
                return;
            } else {
                showBleWifiConfigScreen("Wrong IP segment!");
                tftManager.setTextColor(ST7789_RED, ST7789_BLACK);
                tftManager.setCursor(50, 150);
                tftManager.print("IP: ");
                tftManager.print(ipBuf);
                delay(3000);
                wifi_config_autoreconnect(0, 0, 0);
                WiFi.disconnect();
                delay(500);
                inBleWifiConfig = false;
                bleWifiConfigActive = false;
                showMenu();
                return;
            }
        }

        unsigned long elapsed = millis() - bleStartTime;
        if (elapsed % 5000 < 500 && !bleExitDialogShown) {
            char statusMsg[32];
            sprintf(statusMsg, "BLE Waiting... %lus", (bleTimeout - elapsed) / 1000);
            showBleWifiConfigScreen(statusMsg);
        }

        if (StateManager::getInstance().isButtonPressDetected()) {
            StateManager::getInstance().setButtonPressDetected(false);
            Utils_Logger::info("[BLE_WIFI] 用户按下按钮，显示退出确认对话框");
            showBleExitDialog();
        }

        if (!bleWifiConfigActive) {
            return;
        }

        if (bleExitDialogShown) {
            delay(100);
            continue;
        }

        delay(200);
    }

    wifiConnector->stopBLEConfig();

    if (bleWifiConfigActive) {
        showBleWifiConfigScreen("BLE Timeout!");
        delay(2000);
    }

    inBleWifiConfig = false;
    bleWifiConfigActive = false;
    showMenu();
}

void MenuContext::showBleWifiConfigScreen(const char* statusMsg)
{
    if (bleExitDialogShown) {
        return;
    }

    tftManager.fillScreen(ST7789_BLACK);

    tftManager.setTextColor(ST7789_CYAN, ST7789_BLACK);
    tftManager.setTextSize(2);
    tftManager.setCursor(60, 30);
    tftManager.print("BLE WiFi Config");

    tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
    tftManager.setTextSize(1);
    tftManager.setCursor(50, 80);
    tftManager.print("Target IP seg3: ");
    char ipSegStr[8];
    sprintf(ipSegStr, "%d", otaServerIp[2]);
    tftManager.print(ipSegStr);

    tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
    tftManager.setTextSize(1);
    tftManager.setCursor(50, 110);
    tftManager.print("Status: ");
    tftManager.print(statusMsg);

    tftManager.setTextColor(ST7789_GRAY, ST7789_BLACK);
    tftManager.setCursor(50, 210);
    tftManager.print("Press btn to cancel");
}

const char* MenuContext::getTimeSyncStateText(TimeSyncState state) {
    switch (state) {
        case TIME_SYNC_IDLE:           return "Idle";
        case TIME_SYNC_CONNECTING_WIFI: return "Connecting WiFi...";
        case TIME_SYNC_CONNECTED_WIFI:  return "WiFi Connected";
        case TIME_SYNC_NTP_INIT:        return "NTP Initializing...";
        case TIME_SYNC_NTP_SYNCING:     return "Syncing NTP...";
        case TIME_SYNC_NTP_SUCCESS:     return "NTP Sync Success";
        case TIME_SYNC_NTP_FAILED:      return "NTP Sync Failed";
        case TIME_SYNC_UPDATING_DS3231: return "Updating RTC...";
        case TIME_SYNC_COMPLETE:        return "Sync Complete";
        case TIME_SYNC_DISCONNECTING:   return "Disconnecting...";
        default:                        return "Unknown";
    }
}

void MenuContext::setTimeSyncState(TimeSyncState state, const char* status, int progress, const char* server) {
    timeSyncState = state;
    if (status) {
        strncpy(timeSyncStatus, status, sizeof(timeSyncStatus) - 1);
        timeSyncStatus[sizeof(timeSyncStatus) - 1] = '\0';
    }
    timeSyncProgress = progress;
    if (server) {
        strncpy(timeSyncServer, server, sizeof(timeSyncServer) - 1);
        timeSyncServer[sizeof(timeSyncServer) - 1] = '\0';
    }
}

void MenuContext::showTimeSyncWindow() {
    inTimeSyncWindow = true;
    timeSyncState = TIME_SYNC_IDLE;
    timeSyncProgress = 0;
    strcpy(timeSyncStatus, "Initializing...");
    strcpy(timeSyncServer, "-");
    timeSyncStartTime = millis();

    g_timeSyncStatus.state = TIME_SYNC_IDLE;
    g_timeSyncStatus.progress = 0;
    strcpy(g_timeSyncStatus.status, "Initializing...");
    strcpy(g_timeSyncStatus.server, "-");
    g_timeSyncStatus.updated = false;

    tftManager.fillScreen(ST7789_BLACK);

    tftManager.setTextColor(ST7789_CYAN, ST7789_BLACK);
    tftManager.setTextSize(2);
    tftManager.setCursor(70, 15);
    tftManager.print("Time Sync");

    tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
    tftManager.setTextSize(2);
    tftManager.setCursor(80, 50);
    tftManager.print("00:00:00");

    tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
    tftManager.setTextSize(1);
    tftManager.setCursor(50, 85);
    tftManager.print("----------------------");

    tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
    tftManager.setCursor(50, 100);
    tftManager.print("State: ");
    tftManager.print(timeSyncStatus);

    tftManager.setCursor(50, 120);
    tftManager.print("Server: ");
    tftManager.print(timeSyncServer);

    tftManager.setCursor(50, 140);
    tftManager.print("Progress: ");
    tftManager.fillRectangle(100, 138, 80, 16, ST7789_BLACK);
    tftManager.setCursor(115, 140);
    char progStr[8];
    sprintf(progStr, "%d%%", timeSyncProgress);
    tftManager.print(progStr);

    tftManager.fillRectangle(50, 155, 220, 12, ST7789_GRAY);
    tftManager.fillRectangle(50, 155, 220 * timeSyncProgress / 100, 12, ST7789_GREEN);

    tftManager.setCursor(50, 175);
    tftManager.setTextColor(ST7789_GRAY, ST7789_BLACK);
    tftManager.print("Press switch to close");

    updateTimeSyncWindow();
}

void MenuContext::hideTimeSyncWindow() {
    inTimeSyncWindow = false;
    timeSyncButtonHandled = false;
}

void MenuContext::updateTimeSyncWindow() {
    DS3231_Time dsTime;
    if (clockModule.readTime(dsTime)) {
        char timeStr[16];
        sprintf(timeStr, "%02d:%02d:%02d", dsTime.hours, dsTime.minutes, dsTime.seconds);
        strncpy(currentTimeStr, timeStr, sizeof(currentTimeStr) - 1);

        tftManager.fillRectangle(78, 48, 140, 20, ST7789_BLACK);
        tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
        tftManager.setTextSize(2);
        tftManager.setCursor(80, 50);
        tftManager.print(timeStr);
        tftManager.setTextSize(1);
    }

    timeSyncState = g_timeSyncStatus.state;
    strncpy(timeSyncStatus, g_timeSyncStatus.status, sizeof(timeSyncStatus) - 1);
    timeSyncProgress = g_timeSyncStatus.progress;
    strncpy(timeSyncServer, g_timeSyncStatus.server, sizeof(timeSyncServer) - 1);

    tftManager.fillRectangle(95, 98, 200, 16, ST7789_BLACK);
    tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
    tftManager.setCursor(95, 100);
    tftManager.print(timeSyncStatus);

    tftManager.fillRectangle(108, 118, 180, 16, ST7789_BLACK);
    tftManager.setCursor(108, 120);
    tftManager.print(timeSyncServer);

    tftManager.fillRectangle(100, 138, 80, 16, ST7789_BLACK);
    tftManager.setCursor(115, 140);
    char progStr[8];
    sprintf(progStr, "%d%%", timeSyncProgress);
    tftManager.print(progStr);

    tftManager.fillRectangle(50, 155, 220, 12, ST7789_GRAY);
    uint16_t progressColor = (timeSyncState == TIME_SYNC_COMPLETE) ? ST7789_GREEN :
                             (timeSyncState == TIME_SYNC_NTP_FAILED) ? ST7789_RED : ST7789_GREEN;
    tftManager.fillRectangle(50, 155, 220 * timeSyncProgress / 100, 12, progressColor);
}

void MenuContext::updateBleWifiConfigDisplay()
{
}

void MenuContext::showBleExitDialog()
{
    Utils_Logger::info("[BLE_EXIT] 显示BLE配网退出确认对话框");
    bleExitDialogShown = true;
    bleExitDefaultBack = true;

    const int screenWidth = 320;
    const int screenHeight = 240;

    const int dialogWidth = 220;
    const int dialogHeight = 100;
    const int dialogX = (screenWidth - dialogWidth) / 2;
    const int dialogY = (screenHeight - dialogHeight) / 2;

    tftManager.fillRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_BLACK);
    tftManager.drawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, ST7789_WHITE);

    const int titleY = dialogY + 20;
    tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
    tftManager.setCursor(dialogX + 55, titleY);
    tftManager.print("Exit BLE WiFi?");

    const int btnY = dialogY + 55;
    const int btnAreaWidth = dialogWidth - 20;
    const int backBtnWidth = fontRenderer.getStringLength(strBack) * 16;
    const int confirmBtnWidth = fontRenderer.getStringLength(strConfirmExit) * 16;
    const int totalBtnWidth = backBtnWidth + 20 + confirmBtnWidth;
    const int btnStartX = dialogX + (btnAreaWidth - totalBtnWidth) / 2;

    const int backTextX = btnStartX + backBtnWidth;
    const int confirmTextX = backTextX + 20 + confirmBtnWidth;

    if (bleExitDefaultBack) {
        tftManager.setCursor(btnStartX - 12, btnY);
        tftManager.setTextColor(ST7789_YELLOW);
        tftManager.print(">");
        fontRenderer.drawChineseString(btnStartX, btnY, strBack, ST7789_YELLOW, ST7789_BLACK);

        tftManager.setCursor(backTextX + 8, btnY);
        tftManager.setTextColor(ST7789_WHITE);
        tftManager.print(">");
        fontRenderer.drawChineseString(confirmTextX, btnY, strConfirmExit, ST7789_WHITE, ST7789_BLACK);
    } else {
        tftManager.setCursor(btnStartX - 12, btnY);
        tftManager.setTextColor(ST7789_WHITE);
        tftManager.print(">");
        fontRenderer.drawChineseString(btnStartX, btnY, strBack, ST7789_WHITE, ST7789_BLACK);

        tftManager.setCursor(backTextX + 8, btnY);
        tftManager.setTextColor(ST7789_YELLOW);
        tftManager.print(">");
        fontRenderer.drawChineseString(confirmTextX, btnY, strConfirmExit, ST7789_YELLOW, ST7789_BLACK);
    }
}

void MenuContext::handleBleExitRotation(RotationDirection direction)
{
    if (!inBleWifiConfig) return;

    if (bleExitDialogShown) {
        bleExitDefaultBack = !bleExitDefaultBack;
        showBleExitDialog();
    } else {
        showBleExitDialog();
    }
}

void MenuContext::handleBleExitButton()
{
    if (!inBleWifiConfig) return;

    Utils_Logger::info("[BLE_EXIT] handleBleExitButton 被调用");

    if (bleExitDefaultBack) {
        Utils_Logger::info("[BLE_EXIT] 用户选择返回，继续BLE配网");
        bleExitDialogShown = false;
        bleExitDefaultBack = true;
        showBleWifiConfigScreen("Configuring...");
    } else {
        Utils_Logger::info("[BLE_EXIT] 用户选择确认，退出BLE配网");
        bleExitDialogShown = false;
        bleExitDefaultBack = true;
        stopBleWifiConfig();
    }
}

void MenuContext::stopBleWifiConfig()
{
    Utils_Logger::info("[BLE_WIFI] 停止BLE配网");

    bleWifiConfigActive = false;

    if (wifiConnector != nullptr) {
        wifiConnector->cancel();
        if (wifiConnector->isBLEConfigRunning()) {
            wifiConnector->stopBLEConfig();
        }
    }

    wifi_config_autoreconnect(0, 0, 0);
    WiFi.disconnect();
    delay(500);

    inBleWifiConfig = false;
    bleExitDialogShown = false;
    bleExitDefaultBack = true;

    showMenu();
    triangleController.moveToPosition(TriangleController::POSITION_C);
}

void MenuContext::showTransferModeSelect() {
    Utils_Logger::info("[TRANSFER] 进入传输模式选择界面");

    inTransferModeSelect = true;
    transferModeSelectIndex = 0;

    tftManager.fillScreen(ST7789_BLACK);

    int16_t titleX = fontRenderer.calculateCenterPosition(320, strTransferMode);
    fontRenderer.drawChineseString(titleX, 30, strTransferMode, ST7789_WHITE, ST7789_BLACK);

    const int optionY[3] = {90, 145, 200};
    const char* labels[3] = {"USB", "WEB", ""};
    const int triX = 60;
    const int asciiCharWidth = 16;
    const int screenWidth = 320;

    for (int i = 0; i < 3; i++) {
        if (transferModeSelectIndex == i) {
            tftManager.setCursor(triX, optionY[i]);
            tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
            tftManager.print(">");
        } else {
            tftManager.setCursor(triX, optionY[i]);
            tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
            tftManager.print(" ");
        }

        if (i == 2) {
            int16_t backX = fontRenderer.calculateCenterPosition(320, strBack);
            if (transferModeSelectIndex == 2) {
                fontRenderer.drawChineseString(backX, optionY[i], strBack, ST7789_YELLOW, ST7789_BLACK);
            } else {
                fontRenderer.drawChineseString(backX, optionY[i], strBack, ST7789_WHITE, ST7789_BLACK);
            }
        } else {
            tftManager.setTextSize(2);
            int labelLen = strlen(labels[i]);
            int labelPixelWidth = labelLen * asciiCharWidth;
            int16_t labelX = (screenWidth - labelPixelWidth) / 2;
            if (transferModeSelectIndex == i) {
                tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
            } else {
                tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
            }
            tftManager.setCursor(labelX, optionY[i]);
            tftManager.print(labels[i]);
            tftManager.setTextSize(1);
        }
    }
}

void MenuContext::updateTransferModeSelectDisplay() {
    const int optionY[3] = {90, 145, 200};
    const char* labels[3] = {"USB", "WEB", ""};
    const int triX = 60;
    const int asciiCharWidth = 16;
    const int screenWidth = 320;

    tftManager.fillRectangle(40, 80, 240, 130, ST7789_BLACK);

    for (int i = 0; i < 3; i++) {
        if (transferModeSelectIndex == i) {
            tftManager.setCursor(triX, optionY[i]);
            tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
            tftManager.print(">");
        } else {
            tftManager.setCursor(triX, optionY[i]);
            tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
            tftManager.print(" ");
        }

        if (i == 2) {
            int16_t backX = fontRenderer.calculateCenterPosition(320, strBack);
            if (transferModeSelectIndex == 2) {
                fontRenderer.drawChineseString(backX, optionY[i], strBack, ST7789_YELLOW, ST7789_BLACK);
            } else {
                fontRenderer.drawChineseString(backX, optionY[i], strBack, ST7789_WHITE, ST7789_BLACK);
            }
        } else {
            tftManager.setTextSize(2);
            int labelLen = strlen(labels[i]);
            int labelPixelWidth = labelLen * asciiCharWidth;
            int16_t labelX = (screenWidth - labelPixelWidth) / 2;
            if (transferModeSelectIndex == i) {
                tftManager.setTextColor(ST7789_YELLOW, ST7789_BLACK);
            } else {
                tftManager.setTextColor(ST7789_WHITE, ST7789_BLACK);
            }
            tftManager.setCursor(labelX, optionY[i]);
            tftManager.print(labels[i]);
            tftManager.setTextSize(1);
        }
    }
}

void MenuContext::handleTransferModeSelectRotation(RotationDirection direction) {
    if (!inTransferModeSelect) {
        return;
    }

    if (direction == ROTATION_CW) {
        transferModeSelectIndex = (transferModeSelectIndex + 1) % 3;
    } else if (direction == ROTATION_CCW) {
        transferModeSelectIndex = (transferModeSelectIndex + 2) % 3;
    }

    const char* optionNames[3] = {"USB", "WEB", "返回"};
    Utils_Logger::info("[TRANSFER] 传输模式切换: %s", optionNames[transferModeSelectIndex]);

    updateTransferModeSelectDisplay();
}

void MenuContext::handleTransferModeSelectButton() {
    if (!inTransferModeSelect) {
        return;
    }

    const char* optionNames[3] = {"USB", "WEB", "返回"};
    Utils_Logger::info("[TRANSFER] 确认选择: %s", optionNames[transferModeSelectIndex]);

    inTransferModeSelect = false;

    if (transferModeSelectIndex == 0) {
        usbMassStorageModule.enter();
    } else if (transferModeSelectIndex == 1) {
        Utils_Logger::info("[TRANSFER] 启动WiFi文件服务器...");
        if (TaskFactory::createDefaultTask(TaskManager::TASK_FUNCTION_E)) {
            StateManager::getInstance().setCurrentState(STATE_CAMERA_PREVIEW);
        }
    } else {
        Utils_Logger::info("[TRANSFER] 返回主菜单，释放文件传输功能资源...");
        returnFromTransferMode();
    }
}

void MenuContext::returnFromUsbMode() {
    Utils_Logger::info("[USB_EXIT] 从USB模式返回主菜单");

    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    StateManager::getInstance().setButtonPressDetected(false);

    menuManager.switchToPageByType(MENU_PAGE_MAIN);
    triangleController.moveToPosition(TriangleController::POSITION_E);
    showMenu();

    Utils_Logger::info("[USB_EXIT] 已返回主菜单，三角形指向选项E");
}

void MenuContext::returnFromTransferMode() {
    Utils_Logger::info("[TRANSFER_EXIT] 从文件传输功能返回主菜单");

    inTransferModeSelect = false;

    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    StateManager::getInstance().setButtonPressDetected(false);

    menuManager.switchToPageByType(MENU_PAGE_MAIN);
    triangleController.moveToPosition(TriangleController::POSITION_E);
    showMenu();

    Utils_Logger::info("[TRANSFER_EXIT] 已返回主菜单，三角形指向选项E");
}