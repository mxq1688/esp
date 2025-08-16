#!/bin/bash

# AI小智助手构建脚本
# 确保在脚本出错时立即退出
set -e

# 定义ESP-IDF路径和项目路径
ESP_IDF_PATH="/Users/meng/stu/esp/esp-idf"
PROJECT_PATH="/Users/meng/stu/esp/projects/esp32-c3/ai_assistant"
SERIAL_PORT="/dev/cu.usbmodem1101" # ESP32-C3设备端口
BAUD_RATE="115200"

echo "=== AI小智助手构建脚本 ==="
echo "项目路径: $PROJECT_PATH"
echo "设备端口: $SERIAL_PORT"

# 检查项目目录是否存在
if [ ! -d "$PROJECT_PATH" ]; then
    echo "错误: 项目目录不存在: $PROJECT_PATH"
    exit 1
fi

echo "--- 激活ESP-IDF环境 ---"
source "$ESP_IDF_PATH/export.sh"

echo "--- 进入项目目录: $PROJECT_PATH ---"
cd "$PROJECT_PATH"

echo "--- 清理之前的构建 ---"
# 检查build目录是否存在，如果存在才进行清理
if [ -d "build" ]; then
    echo "发现现有构建目录，正在清理..."
    idf.py fullclean
else
    echo "这是首次构建，跳过清理步骤"
fi

echo "--- 配置项目 ---"
echo "提示: 如果需要配置WiFi等信息，请运行: idf.py menuconfig"

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
echo ""
echo "=== AI小智助手启动成功 ==="
echo "使用说明:"
echo "1. 设备启动后会自动连接WiFi"
echo "2. 通过串口监视器查看IP地址"
echo "3. 在浏览器中访问 http://[设备IP]"
echo "4. 开始与小智对话"
echo ""
echo "常用命令:"
echo "- '你好小智' - 问候"
echo "- '现在几点了' - 查询时间"
echo "- '开灯' - 控制设备"
echo "- '帮助' - 获取帮助信息"
echo ""
echo "您现在可以手动启动监视器查看日志："
echo "idf.py -p $SERIAL_PORT -b $BAUD_RATE monitor"
