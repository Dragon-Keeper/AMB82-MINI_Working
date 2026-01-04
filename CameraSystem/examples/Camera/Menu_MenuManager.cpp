/*
 * Menu_MenuManager.cpp - 菜单管理器实现
 * 封装菜单页面管理、切换和显示功能
 * 阶段六：菜单模块优化 - 菜单管理器重构
 */

#include "Menu_MenuManager.h"

// 初始化菜单管理器
bool MenuManager::init()
{
    Utils_Logger::info("[MenuManager] 初始化");
    
    // 默认使用全局定义的菜单页面数据
    setMenuPages(::menuPages, MENU_PAGE_COUNT);
    
    // 根据当前页面类型设置默认菜单项配置
    updateMenuConfigForPage(currentPageIndex);
    
    Utils_Logger::info("[MenuManager] 初始化完成");
    return true;
}

// 设置菜单页面数据
void MenuManager::setMenuPages(const MenuPageData *pages, int count)
{
    if (pages == nullptr || count <= 0) {
        Utils_Logger::error("[MenuManager] 错误: 无效的菜单页面数据");
        return;
    }
    
    menuPages = pages;
    menuCount = count;
    currentPageIndex = 0;
    
    // 更新菜单项配置
    updateMenuConfigForPage(currentPageIndex);
    
    Utils_Logger::info("[MenuManager] 设置 %d 个菜单页面", menuCount);
}

// 设置当前菜单项配置
void MenuManager::setCurrentMenuConfig(const MenuConfig *config)
{
    currentMenuConfig = config;
    if (config != nullptr) {
        Utils_Logger::info("[MenuManager] 设置菜单项配置，共 %d 个菜单项", config->menuItemCount);
    } else {
        Utils_Logger::info("[MenuManager] 清除菜单项配置");
    }
}

// 根据页面索引更新菜单项配置
void MenuManager::updateMenuConfigForPage(int pageIndex)
{
    if (pageIndex < 0 || pageIndex >= menuCount || menuPages == nullptr) {
        return;
    }
    
    const MenuPageData &page = menuPages[pageIndex];
    
    // 根据页面类型设置对应的菜单项配置
    switch (page.type) {
        case MENU_PAGE_MAIN:
            setCurrentMenuConfig(&mainMenuConfig);
            break;
        case MENU_PAGE_SUB:
            setCurrentMenuConfig(&subMenuConfig);
            break;
        default:
            setCurrentMenuConfig(nullptr);
            break;
    }
}

// 显示当前菜单页面
void MenuManager::showCurrentMenu()
{
    if (menuPages == nullptr || menuCount == 0) {
        Utils_Logger::error("[MenuManager] 错误: 没有可用的菜单页面");
        return;
    }
    
    if (currentPageIndex < 0 || currentPageIndex >= menuCount) {
        currentPageIndex = 0;
        Utils_Logger::info("[MenuManager] 页面索引超出范围，重置为0");
    }
    
    const MenuPageData &page = menuPages[currentPageIndex];
    
    if (!page.imageData || page.dataSize == 0) {
        Utils_Logger::error("[MenuManager] 错误: 当前菜单页面数据无效");
        return;
    }
    
    if (page.width != tftManager.getTFT().getWidth() || page.height != tftManager.getTFT().getHeight()) {
        Utils_Logger::info("[MenuManager] 菜单尺寸不匹配 %dx%d vs 屏幕 %dx%d", 
                             page.width, page.height, 
                             tftManager.getTFT().getWidth(), tftManager.getTFT().getHeight());
    }
    
    Utils_Logger::info("[MenuManager] 显示菜单页面 %d/%d (类型: %d)", 
                      currentPageIndex + 1, menuCount, page.type);
    
    // 使用DMA方式显示图像
    tftManager.drawBitmap(0, 0, page.width, page.height, page.imageData);
}

// 切换到下一个菜单页面
void MenuManager::nextMenu()
{
    if (menuCount > 1) {
        currentPageIndex = (currentPageIndex + 1) % menuCount;
        Utils_Logger::info("[MenuManager] 切换到下一个页面: %d/%d", 
                          currentPageIndex + 1, menuCount);
        
        // 更新菜单项配置
        updateMenuConfigForPage(currentPageIndex);
        
        showCurrentMenu();
    }
}

// 切换到上一个菜单页面
void MenuManager::previousMenu()
{
    if (menuCount > 1) {
        currentPageIndex = (currentPageIndex - 1 + menuCount) % menuCount;
        Utils_Logger::info("[MenuManager] 切换到上一个页面: %d/%d", 
                          currentPageIndex + 1, menuCount);
        
        // 更新菜单项配置
        updateMenuConfigForPage(currentPageIndex);
        
        showCurrentMenu();
    }
}

// 直接切换到指定页面
void MenuManager::switchToPage(int pageIndex)
{
    if (pageIndex >= 0 && pageIndex < menuCount) {
        currentPageIndex = pageIndex;
        Utils_Logger::info("[MenuManager] 切换到指定页面: %d/%d", 
                          currentPageIndex + 1, menuCount);
        
        // 更新菜单项配置
        updateMenuConfigForPage(currentPageIndex);
        
        showCurrentMenu();
    } else {
        Utils_Logger::error("[MenuManager] 错误: 页面索引超出范围");
    }
}

// 直接切换到指定类型的页面
void MenuManager::switchToPageByType(MenuPageType type)
{
    for (int i = 0; i < menuCount; i++) {
        if (menuPages[i].type == type) {
            switchToPage(i);
            return;
        }
    }
    
    Utils_Logger::error("[MenuManager] 错误: 未找到指定类型的页面 (%d)", type);
}

// 获取当前页面对应的菜单项配置
const MenuConfig *MenuManager::getCurrentMenuConfig() const
{
    return currentMenuConfig;
}

// 获取当前页面类型
MenuPageType MenuManager::getCurrentPageType() const
{
    if (menuPages != nullptr && currentPageIndex >= 0 && currentPageIndex < menuCount) {
        return menuPages[currentPageIndex].type;
    }
    return static_cast<MenuPageType>(-1); // 返回无效类型
}
