# ESP32-S3 4G 远程摄像头系统

## 项目概述

这是一个基于 ESP32-S3 和 ML307R 4G 模块的智能远程摄像头系统。通过 4G 网络，可以在任何地方实时查看摄像头画面，支持 MJPEG 视频流传输和图像抓拍功能。

## 主要特性

### 🎥 摄像头功能
- **实时视频流** - MJPEG 格式实时视频传输
- **图像抓拍** - 单张图像捕获和下载
- **多种分辨率** - 支持 QVGA(320x240)、VGA(640x480)、SVGA(800x600)
- **质量调节** - 可调节 JPEG 压缩质量
- **自动控制** - 自动曝光、白平衡等

### 📡 网络功能
- **WiFi AP 模式** - ESP32-S3 创建无线热点
- **4G 网络连接** - 通过 ML307R 模块接入 4G 网络
- **远程访问** - 支持通过 4G 网络远程访问
- **网络状态监控** - 实时显示网络连接状态和信号强度

### 🌐 Web 界面
- **实时预览** - 网页端实时查看摄像头画面
- **响应式设计** - 自适应 PC 和移动设备
- **系统监控** - 显示系统状态、内存使用等信息
- **摄像头控制** - 在线调整分辨率和图像质量

### 🚀 系统特性
- **双核处理** - 充分利用 ESP32-S3 双核性能
- **PSRAM 支持** - 使用外部 PSRAM 存储图像缓冲
- **任务监控** - 自动监控和重启异常任务
- **低延迟** - 优化的视频流传输

## 硬件要求

### 必需硬件
- **ESP32-S3 开发板** - 建议使用带 PSRAM 的版本 (如 ESP32-S3-NXRX)
- **摄像头模块** - OV2640 或兼容的摄像头 (支持 JPEG 输出)
- **电源** - 5V/2A 或更高功率

### 可选硬件
- **ML307R 4G 模块** - 用于 4G 网络连接
- **SIM 卡** - 4G 数据卡 (如果使用 4G 功能)

## 硬件连接

### 摄像头连接 (标准 ESP32-S3-EYE 引脚)

```
ESP32-S3          摄像头模块 (OV2640)
├── GPIO15  ────── XCLK
├── GPIO4   ────── SIOD (SDA)
├── GPIO5   ────── SIOC (SCL)
├── GPIO16  ────── D7
├── GPIO17  ────── D6
├── GPIO18  ────── D5
├── GPIO12  ────── D4
├── GPIO10  ────── D3
├── GPIO8   ────── D2
├── GPIO9   ────── D1
├── GPIO11  ────── D0
├── GPIO6   ────── VSYNC
├── GPIO7   ────── HREF
├── GPIO13  ────── PCLK
├── 3.3V    ────── VCC
└── GND     ────── GND
```

### ML307R 4G 模块连接 (可选)

```
ESP32-S3          ML307R
├── GPIO17 (TX) ── RXD
├── GPIO18 (RX) ── TXD
├── 5V/3.3V     ── VCC
└── GND         ── GND
```

**注意**: 根据您的开发板型号，引脚可能有所不同。请参考开发板原理图进行连接。

## 软件环境

### 安装 ESP-IDF

```bash
# 克隆 ESP-IDF 仓库
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# 安装工具链
./install.sh esp32s3

# 设置环境变量
source export.sh
```

### 环境要求
- **ESP-IDF**: v5.0 或更高版本
- **Python**: 3.9+
- **操作系统**: macOS, Linux, Windows (WSL)

## 构建和烧录

### 方法 1: 使用构建脚本 (推荐)

```bash
# 进入项目目录
cd /Users/meng/stu/esp/projects/esp32-s3/esp32s3_4g_camera

# 添加执行权限
chmod +x build_s3.sh

# 构建并烧录
./build_s3.sh
```

构建脚本会自动：
- 检测并设置 ESP-IDF 环境
- 配置目标芯片为 ESP32-S3
- 编译项目
- 自动检测串口
- 烧录固件并启动监视器

### 方法 2: 手动构建

```bash
# 1. 设置 ESP-IDF 环境
source ~/stu/esp/esp-idf/export.sh

# 2. 进入项目目录
cd /Users/meng/stu/esp/projects/esp32-s3/esp32s3_4g_camera

# 3. 设置目标芯片
idf.py set-target esp32s3

# 4. 配置项目 (可选)
idf.py menuconfig

# 5. 构建项目
idf.py build

# 6. 烧录固件
idf.py -p /dev/cu.usbmodem101 flash

# 7. 查看日志
idf.py -p /dev/cu.usbmodem101 monitor
```

### 方法 3: DFU 烧录

