# LED Blink Example for ESP32-C3-DevKitM-1

这个示例演示如何在 ESP32-C3-DevKitM-1 开发板上控制板载 LED 闪烁。

## 硬件说明

ESP32-C3-DevKitM-1 开发板配备了一个 WS2812 RGB LED，连接到 GPIO8。这是一个可编程的智能 LED，可以显示不同的颜色。

## 功能特性

- 控制板载 WS2812 RGB LED 闪烁
- 可配置的闪烁周期（100ms - 3000ms）
- 使用白色光（RGB: 16,16,16）进行闪烁
- 包含详细的日志输出

## 如何使用

### 1. 环境准备

确保您已经安装并配置了 ESP-IDF 开发环境。

### 2. 构建项目

```bash
cd /Users/meng/stu/esp/projects/esp32-c3/led_blink
idf.py set-target esp32c3
idf.py build
```

### 3. 配置选项

运行以下命令打开配置菜单：

```bash
idf.py menuconfig
```

在 "LED Blink Configuration" 菜单中，您可以设置：
- **Blink period in ms**: LED 闪烁周期（默认 1000ms）

### 4. 烧录程序

连接 ESP32-C3-DevKitM-1 开发板到电脑，然后运行：

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

*注意：请将 `/dev/ttyUSB0` 替换为您系统中对应的串口*

### 5. 预期效果

- 开发板上的 RGB LED 将会每秒闪烁一次（默认设置）
- 串口监视器会显示 LED 状态的日志信息
- LED 显示白色光进行闪烁

## 技术细节

### 硬件连接
- **LED 类型**: WS2812 RGB LED
- **GPIO 引脚**: GPIO8
- **数据协议**: WS2812 时序协议

### 代码结构
- `led_blink_main.c`: 主程序文件，包含 LED 控制逻辑
- `Kconfig.projbuild`: 配置选项定义
- `CMakeLists.txt`: 构建配置，包含必要的组件依赖

### 使用的 ESP-IDF 组件
- `led_strip`: WS2812 LED 控制组件
- `driver`: GPIO 和 RMT 驱动
- `freertos`: 任务调度和延时

## 故障排除

### LED 不亮
1. 检查开发板型号是否为 ESP32-C3-DevKitM-1
2. 确认使用正确的 GPIO 引脚（GPIO8）
3. 检查硬件连接是否正常

### 编译错误
1. 确保 ESP-IDF 版本支持 led_strip 组件
2. 检查目标芯片设置：`idf.py set-target esp32c3`

### 烧录失败
1. 检查串口连接和权限
2. 尝试按住 BOOT 按钮进入下载模式
3. 降低波特率重试

## 扩展功能

您可以基于这个示例进行以下扩展：
- 改变 LED 颜色（修改 RGB 值）
- 添加不同的闪烁模式
- 通过按钮控制 LED 状态
- 添加 WiFi 远程控制功能

## 技术支持

- [ESP32-C3 技术文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/)
- [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/en/latest/)