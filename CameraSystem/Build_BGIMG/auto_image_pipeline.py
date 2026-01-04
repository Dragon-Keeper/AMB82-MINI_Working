#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
图片处理全自动流水线
自动搜索.png图片 → 自动转换格式 → 自动生成配置文件
"""

import os
import glob
import subprocess
import sys
import locale

# 设置系统默认编码
try:
    # 获取系统默认编码
    default_encoding = locale.getpreferredencoding()
    print(f"系统默认编码: {default_encoding}")
except:
    default_encoding = "utf-8"
    print("使用默认编码: utf-8")


def scan_png_files():
    """扫描当前目录下的.png图片文件"""
    print("正在扫描.png图片文件...")

    # 获取所有.png文件
    png_files = glob.glob("*.png")

    if not png_files:
        print("❌ 未找到任何.png图片文件！")
        return []

    print(f"✅ 扫描到 {len(png_files)} 个.png图片:")
    for i, file in enumerate(png_files, 1):
        print(f"   {i}. {file}")

    return png_files


def convert_images(png_files):
    """使用convert_image.py转换图片为.h文件"""
    print("\n" + "=" * 50)
    print("开始转换图片格式...")

    converted_files = []

    for png_file in png_files:
        # 生成对应的.h文件名
        h_file = png_file.replace(".png", ".h")
        h_file = "Menu_" + h_file

        print(f"\n转换: {png_file} → {h_file}")

        # 运行convert_image.py
        try:
            # 使用shell=True和正确的编码处理
            result = subprocess.run(
                f'python convert_image.py "{png_file}" "{h_file}"',
                shell=True,
                capture_output=True,
                text=True,
                encoding=default_encoding,
                errors="ignore",  # 忽略编码错误
            )

            if result.returncode == 0:
                print(f"✅ 成功转换 {png_file}")
                converted_files.append(h_file)

                # 显示有用的输出信息
                if result.stdout and result.stdout.strip():
                    # 过滤掉颜色验证的详细信息，只显示关键信息
                    lines = result.stdout.strip().split("\n")
                    for line in lines:
                        if (
                            "Image size:" in line
                            or "Generated variable names:" in line
                            or "Successfully converted" in line
                        ):
                            print(f"   {line}")
            else:
                print(f"❌ 转换失败 {png_file}")
                if result.stderr and result.stderr.strip():
                    print(f"   错误: {result.stderr.strip()}")

        except Exception as e:
            print(f"❌ 转换异常 {png_file}: {e}")

    return converted_files


def generate_config(h_files):
    """生成ImageConfig.h配置文件"""
    print("\n" + "=" * 50)
    print("开始生成配置文件...")

    # 提取图片名称（不含扩展名）
    image_names = [os.path.splitext(file)[0] for file in h_files]

    config_content = """/*
 * Camera_ImageConfig.h - 图像配置头文件
 * 统一管理所有图像变量名和配置
 * 阶段五：相机模块开发 - 图像资源配置
 */

#ifndef _IMAGE_CONFIG_H_
#define _IMAGE_CONFIG_H_

"""

    # 为每个图片生成配置块
    for name in image_names:
        prefix = name.upper()
        config_content += f"""// {name}图像配置 ({name}.png)
#define {prefix}_IMAGE_DATA {prefix}_PIC
#define {prefix}_IMAGE_SIZE sizeof({prefix}_PIC)
#define {prefix}_IMAGE_WIDTH {prefix}_WIDTH
#define {prefix}_IMAGE_HEIGHT {prefix}_HEIGHT

"""

    config_content += "#endif // _IMAGE_CONFIG_H_"

    # 写入文件
    with open("Camera_ImageConfig.h", "w", encoding="utf-8") as f:
        f.write(config_content)

    print(f"✅ 成功生成 Camera_ImageConfig.h")
    print("\n生成的宏定义:")
    for name in image_names:
        prefix = name.upper()
        print(f"  • {prefix}_IMAGE_DATA -> {prefix}_PIC")
        print(f"  • {prefix}_IMAGE_SIZE -> sizeof({prefix}_PIC)")
        print(f"  • {prefix}_IMAGE_WIDTH -> {prefix}_WIDTH")
        print(f"  • {prefix}_IMAGE_HEIGHT -> {prefix}_HEIGHT")

    return image_names


def main():
    """主函数"""
    print("=== 图片处理全自动流水线 ===")
    print("功能: 搜索.png → 转换格式 → 生成配置")
    print("-" * 50)

    # 1. 扫描.png图片
    png_files = scan_png_files()
    if not png_files:
        print("\n请将.png图片文件放在当前目录后重新运行")
        return 1

    # 确认是否继续
    confirm = input("\n是否继续执行全自动处理? (y/n): ").strip().lower()
    if confirm != "y":
        print("操作已取消")
        return 0

    # 2. 转换图片格式
    h_files = convert_images(png_files)
    if not h_files:
        print("\n❌ 图片转换失败，无法继续生成配置")
        return 1

    # 3. 生成配置文件
    image_names = generate_config(h_files)

    print("\n" + "=" * 50)
    print("🎉 全自动处理完成！")
    print(f"📊 处理统计:")
    print(f"  • 扫描到 {len(png_files)} 个.png图片")
    print(f"  • 成功转换 {len(h_files)} 个.h文件")
    print(f"  • 生成 {len(image_names)} 个图像配置")

    print(f"\n📁 生成的文件:")
    for h_file in h_files:
        print(f"  • {h_file}")
    print(f"  • Camera_ImageConfig.h")

    print(f"\n📋 使用说明:")
    print("1. 将此 Camera_ImageConfig.h 文件复制到项目目录")
    print("2. 替换原有的 Camera_ImageConfig.h 文件")
    print("3. 重新编译 Arduino 项目")

    return 0


if __name__ == "__main__":
    try:
        exit_code = main()
        exit(exit_code)
    except Exception as e:
        print(f"❌ 发生错误: {e}")
        exit(1)
