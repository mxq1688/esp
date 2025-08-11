#!/bin/bash

echo "=== ESP32-C3 LED Blink 项目验证 ==="
echo ""

# 检查项目结构
echo "1. 检查项目文件结构："
echo "✓ 根目录 CMakeLists.txt: $([ -f CMakeLists.txt ] && echo "存在" || echo "缺失")"
echo "✓ main/CMakeLists.txt: $([ -f main/CMakeLists.txt ] && echo "存在" || echo "缺失")"
echo "✓ 主程序文件: $([ -f main/led_blink_main.c ] && echo "存在" || echo "缺失")"
echo "✓ 配置文件: $([ -f main/Kconfig.projbuild ] && echo "存在" || echo "缺失")"
echo ""

# 检查环境变量
echo "2. 检查环境变量："
echo "✓ IDF_PATH: ${IDF_PATH:-"未设置"}"
echo ""

# 检查 ESP-IDF 工具
echo "3. 检查 ESP-IDF 工具："
if command -v idf.py &> /dev/null; then
    echo "✓ idf.py: 可用"
    idf.py --version 2>/dev/null || echo "版本信息获取失败"
else
    echo "✗ idf.py: 不可用"
    echo "  需要运行: source $IDF_PATH/export.sh"
fi
echo ""

echo "4. 编译建议："
echo "如果环境正确，运行以下命令编译："
echo "  idf.py set-target esp32c3"
echo "  idf.py build"
echo ""

echo "如果 idf.py 不可用，请先设置环境："
echo "  source /Users/meng/stu/esp/esp-idf/export.sh"
echo "  或者"
echo "  source /Users/meng/esp/v5.4/esp-idf/export.sh"