# 英文UI内容本地化清单

> 扫描日期: 2026-04-20
> 项目: AMB82-MINI 相机控制系统
> 目的: 为后续中文本地化替换工作提供准确依据

---

## 一、USB MSC 模式 (USB_MassStorageModule.cpp)

| 序号 | 英文内容 | 译文 | 所在函数 | 屏幕位置 | 上下文说明 |
|------|---------|------|---------|---------|-----------|
| 1 | `USB MSC Mode` | 'U盘模式' | enter() / updateDisplay() | 屏幕顶部居中(y=20/y=40) | USB MSC模式标题 |
| 2 | `USB Connected!` | 'USB已连接！' | updateDisplay() | 屏幕中部(y=90) | USB已连接提示 |
| 3 | `PC can access SD card` | '电脑可访问SD卡' | updateDisplay() | 屏幕中部(y=130) | 电脑可访问SD卡提示 |
| 4 | `Rotate: Exit dialog` | '旋转：退出对话框' | updateDisplay() | 屏幕底部(y=170) | 操作提示：旋转退出对话框 |
| 5 | `Exit USB MSC?` | '退出U盘模式？' | showExitDialog() | 对话框标题(y=dialogY+20) | 退出USB MSC确认对话框标题 |

## 二、OTA 升级相关 (Menu_MenuContext.cpp)

| 序号 | 英文内容 | 译文 | 所在函数 | 屏幕位置 | 上下文说明 |
|------|---------|------|---------|---------|-----------|
| 6 | `State:` | '状态：' | updateOtaDisplay() | 屏幕中部(y=150) | OTA状态标签 |
| 7 | `Device IP:` | '设备IP：' | updateOtaDisplay() / showOtaProgressScreen() | 屏幕中部(y=170/y=130) | 设备IP地址标签 |
| 8 | `OTA failed, retrying...` | 'OTA升级失败，正在重试...' | updateOtaDisplay() | 屏幕底部(y=200) | OTA升级失败提示 |
| 9 | `OTA Server: ` | 'OTA服务器：' | showOtaProgressScreen() / executeOTA() | 屏幕中部(y=180) | OTA服务器地址标签 |
| 10 | `Exit OTA` | '退出OTA' | showOtaProgressExitDialog() | 对话框标题(y=dialogY+20) | 退出OTA确认对话框标题 |
| 11 | `Exiting OTA...` | '正在退出OTA...' | handleOtaProgressExitButton() | 屏幕中部(y=120) | 正在退出OTA提示 |
| 12 | `OTA Server IP Config` | 'OTA服务器IP配置' | showOtaIpConfig() / handleOtaIpSelectButton() | 屏幕顶部(y=30) | OTA服务器IP配置界面标题 |
| 13 | `1. ` | '1.' | showOtaIpConfig() | 屏幕左侧(y=60) | 选项1编号（后跟中文"确认"） |
| 14 | `2. ` | '2.' | showOtaIpConfig() | 屏幕左侧(y=90) | 选项2编号（后跟中文"设置IP"） |
| 15 | `192.168.1.50` | '192.168.1.50' | showOtaIpConfig() | 屏幕中部(y=120) | 默认OTA服务器IP地址（无需本地化） |
| 16 | `Rotate: Select option` | '旋转：选择选项' | showOtaIpConfig() | 屏幕底部(y=180) | 操作提示：旋转选择选项 |
| 17 | `Press: Confirm` | '按压：确认' | showOtaIpConfig() | 屏幕底部(y=195) | 操作提示：按压确认 |
| 18 | `Field ` + `1/4`~`4/4` | '字段 ' + `1/4`~`4/4` | updateOtaIpDisplay() | 屏幕中部(y=ipY+40) | IP字段编辑提示（如"Field 1/4"） |
| 19 | `Using IP:` | '使用IP：' | handleOtaIpSelectButton() | 屏幕中部(y=100) | 使用IP地址提示 |
| 20 | `Rotate: +/-1` | '旋转：+/-1' | handleOtaIpSelectButton() | 屏幕中部(y=60) | 操作提示：旋转增减1 |
| 21 | `Press: Confirm field` | '按压：确认字段' | handleOtaIpSelectButton() | 屏幕中部(y=75) | 操作提示：按压确认字段 |
| 22 | `IP Saved!` | 'IP已保存！' | commitOtaServerIp() | 屏幕中部(y=175) | IP地址已保存提示 |
| 23 | `Connecting ` + SSID + ` ` + 重试次数 | '正在连接 ' + SSID + ` ` + 重试次数 | executeOTA() | 屏幕底部(y=210) | WiFi连接状态提示（如"Connecting Force 1/3"） |
| 24 | `Saved: ` + SSID | '已保存：' + SSID | executeOTA() | 屏幕底部(y=210) | 尝试保存的SSID提示 |
| 25 | `BLE WiFi Config...   ` | 'BLE WiFi配网...' | executeOTA() | 屏幕底部(y=210) | BLE WiFi配网启动提示 |
| 26 | `Device IP: ` | '设备IP：' | executeOTA() | 屏幕中部(y=130) | 设备IP地址标签（OTA启动时） |

