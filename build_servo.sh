#!/bin/bash

# ESP32-C3 舵机控制项目编译烧录脚本
# 基于原始 build_c3.sh 脚本修改

# 确保在脚本出错时立即退出
set -e

# 定义ESP-IDF路径和项目路径
ESP_IDF_PATH="/Users/meng/stu/esp/esp-idf"
PROJECT_PATH="/Users/meng/stu/esp/projects/esp32-c3/servo_control"
SERIAL_PORT="/dev/cu.usbmodem1101" # ESP32-C3设备端口
BAUD_RATE="115200"

echo "=========================================="
echo "  ESP32-C3 舵机控制项目编译烧录脚本"
echo "=========================================="
echo ""

echo "--- 检查项目路径 ---"
if [ ! -d "$PROJECT_PATH" ]; then
    echo "错误: 项目路径不存在: $PROJECT_PATH"
    echo "请确保舵机控制项目已正确创建"
    exit 1
fi

echo "项目路径: $PROJECT_PATH"
echo "ESP-IDF路径: $ESP_IDF_PATH"
echo ""

echo "--- 激活ESP-IDF环境 ---"
if [ ! -f "$ESP_IDF_PATH/export.sh" ]; then
    echo "错误: ESP-IDF export.sh 文件不存在: $ESP_IDF_PATH/export.sh"
    echo "请检查ESP-IDF是否正确安装"
    exit 1
fi

source "$ESP_IDF_PATH/export.sh"
echo "ESP-IDF环境激活成功"
echo ""

echo "--- 进入项目目录: $PROJECT_PATH ---"
cd "$PROJECT_PATH"
echo "当前目录: $(pwd)"
echo ""

echo "--- 清理之前的构建 ---"
if [ -d "build" ]; then
    echo "删除旧的构建目录..."
    rm -rf build
fi

echo "--- 编译舵机控制项目 ---"
echo "开始编译..."
idf.py build

if [ $? -eq 0 ]; then
    echo "编译成功！"
else
    echo "编译失败！请检查代码"
    exit 1
fi
echo ""

echo "--- 准备烧写设备 ---"
# 尝试终止任何可能占用串口的进程
echo "尝试释放串口: $SERIAL_PORT"
lsof -t "$SERIAL_PORT" 2>/dev/null | xargs -I {} kill -9 {} || true
sleep 1 # 等待进程结束

echo "--- 设备连接检查 ---"
if [ ! -e "$SERIAL_PORT" ]; then
    echo "警告: 串口设备 $SERIAL_PORT 不存在"
    echo "请检查以下事项："
    echo "1. ESP32-C3设备是否已连接"
    echo "2. USB驱动是否已安装"
    echo "3. 串口设备名称是否正确"
    echo ""
    echo "常见的ESP32-C3串口名称："
    echo "  macOS: /dev/cu.usbmodem* 或 /dev/cu.SLAB_USBtoUART"
    echo "  Linux: /dev/ttyUSB* 或 /dev/ttyACM*"
    echo ""
    read -p "是否继续烧录？(y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "烧录已取消"
        exit 1
    fi
fi

echo "--- 烧写设备 ---"
echo "请确保您的ESP32-C3处于下载模式："
echo "1. 按住 BOOT 按钮"
echo "2. 短按并释放 RST 按钮"
echo "3. 几秒后松开 BOOT 按钮"
echo ""
echo "正在烧写到端口: $SERIAL_PORT (波特率: $BAUD_RATE)"
echo ""

# 执行烧写命令
echo "开始烧录和监控..."
idf.py -p "$SERIAL_PORT" -b "$BAUD_RATE" flash monitor

if [ $? -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "         烧录完成！                       "
    echo "=========================================="
    echo ""
    echo "舵机控制程序已成功烧录到ESP32-C3"
    echo ""
    echo "硬件连接提醒："
    echo "  舵机信号线(橙/黄) -> GPIO2"
    echo "  舵机电源线(红)     -> 5V/3.3V"
    echo "  舵机地线(棕/黑)   -> GND"
    echo ""
    echo "程序功能："
    echo "  - 基本角度定位演示"
    echo "  - 平滑扫描运动"
    echo "  - 振荡演示"
    echo "  - 步进移动演示"
    echo ""
    echo "如需重新启动监视器查看日志："
    echo "idf.py -p $SERIAL_PORT -b $BAUD_RATE monitor"
else
    echo ""
    echo "烧录失败！请检查："
    echo "1. 设备是否处于下载模式"
    echo "2. 串口连接是否正常"
    echo "3. 串口是否被其他程序占用"
fi
