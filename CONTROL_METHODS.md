# ESP32 LED控制方式大全

## 🎮 控制方式概览

| 控制方式 | 特点 | 使用场景 | 复杂度 |
|---------|------|----------|--------|
| 🌐 **Web API** | HTTP接口，跨平台 | 远程控制，Web应用 | ⭐️⭐️ |
| 🔌 **串口控制** | 直接连接，调试方便 | 开发调试，有线控制 | ⭐️ |
| 📱 **MQTT** | 物联网标准，支持订阅 | 物联网系统，远程监控 | ⭐️⭐️⭐️ |
| 🔵 **蓝牙BLE** | 近距离无线，低功耗 | 移动应用，近距离控制 | ⭐️⭐️⭐️⭐️ |
| 🌐 **TCP Socket** | 高性能，实时性好 | 实时控制，游戏应用 | ⭐️⭐️⭐️ |
| 📻 **UDP广播** | 局域网发现，群控 | 多设备控制，设备发现 | ⭐️⭐️⭐️ |

## 🚀 快速开始

### 1. Web API控制 (推荐)
```bash
# 使用统一控制器
python3 led_controller.py color 255 0 0    # 红色
python3 led_controller.py power on         # 开启
python3 led_controller.py demo-color       # 颜色演示

# 直接HTTP请求
curl -X POST -H "Content-Type: application/json" \
  -d '{"red": 255, "green": 0, "blue": 0}' \
  http://10.30.6.226/api/led/color
```

### 2. 串口控制
```bash
# 安装依赖
pip3 install pyserial

# 使用串口控制器
python3 serial_led_controller.py color 255 0 0
python3 serial_led_controller.py interactive

# 命令行使用
python3 serial_led_controller.py -p /dev/tty.usbmodem1201 color 0 255 0
```

### 3. MQTT控制
```bash
# 安装依赖
pip3 install paho-mqtt

# 启动MQTT代理 (如果没有)
brew install mosquitto
mosquitto &

# 使用MQTT控制器
python3 mqtt_led_controller.py color 0 0 255
python3 mqtt_led_controller.py listen
```

### 4. 蓝牙BLE控制
```bash
# 安装依赖 (需要ESP32固件支持BLE)
pip3 install bleak

# 使用蓝牙控制器
python3 bluetooth_led_controller.py
```

### 5. TCP Socket控制
```bash
# 使用TCP控制器 (需要ESP32固件支持TCP服务器)
python3 tcp_led_controller.py color 255 255 0
python3 tcp_led_controller.py interactive
```

### 6. UDP广播控制
```bash
# 使用UDP控制器 (需要ESP32固件支持UDP)
python3 udp_led_controller.py discover
python3 udp_led_controller.py color 255 0 255
```

## 📝 命令示例

### 颜色控制
```bash
# 基本颜色
python3 led_controller.py color 255 0 0      # 红色
python3 led_controller.py color 0 255 0      # 绿色  
python3 led_controller.py color 0 0 255      # 蓝色
python3 led_controller.py color 255 255 255  # 白色

# 预设颜色
python3 led_controller.py preset red
python3 led_controller.py preset blue
python3 led_controller.py preset purple
```

### 电源控制
```bash
python3 led_controller.py on     # 开启
python3 led_controller.py off    # 关闭
python3 led_controller.py power on
```

### 亮度控制
```bash
python3 led_controller.py brightness 50   # 50%亮度
python3 led_controller.py brightness 100  # 最亮
```

### 演示模式
```bash
python3 led_controller.py demo-color      # 颜色演示
python3 led_controller.py demo-brightness # 亮度演示
```

### 设备状态
```bash
python3 led_controller.py status          # 获取状态
```

## 🔧 高级用法

### 1. 交互模式
```bash
# 串口交互
python3 serial_led_controller.py interactive

# TCP交互  
python3 tcp_led_controller.py interactive
```

### 2. 指定设备
```bash
# 指定IP地址
python3 led_controller.py --ip 192.168.1.100 color 255 0 0

# 指定串口
python3 serial_led_controller.py -p /dev/ttyUSB0 color 0 255 0
```

### 3. 批量控制
```bash
# UDP广播控制多设备
python3 udp_led_controller.py color 255 0 0  # 控制所有设备

# 指定设备控制
python3 udp_led_controller.py -t 10.30.6.226 color 0 255 0
```

## 🛠️ 开发扩展

### 添加新的控制方式
1. 继承基础控制器类
2. 实现通信协议
3. 添加到统一控制器

### 自定义命令
1. 修改对应的控制器脚本
2. 添加新的命令解析
3. 实现控制逻辑

## 📱 移动端控制

### iOS/Android应用
- 使用HTTP API调用
- 集成蓝牙BLE控制
- MQTT客户端库

### 微信小程序
- 通过HTTP API
- WebSocket实时连接

## 🏠 智能家居集成

### Home Assistant
```yaml
# configuration.yaml
light:
  - platform: template
    lights:
      esp32_led:
        friendly_name: "ESP32 LED"
        turn_on:
          service: rest_command.esp32_led_on
        turn_off:
          service: rest_command.esp32_led_off

rest_command:
  esp32_led_on:
    url: "http://10.30.6.226/api/led/power"
    method: POST
    payload: '{"power": true}'
    content_type: 'application/json'
```

### HomeKit (通过HomeBridge)
```json
{
  "accessories": [{
    "accessory": "HttpMultiSwitch",
    "name": "ESP32 LED",
    "http_method": "POST",
    "base_url": "http://10.30.6.226/api/led"
  }]
}
```

## 🎯 使用建议

1. **开发调试**: 使用串口控制，实时查看日志
2. **Web应用**: 使用HTTP API，简单易用
3. **物联网**: 使用MQTT，支持发布订阅
4. **移动应用**: 使用蓝牙BLE，用户体验好
5. **高性能**: 使用TCP Socket，实时性最佳
6. **多设备**: 使用UDP广播，统一管理

## 🔍 故障排除

### 连接问题
1. 检查网络连接
2. 验证IP地址
3. 确认端口开放
4. 查看防火墙设置

### 串口问题
1. 检查USB连接
2. 确认串口权限
3. 验证波特率设置
4. 检查设备驱动

### 蓝牙问题
1. 确认蓝牙开启
2. 检查设备配对
3. 验证权限设置
4. 查看蓝牙日志