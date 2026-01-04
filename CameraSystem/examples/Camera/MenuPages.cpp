/*
 * MenuPages.cpp - 菜单页面数据实现
 * 封装菜单页面数据的统一管理和访问接口
 * 阶段六：菜单模块优化 - 菜单页面数据集中化
 */

#include "MenuPages.h"

// 包含菜单页面数据
#include "Menu_MM.h" // 主菜单页面数据
#include "Menu_SM.h" // 子菜单页面数据

// 菜单页面数据定义
const MenuPageData menuPages[MENU_PAGE_COUNT] = {
    {
        // 主菜单页面
        .imageData = MM_PIC,
        .dataSize = MM_WIDTH * MM_HEIGHT * sizeof(uint16_t),
        .width = MM_WIDTH,
        .height = MM_HEIGHT,
        .type = MENU_PAGE_MAIN
    },
    {
        // 子菜单页面
        .imageData = SM_PIC,
        .dataSize = SM_WIDTH * SM_HEIGHT * sizeof(uint16_t),
        .width = SM_WIDTH,
        .height = SM_HEIGHT,
        .type = MENU_PAGE_SUB
    }
};
