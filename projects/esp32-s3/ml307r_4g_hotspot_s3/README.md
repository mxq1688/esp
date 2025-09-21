# ESP32-S3 ML307R 4G热点控制器

## 项目概述

这是一个基于ESP32-S3和ML307R 4G模块的智能热点控制器项目，通过Web界面管理4G网络连接和热点功能。项目集成了现代化的Web控制界面，支持实时监控网络状态、配置热点参数和系统管理。

## 主要特性

### 🚀 核心功能
- **ESP32-S3双核处理器** - 专为ESP32-S3芯片优化
- **ML307R 4G模块集成** - 支持4G网络连接和热点功能
- **WiFi STA/AP双模式** - 支持Station模式和Access Point模式
- **现代化Web控制界面** - 响应式设计，适配PC和移动设备
- **RESTful API** - 标准化的API接口
- **实时状态监控** - 实时显示网络状态和系统信息
- **AT命令控制** - 完整的ML307R AT命令支持

### 📱 设备适配
- **PC端优化** - 大屏幕显示，完整功能体验
- **移动端优化** - 触摸友好，响应式设计
- **跨平台支持** - 支持各种现代浏览器

### 🎨 界面特性
- **实时网络信息** - 运营商、信号强度、网络类型显示
- **热点配置管理** - SSID、密码、连接数配置
- **WiFi连接管理** - 支持连接外部WiFi网络
- **系统状态监控** - 内存使用、运行时间等信息
- **模块控制** - ML307R重启和状态管理

## 硬件连接

### ESP32-S3 与 ML307R 连接图
```
ESP32-S3 NXRX开发板          ML307R模块
├── GPIO17 (TX) ──────────── RXD
├── GPIO18 (RX) ──────────── TXD  
├── 5V/3.3V    ──────────── VCC
└── GND        ──────────── GND
```

**重要说明**：
- 根据ESP32-S3 NXRX开发板原理图，GPIO43/44没有引出
- 使用GPIO17/18作为UART1通信引脚
- 电源和复位控制引脚设为可选（代码中为-1，表示不使用）

### 详细连接说明
- **UART通信**: ESP32-S3的UART1与ML307R通信
- **电源控制**: GPIO13控制ML307R电源开关
- **复位控制**: GPIO14控制ML307R复位
- **供电**: 使用5V供电，确保电流充足（建议2A以上）

## 技术架构

### 后端技术栈
- **ESP-IDF v5.0+** - 官方开发框架
- **FreeRTOS** - 实时操作系统
- **ESP HTTP Server** - 内置Web服务器
- **cJSON** - JSON数据处理
- **UART驱动** - ML307R通信驱动

### 前端技术栈
- **原生HTML5/CSS3/JavaScript** - 无框架依赖
- **响应式CSS Grid/Flexbox** - 现代布局
- **Fetch API** - 异步数据交互
- **现代CSS动画** - 流畅的用户体验

### 文件结构
```
ml307r_4g_hotspot_s3/
├── main/
│   ├── main.c                 # 主程序入口
│   ├── ml307r_driver.c        # ML307R驱动实现
│   ├── web_server.c           # Web服务器实现
│   ├── api_handlers.c         # API处理器
│   ├── web_files.c            # 嵌入的Web文件
│   ├── wifi_manager.c         # WiFi管理
│   └── include/
│       ├── ml307r_driver.h    # ML307R驱动接口
│       ├── web_server.h       # Web服务器接口
│       ├── api_handlers.h     # API处理器接口
│       ├── wifi_manager.h     # WiFi管理接口
│       └── web_files.h        # Web文件声明
├── CMakeLists.txt             # 项目构建配置
├── partitions.csv             # 分区表配置
├── build_s3.sh               # 构建脚本
├── build_s3_dfu.sh           # DFU构建脚本
└── README.md                  # 项目说明
```

## 安装和配置

### 1. 环境准备
```bash
# 安装ESP-IDF v5.0+
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
source export.sh
```

### 2. 项目构建
```bash
# 进入项目目录
cd /Users/meng/stu/esp/projects/esp32-s3/ml307r_4g_hotspot_s3

# 使用构建脚本
./build_s3.sh

# 或手动构建
idf.py build
```

### 3. 烧录固件

#### 方法1: 使用构建脚本（推荐）
```bash
./build_s3.sh  # 自动构建并烧录
```

#### 方法2: 手动烧录
```bash
# 自动检测端口烧录
idf.py flash

# 指定端口烧录
idf.py -p /dev/cu.usbmodem101 flash
```

#### 方法3: DFU烧录
```bash
# 生成DFU文件并烧录
./build_s3_dfu.sh
```

### 4. 监控输出
```bash
# 启动串口监视器
idf.py monitor

# 或指定端口
idf.py -p /dev/cu.usbmodem101 monitor
```

## 使用说明

