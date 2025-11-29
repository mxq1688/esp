#!/bin/bash

# ESP32 ML307R 4G热点项目构建脚本

echo "ESP32 ML307R 4G Hotspot Project Build Script"
echo "============================================="

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

# 清理之前的构建
echo "清理之前的构建..."
idf.py fullclean

# 配置项目
echo "配置项目..."
idf.py set-target esp32

# 构建项目
echo "构建项目..."
idf.py build

if [ $? -eq 0 ]; then
    echo "构建成功!"
    echo ""
    echo "烧录命令:"
    echo "idf.py -p /dev/ttyUSB0 flash monitor"
    echo ""
    echo "或者使用以下命令烧录到指定端口:"
    echo "idf.py -p [端口] flash monitor"
    echo ""
    echo "例如:"
    echo "idf.py -p /dev/ttyUSB0 flash monitor"
    echo "idf.py -p COM3 flash monitor  # Windows"
    echo "idf.py -p /dev/cu.usbserial-* flash monitor  # macOS"
else
    echo "构建失败!"
    exit 1
fi
