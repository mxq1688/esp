# NeoPixel Clock - ESP-IDF 版本

这是一个使用 ESP-IDF 框架开发的 NeoPixel 时钟项目，适用于 ESP32-C3 微控制器。

## 项目简介

使用 60 颗 NeoPixel LED 灯带创建一个圆形时钟，通过 WiFi 连接并使用 NTP 服务器自动同步时间。

### 功能特点

- ✅ 使用 RMT 外设驱动 WS2812B LED 灯带
- ✅ WiFi 自动连接和重连机制
- ✅ NTP 时间同步（每小时自动更新）
- ✅ 时、分、秒三针显示，不同颜色区分
- ✅ WiFi 连接中显示蓝色脉冲动画
- ✅ 连接失败显示红色错误指示

## 硬件要求

- **ESP32-C3** 开发板（推荐 XIAO ESP32C3）
- **WS2812B NeoPixel LED 灯带**（60 颗 LED）
- **5V 电源**（根据 LED 数量选择合适功率）
- **数据线**：LED 数据线连接到 GPIO 10

## 软件要求

- ESP-IDF v5.0 或更高版本
- Python 3.8+

## 配置说明

### 1. WiFi 配置

编辑 `main/include/wifi_manager.h` 文件，修改以下内容：

```c
#define WIFI_SSID      "你的WiFi名称"
#define WIFI_PASSWORD  "你的WiFi密码"
```

### 2. 时区配置

在 `main/include/wifi_manager.h` 中修改时区偏移：

```c
#define GMT_OFFSET_SEC (-18000)      // GMT 偏移（秒），例如 -18000 为 EST
#define DAYLIGHT_OFFSET_SEC (3600)   // 夏令时偏移（秒）
```

### 3. LED 配置

如果需要修改 LED 引脚或数量，编辑 `main/include/neopixel_driver.h`：

```c
#define LED_STRIP_GPIO      10      // LED 数据引脚
#define LED_STRIP_NUM_LEDS  60      // LED 数量
```

### 4. 时钟表盘配置

如果需要调整 12 点钟位置的 LED 偏移，编辑 `main/include/clock_display.h`：

```c
#define LED_OFFSET 27  // 12 点钟位置的 LED 编号
```

## 编译和烧写

### 方法 1：使用脚本（推荐）

1. 修改 `build_c3.sh` 中的串口设置：
   ```bash
   SERIAL_PORT="/dev/cu.usbmodem1101"  # 修改为你的设备端口
   ```

2. 给脚本添加执行权限：
   ```bash
   chmod +x build_c3.sh
   ```

3. 运行脚本：
   ```bash
   ./build_c3.sh
   ```

### 方法 2：手动编译

1. 激活 ESP-IDF 环境：
   ```bash
   source ~/esp/esp-idf/export.sh
   ```

2. 配置项目（首次编译）：
   ```bash
   idf.py menuconfig
   ```

3. 编译项目：
   ```bash
   idf.py build
   ```

4. 烧写到设备：
   ```bash
   idf.py -p /dev/cu.usbmodem1101 flash
   ```

5. 查看串口输出：
   ```bash
   idf.py -p /dev/cu.usbmodem1101 monitor
   ```

## 项目结构

```
neopixel_clock/
├── CMakeLists.txt              # 项目 CMake 配置
├── sdkconfig.defaults          # 默认配置
├── build_c3.sh                 # 编译烧写脚本
├── README.md                   # 项目说明
└── main/
    ├── CMakeLists.txt          # 主组件 CMake 配置
    ├── idf_component.yml       # 组件依赖配置
    ├── main.c                  # 主程序入口
    ├── neopixel_driver.c       # NeoPixel 驱动实现
    ├── wifi_manager.c          # WiFi 和 NTP 管理
    ├── clock_display.c         # 时钟显示逻辑
    └── include/
        ├── neopixel_driver.h   # NeoPixel 驱动头文件
        ├── wifi_manager.h      # WiFi 管理头文件
        └── clock_display.h     # 时钟显示头文件
```

## 颜色配置

时钟指针颜色在 `main/include/clock_display.h` 中定义：

- **时针**：橙色 (RGB: 128, 50, 35)
- **分针**：黄色 (RGB: 192, 164, 164)
- **秒针**：蓝白色 (RGB: 16, 16, 64)
- **时针边缘**：暗橙色 (RGB: 8, 4, 2)

可以根据喜好修改这些值。

## 故障排除

### LED 不亮

1. 检查电源是否足够（每颗 LED 最大 60mA）
2. 确认 GPIO 10 连接正确
3. 检查 LED 灯带的电源地和 ESP32-C3 的地是否连接

### WiFi 连接失败

1. 检查 WiFi 凭据是否正确
2. 确认 WiFi 信号强度
3. 查看串口日志获取详细错误信息

### 时间不准确

1. 检查时区配置是否正确
2. 确认 NTP 服务器可访问
3. 等待几分钟让时间同步完成

### 编译错误

1. 确保 ESP-IDF 版本 >= 5.0
2. 运行 `idf.py fullclean` 清理后重新编译
3. 检查是否正确安装了依赖组件

## 与 Arduino 版本的区别

| 特性 | Arduino 版本 | ESP-IDF 版本 |
|------|-------------|-------------|
| 开发框架 | Arduino | ESP-IDF |
| LED 驱动 | Adafruit_NeoPixel | ESP LED Strip (RMT) |
| WiFi 管理 | WiFi.h | esp_wifi |
| NTP 同步 | configTime() | esp_sntp |
| 性能 | 较低 | 更高 |
| 代码大小 | 较大 | 更小 |
| 可定制性 | 有限 | 完全控制 |

## 许可证

本项目基于原 Arduino 版本改写，遵循相同的开源许可证。

## 参考资料

- [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32c3/)
- [LED Strip 组件文档](https://components.espressif.com/components/espressif/led_strip)
- [原 Arduino 版本](https://github.com/user/neopixel-light-clock)

## 作者

ESP-IDF 版本由 AI 助手基于原 Arduino 代码改写。

