#!/bin/bash

# ESP32-S3 4G Camera 构建脚本
# 自动检测串口并构建、烧录固件

echo "================================="
echo "ESP32-S3 4G Camera 构建脚本"
echo "================================="

# 设置ESP-IDF环境
if [ -z "$IDF_PATH" ]; then
    echo "⚠️  IDF_PATH未设置，尝试加载ESP-IDF环境..."
    if [ -f "$HOME/stu/esp/esp-idf/export.sh" ]; then
        source "$HOME/stu/esp/esp-idf/export.sh"
    elif [ -f "/Users/meng/stu/esp/esp-idf/export.sh" ]; then
        source "/Users/meng/stu/esp/esp-idf/export.sh"
    else
        echo "❌ 找不到ESP-IDF，请手动设置IDF_PATH"
        exit 1
    fi
fi

echo "✅ ESP-IDF环境: $IDF_PATH"

# 进入项目目录
cd "$(dirname "$0")"

# 设置目标芯片
echo "📌 设置目标芯片: ESP32-S3"
idf.py set-target esp32s3

# 清理构建（可选，取消注释以启用）
# echo "🧹 清理旧构建..."
# idf.py fullclean

# 构建项目
echo "🔨 开始构建..."
idf.py build

if [ $? -eq 0 ]; then
    echo "✅ 构建成功！"
    
    # 检测串口
    PORT=""
    if [ -e /dev/cu.usbmodem* ]; then
        PORT=$(ls /dev/cu.usbmodem* | head -n 1)
    elif [ -e /dev/cu.usbserial* ]; then
        PORT=$(ls /dev/cu.usbserial* | head -n 1)
    elif [ -e /dev/ttyUSB* ]; then
        PORT=$(ls /dev/ttyUSB* | head -n 1)
    fi
    
    if [ -n "$PORT" ]; then
        echo "📱 检测到串口: $PORT"
        echo "⚡ 开始烧录..."
        idf.py -p $PORT flash monitor
    else
        echo "⚠️  未检测到串口设备"
        echo "请手动烧录: idf.py -p <端口> flash monitor"
    fi
else
    echo "❌ 构建失败！"
    exit 1
fi

