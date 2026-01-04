/*
 * System_StateManager.h - 系统状态管理器
 * 统一管理所有系统状态变量
 * 实现状态切换和持久化
 * 模块化移植：阶段八 - 系统模块
 */

#ifndef SYSTEM_STATEMANAGER_H
#define SYSTEM_STATEMANAGER_H

#include "Shared_Types.h"
#include "Utils_Logger.h"

class StateManager {
public:
    // 单例模式获取实例
    static StateManager& getInstance();
    
    // 初始化状态管理器
    bool init();
    
    // 状态获取方法
    SystemState getCurrentState() const;
    bool isButtonPressDetected() const;
    bool isSDCardInitialized() const;
    bool isCameraModeActive() const;
    
    // 状态设置方法
    void setCurrentState(SystemState state);
    void setButtonPressDetected(bool detected);
    void setSDCardInitialized(bool initialized);
    void setCameraModeActive(bool active);
    
    // 状态切换方法
    void switchToMainMenu();
    void switchToSubMenu();
    void switchToCameraPreview();
    
    // 状态验证方法
    bool isValidState(SystemState state) const;
    
    // 状态持久化（可选）
    bool saveState() const;
    bool loadState();
    
    // 清理资源
    void cleanup();
    
private:
    // 私有构造函数（单例模式）
    StateManager();
    
    // 禁止复制构造和赋值操作
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    
    // 静态实例（饿汉式单例）
    static StateManager m_instance;
    
    // 系统状态变量
    SystemState m_currentState;
    bool m_buttonPressDetected;
    bool m_sdcardInitialized;
    bool m_cameraModeActive;
    
    // Utils_Logger是静态类，直接使用静态方法调用
};

#endif // SYSTEM_STATEMANAGER_H