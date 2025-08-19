# ESP32-C3 摇杆舵机LED综合控制项目

这个项目演示了如何使用ESP32-C3控制舵机、摇杆和LED的综合控制系统。

## 功能特性

- 使用LEDC PWM模块生成精确的舵机控制信号
- **支持0-360度连续旋转控制** ✨
- **连续旋转速度控制（正转/反转/停止）** ✨
- **摇杆ADC采集控制** 🕹️
- **摇杆按钮LED开关控制** 💡
- **WS2812 RGB LED彩色显示** 🌈
- 平滑移动功能
- 多种演示模式：
  - 扩展范围定位（0-360度）
  - **连续旋转速度演示**
  - **摇杆实时控制演示** 🕹️
  - 平滑扫描、振荡、步进移动

## 硬件连接

### 完整硬件连接图
```
ESP32-C3-DevKitM-1
    ┌──────────────────────┐
    │       USB-C          │
    └──────────────────────┘
    │                      │
GPIO0 │(摇杆X轴)        GPIO8│ ──→ WS2812 LED 
GPIO1 │(摇杆Y轴)        GPIO2│ ──→ 舵机信号线
GPIO3 │(摇杆按钮)        GND │ ──→ 公共地线
  3V3 │                  5V │ ──→ 舵机电源
  GND │                     │
    └─┼─────────────────────┘
      │
   摇杆模块
```

### 舵机连接
- **电源（红线）**: 连接到5V（推荐）或3.3V电源
- **地线（棕/黑线）**: 连接到GND
- **信号线（橙/黄线）**: 连接到GPIO2

### 摇杆模块连接
- **VCC**: 连接到3.3V
- **GND**: 连接到GND
- **X轴输出**: 连接到GPIO0（ADC1_CH0）
- **Y轴输出**: 连接到GPIO1（ADC1_CH1）
- **按钮**: 连接到GPIO3（内部上拉）

### LED连接
- **WS2812 RGB LED**: 连接到GPIO8（ESP32-C3板载LED）
- **VCC**: 3.3V
- **GND**: GND
- **数据线**: GPIO8

## 项目结构

```
servo_control/
├── CMakeLists.txt                    # 项目构建配置
├── README.md                         # 项目说明文档
├── sdkconfig.defaults                # 默认配置
└── main/
    ├── CMakeLists.txt                # 主模块构建配置
    ├── idf_component.yml             # 组件依赖
    ├── servo_control_main.c          # 主应用程序
    ├── servo_driver.c                # 舵机驱动实现
    ├── joystick_led_controller.c     # 摇杆和LED控制器实现 🆕
    └── include/
        ├── servo_driver.h            # 舵机驱动头文件
        └── joystick_led_controller.h # 摇杆和LED控制器头文件 🆕
```

## 配置说明

### 舵机参数配置

在 `servo_control_main.c` 中可以修改以下参数：

```c
#define SERVO_GPIO_PIN          GPIO_NUM_2     // 舵机信号引脚
#define SERVO_LEDC_CHANNEL      LEDC_CHANNEL_0 // LEDC通道
#define SERVO_MIN_PULSE_WIDTH   500            // 最小脉冲宽度（μs）
#define SERVO_MAX_PULSE_WIDTH   2500           // 最大脉冲宽度（μs）
#define SERVO_MAX_ANGLE         360            // 最大角度（支持360度）
#define SERVO_STOP_PULSE_WIDTH  1500           // 停止脉冲宽度（连续旋转模式）
```

### 摇杆和LED配置

```c
/* Joystick and LED configuration */
#define JOYSTICK_X_AXIS_CHANNEL ADC_CHANNEL_0  // GPIO0 - X轴ADC通道
#define JOYSTICK_Y_AXIS_CHANNEL ADC_CHANNEL_1  // GPIO1 - Y轴ADC通道
#define JOYSTICK_BUTTON_PIN     GPIO_NUM_3     // GPIO3 - 摇杆按钮
#define LED_GPIO_PIN            GPIO_NUM_8     // GPIO8 - WS2812 LED
#define LED_COUNT               1              // LED数量
```

### 连续旋转模式

```c
#define CONTINUOUS_ROTATION_ENABLED  true     // 启用连续旋转模式
#define JOYSTICK_CONTROL_ENABLED     true     // 启用摇杆控制模式
```

**连续旋转控制说明：**
- **角度 0-90**: 反向旋转（速度递增）
- **角度 90**: 停止
- **角度 90-180**: 正向旋转（速度递增）
- **角度 180-360**: 扩展控制范围（支持更精细的速度控制）

**摇杆控制说明：**
- **X轴**: 控制舵机角度（左右移动摇杆）
- **Y轴**: 控制LED颜色（上下移动摇杆）
- **按钮**: 切换LED开关状态

### 演示参数

```c
#define DEMO_STEP_DELAY_MS      50             // 平滑移动延迟
#define DEMO_PAUSE_MS           2000           // 演示间隔
```

## 编译和烧录

### 使用提供的脚本（推荐）

