# ESP32-S3 AI Assistant Project
# 智能语音助手项目 - 支持语音识别、自然语言处理和语音合成
# Compatible with ESP-IDF v5.0+

cmake_minimum_required(VERSION 3.16)

# 设置目标芯片
set(IDF_TARGET esp32s3)

# 设置分区表文件 - 必须在include之前设置
set(PARTITION_TABLE_FILE "partitions.csv")

# 项目信息
set(PROJECT_NAME "esp32s3_ai_assistant")
set(PROJECT_VER "1.0.0")
set(PROJECT_DESCRIPTION "ESP32-S3 AI Assistant with Voice Recognition and Synthesis")

# 包含ESP-IDF构建系统
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 定义项目
project(${PROJECT_NAME})

# 编译选项
target_compile_options(${PROJECT_NAME}.elf PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -DCONFIG_ESP32S3_SPIRAM_SUPPORT=1
)

# 项目元信息
set_property(TARGET ${PROJECT_NAME}.elf PROPERTY
    DESCRIPTION "${PROJECT_DESCRIPTION}"
)