```bash
# 生成 DFU 固件
chmod +x build_s3_dfu.sh
./build_s3_dfu.sh

# DFU 文件将生成在: build/esp32s3_4g_camera_dfu.bin
```

## 使用说明

### 首次启动

1. **烧录固件**后，设备会自动启动
2. **WiFi 热点**会自动创建
   - SSID: `ESP32-S3-Camera`
   - 密码: `12345678`
3. **连接热点**后，打开浏览器访问: `http://192.168.4.1`

### Web 界面功能

#### 实时视频流
- 自动显示摄像头实时画面
- 点击 "开始" 按钮刷新视频流
- 点击 "暂停" 按钮停止视频流
- 点击 "抓拍" 按钮捕获当前图像

#### 摄像头控制
- **图像质量**: 拖动滑块调整 JPEG 压缩质量 (0-63，数值越小质量越高)
- **分辨率选择**: 
  - QVGA (320x240) - 低分辨率，高帧率
  - VGA (640x480) - 中等分辨率
  - SVGA (800x600) - 高分辨率，需要更多带宽

#### 系统状态
- **摄像头状态**: 显示摄像头是否就绪
- **4G 状态**: 显示 4G 网络连接状态
- **信号强度**: 显示 4G 信号强度 (dBm)
- **可用内存**: 显示系统可用内存

### API 接口

#### 获取系统状态
```http
GET /api/status
```

返回示例:
```json
{
  "camera_ready": true,
  "network_connected": true,
  "signal_strength": -75,
  "free_heap": 245760,
  "ml307r_state": 3
}
```

#### 获取摄像头配置
```http
GET /api/camera/config
```

#### 视频流
```http
GET /api/camera/stream
```
返回 MJPEG 格式的视频流

#### 图像抓拍
```http
GET /api/camera/capture
```
返回单张 JPEG 图像

#### 设置图像质量
```http
GET /api/camera/quality?value=12
```

#### 设置分辨率
```http
GET /api/camera/resolution?value=SVGA
```

#### 获取网络信息
```http
GET /api/network/info
```

## 配置说明

### 修改 WiFi 配置

编辑 `main/main.c` 文件:

```c
// WiFi配置
#define WIFI_AP_SSID      "ESP32-S3-Camera"    // 修改热点名称
#define WIFI_AP_PASSWORD  "12345678"           // 修改密码
#define WIFI_AP_CHANNEL   1                    // 修改信道
#define WIFI_AP_MAX_CONN  4                    // 最大连接数
```

### 修改摄像头引脚

编辑 `main/include/camera_driver.h` 文件:

```c
// 摄像头引脚定义
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5
// ... 其他引脚定义
```

### 修改 ML307R 配置

编辑 `main/include/ml307r_driver.h` 文件:

```c
// ML307R UART配置
#define ML307R_UART_NUM         UART_NUM_1
#define ML307R_UART_TX_PIN      17
#define ML307R_UART_RX_PIN      18
#define ML307R_UART_BAUD_RATE   115200
```

## 故障排除

### 摄像头初始化失败

**问题**: 启动时显示 "摄像头初始化失败"

**解决方法**:
1. 检查摄像头连接是否正确
2. 确认引脚定义与硬件匹配
3. 检查摄像头模块是否损坏
4. 确保电源供应充足 (建议 5V/2A)

```bash
# 查看详细日志
idf.py monitor
```

### WiFi 连接问题

**问题**: 无法连接到 WiFi 热点

**解决方法**:
1. 确认 SSID 和密码正确
2. 重启设备
3. 检查设备是否处于 AP 模式
4. 查看日志确认 WiFi 初始化成功

### Web 界面无法访问

**问题**: 浏览器无法打开 `http://192.168.4.1`

**解决方法**:
1. 确认已连接到设备的 WiFi 热点
2. 检查 IP 地址是否正确 (默认为 192.168.4.1)
3. 尝试清除浏览器缓存
4. 使用其他浏览器或设备测试

### 视频流卡顿或无法播放

**问题**: 视频流播放不流畅或无法显示

**解决方法**:
1. 降低图像分辨率 (选择 QVGA 或 VGA)
2. 增加 JPEG 压缩质量值 (减小图像大小)
3. 检查网络连接质量
4. 确认摄像头工作正常

### ML307R 通信失败

**问题**: 4G 模块无法正常通信

**解决方法**:
1. 检查 UART 连接 (TX/RX 是否交叉连接)
2. 确认波特率设置正确 (默认 115200)
3. 检查 ML307R 模块电源
4. 确认 SIM 卡已插入并激活

### 内存不足

**问题**: 系统提示内存不足或崩溃

**解决方法**:
1. 确保使用带 PSRAM 的 ESP32-S3 开发板
2. 降低图像分辨率
3. 减少视频流帧率
4. 检查内存泄漏

