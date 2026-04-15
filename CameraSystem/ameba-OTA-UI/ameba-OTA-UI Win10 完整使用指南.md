# ameba-OTA-UI Windows 10 完整使用指南

根据 [README.md](file:///c:\Users\Debug\AppData\Local\Arduino15\packages\realtek\hardware\AmebaPro2\4.0.9-build20250528\libraries\CameraSystem\ameba-OTA-UI\README.md) 文件提供的官方指导，以下是在 **Windows 10** 操作系统环境下运行该项目的详细步骤：

---

## 📋 一、项目概述

**ameba-OTA-UI** 是一个用于 Ameba Pro2 开发板的 OTA（Over-The-Air）升级界面工具。该项目是一个 Node.js 应用程序，主要用于通过 Web 界面管理固件的无线升级。

> ⚠️ **注意**：官方 README 原本针对 Ubuntu 22.04 和 Arduino SDK 环境，以下为适配 Windows 10 的操作说明。

---

## 🔧 二、系统环境要求

### 1. 操作系统
- **Windows 10**（64位，推荐版本 1903 或更高）

### 2. 必需软件

| 软件 | 推荐版本 | 用途 |
|------|---------|------|
| **Git** | 最新版 | 克隆代码仓库 |
| **Node.js** | v18.20.3（推荐）或 v18 LTS 系列 | 运行 JavaScript 运行时 |
| **npm** | 10.8.1（推荐）或兼容版本 | 包管理器 |

### 3. 硬件要求
- **Ameba Pro2 开发板**
- **USB 数据线**（用于连接开发板）
- **网络连接**（WiFi 或以太网）

---

## 📥 三、安装步骤（Windows 10）

### 步骤 1：检查/安装 Git

打开 **PowerShell** 或 **命令提示符（CMD）**，输入：

```powershell
git --version
```

如果未安装，请从 https://git-scm.com/download/win 下载并安装 Git for Windows。

---

### 步骤 2：安装 Node.js 和 npm

**方法一：使用 nvm-windows（推荐）**

1. 从 GitHub 下载 **nvm-windows**：
   - 访问：https://github.com/coreybutler/nvm-windows/releases
   - 下载 `nvm-setup.exe` 并安装

2. 安装完成后，**重新打开终端**，然后执行：

```powershell
nvm install v18.20.3
nvm use v18.20.3
npm install npm@10.8.1 -g
```

3. 验证安装：

```powershell
node -v    # 应显示: v18.20.3
npm -v     # 应显示: 10.8.1 (或相近版本)
```

**方法二：直接安装（备选）**

如果不想使用 nvm，可直接从 https://nodejs.org 下载 Node.js v18 LTS 安装包进行安装。

---

### 步骤 3：克隆项目仓库

```powershell
cd C:\Projects   # 或你希望存放项目的任意目录
git clone https://github.com/Ameba-AIoT/ameba-OTA-UI.git
```

或者，如果你已经在 Arduino 库目录中拥有此文件（如当前路径所示），可以直接使用现有文件：

```powershell
cd "c:\Users\Debug\AppData\Local\Arduino15\packages\realtek\hardware\AmebaPro2\4.0.9-build20250528\libraries\CameraSystem\ameba-OTA-UI"
```

---

### 步骤 4：安装项目依赖

进入项目目录后执行：

```powershell
cd ameba-OTA-UI
npm install
```

> 此命令会读取 `package.json` 文件并自动下载所有所需的依赖包。首次安装可能需要几分钟时间。

---

## 🚀 四、启动程序

### 方式一：生产模式运行（推荐用于正式使用）

按照 README 指导，依次执行以下两条命令：

```powershell
npm run build
npm start
```

**命令解析：**
- `npm run build`：将源代码编译打包成生产环境的优化版本
- `npm start`：启动 Web 服务器

### 方式二：开发模式运行（仅限开发调试时使用）

如果项目支持开发模式（需查看 package.json 中的 scripts 配置）：

```powershell
npm run dev
# 或
npm run start:dev
```

---

## 🌐 五、访问与基本使用

### 1. 打开浏览器

服务器启动成功后，在浏览器中访问：

```
http://localhost:3000
```
> （端口号可能因配置而异，请以终端输出的实际地址为准）

### 2. 主要功能界面

通常包括：
- **固件上传区域**：选择或拖拽 `.bin` 固件文件
- **设备列表/选择**：显示已连接的 Ameba 设备
- **升级控制按钮**：开始/暂停/取消 OTA 升级
- **进度显示**：实时显示升级进度和状态
- **日志输出窗口**：显示详细的操作日志

### 3. 基本操作流程

1. 确保 Ameba Pro2 开发板已上电并连接到网络
2. 在 UI 界面中选择目标设备
3. 上传待升级的固件文件（`.bin` 格式）
4. 点击"开始升级"按钮
5. 等待升级完成，观察进度条和状态信息
6. 升级成功后，设备会自动重启

---

## ⚠️ 六、常见问题排查

### 问题 1：`npm install` 失败或报错

**可能原因与解决方案：**

| 错误信息 | 解决方案 |
|---------|---------|
| `EACCES permission denied` | 以管理员身份运行 PowerShell |
| `network timeout` | 检查网络连接，或切换 npm 镜像源：<br>`npm config set registry https://registry.npmmirror.com` |
| `node-gyp build failed` | 安装 Windows 构建工具：<br>`npm install --global windows-build-tools` |

---

### 问题 2：`npm run build` 编译失败

**排查步骤：**
1. 确认 Node.js 版本是否为 v18.x（避免使用过新或过旧的版本）
2. 删除 `node_modules` 文件夹和 `package-lock.json`，重新执行：
   ```powershell
   rmdir /s /q node_modules
   del package-lock.json
   npm install
   npm run build
   ```

---

### 问题 3：端口被占用

**错误信息：** `Port 3000 is already in use`

**解决方案：**
1. 查找占用端口的进程：
   ```powershell
   netstat -ano | findstr :3000
   ```
2. 终止该进程（用上面查到的 PID 替换 `<PID>`）：
   ```powershell
   taskkill /PID <PID> /F
   ```
3. 重新运行 `npm start`

---

### 问题 4：无法连接到 Ameba 设备

**排查清单：**
- ✅ 确认设备和电脑在同一局域网内
- ✅ 检查设备的 WiFi 连接状态
- ✅ 关闭 Windows 防火墙或添加例外规则
- ✅ 确认设备 IP 地址正确（可在 Arduino 串口监视器中查看）
- ✅ 检查路由器是否开启了 AP 隔离功能（需关闭）

---

### 问题 5：升级过程中断或失败

**解决方案：**
1. 检查网络稳定性（建议使用有线连接而非 WiFi）
2. 确保固件文件完整性（重新下载固件）
3. 查看 UI 界面的错误日志获取具体原因
4. 尝试降低波特率或减小固件包大小

---

## 💡 七、进阶配置（可选）

### 修改默认端口

编辑项目根目录下的配置文件（通常是 `.env`、`config.js` 或 `server.js`），找到端口设置并修改：

```javascript
// 示例：将端口改为 8080
const PORT = process.env.PORT || 8080;
```

### 配置 HTTPS（生产环境推荐）

如果需要在安全连接下运行，可生成自签名证书并修改服务器配置启用 HTTPS。

---

## 📝 八、快速参考卡片

```powershell
# === 一键启动流程 ===

# 1. 进入项目目录
cd "c:\Users\Debug\AppData\Local\Arduino15\packages\realtek\hardware\AmebaPro2\4.0.9-build20250528\libraries\CameraSystem\ameba-OTA-UI"

# 2. 安装依赖（首次运行时）
npm install

# 3. 构建并启动
npm run build
npm start

# 4. 浏览器访问 http://localhost:3000
```

---

## 📚 九、相关资源

- **GitHub 仓库**：https://github.com/Ameba-AIoT/ameba-OTA-UI
- **Ameba 官方文档**：https://www.ambiq.com/ (或 Realtek Ameba 官方站点)
- **Node.js 官网**：https://nodejs.org/
- **npm 文档**：https://docs.npmjs.com/

---

如有其他问题，欢迎随时提问！