/*
 * Menu_MenuItems.cpp - 菜单项数据实现
 * 定义和管理系统中的各种菜单项
 * 阶段六：菜单模块优化 - 菜单结构标准化
 */

#include "Menu_MenuItems.h"

// 主菜单项定义
static const MenuItem mainMenuItems[] = {
    {
        .label = "拍照",
        .type = MENU_ITEM_TYPE_FUNCTION,
        .operation = MENU_OPERATION_CAPTURE,
        .dataIndex = 0
    },
    {
        .label = "",
        .type = MENU_ITEM_TYPE_NONE,
        .operation = MENU_OPERATION_NONE,
        .dataIndex = 1
    },
    {
        .label = "",
        .type = MENU_ITEM_TYPE_NONE,
        .operation = MENU_OPERATION_NONE,
        .dataIndex = 2
    },
    {
        .label = "参数设置",
        .type = MENU_ITEM_TYPE_PARAM_SETTINGS,
        .operation = MENU_OPERATION_PARAM_SETTINGS,
        .dataIndex = 3
    },
    {
        .label = "",
        .type = MENU_ITEM_TYPE_NONE,
        .operation = MENU_OPERATION_NONE,
        .dataIndex = 4
    },
    {
        .label = "退出",
        .type = MENU_ITEM_TYPE_SUBMENU,
        .operation = MENU_OPERATION_SETTINGS,
        .dataIndex = 5
    }
};

// 子菜单项定义
static const MenuItem subMenuItems[] = {
    {
        .label = "图像设置",
        .type = MENU_ITEM_TYPE_SETTING,
        .operation = MENU_OPERATION_SETTINGS,
        .dataIndex = 0
    },
    {
        .label = "录像设置",
        .type = MENU_ITEM_TYPE_SETTING,
        .operation = MENU_OPERATION_SETTINGS,
        .dataIndex = 1
    },
    {
        .label = "系统设置",
        .type = MENU_ITEM_TYPE_PARAM_SETTINGS,
        .operation = MENU_OPERATION_PARAM_SETTINGS,
        .dataIndex = 2
    },
    {
        .label = "返回",
        .type = MENU_ITEM_TYPE_FUNCTION,
        .operation = MENU_OPERATION_BACK,
        .dataIndex = 3
    },
    {
        .label = "",
        .type = MENU_ITEM_TYPE_NONE,
        .operation = MENU_OPERATION_NONE,
        .dataIndex = 4
    },
    {
        .label = "",
        .type = MENU_ITEM_TYPE_NONE,
        .operation = MENU_OPERATION_NONE,
        .dataIndex = 5
    }
};

// 参数设置菜单项定义
static const MenuItem paramSettingsItems[] = {
    {
        .label = "曝光模式",
        .type = MENU_ITEM_TYPE_EXPOSURE_MODE,
        .operation = MENU_OPERATION_EXPOSURE_MODE,
        .dataIndex = 0
    },
    {
        .label = "亮度",
        .type = MENU_ITEM_TYPE_SLIDER,
        .operation = MENU_OPERATION_BRIGHTNESS,
        .dataIndex = 1
    },
    {
        .label = "对比度",
        .type = MENU_ITEM_TYPE_SLIDER,
        .operation = MENU_OPERATION_CONTRAST,
        .dataIndex = 2
    },
    {
        .label = "饱和度",
        .type = MENU_ITEM_TYPE_SLIDER,
        .operation = MENU_OPERATION_SATURATION,
        .dataIndex = 3
    },
    {
        .label = "重置默认参数",
        .type = MENU_ITEM_TYPE_FUNCTION,
        .operation = MENU_OPERATION_RESET_DEFAULTS,
        .dataIndex = 4
    },
    {
        .label = "返回主菜单",
        .type = MENU_ITEM_TYPE_FUNCTION,
        .operation = MENU_OPERATION_BACK,
        .dataIndex = 5
    }
};

// 主菜单配置
const MenuConfig mainMenuConfig = {
    .menuItemCount = sizeof(mainMenuItems) / sizeof(MenuItem),
    .menuItems = mainMenuItems
};

// 子菜单配置
const MenuConfig subMenuConfig = {
    .menuItemCount = sizeof(subMenuItems) / sizeof(MenuItem),
    .menuItems = subMenuItems
};

// 参数设置菜单配置
const MenuConfig paramSettingsConfig = {
    .menuItemCount = sizeof(paramSettingsItems) / sizeof(MenuItem),
    .menuItems = paramSettingsItems
};