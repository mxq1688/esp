# 快速开始指南

## 5 分钟上手

### 1. 配置 WiFi

编辑 `main/include/wifi_manager.h`：

```c
#define WIFI_SSID      "你的WiFi名称"
#define WIFI_PASSWORD  "你的WiFi密码"
```

### 2. 连接硬件

```
ESP32-C3          NeoPixel LED 灯带
---------         ----------------
GPIO 10    ---->  DIN (数据输入)
GND        ---->  GND
5V         ---->  5V (或使用外部电源)
```

**注意**：如果 LED 数量较多（>10 颗），建议使用外部 5V 电源，并确保 ESP32-C3 和电源共地。

### 3. 编译烧写

```bash
# 修改串口（如果需要）
nano build_c3.sh  # 修改 SERIAL_PORT 变量

# 运行编译烧写脚本
./build_c3.sh
```

### 4. 查看日志

烧写完成后会自动进入监视器模式，你会看到：

```
I (xxx) main: === NeoPixel Clock Starting ===
I (xxx) wifi_manager: Connecting to WiFi...
I (xxx) wifi_manager: Connected to WiFi successfully
I (xxx) wifi_manager: Time synchronized with NTP server
I (xxx) main: === NeoPixel Clock Started ===
```

按 `Ctrl + ]` 退出监视器。

## 常见问题

### Q: 如何修改时区？

A: 编辑 `main/include/wifi_manager.h`：

```c
// 中国时区 (UTC+8)
#define GMT_OFFSET_SEC (28800)      // 8 * 3600
#define DAYLIGHT_OFFSET_SEC (0)     // 中国不使用夏令时

// 美国东部时区 (EST, UTC-5)
#define GMT_OFFSET_SEC (-18000)     // -5 * 3600
#define DAYLIGHT_OFFSET_SEC (3600)  // 夏令时 +1 小时
```

### Q: LED 颜色如何调整？

A: 编辑 `main/include/clock_display.h`，修改颜色定义：

```c
// 时针 - 橙色
#define HOUR_COLOR_R    128
#define HOUR_COLOR_G    50
#define HOUR_COLOR_B    35

// 分针 - 黄色
#define MINUTE_COLOR_R  192
#define MINUTE_COLOR_G  164
#define MINUTE_COLOR_B  164

// 秒针 - 蓝白色
#define SECOND_COLOR_R  16
#define SECOND_COLOR_G  16
#define SECOND_COLOR_B  64
```

### Q: 如何修改 LED 引脚？

A: 编辑 `main/include/neopixel_driver.h`：

```c
#define LED_STRIP_GPIO      10      // 改为你想要的 GPIO
```

### Q: 12 点钟位置不对怎么办？

A: 编辑 `main/include/clock_display.h`，调整偏移值：

```c
#define LED_OFFSET 27  // 修改这个值 (0-59)
```

## 调试技巧

### 查看详细日志

编辑 `sdkconfig.defaults`，启用详细日志：

```
CONFIG_LOG_DEFAULT_LEVEL_DEBUG=y
```

### 单独运行监视器

```bash
idf.py -p /dev/cu.usbmodem1101 monitor
```

### 清理重新编译

```bash
idf.py fullclean
idf.py build
```

## 性能优化

### 降低功耗

在 `main/main.c` 中增加延迟：

```c
vTaskDelay(pdMS_TO_TICKS(100)); // 从 50ms 改为 100ms
```

### 提高响应速度

在 `main/main.c` 中减少延迟：

```c
vTaskDelay(pdMS_TO_TICKS(20)); // 从 50ms 改为 20ms
```

## 下一步

- 添加 Web 界面进行配置
- 实现多种显示模式
- 添加环境光传感器自动调节亮度
- 支持 MQTT 远程控制

祝你玩得开心！🎉

