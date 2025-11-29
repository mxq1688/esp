# ESP32-C3 WiFi LED Web控制器

## 项目概述

这是一个基于ESP32-C3的WiFi LED Web控制器项目，提供现代化的Web界面来控制WS2812 RGB LED灯带。项目已优化为适配PC和H5设备，并移除了SPIFFS文件系统依赖。

## 主要特性

### 🚀 核心功能
- **ESP32-C3 RISC-V处理器优化** - 专为ESP32-C3芯片优化
- **WiFi STA/AP双模式** - 支持Station模式和Access Point模式
- **现代化Web控制界面** - 响应式设计，适配PC和H5设备
- **RESTful API** - 标准化的API接口
- **WS2812 RGB LED控制** - 完整的颜色和亮度控制
- **实时状态监控** - 实时显示连接状态和系统信息
- **低功耗模式** - 优化的电源管理

### 📱 设备适配
- **PC端优化** - 大屏幕显示，完整功能体验
- **H5移动端优化** - 触摸友好，PWA支持
- **响应式设计** - 自适应不同屏幕尺寸
- **PWA应用** - 可安装为移动应用
- **触摸优化** - 针对触摸设备优化交互

### 🎨 界面特性
- **实时颜色预览** - 直观的颜色选择器
- **RGB滑块控制** - 精确的颜色调节
- **亮度控制** - 0-100%亮度调节
- **预设颜色** - 快速选择常用颜色
- **特效模式** - 彩虹、呼吸、闪烁等特效
- **状态指示** - 实时连接状态显示

## 技术架构

### 后端技术栈
- **ESP-IDF v6.0** - 官方开发框架
- **FreeRTOS** - 实时操作系统
- **ESP HTTP Server** - 内置Web服务器
- **cJSON** - JSON数据处理
- **LED Strip库** - WS2812控制库

### 前端技术栈
- **原生HTML5/CSS3/JavaScript** - 无框架依赖
- **响应式CSS Grid/Flexbox** - 现代布局
- **PWA技术** - Service Worker + Manifest
- **触摸事件优化** - 移动端友好
- **深色模式支持** - 自动适配系统主题

### 文件结构
```
wifi_led_web_controller/
├── main/
│   ├── main.c                 # 主程序入口
│   ├── web_server.c           # Web服务器实现
│   ├── api_handlers.c         # API处理器
│   ├── web_files.c            # 嵌入的Web文件
│   ├── wifi_manager.c         # WiFi管理
│   ├── led_controller.c       # LED控制
│   └── include/
│       ├── web_files.h        # Web文件声明
│       ├── web_server.h       # Web服务器接口
│       ├── wifi_manager.h     # WiFi管理接口
│       └── led_controller.h   # LED控制接口
├── CMakeLists.txt             # 项目构建配置
└── README.md                  # 项目说明
```

## 硬件连接

### ESP32-C3-DevKitM-1 连接图
```
ESP32-C3-DevKitM-1
├── WS2812 RGB LED: GPIO8 (板载)
├── Button:         GPIO9 (内置按钮)
└── USB-C:         编程和供电
```

### 外部WS2812连接（可选）
```
WS2812 LED Strip
├── VCC:  5V电源
├── GND:  地线
└── DIN:  GPIO8 (数据线)
```

## 安装和配置

### 1. 环境准备
```bash
# 安装ESP-IDF v6.0
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32c3
source export.sh
```

### 2. 项目构建
```bash
# 克隆项目
git clone <repository-url>
cd wifi_led_web_controller

# 构建项目
idf.py build
```

### 3. 烧录固件
```bash
# 自动检测端口烧录
idf.py flash

# 指定端口烧录
idf.py -p /dev/cu.usbmodem1101 flash
```

### 4. 监控输出
```bash
# 启动串口监视器
idf.py monitor
```

## 使用说明

### 首次启动
1. **烧录固件**后，设备会自动启动
2. **AP模式**：设备会创建WiFi热点 `ESP32-C3-LED`
3. **连接热点**：使用手机或电脑连接该WiFi
4. **访问控制界面**：浏览器访问 `http://192.168.4.1`

### WiFi配置
1. **连接设备热点**后，访问控制界面
2. **输入WiFi信息**：SSID和密码
3. **保存配置**：设备会连接到指定WiFi
4. **获取IP地址**：查看串口输出获取设备IP

### 控制界面
- **颜色控制**：使用RGB滑块调节颜色
- **亮度控制**：调节整体亮度
- **电源控制**：开启/关闭LED
- **特效模式**：选择不同的灯光特效
- **预设颜色**：快速选择常用颜色

### 按钮操作
- **短按**：切换LED开关
- **长按3秒**：重置WiFi配置

## API接口

### 系统状态
```http
GET /api/status
```

### LED颜色控制
```http
POST /api/led/color
Content-Type: application/json

{
    "r": 255,
    "g": 128,
    "b": 64,
    "brightness": 75
}
```

### LED电源控制
```http
POST /api/led/power
Content-Type: application/json

{
    "power": true
}
```

### LED特效控制
```http
POST /api/led/effect
Content-Type: application/json

{
    "effect": "rainbow"
}
```

## 技术亮点

### 🎯 无SPIFFS设计
- **直接嵌入**：HTML/CSS/JS直接编译到固件中
- **快速启动**：无需文件系统初始化
- **节省空间**：减少Flash占用
- **提高可靠性**：避免文件系统损坏

### 📱 移动端优化
- **触摸友好**：44px最小触摸目标
- **手势支持**：防止误触和缩放
- **PWA支持**：可安装为应用
- **离线缓存**：Service Worker缓存

### 🎨 响应式设计
- **PC端**：大屏幕完整布局
- **平板端**：中等屏幕适配
- **手机端**：小屏幕优化
- **深色模式**：自动适配系统主题

### ⚡ 性能优化
- **内存优化**：减少动态内存分配
- **网络优化**：CORS支持，JSON压缩
- **渲染优化**：CSS硬件加速
- **交互优化**：防抖和节流

## 故障排除

### 常见问题

#### 1. 编译错误
```bash
# 确保ESP-IDF环境正确设置
source /path/to/esp-idf/export.sh
```

#### 2. 烧录失败
```bash
# 检查串口连接
ls /dev/cu.*
# 确保设备处于下载模式
```

#### 3. WiFi连接失败
- 检查WiFi密码是否正确
- 确保信号强度足够
- 查看串口输出的错误信息

#### 4. Web界面无法访问
- 确认设备IP地址
- 检查防火墙设置
- 尝试AP模式访问

### 调试信息
```bash
# 查看详细日志
idf.py monitor

# 查看构建信息
idf.py size-components
```

## 版本历史

### v2.0.0 (当前版本)
- ✅ 移除SPIFFS依赖，直接嵌入Web文件
- ✅ 优化PC和H5设备适配
- ✅ 添加PWA支持
- ✅ 改进响应式设计
- ✅ 增强触摸设备支持
- ✅ 添加深色模式支持

### v1.0.0
- 基础WiFi LED控制功能
- SPIFFS文件系统
- 基础Web界面

## 贡献指南

欢迎提交Issue和Pull Request来改进项目！

### 开发环境
- ESP-IDF v6.0+
- Python 3.9+
- 支持ESP32-C3的开发板

### 代码规范
- 遵循ESP-IDF编码规范
- 添加适当的注释
- 保持代码简洁清晰

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 提交GitHub Issue
- 发送邮件至：[your-email@example.com]

---

**注意**：本项目专为ESP32-C3芯片优化，其他ESP32系列芯片可能需要适配。
