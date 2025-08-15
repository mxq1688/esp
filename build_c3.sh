#!/bin/bash

# 确保在脚本出错时立即退出
set -e

# 定义ESP-IDF路径和项目路径
ESP_IDF_PATH="/Users/meng/stu/esp/esp-idf"
PROJECT_PATH="/Users/meng/stu/esp/projects/esp32-c3/wifi_led_web_controller"
SERIAL_PORT="/dev/cu.usbmodem101" # 请根据实际情况修改此端口
BAUD_RATE="115200"

echo "--- 激活ESP-IDF环境 ---"
source "$ESP_IDF_PATH/export.sh"

echo "--- 进入项目目录: $PROJECT_PATH ---"
cd "$PROJECT_PATH"

echo "--- 编译项目 ---"
idf.py build

echo "--- 准备烧写设备 ---"
# 尝试终止任何可能占用串口的进程
echo "尝试释放串口: $SERIAL_PORT"
lsof -t "$SERIAL_PORT" 2>/dev/null | xargs -I {} kill -9 {} || true
sleep 1 # 等待进程结束

echo "--- 烧写设备 ---"
echo "请确保您的ESP32-C3处于下载模式："
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