## 三、版本信息界面 (Menu_MenuContext.cpp)

| 序号 | 英文内容 | 译文 | 所在函数 | 屏幕位置 | 上下文说明 |
|------|---------|------|---------|---------|-----------|
| 27 | `Ver: ` | '版本：' | showVersionInfo() | 对话框内(textLeft, lineY) | 版本号标签（后跟版本号如"V1.47"） |
| 28 | `FW: v4.0.9` | '固件：v4.0.9' | showVersionInfo() | 对话框内(textLeft, lineY+22) | 固件版本号 |
| 29 | `Build: ` | '编译：' | showVersionInfo() | 对话框内(textLeft, lineY+44) | 编译日期标签（后跟日期） |
| 30 | `Cam: GC2053` | '摄像头：GC2053' | showVersionInfo() | 对话框内(textLeft, lineY+66) | 摄像头型号（无需本地化） |
| 31 | `Board: AMB82-MINI` | '板卡：AMB82-MINI' | showVersionInfo() | 对话框内(textLeft, lineY+88) | 开发板型号（无需本地化） |

## 四、BLE WiFi 配网界面 (Menu_MenuContext.cpp)

| 序号 | 英文内容 | 译文 | 所在函数 | 屏幕位置 | 上下文说明 |
|------|---------|------|---------|---------|-----------|
| 32 | `BLE WiFi Config` | 'BLE WiFi配网' | showBleWifiConfigScreen() | 屏幕顶部居中(y=30) | BLE WiFi配网界面标题 |
| 33 | `Target IP seg3: ` | '目标IP第3段：' | showBleWifiConfigScreen() | 屏幕中部(y=80) | 目标IP第3段标签 |
| 34 | `Status: ` | '状态：' | showBleWifiConfigScreen() | 屏幕中部(y=110) | 状态标签（后跟状态消息） |
| 35 | `Press btn to cancel` | '按压按钮取消' | showBleWifiConfigScreen() | 屏幕底部(y=210) | 操作提示：按压按钮取消 |
| 36 | `Exit BLE WiFi?` | '退出BLE WiFi配网？' | showBleExitDialog() | 对话框标题(y=dialogY+20) | 退出BLE WiFi确认对话框标题 |
| 37 | `WiFi Connected!` | 'WiFi已连接！' | executeBleWifiConfig() | 状态消息 | WiFi已连接成功提示 |
| 38 | `IP: ` | 'IP：' | executeBleWifiConfig()（多处） | 屏幕中部(y=150) | IP地址标签 |
| 39 | `SSID: ` | 'SSID：' | executeBleWifiConfig()（多处） | 屏幕中部(y=170) | SSID标签 |
| 40 | `Wrong IP, reconnect...` | 'IP错误，重新连接...' | executeBleWifiConfig() | 状态消息 | IP号段不对，重新连接提示 |
| 41 | `Connecting Force...` | '正在连接Force...' | executeBleWifiConfig() | 状态消息 | 正在连接Force WiFi提示 |
| 42 | `Connecting ` + SSID + ` ` + 重试次数 | '正在连接 ' + SSID + ` ` + 重试次数 | executeBleWifiConfig() | 状态消息 | WiFi连接状态（如"Connecting Force 1/3"） |
| 43 | `Trying saved SSIDs...` | '尝试已保存的SSID...' | executeBleWifiConfig() | 状态消息 | 尝试保存的SSID提示 |
| 44 | `Saved: ` + SSID | '已保存：' + SSID | executeBleWifiConfig() | 状态消息 | 保存的SSID提示 |
| 45 | `BLE Config...` | 'BLE配网...' | executeBleWifiConfig() | 状态消息 | BLE配网进行中提示 |
| 46 | `BLE Start Failed!` | 'BLE启动失败！' | executeBleWifiConfig() | 状态消息 | BLE配网启动失败提示 |
| 47 | `BLE Waiting... ` + 倒计时秒数 | 'BLE等待中... ' + 倒计时秒数 | executeBleWifiConfig() | 状态消息 | BLE等待连接倒计时提示 |
| 48 | `Wrong IP segment!` | 'IP段错误！' | executeBleWifiConfig() | 状态消息 | IP号段不匹配提示 |
| 49 | `BLE Timeout!` | 'BLE超时！' | executeBleWifiConfig() | 状态消息 | BLE配网超时提示 |
| 50 | `Configuring...` | '配置中...' | handleBleExitButton() | 状态消息 | BLE配网配置中提示 |

