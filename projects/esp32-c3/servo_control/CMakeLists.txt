# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16) # 指定最低CMake版本要求

include($ENV{IDF_PATH}/tools/cmake/project.cmake) # 包含ESP-IDF的构建规则
project(servo_control)  # 定义项目名称
