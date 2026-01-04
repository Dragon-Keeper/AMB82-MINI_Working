/*
 * System_StateManager.cpp - 系统状态管理器实现
 * 统一管理所有系统状态变量
 * 实现状态切换和持久化
 * 模块化移植：阶段八 - 系统模块
 */

#include "System_StateManager.h"

// 静态实例初始化（饿汉式单例）
StateManager StateManager::m_instance;

// 单例模式实现
StateManager& StateManager::getInstance() {
    return m_instance;
}

// 构造函数
StateManager::StateManager() 
    : m_currentState(STATE_MAIN_MENU)
    , m_buttonPressDetected(false)
    , m_sdcardInitialized(false)
    , m_cameraModeActive(false)
{
}

// 初始化状态管理器
bool StateManager::init() {
    Utils_Logger::info("初始化系统状态管理器");
    
    // 初始化默认状态
    m_currentState = STATE_MAIN_MENU;
    m_buttonPressDetected = false;
    m_sdcardInitialized = false;
    m_cameraModeActive = false;
    
    Utils_Logger::info("系统状态管理器初始化完成");
    return true;
}

// 获取当前系统状态
SystemState StateManager::getCurrentState() const {
    return m_currentState;
}

// 获取按钮按下状态
bool StateManager::isButtonPressDetected() const {
    return m_buttonPressDetected;
}

// 获取SD卡初始化状态
bool StateManager::isSDCardInitialized() const {
    return m_sdcardInitialized;
}

// 获取相机模式激活状态
bool StateManager::isCameraModeActive() const {
    return m_cameraModeActive;
}

// 设置当前系统状态
void StateManager::setCurrentState(SystemState state) {
    if (isValidState(state)) {
        SystemState oldState = m_currentState;
        m_currentState = state;
        
        if (oldState != state) {
            Utils_Logger::info("系统状态已从 %d 切换到 %d", oldState, state);
        }
    } else {
        Utils_Logger::error("尝试设置无效的系统状态: %d", state);
    }
}

// 设置按钮按下状态
void StateManager::setButtonPressDetected(bool detected) {
    m_buttonPressDetected = detected;
    
    if (detected) {
        Utils_Logger::info("检测到按钮按下");
    }
}

// 设置SD卡初始化状态
void StateManager::setSDCardInitialized(bool initialized) {
    m_sdcardInitialized = initialized;
    
    if (initialized) {
        Utils_Logger::info("SD卡已初始化");
    } else {
        Utils_Logger::info("SD卡未初始化");
    }
}

// 设置相机模式激活状态
void StateManager::setCameraModeActive(bool active) {
    m_cameraModeActive = active;
    
    if (active) {
        Utils_Logger::info("相机模式已激活");
    } else {
        Utils_Logger::info("相机模式已关闭");
    }
}

// 切换到主菜单
void StateManager::switchToMainMenu() {
    setCurrentState(STATE_MAIN_MENU);
    setCameraModeActive(false);
    Utils_Logger::info("切换到主菜单");
}

// 切换到子菜单
void StateManager::switchToSubMenu() {
    setCurrentState(STATE_SUB_MENU);
    Utils_Logger::info("切换到子菜单");
}

// 切换到相机预览模式
void StateManager::switchToCameraPreview() {
    setCurrentState(STATE_CAMERA_PREVIEW);
    setCameraModeActive(true);
    Utils_Logger::info("切换到相机预览模式");
}

// 验证状态是否有效
bool StateManager::isValidState(SystemState state) const {
    return (state >= STATE_MAIN_MENU && state <= STATE_CAMERA_PREVIEW);
}

// 保存状态（可选实现）
bool StateManager::saveState() const {
    // 这里可以实现状态持久化到EEPROM或文件
    Utils_Logger::info("保存系统状态");
    return true;
}

// 加载状态（可选实现）
bool StateManager::loadState() {
    // 这里可以实现从EEPROM或文件加载状态
    Utils_Logger::info("加载系统状态");
    return true;
}

// 清理资源
void StateManager::cleanup() {
    Utils_Logger::info("清理系统状态管理器");
    // 这里可以添加清理代码
}