## 五、传输模式选择界面 (Menu_MenuContext.cpp)

| 序号 | 英文内容 | 译文 | 所在函数 | 屏幕位置 | 上下文说明 |
|------|---------|------|---------|---------|-----------|
| 51 | `1. ` | '1.' | showTransferModeSelect() | 屏幕左侧(y=80) | 选项1编号（后跟"USB"） |
| 52 | `2. ` | '2.' | showTransferModeSelect() | 屏幕左侧(y=120) | 选项2编号（后跟"WEB"） |
| 53 | `Rotate: Select option` | '旋转：选择选项' | showTransferModeSelect() | 屏幕底部(y=190) | 操作提示：旋转选择选项 |
| 54 | `Press: Confirm` | '按压：确认' | showTransferModeSelect() | 屏幕底部(y=210) | 操作提示：按压确认 |
| 55 | `USB` | 'USB' | updateTransferModeSelectDisplay() | 屏幕中部(y=80) | USB传输模式选项文本（技术术语保留） |
| 56 | `WEB` | 'WEB' | updateTransferModeSelectDisplay() | 屏幕中部(y=120) | WEB传输模式选项文本（技术术语保留） |

## 六、系统关闭界面 (Menu_MenuContext.cpp)

| 序号 | 英文内容 | 译文 | 所在函数 | 屏幕位置 | 上下文说明 |
|------|---------|------|---------|---------|-----------|
| 57 | `entering deep sleep...` | '正在进入深度睡眠...' | executeShutdown() | 屏幕中部(y=150) | 进入深度睡眠提示 |

## 七、ISP 参数设置界面 (ISP_ConfigUI.cpp)

| 序号 | 英文内容 | 译文 | 所在函数 | 屏幕位置 | 上下文说明 |
|------|---------|------|---------|---------|-----------|
| 58 | `A: Exposure` | 'A: 曝光' | getMenuItemLabel() | 屏幕右侧菜单 | 曝光模式菜单项标签 |
| 59 | `B: Brightness` | 'B: 亮度' | getMenuItemLabel() | 屏幕右侧菜单 | 亮度菜单项标签 |
| 60 | `C: Contrast` | 'C: 对比度' | getMenuItemLabel() | 屏幕右侧菜单 | 对比度菜单项标签 |
| 61 | `D: Saturation` | 'D: 饱和度' | getMenuItemLabel() | 屏幕右侧菜单 | 饱和度菜单项标签 |
| 62 | `E: Reset` | 'E: 重置' | getMenuItemLabel() | 屏幕右侧菜单 | 重置菜单项标签 |
| 63 | `F: Back` | 'F: 返回' | getMenuItemLabel() | 屏幕右侧菜单 | 返回菜单项标签 |
| 64 | `Manual` | '手动' | drawMenuItem() | 参数值位置 | 手动曝光模式文本 |
| 65 | `Auto` | '自动' | drawMenuItem() | 参数值位置 | 自动曝光模式文本 |

## 八、WiFi 文件服务器界面 (WiFi_WiFiFileServer.cpp)

