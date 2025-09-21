#!/bin/bash

# ESP32-S3 ML307R 4G Hotspot Controller DFU Build Script
# 支持USB DFU烧录模式

# 确保在脚本出错时立即退出
set -e

# 定义ESP-IDF路径和项目路径
ESP_IDF_PATH="/Users/meng/stu/esp/esp-idf"
PROJECT_PATH="/Users/meng/stu/esp/projects/esp32-s3/ml307r_4g_hotspot_s3"

echo "=========================================="
echo "ESP32-S3 ML307R 4G热点控制器"
echo "DFU构建脚本 v1.0.0"
echo "=========================================="

# 检查ESP-IDF路径是否存在
if [ ! -d "$ESP_IDF_PATH" ]; then
    echo "❌ 错误: ESP-IDF路径不存在: $ESP_IDF_PATH"
    echo ""
    echo "请先安装ESP-IDF:"
    echo "  git clone --recursive https://github.com/espressif/esp-idf.git"
    echo "  cd esp-idf"
    echo "  ./install.sh esp32s3"
    echo ""
    exit 1
fi

echo "--- 激活ESP-IDF环境 ---"
source "$ESP_IDF_PATH/export.sh"

echo "✅ ESP-IDF环境已设置: $IDF_PATH"
echo "ESP-IDF版本: $(idf.py --version)"

# 设置目标芯片
export IDF_TARGET=esp32s3
echo "🎯 目标芯片: $IDF_TARGET"

echo "--- 进入项目目录: $PROJECT_PATH ---"
cd "$PROJECT_PATH"

# 清理之前的构建
echo "🧹 清理之前的构建..."
idf.py fullclean

echo "--- 编译项目 ---"
idf.py build

# 检查构建结果
if [ $? -eq 0 ]; then
    echo "✅ 构建成功!"
    echo ""
    echo "📊 构建信息:"
    echo "----------------------------------------"
    idf.py size-components
    echo "----------------------------------------"
    echo ""
    
    echo "--- 生成DFU文件 ---"
    # 生成DFU格式的固件文件
    python "$ESP_IDF_PATH/tools/mkdfu.py" write \
        -o "build/esp32s3_ml307r_4g_hotspot.dfu" \
        --json "build/flasher_args.json"
    
    if [ $? -eq 0 ]; then
        echo "✅ DFU文件生成成功: build/esp32s3_ml307r_4g_hotspot.dfu"
        echo ""
        echo "--- DFU烧录说明 ---"
        echo "1. 将ESP32-S3进入DFU模式："
        echo "   - 按住 BOOT 按钮"
        echo "   - 短按 RST 按钮"
        echo "   - 松开 RST 按钮，继续按住 BOOT 按钮2秒"
        echo "   - 松开 BOOT 按钮"
        echo ""
        echo "2. 使用以下命令烧录DFU文件："
        echo "   idf.py dfu-flash"
        echo ""
        echo "   或者手动使用dfu-util："
        echo "   dfu-util -D build/esp32s3_ml307r_4g_hotspot.dfu"
        echo ""
        echo "3. 烧录完成后按RST按钮重启设备"
        echo ""
        
        # 检查是否有DFU设备连接
        if command -v dfu-util >/dev/null 2>&1; then
            echo "--- 检查DFU设备 ---"
            dfu_devices=$(dfu-util -l 2>/dev/null | grep -i "esp32-s3" || true)
            if [ -n "$dfu_devices" ]; then
                echo "✅ 发现ESP32-S3 DFU设备:"
                echo "$dfu_devices"
                echo ""
                read -p "是否立即烧录DFU文件? (y/N): " -n 1 -r
                echo
                if [[ $REPLY =~ ^[Yy]$ ]]; then
                    echo "正在烧录DFU文件..."
                    idf.py dfu-flash
                    echo "✅ DFU烧录完成!"
                fi
            else
                echo "⚠️  未发现ESP32-S3 DFU设备，请检查设备连接和DFU模式"
            fi
        else
            echo "⚠️  未安装dfu-util工具，请安装后使用DFU功能"
            echo "   macOS: brew install dfu-util"
            echo "   Ubuntu: sudo apt install dfu-util"
        fi
        
    else
        echo "❌ DFU文件生成失败!"
        exit 1
    fi
    
    echo ""
    echo "🚀 使用说明:"
    echo "  1. 设备启动后会自动创建WiFi热点: ESP32-S3-ML307R"
    echo "  2. 密码: 12345678"
    echo "  3. 连接热点后访问: http://192.168.4.1"
    echo "  4. 在Web界面中配置ML307R 4G热点功能"
    echo ""
else
    echo "❌ 构建失败!"
    exit 1
fi
