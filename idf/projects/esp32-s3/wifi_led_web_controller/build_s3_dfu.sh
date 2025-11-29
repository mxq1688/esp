#!/bin/bash

# ESP32-S3 WiFi LED Web Controller DFU Build Script
# 使用DFU模式烧写，无需按键操作

# 确保在脚本出错时立即退出
set -e

# 定义ESP-IDF路径和项目路径
ESP_IDF_PATH="/Users/meng/stu/esp/esp-idf"
PROJECT_PATH="/Users/meng/stu/esp/projects/esp32-s3/wifi_led_web_controller"
SERIAL_PORT="/dev/cu.usbmodem101" # ESP32-S3设备端口
BAUD_RATE="115200"

echo "=========================================="
echo "ESP32-S3 WiFi LED Web Controller"
echo "DFU模式构建脚本 v1.0.0"
echo "=========================================="
echo "🚀 使用DFU模式，无需手动按键！"
echo ""

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
    
    echo "--- DFU模式烧写准备 ---"
    echo "🔌 使用DFU模式，无需手动按键！"
    echo "📱 请确保ESP32-S3通过USB-C连接到电脑"
    echo "💡 系统会自动检测并进入下载模式"
    echo ""
    
    # 尝试终止任何可能占用串口的进程
    echo "尝试释放串口: $SERIAL_PORT"
    lsof -t "$SERIAL_PORT" 2>/dev/null | xargs -I {} kill -9 {} || true
    sleep 1 # 等待进程结束

    echo "--- 开始DFU模式烧写 ---"
    echo "🚀 执行: idf.py dfu-flash"
    echo ""
    
    # 执行DFU烧写命令
    idf.py dfu-flash

    echo ""
    echo "--- DFU烧写完成！ ---"
    echo "✅ 固件已成功烧写到ESP32-S3"
    echo ""
    
    echo "--- 启动串口监视器 ---"
    echo "📡 正在启动监视器查看运行日志..."
    echo "💡 按 Ctrl+] 退出监视器"
    echo ""
    
    # 启动监视器
    idf.py monitor
    
    echo ""
    echo "🚀 其他有用命令:"
    echo "  idf.py dfu-flash              # DFU模式烧写"
    echo "  idf.py flash                  # 传统模式烧写"
    echo "  idf.py monitor                # 串口监视器"
    echo "  idf.py menuconfig             # 配置菜单"
    echo "  idf.py size-files             # 文件大小分析"
    echo "  idf.py erase-flash            # 擦除Flash"
    echo ""
    
else
    echo "❌ 构建失败!"
    exit 1
fi