| 序号 | 英文内容 | 译文 | 所在函数 | 屏幕位置 | 上下文说明 |
|------|---------|------|---------|---------|-----------|
| 66 | `SSID: ` | 'SSID：' | displayInfo() | 屏幕左侧(y=35) | WiFi SSID标签 |
| 67 | `PASS: ` | '密码：' | displayInfo() | 屏幕左侧(y=50) | WiFi密码标签 |
| 68 | `IP: ` | 'IP：' | displayInfo() | 屏幕左侧(y=70) | IP地址标签 |
| 69 | `http://` | 'http://' | displayInfo() | 屏幕左侧(y=112) | 网页访问地址前缀（无需本地化） |
| 70 | `Exit File Transfer?` | '退出文件传输？' | displayConfirmExit() | 屏幕顶部居中(y=15) | 退出文件传输确认对话框标题 |
| 71 | `[*] Cancel` | '[*] 取消' | updateExitSelectionDisplay() | 屏幕中部(y=60) | 取消选项文本 |
| 72 | `[ ] Confirm` | '[ ] 确认' | updateExitSelectionDisplay() | 屏幕中部(y=85) | 确认选项文本 |
| 73 | `Rotate: Switch option` | '旋转：切换选项' | updateExitSelectionDisplay() | 屏幕底部(y=130) | 操作提示：旋转切换选项 |
| 74 | `Press: Execute action` | '按压：执行操作' | updateExitSelectionDisplay() | 屏幕底部(y=148) | 操作提示：按压执行操作 |
| 75 | `Cancelled` | '已取消' | displayCancelFeedback() | 屏幕中部(y=60) | 已取消提示 |
| 76 | `Returning to info...` | '返回信息界面...' | displayCancelFeedback() | 屏幕底部(y=100) | 返回信息界面提示 |
| 77 | `Exiting...` | '正在退出...' | displayExitProgress() | 屏幕中部(y=60) | 正在退出提示 |
| 78 | `Closing WiFi Server` | '正在关闭WiFi服务器' | displayExitProgress() | 屏幕底部(y=100) | 正在关闭WiFi服务器提示 |

## 九、媒体文件浏览与播放界面 (VideoRecorder.cpp)

| 序号 | 英文内容 | 译文 | 所在函数 | 屏幕位置 | 上下文说明 |
|------|---------|------|---------|---------|-----------|
| 79 | `SD card not inserted` | 'SD卡未插入' | enterFileListMode() / playVideoFile() / viewImageFile() | 屏幕左上(y=10) | SD卡未插入错误提示（红色文字） |
| 80 | `Invalid file name` | '无效的文件名' | playVideoFile() / viewImageFile() | 屏幕左上(y=10) | 无效文件名错误提示（红色文字） |
| 81 | `File not found` | '文件未找到' | playVideoFile() / viewImageFile() | 屏幕左上(y=10) | 文件未找到错误提示（红色文字） |
| 82 | `Unsupported video format` | '不支持的视频格式' | playVideoFile() | 屏幕左上(y=10) | 不支持的视频格式错误提示（红色文字） |
| 83 | `Memory error` | '内存错误' | viewImageFile() | 屏幕左上(y=10) | 内存分配失败错误提示（红色文字） |
| 84 | `Read error` | '读取错误' | viewImageFile() | 屏幕左上(y=10) | 文件读取失败错误提示（红色文字） |
| 85 | `Media Files` | '媒体文件' | drawFileList() | 屏幕左上(y=10) | 媒体文件列表界面标题 |
| 86 | `No media files found` | '未找到媒体文件' | drawFileList() | 屏幕中部(y=30) | 无媒体文件提示 |
| 87 | `Back` | '返回' | drawFileList() | 右上角返回按钮(BACK_BUTTON_X+10, BACK_BUTTON_Y+5) | 返回按钮文本 |
| 88 | `Decode Error` | '解码错误' | drawFileList() | 缩略图位置(x+5, y+30) | 缩略图解码失败提示（红色文字） |
| 89 | `No Preview` | '无预览' | drawFileList() | 缩略图位置(x+5, y+30) | 无预览图占位文本（白色文字） |
| 90 | `VIDEO` | '视频' | drawFileList() | 缩略图位置(x+5, y+5) | 视频类型标识（蓝色文字） |
| 91 | `IMAGE` | '图片' | drawFileList() | 缩略图位置(x+5, y=5) | 图片类型标识（绿色文字） |

## 十、菜单项标签 (Menu_MenuItems.cpp)

| 序号 | 英文内容 | 译文 | 所在位置 | 上下文说明 |
|------|---------|------|---------|-----------|
| 92 | `BLE WiFi` | 'BLE WiFi' | 子菜单第3项(dataIndex=2) | BLE WiFi配网菜单项标签（混合中英文，技术术语保留） |

## 十一、OTA 状态文本 (Menu_MenuContext.cpp - getOtaStateText())

