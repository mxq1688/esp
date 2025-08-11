# 🌈 ESP32-S3 RGB LED PWA 控制器

这是一个功能强大的ESP32-S3 RGB LED控制系统，包含ESP32-S3固件和渐进式Web应用(PWA)控制界面。

## ✨ 主要功能

### 🎯 ESP32-S3 固件功能
- **RGB PWM控制** - 支持3通道RGB LED精确调光
- **WiFi连接** - 连接到家庭或办公网络
- **HTTP API** - 提供RESTful API接口
- **实时颜色控制** - 支持颜色和亮度的实时调节
- **CORS支持** - 允许跨域访问

### 📱 PWA Web应用功能
- **颜色选择器** - 直观的颜色选择界面
- **RGB滑块控制** - 精确的红绿蓝三色调节
- **亮度控制** - 0-100%亮度调节
- **预设颜色** - 8种常用颜色快速选择
- **特效模式** - 彩虹循环、呼吸灯、闪烁效果
- **快速控制** - 一键开关、最大亮度、夜灯模式
- **PWA支持** - 可安装到手机主屏幕
- **离线功能** - 支持离线使用
- **自动更新** - 实时同步ESP32状态

## 🔧 硬件要求

### ESP32-S3开发板
- ESP32-S3 DevKitC-1 或兼容开发板
- USB-C数据线

### RGB LED连接
```
ESP32-S3 引脚 -> RGB LED
GPIO38       -> 红色LED正极
GPIO39       -> 绿色LED正极  
GPIO40       -> 蓝色LED正极
GND          -> 所有LED负极
```

### 可选：限流电阻
- 每个LED通道建议串联220Ω-330Ω电阻

## 🚀 快速开始

### 1. 准备ESP32-S3固件

1. **修改WiFi配置** - 编辑 `main/main.c` 文件：
```c
#define WIFI_SSID      "你的WiFi名称"
#define WIFI_PASSWORD  "你的WiFi密码"
```

2. **检查引脚配置** - 如果需要修改LED引脚：
```c
#define LED_RED_GPIO    GPIO_NUM_38
#define LED_GREEN_GPIO  GPIO_NUM_39
#define LED_BLUE_GPIO   GPIO_NUM_40
```

### 2. 编译和烧录固件

```bash
# 进入项目目录
cd /Users/meng/stu/esp/projects/esp32-s3

# 设置ESP-IDF环境
. /Users/meng/stu/esp/esp-idf/export.sh

# 编译项目
idf.py build

# 烧录到ESP32-S3（需要先进入下载模式）
idf.py -p /dev/cu.usbmodem* flash monitor
```

### 3. 获取ESP32-S3的IP地址

烧录成功后，在串口监视器中查找类似信息：
```
I (2500) RGB_LED_PWA: got ip:192.168.1.100
```

### 4. 使用PWA应用

1. **打开Web应用** - 在浏览器中访问 `web/index.html`
2. **输入ESP32-S3 IP地址** - 在设置面板输入获取到的IP
3. **点击连接** - 建立与ESP32-S3的连接
4. **开始控制** - 使用各种控制功能

## 📱 PWA使用指南

### 安装PWA到手机
1. 在Chrome/Safari中打开web应用
2. 点击浏览器菜单中的"添加到主屏幕"
3. 确认安装，获得原生应用体验

### 主要控制面板

#### 🎨 颜色选择
- **颜色选择器** - 点击圆形选择器选择任意颜色
- **RGB滑块** - 精确调节红绿蓝数值（0-255）
- **亮度控制** - 调节整体亮度（0-100%）

#### 🌟 预设颜色
快速选择常用颜色：红、绿、蓝、白、黄、紫、青、橙

#### ✨ 特效模式
- **🌈 彩虹循环** - 颜色循环变化
- **💨 呼吸灯** - 亮度渐变效果
- **⚡ 闪烁** - 快速开关闪烁
- **⏹️ 停止特效** - 停止当前特效

#### ⚡ 快速控制
- **🔴 关闭** - 立即关闭LED
- **🟢 开启** - 恢复之前亮度
- **☀️ 最大亮度** - 设置为100%亮度
- **🌙 夜灯模式** - 低亮度暖白光

## 🛠️ API接口文档

### 获取当前状态
```http
GET /api/status
```

**响应示例：**
```json
{
  "status": "ok",
  "color": {
    "red": 255,
    "green": 100,
    "blue": 50,
    "brightness": 75
  }
}
```

### 设置颜色
```http
POST /api/color
Content-Type: application/json

{
  "red": 255,
  "green": 0,
  "blue": 0,
  "brightness": 50
}
```

**响应：**
```json
{
  "status": "ok"
}
```

## 🔄 项目结构

```
esp32-s3/
├── main/
│   ├── main.c              # ESP32-S3主程序
│   └── CMakeLists.txt      # 组件构建配置
├── web/                    # PWA Web应用
│   ├── index.html          # 主页面
│   ├── style.css           # 样式文件
│   ├── app.js              # JavaScript逻辑
│   ├── manifest.json       # PWA清单
│   ├── sw.js               # Service Worker
│   └── icon-192.svg        # 应用图标
├── CMakeLists.txt          # 项目构建配置
├── sdkconfig.defaults      # ESP32-S3默认配置
└── README.md               # 项目说明
```

## 🎯 高级功能

### 自定义特效
可以在 `app.js` 中添加更多特效模式：

```javascript
startCustomEffect() {
    this.stopEffect();
    this.effectRunning = 'custom';
    // 添加自定义特效逻辑
}
```

### 扩展API
在 `main.c` 中添加新的HTTP处理器：

```c
static esp_err_t custom_handler(httpd_req_t *req) {
    // 自定义API逻辑
    return ESP_OK;
}
```

### 硬件扩展
- 添加更多LED通道
- 集成温度传感器
- 添加声音控制
- 集成运动检测

## 🐛 故障排除

### ESP32-S3连接问题
1. **检查WiFi配置** - 确认SSID和密码正确
2. **检查网络** - 确保设备在同一网络
3. **重启ESP32-S3** - 按RESET按钮重启
4. **查看串口输出** - 检查错误信息

### PWA应用问题
1. **清除浏览器缓存** - 强制刷新页面
2. **检查IP地址** - 确认ESP32-S3的IP正确
3. **检查网络连接** - 确保网络正常
4. **HTTPS问题** - 在开发环境中使用HTTP

### LED不亮问题
1. **检查接线** - 确认引脚连接正确
2. **检查电阻** - 确保限流电阻值合适
3. **检查电源** - 确保ESP32-S3供电充足
4. **检查引脚配置** - 确认GPIO配置正确

## 📈 性能优化

### ESP32-S3优化
- 使用更高的PWM频率获得更平滑的调光
- 启用硬件看门狗增强稳定性
- 优化WiFi功耗设置

### PWA优化
- 启用Service Worker缓存
- 压缩CSS和JavaScript文件
- 优化图标文件大小

## 🔒 安全考虑

- **网络安全** - 仅在受信任网络中使用
- **访问控制** - 可添加身份验证机制
- **HTTPS** - 生产环境建议使用HTTPS
- **固件更新** - 定期更新ESP-IDF版本

## 📄 许可证

MIT License - 详见项目根目录的LICENSE文件

## 🤝 贡献

欢迎提交Issue和Pull Request来改进这个项目！

## 📞 支持

如有问题，请在GitHub上提交Issue或联系项目维护者。 