```bash
# 在项目根目录执行
./build_servo.sh
```

### 手动编译

```bash
# 激活ESP-IDF环境
source $ESP_IDF_PATH/export.sh

# 进入项目目录
cd projects/esp32-c3/servo_control

# 编译项目
idf.py build

# 烧录到设备
idf.py -p /dev/cu.usbmodem1101 flash monitor
```

## 演示模式

程序启动后会循环执行以下演示：

1. **扩展范围定位**: 
   - 连续旋转模式：测试0°到360°全范围定位
   - 标准模式：传统0°到180°定位

2. **连续旋转演示** ✨：
   - 正向旋转（慢速→中速→快速）
   - 停止控制
   - 反向旋转（慢速→中速→快速）
   - 精确速度控制演示

3. **摇杆实时控制演示** 🕹️✨：
   - 30秒交互式控制体验
   - X轴控制舵机角度（左右移动）
   - Y轴控制LED颜色（上下移动）
   - 按钮切换LED开关
   - 实时状态日志显示

4. **平滑扫描**: 平滑地从0°扫描到最大角度，然后返回
5. **振荡运动**: 快速振荡演示
6. **步进移动**: 以固定步长移动舵机

## API 使用说明

### 初始化舵机

```c
servo_handle_t servo;
servo_config_t config = {
    .gpio_pin = GPIO_NUM_2,
    .ledc_channel = LEDC_CHANNEL_0,
    .min_pulse_width_us = 500,
    .max_pulse_width_us = 2500,
    .max_angle = 180
};

esp_err_t ret = servo_init(&servo, &config);
```

### 设置角度

```c
// 快速设置角度
servo_set_angle(&servo, 90);  // 设置到90度

// 平滑移动到目标角度
servo_move_smooth(&servo, 180, 50);  // 50ms步进延迟
```

### 清理资源

```c
servo_deinit(&servo);
```

### 初始化摇杆和LED

```c
joystick_led_handle_t controller;
joystick_led_config_t config = {
    .x_axis_channel = ADC_CHANNEL_0,
    .y_axis_channel = ADC_CHANNEL_1,
    .button_pin = GPIO_NUM_3,
    .led_pin = GPIO_NUM_8,
    .led_count = 1,
    .deadzone = 100
};

esp_err_t ret = joystick_led_init(&controller, &config);
```

### 读取摇杆数据

```c
joystick_data_t data;
joystick_read(&controller, &data);

printf("X: %d, Y: %d, Button: %s\n", 
       data.x_value, data.y_value, 
       data.button_pressed ? "Pressed" : "Released");
```

### 控制LED

```c
// 设置RGB颜色
led_set_color(&controller, 255, 0, 0);  // 红色

// 设置HSV颜色
led_set_hsv(&controller, 120, 255, 100); // 绿色

// 切换LED状态
led_toggle(&controller);

// 开关LED
led_set_state(&controller, true);  // 打开
led_set_state(&controller, false); // 关闭
```

### 清理资源

```c
joystick_led_deinit(&controller);
```

## 故障排除

### 舵机不动
- 检查电源连接是否正确
- 确认GPIO引脚配置
- 检查脉冲宽度范围是否适合您的舵机

### 舵机抖动
- 调整脉冲宽度范围
- 检查电源是否稳定
- 减少移动步长

### 角度不准确
- 校准最小/最大脉冲宽度
- 确认舵机的实际角度范围

### 摇杆读数异常
- 检查ADC连接是否正确（GPIO0、GPIO1）
- 确认摇杆供电电压（3.3V）
- 运行校准程序获取中心值
- 调整死区参数

### LED不亮或颜色异常
- 检查LED连接（GPIO8）
- 确认LED类型为WS2812
- 检查LED供电是否充足
- 验证数据线连接

### 按钮无响应
- 检查按钮连接（GPIO3）
- 确认内部上拉电阻配置
- 检查按钮接地连接

## 技术原理

### 舵机控制原理
舵机控制使用PWM信号，参数如下：
- **频率**: 50Hz（20ms周期）
- **脉冲宽度**: 
  - 0.5-1.0ms → 0°
  - 1.5ms → 90°
  - 2.0-2.5ms → 180°

### 摇杆ADC采集原理
- **ADC分辨率**: 12位（0-4095）
- **参考电压**: 3.3V
- **采样通道**: ADC1_CH0（GPIO0）、ADC1_CH1（GPIO1）
- **校准机制**: 自动检测中心值，设置死区避免抖动
- **数据映射**: ADC值映射到-100至+100的相对坐标

### LED WS2812控制原理
- **协议**: WS2812单总线协议
- **数据格式**: 24位RGB（每色8位）
- **传输方式**: RMT（Remote Control Transceiver）模块
- **色彩空间**: 支持RGB和HSV色彩模式
- **更新频率**: 可达数百Hz

### 系统集成架构
```
摇杆输入 → ADC采集 → 数据处理 → 舵机控制
   ↓
按钮输入 → GPIO检测 → 事件处理 → LED控制
```

## 许可证

此项目基于MIT许可证开源。