| 序号 | 英文内容 | 译文 | 所在函数 | 上下文说明 |
|------|---------|------|---------|-----------|
| 93 | `空闲` | '空闲' | getOtaStateText() | OTA空闲状态（已为中文，无需替换） |
| 94 | `已接收信号` | '已接收信号' | getOtaStateText() | OTA已接收信号（已为中文，无需替换） |
| 95 | `下载中...` | '下载中...' | getOtaStateText() | OTA下载中（已为中文，无需替换） |
| 96 | `下载完成` | '下载完成' | getOtaStateText() | OTA下载完成（已为中文，无需替换） |
| 97 | `准备重启` | '准备重启' | getOtaStateText() | OTA准备重启（已为中文，无需替换） |
| 98 | `升级失败` | '升级失败' | getOtaStateText() | OTA升级失败（已为中文，无需替换） |

> 注: OTA状态文本已为中文，无需本地化替换，此处仅作记录。

---

## 统计汇总

| 分类 | 英文条目数 | 已本地化 | 保留英文 |
|------|-----------|---------|---------|
| USB MSC 模式 | 5 | 5 | 0 |
| OTA 升级相关 | 21 | 19 | 2 |
| 版本信息界面 | 5 | 3 | 2 |
| BLE WiFi 配网 | 19 | 19 | 0 |
| 传输模式选择 | 6 | 4 | 2 |
| 系统关闭 | 1 | 1 | 0 |
| ISP 参数设置 | 8 | 8 | 0 |
| WiFi 文件服务器 | 13 | 12 | 1 |
| 媒体文件浏览与播放 | 13 | 13 | 0 |
| 菜单项标签 | 1 | 0 | 1 |
| OTA 状态文本 | 6 | 0 | 0 |
| **合计** | **98** | **84** | **8** |

---

## 本地化优先级建议

### 高优先级（用户高频交互界面）
- USB MSC模式标题和状态提示（序号1-5）- 全部已完成
- BLE WiFi配网界面标题和状态消息（序号32-50）- 全部已完成
- 传输模式选择界面（序号51-56）- 全部已完成
- WiFi文件服务器界面（序号66-78）- 全部已完成
- 媒体文件浏览与播放界面（序号79-91）- 全部已完成

### 中优先级（功能性界面）
- OTA升级相关界面（序号6-26）- 全部已完成
- ISP参数设置界面（序号58-65）- 全部已完成
- 版本信息界面（序号27-31）- 全部已完成

### 低优先级（低频或技术性内容）
- 系统关闭提示（序号57）- 已完成
- 菜单项"BLE WiFi"（序号92）- 技术术语保留
- IP地址、版本号等技术性文本（无需本地化）

---

## 本地化注意事项

1. **技术术语保留**: 如"USB"、"WEB"、"IP"、"SSID"、"BLE"、"OTA"、"HTTP"等国际通用技术术语建议保留英文
2. **版本号保留**: 如"V1.47"、"v4.0.9"、"GC2053"、"AMB82-MINI"等版本和型号信息无需本地化
3. **IP地址保留**: 如"192.168.1.50"等IP地址无需本地化
4. **对话框选择指示符**: ">"、"[*]"、"[ ]"等UI交互符号保留
5. **动态内容**: 带有变量拼接的文本（如"Connecting " + SSID）需要整体考虑本地化方案
6. **中文字库限制**: 需确认Display_font16x16.h字库中是否包含所需中文字符的索引

---

## 待本地化代码替换清单

以下为需要实际修改代码进行替换的UI文本（共84项）：

### 序号1-5: USB MSC模式
1. "USB MSC Mode" → "U盘模式"
2. "USB Connected!" → "USB已连接！"
3. "PC can access SD card" → "电脑可访问SD卡"
4. "Rotate: Exit dialog" → "旋转：退出对话框"
5. "Exit USB MSC?" → "退出U盘模式？"

### 序号6-8: OTA升级状态
6. "State:" → "状态："
7. "Device IP:" → "设备IP："
8. "OTA failed, retrying..." → "OTA升级失败，正在重试..."

### 序号9-12: OTA升级界面
9. "OTA Server: " → "OTA服务器："
10. "Exit OTA" → "退出OTA"
11. "Exiting OTA..." → "正在退出OTA..."
12. "OTA Server IP Config" → "OTA服务器IP配置"

