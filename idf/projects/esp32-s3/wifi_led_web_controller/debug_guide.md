# ESP32-S3 调试指南

## 🎯 调试概述

ESP32-S3提供了多种调试方法，从简单的日志输出到完整的JTAG调试。本指南将详细介绍各种调试技术。

## 🔍 调试方法分类

### 1. 日志调试 (Logging Debug)
- **最简单** - 通过串口输出调试信息
- **最常用** - 适合大多数开发场景
- **实时性** - 可以实时查看程序运行状态

### 2. JTAG调试 (JTAG Debug)
- **最强大** - 支持断点、单步执行、变量查看
- **最专业** - 适合复杂问题调试
- **硬件要求** - 需要JTAG调试器或ESP32-S3内置JTAG

### 3. 性能分析 (Performance Profiling)
- **性能优化** - 分析程序性能瓶颈
- **内存监控** - 监控内存使用情况
- **任务分析** - 分析FreeRTOS任务状态

## 📱 方法一：日志调试

### 基本日志使用
```c
#include "esp_log.h"

static const char *TAG = "MY_MODULE";

// 不同级别的日志
ESP_LOGE(TAG, "错误信息: %s", error_msg);    // 错误
ESP_LOGW(TAG, "警告信息: %d", warning_code); // 警告
ESP_LOGI(TAG, "信息: %s", info_msg);         // 信息
ESP_LOGD(TAG, "调试: %s", debug_msg);        // 调试
ESP_LOGV(TAG, "详细: %s", verbose_msg);      // 详细
```

### 日志配置
```bash
# 设置日志级别
idf.py menuconfig
# Component config → Log output → Default log verbosity
```

### 日志过滤
```bash
# 只显示特定模块的日志
idf.py monitor | grep "WIFI_MANAGER"

# 只显示错误和警告
idf.py monitor | grep -E "(ERROR|WARN)"
```

## 🔧 方法二：JTAG调试

### 硬件准备
ESP32-S3内置USB-Serial-JTAG，无需额外硬件！

### 配置JTAG调试
```bash
# 1. 配置项目
idf.py menuconfig
# Component config → ESP32S3-Specific → ESP32S3 Debug OCD Aware

# 2. 构建项目
idf.py build

# 3. 启动OpenOCD
idf.py openocd
```

### VS Code调试配置
创建 `.vscode/launch.json`:
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "ESP-IDF Debug",
            "type": "cppdbg",
            "request": "launch",
            "cwd": "${workspaceFolder}/build",
            "program": "${workspaceFolder}/build/wifi_led_web_controller.elf",
            "args": [],
            "stopAtEntry": false,
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "${config:idf.toolsPath}/tools/xtensa-esp32s3-elf/esp-12.2.0_20230208/xtensa-esp32s3-elf/bin/xtensa-esp32s3-elf-gdb.exe",
            "setupCommands": [
                {"text": "target remote :3333"},
                {"text": "set remote hardware-watchpoint-limit 2"},
                {"text": "monitor reset halt"},
                {"text": "flushregs"}
            ]
        }
    ]
}
```

## 📊 方法三：性能分析

### 内存监控
```c
#include "esp_heap_caps.h"

// 获取内存信息
size_t free_heap = esp_get_free_heap_size();
size_t min_free_heap = esp_get_minimum_free_heap_size();

ESP_LOGI(TAG, "Free heap: %zu bytes", free_heap);
ESP_LOGI(TAG, "Min free heap: %zu bytes", min_free_heap);
```

### 任务监控
```c
#include "esp_task_wdt.h"

// 获取任务信息
UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
ESP_LOGI(TAG, "Task stack high water mark: %d", uxHighWaterMark);
```

### 性能计数器
```c
#include "esp_timer.h"

uint64_t start_time = esp_timer_get_time();
// ... 执行代码 ...
uint64_t end_time = esp_timer_get_time();
ESP_LOGI(TAG, "执行时间: %llu us", end_time - start_time);
```

## 🚀 调试脚本

### 快速调试脚本
```bash
#!/bin/bash
# 快速启动调试环境

echo "🚀 ESP32-S3 调试环境启动"
echo "=========================="

# 检查设备连接
echo "📱 检查设备连接..."
ls /dev/cu.usbmodem* 2>/dev/null || echo "❌ 未找到ESP32-S3设备"

# 启动OpenOCD
echo "🔧 启动OpenOCD..."
idf.py openocd &

# 等待OpenOCD启动
sleep 3

# 启动GDB
echo "🐛 启动GDB调试器..."
idf.py gdb

echo "✅ 调试环境已启动"
```

## 🎯 常见调试场景

### 1. WiFi连接问题
```bash
# 监控WiFi相关日志
idf.py monitor | grep -E "(WIFI|wifi|WiFi)"

# 查看网络配置
idf.py monitor | grep -E "(IP|DNS|Gateway)"
```

### 2. 内存泄漏
```bash
# 监控内存使用
idf.py monitor | grep "heap"

# 定期检查内存
# 在代码中添加定时器回调
```

### 3. 任务死锁
```bash
# 监控任务状态
idf.py monitor | grep "Task"

# 使用FreeRTOS任务监控
```

## 🔧 调试技巧

### 1. 条件日志
```c
#ifdef CONFIG_DEBUG_ENABLED
    ESP_LOGD(TAG, "调试信息: %s", debug_data);
#endif
```

### 2. 断言调试
```c
#include "assert.h"

// 添加断言检查
assert(ptr != NULL);
ESP_ASSERT(condition);
```

### 3. 错误处理
```c
esp_err_t ret = some_function();
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "函数调用失败: %s", esp_err_to_name(ret));
    // 处理错误
}
```

## 📱 移动端调试

### 远程日志查看
```bash
# 通过网络查看日志
# 在Web界面中添加日志显示功能
```

### 远程配置
```bash
# 通过Web界面修改配置
# 实时更新系统参数
```

## 🎉 调试最佳实践

1. **分层日志** - 不同模块使用不同TAG
2. **合理级别** - 生产环境关闭DEBUG日志
3. **性能考虑** - 避免在循环中输出大量日志
4. **错误处理** - 每个错误都要有明确的日志
5. **版本控制** - 调试代码不要提交到主分支

## 🚨 故障排除

### 常见问题
- **串口无法连接** - 检查USB驱动和端口
- **JTAG连接失败** - 确认OpenOCD配置
- **日志不显示** - 检查日志级别设置
- **断点不工作** - 确认代码已重新编译

### 调试命令速查
```bash
idf.py monitor              # 串口监视器
idf.py openocd              # 启动JTAG服务
idf.py gdb                  # 启动GDB调试器
idf.py size-components      # 查看程序大小
idf.py menuconfig           # 配置菜单
idf.py fullclean            # 完全清理
```
