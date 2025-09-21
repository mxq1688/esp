# ESP32 ML307R 4G热点 - Arduino版本

## 🚀 Arduino IDE使用指南

### 第一步：安装Arduino IDE
1. 下载并安装 Arduino IDE 2.0+
2. 启动Arduino IDE

### 第二步：添加ESP32支持
1. 打开 `文件` → `首选项`
2. 在"附加开发板管理器网址"中添加：
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
3. 点击"确定"
4. 打开 `工具` → `开发板` → `开发板管理器`
5. 搜索"ESP32"，安装"esp32 by Espressif Systems"

### 第三步：配置开发板
1. 选择 `工具` → `开发板` → `ESP32 Arduino` → `ESP32 Dev Module`
2. 选择 `工具` → `端口` → `/dev/cu.usbserial-A5069RR4`（或对应的COM端口）
3. 其他设置保持默认

### 第四步：上传代码
1. 打开 `ml307r_4g_hotspot.ino` 文件
2. 点击"上传"按钮（→）
3. 如果上传失败，按住BOOT按钮，点击上传，看到"Connecting..."时松开BOOT按钮

## 📱 功能说明

### WiFi热点
- **名称**: ESP32_4G_Hotspot
- **密码**: 12345678
- **最大连接数**: 默认支持多个设备

### ML307R 4G模块
- **串口**: UART2 (GPIO16, GPIO17)
- **波特率**: 115200
- **功能**: 自动初始化4G连接

## 🔧 硬件连接

```
ESP32     ML307R
GPIO16 ── RXD
GPIO17 ── TXD
3.3V   ── VCC
GND    ── GND
```

## 📊 监视器输出

上传成功后，打开串口监视器（波特率115200）查看运行状态：

```
ESP32 ML307R 4G Hotspot Starting...
ML307R UART initialized
AP IP address: 192.168.4.1
WiFi Hotspot: ESP32_4G_Hotspot, Password: 12345678
Initializing ML307R 4G module...
Sending: AT
Response: OK
...
System ready! Connect your devices to the hotspot.
```

## 🛠️ 故障排除

### 上传失败
1. 确认选择了正确的端口
2. 按住BOOT按钮进行上传
3. 检查USB线缆连接

### ML307R无响应
1. 检查硬件连接
2. 确认SIM卡已插入
3. 检查电源供应

## 📝 自定义配置

可以修改以下参数：

```cpp
// WiFi配置
const char* ssid = "你的热点名称";
const char* password = "你的密码";

// ML307R引脚配置
#define ML307R_RXD_PIN 16  // 可修改
#define ML307R_TXD_PIN 17  // 可修改
```

## 🎉 使用说明

1. 上传代码到ESP32
2. 插入SIM卡到ML307R模块
3. 手机搜索WiFi：ESP32_4G_Hotspot
4. 输入密码：12345678
5. 连接成功即可上网！
