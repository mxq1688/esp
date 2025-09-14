#!/bin/bash

# ESP32-S3 MicroPython 固件烧录脚本
# 使用方法: ./flash_esp32s3.sh [端口号]
# 例如: ./flash_esp32s3.sh /dev/tty.usbserial-0001

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FIRMWARE_PATH="$SCRIPT_DIR/esp32s3/esp32s3-micropython.bin"

echo -e "${BLUE}=== ESP32-S3 MicroPython 固件烧录工具 ===${NC}"
echo

# 检查固件文件是否存在
if [ ! -f "$FIRMWARE_PATH" ]; then
    echo -e "${RED}❌ 错误: 固件文件不存在: $FIRMWARE_PATH${NC}"
    echo -e "${YELLOW}请确保固件文件已下载到正确位置${NC}"
    exit 1
fi

echo -e "${GREEN}✅ 找到固件文件: $(basename "$FIRMWARE_PATH")${NC}"
echo -e "${BLUE}文件大小: $(ls -lh "$FIRMWARE_PATH" | awk '{print $5}')${NC}"
echo

# 检查是否安装了 esptool
if ! command -v python3 &> /dev/null || ! python3 -c "import esptool" &> /dev/null; then
    echo -e "${RED}❌ 错误: 未找到 esptool${NC}"
    echo -e "${YELLOW}请先安装: pip3 install esptool${NC}"
    exit 1
fi

# 获取端口号
if [ -n "$1" ]; then
    PORT="$1"
else
    echo -e "${YELLOW}🔍 正在搜索可用端口...${NC}"
    
    # 在 macOS 上搜索 USB 串口
    if [[ "$OSTYPE" == "darwin"* ]]; then
        PORTS=$(ls /dev/tty.usbserial-* /dev/tty.SLAB_USBtoUART /dev/tty.usbmodem* 2>/dev/null || true)
    else
        # Linux
        PORTS=$(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || true)
    fi
    
    if [ -z "$PORTS" ]; then
        echo -e "${RED}❌ 未找到可用的串口设备${NC}"
        echo -e "${YELLOW}请手动指定端口: $0 /dev/tty.usbserial-xxxx${NC}"
        exit 1
    fi
    
    # 如果只有一个端口，直接使用
    PORT_COUNT=$(echo "$PORTS" | wc -l)
    if [ "$PORT_COUNT" -eq 1 ]; then
        PORT="$PORTS"
        echo -e "${GREEN}✅ 自动选择端口: $PORT${NC}"
    else
        echo -e "${YELLOW}发现多个端口:${NC}"
        echo "$PORTS" | nl
        echo
        read -p "请选择端口号 (1-$PORT_COUNT): " choice
        PORT=$(echo "$PORTS" | sed -n "${choice}p")
        
        if [ -z "$PORT" ]; then
            echo -e "${RED}❌ 无效选择${NC}"
            exit 1
        fi
    fi
fi

echo -e "${BLUE}使用端口: $PORT${NC}"
echo

# 确认烧录
echo -e "${YELLOW}⚠️  警告: 烧录将清除设备上的所有数据!${NC}"
read -p "确认要烧录 ESP32-S3 固件吗? (y/N): " confirm

if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}❌ 用户取消操作${NC}"
    exit 0
fi

echo
echo -e "${BLUE}🔥 开始烧录过程...${NC}"
echo

# 步骤1: 擦除闪存
echo -e "${YELLOW}📝 步骤 1/2: 擦除闪存...${NC}"
python3 -m esptool --chip esp32s3 --port "$PORT" erase_flash

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ 擦除闪存失败${NC}"
    exit 1
fi

echo -e "${GREEN}✅ 闪存擦除完成${NC}"
echo

# 步骤2: 烧录固件 (注意: ESP32-S3 使用 0x0 地址)
echo -e "${YELLOW}📝 步骤 2/2: 烧录固件...${NC}"
python3 -m esptool --chip esp32s3 --port "$PORT" write_flash -z 0x0 "$FIRMWARE_PATH"

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ 固件烧录失败${NC}"
    exit 1
fi

echo
echo -e "${GREEN}🎉 ESP32-S3 固件烧录成功!${NC}"
echo
echo -e "${BLUE}📋 后续步骤:${NC}"
echo -e "  1. 重启设备 (按 RST 按钮或重新插拔 USB)"
echo -e "  2. 使用串口工具连接 (波特率: 115200)"
echo -e "  3. 应该看到 MicroPython REPL 提示符: >>>"
echo
echo -e "${YELLOW}💡 连接命令示例:${NC}"
echo -e "  screen $PORT 115200"
echo -e "  或者使用 Thonny IDE"
echo
echo -e "${BLUE}📌 ESP32-S3 特性:${NC}"
echo -e "  • 双核 Xtensa LX7 处理器"
echo -e "  • 内置 WiFi + 蓝牙"
echo -e "  • 支持 USB OTG"
echo -e "  • 更大的 RAM 和 Flash"
