# ESP32-C3 WiFi LED Web Controller

基于ESP32-C3的现代化WiFi LED控制器，提供Web界面和RESTful API进行RGB LED控制。

## ✨ 特性

- 🌈 **RGB LED控制**: PWM驱动的精确颜色控制
- 🌐 **双模WiFi**: 支持STA和AP模式，自动回退
- 💻 **现代Web界面**: 响应式设计，支持移动设备
- 📡 **RESTful API**: 完整的JSON API接口
- 💫 **LED特效**: 彩虹、呼吸灯、闪烁等特效
- ⚡ **实时控制**: 即时响应的颜色和亮度调节
- 🔧 **硬件按钮**: 物理按钮控制和长按重置
- 💾 **配置保存**: 自动保存WiFi和LED状态到NVS

## 🛠️ 硬件连接

```
ESP32-C3    →    组件
━━━━━━━━━━━━━━━━━━━━
GPIO 3      →    红色LED正极
GPIO 4      →    绿色LED正极  
GPIO 5      →    蓝色LED正极
GND         →    LED负极 (220Ω电阻)
GPIO 8      →    状态LED (内置)
GPIO 9      →    按钮 (内置)
```

## 🚀 技术栈

- **硬件**: ESP32-C3 (RISC-V 160MHz)
- **框架**: ESP-IDF v5.0+
- **Web服务器**: ESP HTTP Server
- **JSON**: cJSON库
- **PWM**: LEDC控制器
- **前端**: HTML5 + CSS3 + JavaScript

## 🔧 编译和烧录

### 环境准备
```bash
# 安装ESP-IDF v5.0+
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32c3
. ./export.sh
```

### 编译项目
```bash
cd wifi_led_web_controller
idf.py set-target esp32c3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## 📱 使用方法

### 1. 首次启动
- ESP32-C3创建AP热点: `ESP32C3-LED-Controller`
- 密码: `12345678`
- IP地址: `192.168.4.1`

### 2. Web控制
- 连接WiFi热点
- 浏览器访问: `http://192.168.4.1`
- 使用滑块控制颜色和亮度
- 选择特效模式

### 3. 物理控制
- **短按按钮**: 切换LED开关
- **长按3秒**: 恢复出厂设置

## 📡 API接口

### 获取状态
```http
GET /api/status
```

### 控制颜色
```http
POST /api/led/color
Content-Type: application/json

{
  "red": 255,
  "green": 128,
  "blue": 0,
  "brightness": 80
}
```

### 控制电源
```http
POST /api/led/power
Content-Type: application/json

{
  "power": true
}
```

### 设置特效
```http
POST /api/led/effect
Content-Type: application/json

{
  "effect": "rainbow",
  "speed": 50
}
```

支持的特效: `static`, `rainbow`, `breathing`, `blink`

## 🏗️ 项目结构

```
wifi_led_web_controller/
├── CMakeLists.txt              # 项目构建配置
├── README.md                   # 项目文档
└── main/
    ├── CMakeLists.txt          # 主模块构建
    ├── main.c                  # 主程序入口
    ├── wifi_manager.c          # WiFi管理
    ├── led_controller.c        # LED控制
    ├── web_server.c            # HTTP服务器
    ├── api_handlers.c          # API处理
    └── include/
        ├── wifi_manager.h      # WiFi接口
        ├── led_controller.h    # LED接口
        ├── web_server.h        # Web接口
        └── api_handlers.h      # API接口
```

## 🐛 故障排除

### 编译问题
```bash
idf.py fullclean
idf.py build
```

### 烧录问题  
- 检查端口: `ls /dev/tty*`
- 手动进入下载模式: 按住BOOT，按RESET，释放BOOT

### WiFi问题
- 确认2.4GHz频段
- 长按按钮重置配置

### LED问题
- 检查引脚连接
- 确认220Ω电阻
- 检查LED极性

## 📈 性能

- **内存使用**: ~180KB Flash, ~45KB RAM
- **连接时间**: 2-5秒
- **响应时间**: <100ms
- **PWM频率**: 5kHz
- **并发连接**: 10个客户端
- **功耗**: ~80mA @ 3.3V

## 🔮 未来计划

- [ ] WebSocket实时通信
- [ ] 音乐节拍同步
- [ ] 定时器功能
- [ ] OTA无线更新
- [ ] HomeKit集成

## 📄 许可证

MIT License - 详见LICENSE文件

---

⭐ 如果项目对你有帮助，请给个星标！
