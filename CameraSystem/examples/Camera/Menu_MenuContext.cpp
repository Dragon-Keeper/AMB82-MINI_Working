/*
 * Menu_MenuContext.cpp - 菜单上下文实现
 * 管理菜单事件和上下文状态
 * 阶段六：菜单模块优化 - 菜单上下文管理
 */

#include "Menu_MenuContext.h"

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
    
    // 处理按钮事件（只在F位置返回主菜单）
    if (StateManager::getInstance().isButtonPressDetected()) {
        StateManager::getInstance().setButtonPressDetected(false);
        
        if (getCurrentMenuItem() == POS_F) {
            // F位置按下：返回主菜单
            Utils_Logger::info("子菜单F位置：返回主菜单");
            switchToMainMenu();
        } else if (getCurrentMenuItem() == POS_A) {
            // A位置按下：启动后台校时功能
            Utils_Logger::info("子菜单A位置：启动后台校时功能");
            
            // 检查任务状态，如果不是删除状态，就标记为删除状态
            if (TaskManager::getTaskState(TaskManager::TASK_TIME_SYNC) != TASK_STATE_DELETED) {
                Utils_Logger::info("将现有任务标记为删除状态");
                TaskManager::markTaskAsDeleting(TaskManager::TASK_TIME_SYNC);
            }
            
            // 创建新任务
            if (TaskFactory::createDefaultTask(TaskManager::TASK_TIME_SYNC)) {
                Utils_Logger::info("后台校时任务启动成功");
            } else {
                Utils_Logger::error("后台校时任务启动失败");
            }
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
    
    // 清理菜单管理器资源
    menuManager.cleanup();
    
    // 将系统状态设置为主菜单
    StateManager::getInstance().setCurrentState(STATE_MAIN_MENU);
    
    Utils_Logger::info("[MenuContext] 资源清理完成");
}