# ESP32-S3 4G 远程摄像头 - 快速开始指南

## 5 分钟快速上手

### 第 1 步: 硬件连接

#### 必需连接 (摄像头)
```
ESP32-S3        摄像头
GPIO15    →     XCLK
GPIO4     →     SIOD
GPIO5     →     SIOC
GPIO16    →     D7
GPIO17    →     D6
GPIO18    →     D5
GPIO12    →     D4
GPIO10    →     D3
GPIO8     →     D2
GPIO9     →     D1
GPIO11    →     D0
GPIO6     →     VSYNC
GPIO7     →     HREF
GPIO13    →     PCLK
3.3V      →     VCC
GND       →     GND
```

#### 可选连接 (4G模块)
```
ESP32-S3        ML307R
GPIO17(TX) →    RXD
GPIO18(RX) →    TXD
5V         →    VCC
GND        →    GND
```

### 第 2 步: 环境准备

```bash
# 设置 ESP-IDF 环境 (仅需一次)
cd ~/stu/esp/esp-idf
source export.sh
```

### 第 3 步: 构建和烧录

```bash
# 进入项目目录
cd /Users/meng/stu/esp/projects/esp32-s3/esp32s3_4g_camera

# 一键构建和烧录
./build_s3.sh
```

### 第 4 步: 连接和访问

1. **连接 WiFi 热点**
   - SSID: `ESP32-S3-Camera`
   - 密码: `12345678`

2. **打开浏览器访问**
   - 地址: `http://192.168.4.1`

3. **开始使用**
   - 查看实时视频流
   - 抓拍图像
   - 调整摄像头参数

## 常用命令

### 查看日志
```bash
idf.py monitor
```

### 重新烧录
```bash
idf.py -p /dev/cu.usbmodem101 flash
```

### 完整重新构建
```bash
idf.py fullclean
idf.py build
```

## 故障快速修复

### 摄像头无法初始化
- 检查连接
- 确认引脚正确
- 重新上电

### WiFi 无法连接
- 重启设备
- 检查密码
- 更换设备尝试

### Web 界面无法访问
- 确认已连接 WiFi 热点
- 检查 IP: 192.168.4.1
- 清除浏览器缓存

## 下一步

- 阅读完整的 [README.md](README.md)
- 自定义 WiFi 配置
- 集成 4G 模块
- 开发自定义功能

## 技术支持

遇到问题？查看 README.md 的"故障排除"部分。