### 序号16-17: OTA IP配置操作提示
16. "Rotate: Select option" → "旋转：选择选项"
17. "Press: Confirm" → "按压：确认"

### 序号18-22: OTA IP编辑
18. "Field " → "字段 "
19. "Using IP:" → "使用IP："
20. "Rotate: +/-1" → "旋转：+/-1"
21. "Press: Confirm field" → "按压：确认字段"
22. "IP Saved!" → "IP已保存！"

### 序号23-26: OTA WiFi连接状态
23. "Connecting " → "正在连接 "
24. "Saved: " → "已保存："
25. "BLE WiFi Config..." → "BLE WiFi配网..."
26. "Device IP: " → "设备IP："

### 序号27-29: 版本信息
27. "Ver: " → "版本："
28. "FW: v4.0.9" → "固件：v4.0.9"
29. "Build: " → "编译："

### 序号32-50: BLE WiFi配网（全部19项）
32. "BLE WiFi Config" → "BLE WiFi配网"
33. "Target IP seg3: " → "目标IP第3段："
34. "Status: " → "状态："
35. "Press btn to cancel" → "按压按钮取消"
36. "Exit BLE WiFi?" → "退出BLE WiFi配网？"
37. "WiFi Connected!" → "WiFi已连接！"
38. "IP: " → "IP："
39. "SSID: " → "SSID："
40. "Wrong IP, reconnect..." → "IP错误，重新连接..."
41. "Connecting Force..." → "正在连接Force..."
42. "Connecting " → "正在连接 "
43. "Trying saved SSIDs..." → "尝试已保存的SSID..."
44. "Saved: " → "已保存："
45. "BLE Config..." → "BLE配网..."
46. "BLE Start Failed!" → "BLE启动失败！"
47. "BLE Waiting... " → "BLE等待中... "
48. "Wrong IP segment!" → "IP段错误！"
49. "BLE Timeout!" → "BLE超时！"
50. "Configuring..." → "配置中..."

### 序号51-54: 传输模式选择
51. "1. " → "1."
52. "2. " → "2."
53. "Rotate: Select option" → "旋转：选择选项"
54. "Press: Confirm" → "按压：确认"

### 序号57: 系统关闭
57. "entering deep sleep..." → "正在进入深度睡眠..."

### 序号58-65: ISP参数设置
58. "A: Exposure" → "A: 曝光"
59. "B: Brightness" → "B: 亮度"
60. "C: Contrast" → "C: 对比度"
61. "D: Saturation" → "D: 饱和度"
62. "E: Reset" → "E: 重置"
63. "F: Back" → "F: 返回"
64. "Manual" → "手动"
65. "Auto" → "自动"

### 序号66-68,70-78: WiFi文件服务器
66. "SSID: " → "SSID："
67. "PASS: " → "密码："
68. "IP: " → "IP："
70. "Exit File Transfer?" → "退出文件传输？"
71. "[*] Cancel" → "[*] 取消"
72. "[ ] Confirm" → "[ ] 确认"
73. "Rotate: Switch option" → "旋转：切换选项"
74. "Press: Execute action" → "按压：执行操作"
75. "Cancelled" → "已取消"
76. "Returning to info..." → "返回信息界面..."
77. "Exiting..." → "正在退出..."
78. "Closing WiFi Server" → "正在关闭WiFi服务器"

### 序号79-91: 媒体文件浏览与播放（全部13项）
79. "SD card not inserted" → "SD卡未插入"
80. "Invalid file name" → "无效的文件名"
81. "File not found" → "文件未找到"
82. "Unsupported video format" → "不支持的视频格式"
83. "Memory error" → "内存错误"
84. "Read error" → "读取错误"
85. "Media Files" → "媒体文件"
86. "No media files found" → "未找到媒体文件"
87. "Back" → "返回"
88. "Decode Error" → "解码错误"
89. "No Preview" → "无预览"
90. "VIDEO" → "视频"
91. "IMAGE" → "图片"

---

## 附录：待添加到字库的中文字符清单

> 以下81个中文字符在 Font.md 翻译中使用，但尚未存在于 Display_font16x16.h 字库中，需后续添加。

