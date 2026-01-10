# ESP32-S3 Audio Speaker Project
# Compatible with ESP-IDF v5.0+
# 使用 I2S 接口驱动喇叭播放音频

cmake_minimum_required(VERSION 3.16)

# 设置目标芯片
set(IDF_TARGET esp32s3)

# 设置分区表文件 - 必须在include之前设置
set(PARTITION_TABLE_FILE "partitions.csv")

# 项目信息
set(PROJECT_NAME "esp32s3_audio_speaker")
set(PROJECT_VER "1.0.0")
set(PROJECT_DESCRIPTION "ESP32-S3 Audio Speaker using I2S interface")

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

