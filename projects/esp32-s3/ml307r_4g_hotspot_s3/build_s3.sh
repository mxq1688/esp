#!/bin/bash

# ESP32-S3 ML307R 4G Hotspot Controller Build Script
# 适用于ESP32-S3开发板和ML307R 4G模块

# 确保在脚本出错时立即退出
set -e

# 定义ESP-IDF路径和项目路径
ESP_IDF_PATH="/Users/meng/stu/esp/esp-idf"
PROJECT_PATH="/Users/meng/stu/esp/projects/esp32-s3/ml307r_4g_hotspot_s3"
SERIAL_PORT="/dev/cu.usbmodem101" # ESP32-S3设备端口，根据实际情况调整
BAUD_RATE="115200"

echo "=========================================="
echo "ESP32-S3 ML307R 4G热点控制器"
echo "构建脚本 v1.0.0"
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
    
    echo "--- 准备烧写设备 ---"
    # 尝试终止任何可能占用串口的进程
    echo "尝试释放串口: $SERIAL_PORT"
    lsof -t "$SERIAL_PORT" 2>/dev/null | xargs -I {} kill -9 {} || true
    sleep 1 # 等待进程结束

    echo "--- 烧写设备 ---"
    echo "请确保您的ESP32-S3处于下载模式："
    echo "1. 按住 BOOT 按钮。"
    echo "2. 短按并释放 RST 按钮。"
    echo "3. 几秒后松开 BOOT 按钮。"
    echo ""
    echo "正在烧写到端口: $SERIAL_PORT (波特率: $BAUD_RATE)"

    # 执行烧写命令
    idf.py -p "$SERIAL_PORT" -b "$BAUD_RATE" flash monitor

    echo "--- 烧写完成！ ---"
    echo "您现在可以手动启动监视器查看日志："
    echo "idf.py -p $SERIAL_PORT -b $BAUD_RATE monitor"
    echo ""
    echo "🚀 使用说明:"
    echo "  1. 设备启动后会自动创建WiFi热点: ESP32-S3-ML307R"
    echo "  2. 密码: 12345678"
    echo "  3. 连接热点后访问: http://192.168.4.1"
    echo "  4. 在Web界面中配置ML307R 4G热点功能"
    echo ""
    echo "🔧 其他命令:"
    echo "  idf.py flash                    # 自动检测端口"
    echo "  idf.py monitor                  # 串口监视器"
    echo "  idf.py menuconfig               # 配置菜单"
    echo "  idf.py size-files               # 文件大小分析"
    echo ""
else
    echo "❌ 构建失败!"
    exit 1
fi
