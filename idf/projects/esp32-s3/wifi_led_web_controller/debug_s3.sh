#!/bin/bash

# ESP32-S3 WiFi LED Web Controller 调试脚本
# 提供多种调试选项和工具

# 确保在脚本出错时立即退出
set -e

# 定义路径
ESP_IDF_PATH="/Users/meng/stu/esp/esp-idf"
PROJECT_PATH="/Users/meng/stu/esp/projects/esp32-s3/wifi_led_web_controller"
SERIAL_PORT="/dev/cu.usbmodem101"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_header() {
    echo -e "${PURPLE}==========================================${NC}"
    echo -e "${PURPLE}$1${NC}"
    echo -e "${PURPLE}==========================================${NC}"
}

# 显示帮助信息
show_help() {
    print_header "ESP32-S3 调试脚本帮助"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  monitor         启动串口监视器"
    echo "  jtag            启动JTAG调试环境"
    echo "  gdb             启动GDB调试器"
    echo "  openocd         启动OpenOCD服务"
    echo "  log-filter      启动日志过滤器"
    echo "  memory          显示内存使用情况"
    echo "  size            显示程序大小信息"
    echo "  config          打开配置菜单"
    echo "  clean           清理构建文件"
    echo "  build           构建项目"
    echo "  flash           烧录固件"
    echo "  help            显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0 monitor      # 启动串口监视器"
    echo "  $0 jtag         # 启动JTAG调试"
    echo "  $0 log-filter   # 启动日志过滤"
    echo ""
}

# 检查ESP-IDF环境
check_esp_idf() {
    if [ ! -d "$ESP_IDF_PATH" ]; then
        print_error "ESP-IDF路径不存在: $ESP_IDF_PATH"
        exit 1
    fi
    
    print_info "激活ESP-IDF环境..."
    source "$ESP_IDF_PATH/export.sh"
    print_success "ESP-IDF环境已设置: $IDF_PATH"
}

# 检查设备连接
check_device() {
    print_info "检查设备连接..."
    if ls /dev/cu.usbmodem* 1> /dev/null 2>&1; then
        print_success "找到ESP32-S3设备:"
        ls /dev/cu.usbmodem*
    else
        print_warning "未找到ESP32-S3设备"
        print_info "请确保设备已通过USB-C连接"
    fi
}

# 启动串口监视器
start_monitor() {
    print_header "启动串口监视器"
    check_device
    
    print_info "启动串口监视器..."
    print_info "按 Ctrl+] 退出监视器"
    echo ""
    
    idf.py monitor
}

# 启动JTAG调试环境
start_jtag() {
    print_header "启动JTAG调试环境"
    check_device
    check_esp_idf
    
    print_info "启动OpenOCD服务..."
    idf.py openocd &
    OPENOCD_PID=$!
    
    print_info "等待OpenOCD启动..."
    sleep 3
    
    print_info "启动GDB调试器..."
    print_info "调试器已启动，可以在VS Code中设置断点"
    echo ""
    
    # 保持脚本运行
    print_info "按 Ctrl+C 停止调试环境"
    wait $OPENOCD_PID
}

# 启动OpenOCD服务
start_openocd() {
    print_header "启动OpenOCD服务"
    check_device
    check_esp_idf
    
    print_info "启动OpenOCD服务..."
    print_info "服务将在后台运行，端口: 3333"
    echo ""
    
    idf.py openocd
}

# 启动GDB调试器
start_gdb() {
    print_header "启动GDB调试器"
    check_device
    check_esp_idf
    
    print_info "启动GDB调试器..."
    print_info "确保OpenOCD服务正在运行"
    echo ""
    
    idf.py gdb
}

