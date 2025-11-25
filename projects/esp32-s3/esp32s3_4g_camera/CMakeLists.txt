# ESP32-S3 4G Camera Project
cmake_minimum_required(VERSION 3.16)

# 项目配置
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32s3_4g_camera)

