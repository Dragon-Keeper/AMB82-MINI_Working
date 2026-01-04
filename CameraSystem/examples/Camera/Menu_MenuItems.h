/*
 * Menu_MenuItems.h - 菜单项定义头文件
 * 定义菜单项类型和操作类型枚举
 * 阶段六：菜单模块优化 - 菜单结构标准化
 */

#ifndef MENU_MENU_ITEMS_H
#define MENU_MENU_ITEMS_H

#include <Arduino.h>

// 菜单项类型枚举
typedef enum {
    MENU_ITEM_TYPE_NONE = 0,
    MENU_ITEM_TYPE_FUNCTION,    // 功能项
    MENU_ITEM_TYPE_SUBMENU,     // 子菜单
    MENU_ITEM_TYPE_SETTING,     // 设置项
    MENU_ITEM_TYPE_TOGGLE,      // 开关项
    MENU_ITEM_TYPE_VALUE        // 数值项
} MenuItemType;

// 菜单项操作类型
typedef enum {
    MENU_OPERATION_NONE = 0,
    MENU_OPERATION_CAPTURE,     // 拍照
    MENU_OPERATION_RECORD,      // 录像
    MENU_OPERATION_PLAYBACK,    // 回放
    MENU_OPERATION_SETTINGS,    // 设置
    MENU_OPERATION_ABOUT,       // 关于
    MENU_OPERATION_EXIT,        // 退出
    MENU_OPERATION_BACK         // 返回
} MenuOperation;

// 菜单项结构
typedef struct {
    const char *label;          // 菜单项标签
    MenuItemType type;          // 菜单项类型
    MenuOperation operation;    // 菜单项操作
    int dataIndex;              // 相关数据索引
} MenuItem;

// 菜单配置结构
typedef struct {
    int menuItemCount;          // 菜单项数量
    const MenuItem *menuItems;  // 菜单项数组
} MenuConfig;

// 全局菜单配置声明
extern const MenuConfig mainMenuConfig;
extern const MenuConfig subMenuConfig;

#endif // MENU_MENU_ITEMS_H