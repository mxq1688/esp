#!/bin/bash
# ESP32-S3 AI Assistant Build Script
# 用于编译和烧录ESP32-S3 AI助手项目

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 项目信息
PROJECT_NAME="ESP32-S3 AI Assistant"
VERSION="1.0.0"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  $PROJECT_NAME v$VERSION${NC}"
echo -e "${BLUE}  构建脚本${NC}"
echo -e "${BLUE}========================================${NC}"

# 检查ESP-IDF环境
if [ -z "$IDF_PATH" ]; then
    echo -e "${RED}错误: ESP-IDF环境未设置${NC}"
    echo -e "${YELLOW}请运行: . \$HOME/esp/esp-idf/export.sh${NC}"
    exit 1
fi

echo -e "${GREEN}✓ ESP-IDF环境已设置: $IDF_PATH${NC}"

# 设置目标芯片
echo -e "${BLUE}设置目标芯片为ESP32-S3...${NC}"
idf.py set-target esp32s3

# 清理构建目录（可选）
if [ "$1" = "clean" ]; then
    echo -e "${YELLOW}清理构建目录...${NC}"
    idf.py fullclean
fi

# 编译项目
echo -e "${BLUE}开始编译项目...${NC}"
idf.py build

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ 编译成功！${NC}"
    
    # 显示固件信息
    echo -e "${BLUE}固件信息:${NC}"
    ls -lh build/*.bin | grep -E "(bootloader|partition-table|ai_assistant)"
    
    # 显示内存使用情况
    echo -e "${BLUE}内存使用情况:${NC}"
    idf.py size
    
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}  编译完成！${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo -e "${YELLOW}烧录命令:${NC}"
    echo -e "  idf.py -p /dev/ttyUSB0 flash"
    echo -e "${YELLOW}监控串口:${NC}"
    echo -e "  idf.py -p /dev/ttyUSB0 monitor"
    echo -e "${YELLOW}一键烧录并监控:${NC}"
    echo -e "  idf.py -p /dev/ttyUSB0 flash monitor"
else
    echo -e "${RED}✗ 编译失败！${NC}"
    exit 1
fi
