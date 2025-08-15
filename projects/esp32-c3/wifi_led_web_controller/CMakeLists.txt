# ESP32-C3 WiFi LED Web Controller Project
# Compatible with ESP-IDF v5.0+

cmake_minimum_required(VERSION 3.16)

# 设置目标芯片
set(IDF_TARGET esp32c3)

# 设置分区表文件 - 必须在include之前设置
set(PARTITION_TABLE_FILE "partitions.csv")

# 项目信息
set(PROJECT_NAME "esp32c3_wifi_led_controller")
set(PROJECT_VER "2.0.0")
set(PROJECT_DESCRIPTION "ESP32-C3 WiFi LED Web Controller with Modern UI")

# 包含ESP-IDF构建系统
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 定义项目
project(${PROJECT_NAME})

# 编译选项
target_compile_options(${PROJECT_NAME}.elf PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# 项目元信息
set_property(TARGET ${PROJECT_NAME}.elf PROPERTY
    DESCRIPTION "${PROJECT_DESCRIPTION}"
)
