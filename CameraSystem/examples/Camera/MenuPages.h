/*
 * MenuPages.h - 菜单页面数据头文件
 * 定义菜单页面数据结构和统一访问接口
 * 阶段六：菜单模块优化 - 菜单页面数据集中化
 */

#ifndef MENU_PAGES_H
#define MENU_PAGES_H

#include <Arduino.h>

// 菜单页面类型枚举
typedef enum {
    MENU_PAGE_MAIN = 0,     // 主菜单页面
    MENU_PAGE_SUB = 1,      // 子菜单页面
    MENU_PAGE_COUNT         // 菜单页面总数
} MenuPageType;

// 菜单页面数据结构
typedef struct {
    const uint16_t *imageData;    // 页面图像数据
    size_t dataSize;             // 数据大小
    int width;                   // 宽度
    int height;                  // 高度
    MenuPageType type;           // 页面类型
} MenuPageData;

// 菜单页面数据声明（实际数据在Menu_MM.h和Menu_SM.h中定义）
extern const MenuPageData menuPages[MENU_PAGE_COUNT];

#endif // MENU_PAGES_H
