@echo off
chcp 65001 >nul
echo === 图片处理全自动流水线 ===
echo 功能: 搜索.png → 转换格式 → 生成配置
echo.

REM 检查Python是否安装
python --version >nul 2>&1
if errorlevel 1 (
    echo 错误: 未找到Python，请先安装Python
    pause
    exit /b 1
)

REM 运行全自动流水线脚本
python auto_image_pipeline.py

if errorlevel 1 (
    echo.
    echo ❌ 全自动处理失败
    pause
    exit /b 1
)

echo.
echo ✅ 全自动处理完成！
echo 请将生成的 ImageConfig.h 文件复制到项目目录替换旧文件
echo.
pause