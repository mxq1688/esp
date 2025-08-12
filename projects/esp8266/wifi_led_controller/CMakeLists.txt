# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# ESP8266 RTOS SDK项目配置
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/led_strip)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 项目名称
project(esp8266_wifi_led_controller)

# 项目信息
set(PROJECT_VER "1.0.0")
set(PROJECT_DESCRIPTION "ESP8266 WiFi LED Controller with Web Interface")
