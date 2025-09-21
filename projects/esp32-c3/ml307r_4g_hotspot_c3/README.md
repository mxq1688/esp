# ESP32-C3 ML307R 4G热点项目

这是一个基于ESP32-C3开发板的项目，实现ML307R 4G模块连接和WiFi热点共享功能。

## 🚀 ESP32-C3特性

- ✅ **单核RISC-V架构**：32位RISC-V处理器，160MHz
- ✅ **精简设计**：更低功耗，适合IoT应用
- ✅ **WiFi 4 + Bluetooth 5**：完整无线连接方案
- ✅ **22个GPIO引脚**：丰富的外设接口

## 功能特性

- ✅ ESP32-C3作为WiFi热点
- ✅ ML307R 4G模块连接
- ✅ 4G网络共享
- ✅ 单核优化任务调度
- ✅ 低功耗设计

## 硬件连接

### ESP32-C3与ML307R连接

| ESP32-C3引脚 | ML307R引脚 | 功能 |
|-------------|------------|------|
| GPIO4       | RXD        | C3发送→ML307R接收 |
| GPIO5       | TXD        | C3接收←ML307R发送 |
| 5V          | +5V        | 电源（使用板载5V输出） |
| GND         | GND        | 地线 |
| GPIO6 (可选) | EN         | 使能控制（代替RST功能） |

### 引脚说明
- **GPIO4**: UART1_RXD（C3接收ML307R数据）
- **GPIO5**: UART1_TXD（C3发送数据到ML307R）

## 软件配置

### WiFi热点设置
- SSID: `ESP32C3_4G_Hotspot`
- 密码: `12345678`
- 最大连接数: 4个设备
- 频道: 1

### ML307R配置
- 串口: UART1
- 波特率: 115200
- 数据位: 8
- 停止位: 1
- 奇偶校验: 无

## 编译和烧录

### 环境要求
- ESP-IDF v5.0+ (支持ESP32-C3)
- Python 3.8+
- CMake 3.16+

### 编译步骤

1. 设置ESP-IDF环境
```bash
cd /path/to/esp-idf
./install.sh esp32c3
. ./export.sh
```

2. 进入项目目录
```bash
cd /path/to/ml307r_4g_hotspot_c3
```

3. 设置目标芯片
```bash
idf.py set-target esp32c3
```

4. 配置项目（可选）
```bash
idf.py menuconfig
```

5. 编译项目
```bash
idf.py build
```

6. 烧录到ESP32-C3
```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

## ESP32-C3特殊说明

### 单核优化
ESP32-C3是单核处理器，项目已优化：
- 使用`CONFIG_FREERTOS_UNICORE=y`
- 任务栈大小适配C3内存限制
- 中断处理优化

### 内存管理
ESP32-C3内存较少，已优化：
- 减少缓冲区大小
- 优化任务栈分配
- 精简日志输出

### 引脚映射
ESP32-C3引脚与ESP32不同：
- 使用GPIO20/21作为UART1
- 避免冲突的专用引脚
- 优化电源管理

## 使用方法

1. 将ML307R模块正确连接到ESP32-C3
2. 插入有效的SIM卡到ML307R模块
3. 给ESP32-C3上电
4. 等待系统启动完成（约30秒）
5. 使用手机或电脑搜索WiFi网络"ESP32C3_4G_Hotspot"
6. 输入密码"12345678"连接
7. 连接成功后即可通过4G网络上网

## 日志输出

系统启动时会输出详细的日志信息：

```
I (xxxx) ML307R_4G_HOTSPOT_C3: ESP32-C3 ML307R 4G Hotspot Project Starting...
I (xxxx) ML307R_4G_HOTSPOT_C3: Single-core RISC-V ESP32-C3 detected
I (xxxx) ML307R_C3: Initializing ML307R for ESP32-C3 (RISC-V)
I (xxxx) ML307R_C3: ML307R initialized successfully on ESP32-C3 GPIO21(TX) GPIO20(RX)
I (xxxx) ML307R_4G_HOTSPOT_C3: wifi_init_softap finished. SSID:ESP32C3_4G_Hotspot password:12345678 channel:1
I (xxxx) ML307R_C3: C3: Starting 4G connection process...
I (xxxx) ML307R_C3: C3: AT test passed
I (xxxx) ML307R_C3: C3: SIM card is ready
I (xxxx) ML307R_C3: C3: CS network registered (home)
I (xxxx) ML307R_C3: C3: PS network registered (home)
I (xxxx) ML307R_C3: C3: PDP context activated successfully
I (xxxx) ML307R_C3: C3: Got IP address: 10.xxx.xxx.xxx
I (xxxx) ML307R_C3: C3: 4G connection established successfully!
I (xxxx) ML307R_4G_HOTSPOT_C3: System ready! WiFi hotspot: ESP32C3_4G_Hotspot, Password: 12345678
```

## 故障排除

### ESP32-C3特殊问题

1. **烧录失败**
   - 确认选择了esp32c3目标
   - 检查USB-C接口连接
   - 尝试长按BOOT按钮

2. **内存不足**
   - 减少日志输出级别
   - 优化任务栈大小
   - 检查内存泄漏

3. **单核任务调度**
   - 避免阻塞主任务
   - 合理设置任务优先级
   - 使用事件组同步

### 常见问题

1. **ML307R无法连接**
   - 检查GPIO4/5连接
   - 确认SIM卡已插入
   - 检查3.3V电源供应

2. **WiFi热点无法连接**
   - 确认ESP32-C3正确启动
   - 检查SSID和密码
   - 尝试重启设备

## 性能对比

| 特性 | ESP32经典款 | ESP32-C3 |
|------|------------|----------|
| 架构 | Xtensa双核 | RISC-V单核 |
| 频率 | 240MHz | 160MHz |
| RAM | 520KB | 400KB |
| Flash | 支持外置 | 支持外置 |
| 功耗 | 较高 | 更低 |
| 成本 | 标准 | 更低 |

## 项目结构

```
ml307r_4g_hotspot_c3/
├── main/
│   ├── main.c              # 主程序（C3优化）
│   └── CMakeLists.txt      # 主程序构建配置
├── components/
│   └── ml307r_driver/      # ML307R驱动组件
│       ├── include/
│       │   └── ml307r.h    # 驱动头文件（C3适配）
│       ├── ml307r.c        # 驱动实现（C3优化）
│       └── CMakeLists.txt  # 组件构建配置
├── CMakeLists.txt          # 项目构建配置
├── sdkconfig.defaults      # C3默认配置
└── README.md              # 项目说明
```

## 许可证

本项目采用MIT许可证。

## 贡献

欢迎提交Issue和Pull Request来改进这个ESP32-C3项目。