### 首次启动
1. **烧录固件**后，设备会自动启动
2. **AP模式**：设备会创建WiFi热点 `ESP32-S3-ML307R`
3. **连接热点**：密码 `12345678`
4. **访问控制界面**：浏览器访问 `http://192.168.4.1`

### Web界面功能

#### 网络信息监控
- 实时显示运营商信息
- 信号强度监控（dBm）
- 网络类型显示（2G/3G/4G）
- IP地址显示

#### 4G热点控制
- 热点SSID配置
- 密码设置（最少8位）
- 最大连接数设置
- 一键启用/禁用热点

#### WiFi连接管理
- 连接外部WiFi网络
- 显示当前连接状态
- IP地址显示

#### 系统管理
- ML307R模块重启
- 系统状态监控
- 内存使用情况
- 运行时间统计

### AT命令支持

项目支持以下主要AT命令：
- `AT` - 测试连接
- `AT+COPS?` - 查询运营商
- `AT+CSQ` - 查询信号强度
- `AT+CREG?` - 查询网络注册状态
- `AT+CGPADDR=1` - 查询IP地址
- `AT+WIFIAP` - 配置WiFi热点
- `AT+WIFIAPEN` - 启用/禁用WiFi热点

## API接口

### 系统状态
```http
GET /api/status
```
返回系统整体状态，包括ML307R状态、WiFi状态、系统信息等。

### 网络信息
```http
GET /api/network/info
```
获取ML307R网络信息，包括运营商、信号强度、IP地址等。

### 热点控制
```http
POST /api/hotspot/control
Content-Type: application/json

{
    "enable": true,
    "ssid": "MyHotspot",
    "password": "12345678",
    "max_connections": 5
}
```

### 热点配置
```http
GET /api/hotspot/config
POST /api/hotspot/config
```

### ML307R重启
```http
POST /api/ml307r/reset
```

### WiFi连接
```http
POST /api/wifi/connect
Content-Type: application/json

{
    "ssid": "your_wifi_name",
    "password": "your_wifi_password"
}
```

## 配置说明

### ML307R配置
- **UART波特率**: 115200
- **数据位**: 8
- **停止位**: 1
- **校验位**: 无
- **流控**: 无

### WiFi配置
- **AP模式SSID**: ESP32-S3-ML307R
- **AP模式密码**: 12345678
- **AP模式信道**: 1
- **最大连接数**: 4

### GPIO配置
- **ML307R_UART_TX**: GPIO11
- **ML307R_UART_RX**: GPIO12
- **ML307R_POWER**: GPIO13
- **ML307R_RESET**: GPIO14

## 故障排除

### 常见问题

#### 1. 编译错误
```bash
# 确保ESP-IDF环境正确设置
source /Users/meng/stu/esp/esp-idf/export.sh

# 清理并重新构建
idf.py fullclean
idf.py build
```

#### 2. 烧录失败
- 检查USB连接
- 确保设备处于下载模式
- 检查串口权限
- 尝试不同的波特率

#### 3. ML307R通信失败
- 检查UART连接
- 确认电源供应充足
- 检查AT命令响应
- 验证波特率设置

#### 4. Web界面无法访问
- 确认WiFi连接到设备热点
- 检查IP地址 (192.168.4.1)
- 尝试清除浏览器缓存
- 检查防火墙设置

#### 5. 4G网络连接问题
- 确认SIM卡已插入ML307R
- 检查信号强度
- 验证运营商网络
- 查看AT命令响应

### 调试信息
```bash
# 查看详细日志
idf.py monitor

# 查看构建信息
idf.py size-components

# 查看分区信息
idf.py partition-table
```

## 开发指南

### 添加新功能
1. 在相应的头文件中声明接口
2. 在源文件中实现功能
3. 在API处理器中添加HTTP接口
4. 更新Web界面

### 自定义配置
- 修改 `ml307r_driver.h` 中的GPIO定义
- 调整 `wifi_manager.h` 中的WiFi参数
- 更新 `partitions.csv` 分区配置

### 性能优化
- 调整任务优先级和堆栈大小
- 优化UART缓冲区大小
- 减少不必要的日志输出

## 版本历史

### v1.0.0 (当前版本)
- ✅ ESP32-S3与ML307R UART通信
- ✅ 完整的AT命令支持
- ✅ Web控制界面
- ✅ 4G热点管理功能
- ✅ WiFi连接管理
- ✅ 系统状态监控
- ✅ 响应式Web设计

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 贡献指南

欢迎提交Issue和Pull Request来改进项目！

### 开发环境
- ESP-IDF v5.0+
- Python 3.9+
- 支持ESP32-S3的开发板
- ML307R 4G模块

### 代码规范
- 遵循ESP-IDF编码规范
- 添加适当的注释
- 保持代码简洁清晰

## 联系方式

如有问题或建议，请通过以下方式联系：
- 提交GitHub Issue
- 发送邮件至：[your-email@example.com]

---

**注意**：本项目需要ML307R 4G模块和有效的SIM卡才能正常使用4G功能。请确保硬件连接正确，电源供应充足。
