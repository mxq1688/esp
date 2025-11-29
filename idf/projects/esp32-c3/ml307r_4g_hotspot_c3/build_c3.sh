#!/bin/bash

# ESP32-C3 ML307R 4G热点项目构建脚本

echo "ESP32-C3 ML307R 4G Hotspot Project Build Script"
echo "==============================================="

# 检查ESP-IDF环境
if [ -z "$IDF_PATH" ]; then
    echo "错误: ESP-IDF环境未设置"
    echo "请先运行: source /path/to/esp-idf/export.sh"
    exit 1
fi

echo "ESP-IDF路径: $IDF_PATH"

# 设置项目路径
PROJECT_PATH=$(pwd)
echo "项目路径: $PROJECT_PATH"

# 检查ESP32-C3支持
echo "检查ESP32-C3支持..."
if ! idf.py --list-targets | grep -q esp32c3; then
    echo "错误: 当前ESP-IDF版本不支持ESP32-C3"
    echo "请升级到ESP-IDF v4.3+版本"
    exit 1
fi

# 清理之前的构建
echo "清理之前的构建..."
idf.py fullclean

# 设置目标芯片为ESP32-C3
echo "设置目标芯片为ESP32-C3..."
idf.py set-target esp32c3

# 构建项目
echo "构建ESP32-C3项目..."
idf.py build

if [ $? -eq 0 ]; then
    echo "ESP32-C3构建成功!"
    echo ""
    echo "烧录命令:"
    echo "idf.py -p /dev/ttyUSB0 flash monitor"
    echo ""
    echo "或者使用以下命令烧录到指定端口:"
    echo "idf.py -p [端口] flash monitor"
    echo ""
    echo "ESP32-C3常见端口:"
    echo "idf.py -p /dev/ttyACM0 flash monitor  # Linux"
    echo "idf.py -p COM3 flash monitor          # Windows"
    echo "idf.py -p /dev/cu.usbmodem* flash monitor  # macOS"
    echo ""
    echo "注意: ESP32-C3使用USB-C接口，可能显示为不同的设备名"
else
    echo "ESP32-C3构建失败!"
    exit 1
fi
