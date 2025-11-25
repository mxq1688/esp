#!/bin/bash

# ESP32-S3 4G Camera DFU构建脚本
# 生成DFU固件文件

echo "================================="
echo "ESP32-S3 4G Camera DFU构建脚本"
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

# 进入项目目录
cd "$(dirname "$0")"

# 设置目标芯片
echo "📌 设置目标芯片: ESP32-S3"
idf.py set-target esp32s3

# 构建项目
echo "🔨 开始构建..."
idf.py build

if [ $? -eq 0 ]; then
    echo "✅ 构建成功！"
    
    # 生成DFU文件
    echo "📦 生成DFU固件..."
    python -m esptool --chip esp32s3 merge_bin -o build/esp32s3_4g_camera_dfu.bin \
        --flash_mode dio --flash_freq 80m --flash_size 4MB \
        0x0 build/bootloader/bootloader.bin \
        0x8000 build/partition_table/partition-table.bin \
        0x10000 build/esp32s3_4g_camera.bin
    
    if [ $? -eq 0 ]; then
        echo "✅ DFU固件生成成功！"
        echo "📂 输出文件: build/esp32s3_4g_camera_dfu.bin"
        ls -lh build/esp32s3_4g_camera_dfu.bin
    else
        echo "❌ DFU固件生成失败！"
        exit 1
    fi
else
    echo "❌ 构建失败！"
    exit 1
fi

