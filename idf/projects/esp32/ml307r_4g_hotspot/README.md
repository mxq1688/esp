# ESP32 ML307R 4G热点项目

这是一个基于ESP32经典款开发板的项目，实现ML307R 4G模块连接和WiFi热点共享功能。

## 功能特性

- ✅ ESP32作为WiFi热点
- ✅ ML307R 4G模块连接
- ✅ 4G网络共享
- ✅ 自动网络配置
- ✅ 多设备连接支持

## 硬件连接

### ESP32与ML307R连接

| ESP32引脚 | ML307R引脚 | 功能 |
|-----------|------------|------|
| GPIO16    | RXD        | 接收数据 |
| GPIO17    | TXD        | 发送数据 |
| 3.3V      | VCC        | 电源 |
| GND       | GND        | 地线 |

### 电源要求
- ESP32: 3.3V/500mA
- ML307R: 3.3V/2A (建议使用外部电源)

## 软件配置

### WiFi热点设置
- SSID: `ESP32_4G_Hotspot`
- 密码: `12345678`
- 最大连接数: 4个设备
- 频道: 1

### ML307R配置
- 波特率: 115200
- 数据位: 8
- 停止位: 1
- 奇偶校验: 无

## 编译和烧录

### 环境要求
- ESP-IDF v4.4 或更高版本
- Python 3.6+
- CMake 3.16+

### 编译步骤

1. 设置ESP-IDF环境
```bash
cd /path/to/esp-idf
./install.sh
. ./export.sh
```

2. 进入项目目录
```bash
cd /path/to/ml307r_4g_hotspot
```

3. 配置项目
```bash
idf.py menuconfig
```

4. 编译项目
```bash
idf.py build
```

5. 烧录到ESP32
```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

## 使用方法

1. 将ML307R模块正确连接到ESP32
2. 插入有效的SIM卡到ML307R模块
3. 给ESP32上电
4. 等待系统启动完成（约30秒）
5. 使用手机或电脑搜索WiFi网络"ESP32_4G_Hotspot"
6. 输入密码"12345678"连接
7. 连接成功后即可通过4G网络上网

## 日志输出

系统启动时会输出详细的日志信息：

```
I (xxxx) ML307R_4G_HOTSPOT: ESP32 ML307R 4G Hotspot Project Starting...
I (xxxx) ML307R: ML307R hardware initialized
I (xxxx) ML307R_4G_HOTSPOT: wifi_init_softap finished. SSID:ESP32_4G_Hotspot password:12345678 channel:1
I (xxxx) ML307R: Starting 4G connection process...
I (xxxx) ML307R: AT test passed
I (xxxx) ML307R: SIM card is ready
I (xxxx) ML307R: CS network registered (home)
I (xxxx) ML307R: PS network registered (home)
I (xxxx) ML307R: PDP context activated successfully
I (xxxx) ML307R: Got IP address: 10.xxx.xxx.xxx
I (xxxx) ML307R: 4G connection established successfully!
I (xxxx) ML307R_4G_HOTSPOT: System ready! WiFi hotspot: ESP32_4G_Hotspot, Password: 12345678
```

## 故障排除

### 常见问题

1. **ML307R无法连接**
   - 检查硬件连接是否正确
   - 确认SIM卡已插入且有效
   - 检查电源供应是否充足

2. **WiFi热点无法连接**
   - 确认ESP32已正确启动
   - 检查SSID和密码是否正确
   - 尝试重启ESP32

3. **4G网络无法上网**
   - 检查SIM卡是否有流量
   - 确认网络运营商支持
   - 查看日志中的网络注册状态

### 调试方法

1. 使用串口监视器查看详细日志
2. 检查ML307R的AT命令响应
3. 验证网络注册状态
4. 测试IP地址获取

## 项目结构

```
ml307r_4g_hotspot/
├── main/
│   ├── main.c              # 主程序
│   └── CMakeLists.txt      # 主程序构建配置
├── components/
│   └── ml307r_driver/      # ML307R驱动组件
│       ├── include/
│       │   └── ml307r.h    # 驱动头文件
│       ├── ml307r.c        # 驱动实现
│       └── CMakeLists.txt  # 组件构建配置
├── CMakeLists.txt          # 项目构建配置
├── sdkconfig.defaults      # 默认配置
└── README.md              # 项目说明
```

## 许可证

本项目采用MIT许可证。

## 贡献

欢迎提交Issue和Pull Request来改进这个项目。

## 联系方式

如有问题，请通过GitHub Issues联系。