# 启动日志过滤器
start_log_filter() {
    print_header "启动日志过滤器"
    check_device
    
    print_info "启动串口监视器并过滤日志..."
    echo ""
    echo "日志过滤选项:"
    echo "1. WiFi相关日志: grep -E '(WIFI|wifi|WiFi)'"
    echo "2. 错误和警告: grep -E '(ERROR|WARN)'"
    echo "3. 内存相关: grep 'heap'"
    echo "4. 任务相关: grep 'Task'"
    echo "5. 自定义过滤: 输入您的grep表达式"
    echo ""
    
    read -p "选择过滤选项 (1-5) 或输入自定义表达式: " choice
    
    case $choice in
        1)
            filter_expr="grep -E '(WIFI|wifi|WiFi)'"
            ;;
        2)
            filter_expr="grep -E '(ERROR|WARN)'"
            ;;
        3)
            filter_expr="grep 'heap'"
            ;;
        4)
            filter_expr="grep 'Task'"
            ;;
        5)
            read -p "输入自定义grep表达式: " custom_filter
            filter_expr="$custom_filter"
            ;;
        *)
            filter_expr="grep -E '(ERROR|WARN)'"
            ;;
    esac
    
    print_info "应用过滤器: $filter_expr"
    echo ""
    
    idf.py monitor | eval $filter_expr
}

# 显示内存使用情况
show_memory() {
    print_header "内存使用情况"
    check_device
    
    print_info "获取内存信息..."
    echo ""
    
    # 通过串口获取内存信息
    echo "free_heap" | idf.py monitor | grep -E "(Free heap|heap)" | head -5
}

# 显示程序大小信息
show_size() {
    print_header "程序大小信息"
    check_esp_idf
    
    print_info "分析程序大小..."
    echo ""
    
    idf.py size-components
    echo ""
    idf.py size-files
}

# 打开配置菜单
open_config() {
    print_header "配置菜单"
    check_esp_idf
    
    print_info "打开配置菜单..."
    print_info "在菜单中可以调整各种配置选项"
    echo ""
    
    idf.py menuconfig
}

# 清理构建文件
clean_build() {
    print_header "清理构建文件"
    check_esp_idf
    
    print_info "清理构建文件..."
    idf.py clean
    
    print_info "完全清理..."
    idf.py fullclean
    
    print_success "构建文件已清理"
}

# 构建项目
build_project() {
    print_header "构建项目"
    check_esp_idf
    
    print_info "构建项目..."
    idf.py build
    
    if [ $? -eq 0 ]; then
        print_success "构建成功!"
        show_size
    else
        print_error "构建失败!"
        exit 1
    fi
}

# 烧录固件
flash_firmware() {
    print_header "烧录固件"
    check_device
    check_esp_idf
    
    print_info "烧录固件..."
    print_info "选择烧录模式:"
    echo "1. 传统模式 (idf.py flash)"
    echo "2. DFU模式 (idf.py dfu-flash)"
    echo ""
    
    read -p "选择烧录模式 (1-2): " flash_mode
    
    case $flash_mode in
        1)
            print_info "使用传统模式烧录..."
            idf.py flash
            ;;
        2)
            print_info "使用DFU模式烧录..."
            idf.py dfu-flash
            ;;
        *)
            print_warning "无效选择，使用传统模式"
            idf.py flash
            ;;
    esac
    
    print_success "固件烧录完成!"
}

# 主函数
main() {
    print_header "ESP32-S3 WiFi LED Web Controller 调试工具"
    
    if [ $# -eq 0 ]; then
        show_help
        exit 0
    fi
    
    case $1 in
        monitor)
            start_monitor
            ;;
        jtag)
            start_jtag
            ;;
        gdb)
            start_gdb
            ;;
        openocd)
            start_openocd
            ;;
        log-filter)
            start_log_filter
            ;;
        memory)
            show_memory
            ;;
        size)
            show_size
            ;;
        config)
            open_config
            ;;
        clean)
            clean_build
            ;;
        build)
            build_project
            ;;
        flash)
            flash_firmware
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            print_error "未知选项: $1"
            echo ""
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@"
