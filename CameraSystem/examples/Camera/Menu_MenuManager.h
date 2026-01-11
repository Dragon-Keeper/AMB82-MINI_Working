/*
 * Menu_MenuManager.h - 菜单管理器头文件
 * 封装菜单页面管理、切换和显示功能
 * 阶段六：菜单模块优化 - 菜单管理器重构
 */

#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <Arduino.h>
#include "Display_TFTManager.h"
#include "Utils_Logger.h"
#include "MenuPages.h"
#include "Menu_MenuItems.h"

class MenuManager
{
private:
    Display_TFTManager &tftManager; // 使用TFT管理器实例的引用
    
    // 菜单页面管理
    int currentPageIndex = 0;
    int menuCount = 0;
    
    // 预加载的菜单页面数据
    const MenuPageData *menuPages = nullptr;
    
    // 菜单项配置管理
    const MenuConfig *currentMenuConfig = nullptr;
    
    // 内部方法：根据页面索引更新菜单项配置
    void updateMenuConfigForPage(int pageIndex);
    
public:
    // 通过引用接收TFT管理器实例
    MenuManager(Display_TFTManager &tftManager) : tftManager(tftManager) {}

    // 初始化菜单管理器
    bool init();

    // 设置菜单页面数据
    void setMenuPages(const MenuPageData *pages, int count);
    
    // 设置当前菜单项配置
    void setCurrentMenuConfig(const MenuConfig *config);

    // 显示当前菜单页面
    void showCurrentMenu();

    // 切换到下一个菜单页面
    void nextMenu();

    // 切换到上一个菜单页面
    void previousMenu();

    // 直接切换到指定页面
    void switchToPage(int pageIndex);
    
    // 直接切换到指定类型的页面
    void switchToPageByType(MenuPageType type);
    
    // 获取当前页面对应的菜单项配置
    const MenuConfig *getCurrentMenuConfig() const;

    // 获取当前页面索引
    int getCurrentPageIndex() const {
        return currentPageIndex;
    }

    // 获取总页面数
    int getTotalPages() const {
        return menuCount;
    }
    
    // 获取当前页面类型
    MenuPageType getCurrentPageType() const;
    
    // 清理菜单管理器资源
    void cleanup();
};

#endif // MENU_MANAGER_H