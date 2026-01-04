from PIL import Image
import sys
import os


def rgb_to_rgb565(r, g, b):
    """RGB888转RGB565的验证函数"""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def validate_color_conversion():
    """颜色转换验证逻辑"""
    test_colors = [
        (0, 0, 0),  # 黑色
        (255, 0, 0),  # 红色
        (0, 255, 0),  # 绿色
        (0, 0, 255),  # 蓝色
        (255, 0, 255),  # 紫色
    ]

    print("\n颜色转换验证结果:")
    for r, g, b in test_colors:
        rgb565 = rgb_to_rgb565(r, g, b)
        print(f"RGB({r:3d},{g:3d},{b:3d}) -> 0x{rgb565:04X}")


def convert_image_to_header(input_path, output_path):
    try:
        # 打开图片文件
        img = Image.open(input_path)

        # 确保图片是RGB模式
        if img.mode != "RGB":
            img = img.convert("RGB")

        # 获取图片尺寸
        width, height = img.size
        print(f"Image size: {width}x{height}")

        # 根据输入文件名生成变量名
        input_filename = os.path.basename(input_path)
        filename_without_ext = os.path.splitext(input_filename)[0]
        # 去掉Menu_前缀以保持变量名与项目一致
        if filename_without_ext.startswith("Menu_"):
            filename_without_ext = filename_without_ext[5:]

        # 生成变量名（将文件名转换为大写，用下划线连接）
        var_prefix = filename_without_ext.upper()
        width_var = f"{var_prefix}_WIDTH"
        height_var = f"{var_prefix}_HEIGHT"
        pic_var = f"{var_prefix}_PIC"
        header_guard = f"_{var_prefix}_H_"

        print(f"Generated variable names:")
        print(f"  Width variable: {width_var}")
        print(f"  Height variable: {height_var}")
        print(f"  Picture array: {pic_var}")
        print(f"  Header guard: {header_guard}")

        # 准备输出文件（使用UTF-8编码）
        with open(output_path, "w", encoding="utf-8") as f:
            # 写入文件头（标准化注释格式）
            output_filename = os.path.basename(output_path)
            output_name_without_ext = os.path.splitext(output_filename)[0]
            f.write(f"/*\n")
            # 根据文件名生成更具体的描述
            if filename_without_ext.lower() == "mm":
                f.write(f" * {output_filename} - 菜单主界面图像数据\n")
                f.write(f" * 提供菜单主界面的像素数据定义\n")
            elif filename_without_ext.lower() == "sm":
                f.write(f" * {output_filename} - 菜单状态图图像数据\n")
                f.write(f" * 提供菜单状态图的像素数据定义\n")
            else:
                f.write(f" * {output_filename} - {filename_without_ext}图像数据\n")
                f.write(f" * 提供{filename_without_ext}图像的像素数据定义\n")
            if filename_without_ext.lower() == "mm":
                f.write(f" * 阶段六：菜单模块优化 - 主界面资源\n")
            elif filename_without_ext.lower() == "sm":
                f.write(f" * 阶段六：菜单模块优化 - 状态图资源\n")
            else:
                f.write(f" * 阶段六：菜单模块优化 - 图像资源\n")
            f.write(f" */\n\n")
            f.write(f"#ifndef {header_guard}\n")
            f.write(f"#define {header_guard}\n\n")
            # 添加图片尺寸定义
            f.write(f"const uint16_t {width_var} = {width};\n")
            f.write(f"const uint16_t {height_var} = {height};\n\n")
            f.write(f"const uint16_t {pic_var}[{width * height}] PROGMEM = {{\n")

            # 处理每个像素 - 修复颜色转换公式
            pixels = list(img.getdata())
            for i, (r, g, b) in enumerate(pixels):
                # 修正RGB565转换公式 (使用标准位移操作)
                rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

                # 调试输出前10个像素值
                if i < 10:
                    print(f"Pixel {i}: RGB({r},{g},{b}) -> RGB565 0x{rgb565:04X}")

                # 每行写入16个像素值
                if i % 16 == 0:
                    f.write("    ")

                # 写入像素值
                f.write(f"0x{rgb565:04X}")
                if i < len(pixels) - 1:
                    f.write(", ")

                # 换行
                if i % 16 == 15 or i == len(pixels) - 1:
                    f.write("\n")

            # 写入文件尾
            f.write("};\n\n")
            f.write(f"#endif // {header_guard}\n")

        print(f"Successfully converted to {output_path}")

    except Exception as e:
        print(f"Error: {str(e)}")
        sys.exit(1)


if __name__ == "__main__":
    # 在脚本启动时自动运行颜色校验
    validate_color_conversion()

    if len(sys.argv) != 3:
        print("Usage: python convert_image.py input.png output.h")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    convert_image_to_header(input_file, output_file)