| 序号 | 字符 | 拼音 | 来源翻译示例 | 序号 | 字符 | 拼音 | 来源翻译示例 |
|------|------|------|------------|------|------|------|------------|
| 1 | 板 | bǎn | 板卡 | 42 | 密 | mì | 密码 |
| 2 | 备 | bèi | 设备 | 43 | 名 | míng | 文件名 |
| 3 | 闭 | bì | 关闭 | 44 | 内 | nèi | 内存 |
| 4 | 编 | biān | 编译 | 45 | 脑 | nǎo | 电脑 |
| 5 | 不 | bù | 不支持 | 46 | 钮 | niǔ | 按钮 |
| 6 | 盘 | pán | U盘 | 47 | 配 | pèi | 配置/配网 |
| 7 | 尝 | cháng | 尝试 | 48 | 切 | qiè | 切换 |
| 8 | 超 | chāo | 超时 | 49 | 取 | qǔ | 取消 |
| 9 | 出 | chū | 退出 | 50 | 深 | shēn | 深度睡眠 |
| 10 | 的 | de | 无效的 | 51 | 摄 | shè | 摄像头 |
| 11 | 电 | diàn | 电脑 | 52 | 睡 | shuì | 睡眠 |
| 12 | 段 | duàn | 字段/第3段 | 53 | 试 | shì | 重试/尝试 |
| 13 | 等 | děng | 等待 | 54 | 持 | chí | 支持 |
| 14 | 效 | xiào | 无效 | 55 | 头 | tóu | 摄像头 |
| 15 | 框 | kuāng | 对话框 | 56 | 图 | tú | 图片 |
| 16 | 固 | gù | 固件 | 57 | 退 | tuì | 退出 |
| 17 | 关 | guān | 关闭 | 58 | 网 | wǎng | 配网 |
| 18 | 话 | huà | 对话框 | 59 | 未 | wèi | 未插入/未找到 |
| 19 | 换 | huàn | 切换 | 60 | 问 | wèn | 访问 |
| 20 | 媒 | méi | 媒体 | 61 | 无 | wú | 无预览/无效 |
| 21 | 界 | jiè | 界面 | 62 | 误 | wù | 错误 |
| 22 | 进 | jìn | 进入 | 63 | 卡 | kǎ | SD卡 |
| 23 | 可 | kě | 可以 | 64 | 体 | tǐ | 媒体 |
| 24 | 连 | lián | 连接 | 65 | 格 | gé | 格式 |
| 25 | 面 | miàn | 界面 | 66 | 操 | cāo | 操作 |
| 26 | 眠 | mián | 睡眠 | 67 | 接 | jiē | 连接 |
| 27 | 标 | biāo | 目标 | 68 | 解 | jiě | 解码 |
| 28 | 插 | chā | 插入 | 69 | 新 | xīn | 重新 |
| 29 | 错 | cuò | 错误 | 70 | 行 | xíng | 执行 |
| 30 | 待 | dài | 等待 | 71 | 压 | yā | 按压 |
| 31 | 第 | dì | 第3段 | 72 | 已 | yǐ | 已连接/已保存 |
| 32 | 到 | dào | 找到 | 73 | 译 | yì | 编译 |
| 33 | 作 | zuò | 操作 | 74 | 用 | yòng | 使用 |
| 34 | 在 | zài | 正在 | 75 | 找 | zhǎo | 找到 |
| 35 | 正 | zhèng | 正在 | 76 | 执 | zhí | 执行 |
| 36 | 字 | zì | 字段 | 77 | 支 | zhī | 支持 |
| 37 | 码 | mǎ | 密码/解码 | 78 | 已 | yǐ | 已取消 |
| 39 | 败 | bài | — | 79 | 预 | yù | — |
| 40 | 饱 | bǎo | 饱和 | 80 | 曝 | pù | 曝光 |
| 41 | 返 | fǎn | 返回 | 81 | 览 | lǎn | 浏览 |

### 字符统计

| 项目 | 数量 |
|------|------|
| 字库已有中文字符 | 67 |
| 需新增中文字符 | **81** |
| 扩充后中文字符总数 | 148 |
| 字库当前总条目 | 161 |
| 扩充后字库总条目 | 242 |

### 中文标点符号（可选）

| 标点 | Unicode | 出现次数 | 建议替代 |
|------|---------|---------|---------|
| ： | U+FF1A | ~30 | 可用 ASCII `:` 替代 |
| ！ | U+FF01 | ~8 | 可用 ASCII `!` 替代 |
| ？ | U+FF1F | ~4 | 可用 ASCII `?` 替代 |
| ， | U+FF0C | ~3 | 可用 ASCII `,` 替代 |