## 性能优化

### 提高视频流帧率

1. **降低分辨率**: 使用 QVGA (320x240)
2. **增加压缩**: 提高 JPEG 质量值 (减小图像大小)
3. **优化网络**: 确保良好的 WiFi 信号
4. **调整缓冲**: 修改 `sdkconfig` 中的缓冲区大小

### 降低延迟

1. **减少帧间隔**: 修改 `api_handlers.c` 中的 `vTaskDelay` 时间
2. **优化编码**: 使用硬件 JPEG 编码器
3. **网络优化**: 使用有线连接或更快的 WiFi 标准

### 节省内存

1. **减少帧缓冲**: 在 `camera_driver.h` 中设置 `CAM_FB_COUNT = 1`
2. **降低分辨率**: 默认使用较低的分辨率
3. **优化任务栈**: 减少任务栈大小

## 开发指南

### 项目结构

```
esp32s3_4g_camera/
├── main/
│   ├── main.c                 # 主程序入口
│   ├── camera_driver.c        # 摄像头驱动
│   ├── ml307r_driver.c        # ML307R 4G模块驱动
│   ├── image_processor.c      # 图像处理
│   ├── web_server.c           # Web服务器
│   ├── api_handlers.c         # API处理器
│   ├── web_files.c            # Web文件
│   └── include/               # 头文件
│       ├── camera_driver.h
│       ├── ml307r_driver.h
│       ├── image_processor.h
│       ├── web_server.h
│       ├── api_handlers.h
│       └── web_files.h
├── CMakeLists.txt             # CMake配置
├── partitions.csv             # 分区表
├── sdkconfig.defaults         # SDK默认配置
├── idf_component.yml          # 组件依赖
├── build_s3.sh               # 构建脚本
├── build_s3_dfu.sh           # DFU构建脚本
└── README.md                  # 本文档
```

### 添加新功能

1. **添加新的API接口**:
   - 在 `api_handlers.h` 中声明处理函数
   - 在 `api_handlers.c` 中实现处理函数
   - 在 `api_handlers_register` 函数中注册 URI

2. **修改摄像头参数**:
   - 编辑 `camera_driver.c` 中的初始化代码
   - 调整分辨率、质量等参数

3. **扩展Web界面**:
   - 修改 `web_server.c` 中的 HTML 内容
   - 添加新的 JavaScript 功能

## 技术规格

### 系统参数
- **处理器**: ESP32-S3 双核 240MHz
- **内存**: 512KB SRAM + 8MB PSRAM
- **Flash**: 4MB
- **WiFi**: 802.11 b/g/n
- **蓝牙**: BLE 5.0

### 摄像头参数
- **传感器**: OV2640
- **最大分辨率**: 1600x1200 (UXGA)
- **支持格式**: JPEG, RGB565, YUV422
- **帧率**: 最高 30fps (取决于分辨率)

### 网络参数
- **WiFi AP**: 2.4GHz, 支持最多 4 个连接
- **4G**: 支持 LTE Cat.4 (ML307R)
- **Web服务器**: HTTP/1.1
- **视频流**: MJPEG over HTTP

## 许可证

本项目采用 MIT 许可证。详见 LICENSE 文件。

## 贡献

欢迎提交 Issue 和 Pull Request！

### 开发要求
- 遵循 ESP-IDF 编码规范
- 添加适当的注释和文档
- 测试所有新功能
- 更新 README 文档

## 常见问题

**Q: 可以使用其他型号的摄像头吗？**  
A: 可以，但需要修改引脚定义和驱动代码。建议使用 OV2640 或 OV5640。

**Q: 不使用 4G 模块可以吗？**  
A: 可以，系统会检测 ML307R 模块，如果未连接会跳过初始化，仅使用 WiFi 功能。

**Q: 如何实现远程访问？**  
A: 需要配置 ML307R 的 4G 连接，并设置端口转发或使用 VPN 方案。

**Q: 支持录像功能吗？**  
A: 当前版本不支持，但可以通过抓拍功能定时保存图像。未来版本可能添加录像功能。

**Q: 功耗如何？**  
A: 正常运行约 500-800mA (取决于摄像头和网络活动)，建议使用 5V/2A 电源适配器。

## 更新日志

### v1.0.0 (2025-11-25)
- ✅ 初始版本发布
- ✅ 摄像头驱动实现
- ✅ ML307R 4G 模块支持
- ✅ Web 界面和视频流
- ✅ RESTful API 接口
- ✅ 多分辨率支持
- ✅ 图像质量控制

## 联系方式

如有问题或建议，请通过以下方式联系：
- 提交 GitHub Issue
- 发送邮件至项目维护者

---

**祝您使用愉快！📷**

