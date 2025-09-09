# ESP32-S3 AI助手硬件配置指南

## 硬件清单

### 必需组件
- **ESP32-S3-DevKitC-1** 开发板
- **INMP441** 数字麦克风模块
- **MAX98357A** I2S音频放大器模块
- **扬声器** (4Ω-8Ω, 3W推荐)
- **面包板或PCB**
- **杜邦线**

### 可选组件
- LED指示灯 (3mm或5mm)
- 按键开关
- 电阻 (220Ω用于LED限流)
- 电容 (100μF用于电源滤波)

## 详细接线图

### INMP441麦克风模块接线

```
INMP441引脚说明:
┌─────────────┐
│ VDD    GND  │  ← 电源引脚
│ L/R    WS   │  ← 配置和控制引脚  
│ SCK    SD   │  ← I2S时钟和数据引脚
└─────────────┘

接线方案:
INMP441    ESP32-S3    说明
VDD    →   3.3V       电源正极
GND    →   GND        电源负极
L/R    →   GND        左声道选择(接GND=左声道, 接VDD=右声道)
WS     →   GPIO5      字选择信号(Word Select)
SCK    →   GPIO4      串行时钟(Serial Clock)
SD     →   GPIO6      串行数据输出(Serial Data)
```

### MAX98357A音频放大器接线

```
MAX98357A引脚说明:
┌─────────────────┐
│ VIN   GND   SD  │  ← 电源和使能引脚
│ DIN  BCLK  LRC  │  ← I2S数据引脚
│ GAIN  +    -    │  ← 增益设置和扬声器输出
└─────────────────┘

接线方案:
MAX98357A  ESP32-S3    说明
VIN    →   5V/3.3V    电源输入(5V性能更好)
GND    →   GND        电源负极
SD     →   3.3V       关断控制(高电平使能)
DIN    →   GPIO16     I2S数据输入
BCLK   →   GPIO7      位时钟(Bit Clock)
LRC    →   GPIO15     左右声道时钟
GAIN   →   悬空       增益设置(悬空=9dB)
+      →   扬声器+    扬声器正极
-      →   扬声器-    扬声器负极
```

### 完整系统接线图

```
ESP32-S3开发板
┌─────────────────────────────────┐
│                                 │
│  3.3V ──┬─── INMP441 VDD        │
│         └─── MAX98357A SD       │
│                                 │
│  5V ────────── MAX98357A VIN    │
│                                 │
│  GND ───┬─── INMP441 GND        │
│         ├─── INMP441 L/R        │
│         └─── MAX98357A GND      │
│                                 │
│  GPIO4  ──── INMP441 SCK        │
│  GPIO5  ──── INMP441 WS         │
│  GPIO6  ──── INMP441 SD         │
│  GPIO7  ──── MAX98357A BCLK     │
│  GPIO15 ──── MAX98357A LRC      │
│  GPIO16 ──── MAX98357A DIN      │
│                                 │
│  GPIO2  ──── LED指示灯          │
│  GPIO0  ──── 用户按键           │
│                                 │
└─────────────────────────────────┘
```

## I2S配置说明

### 麦克风配置 (I2S_NUM_0)
```c
i2s_config_t i2s_config_mic = {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX,
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};

i2s_pin_config_t pin_config_mic = {
    .bck_io_num = 26,    // INMP441 SCK
    .ws_io_num = 25,     // INMP441 WS
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = 35    // INMP441 SD
};
```

### 扬声器配置 (I2S_NUM_1)
```c
i2s_config_t i2s_config_spk = {
    .mode = I2S_MODE_MASTER | I2S_MODE_TX,
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
};

i2s_pin_config_t pin_config_spk = {
    .bck_io_num = 27,    // MAX98357A BCLK
    .ws_io_num = 25,     // MAX98357A LRC
    .data_out_num = 22,  // MAX98357A DIN
    .data_in_num = I2S_PIN_NO_CHANGE
};
```

## 硬件注意事项

### 电源要求
1. **ESP32-S3**: 3.3V，建议使用开发板的USB供电
2. **INMP441**: 3.3V，功耗约1.4mA
3. **MAX98357A**: 
   - 逻辑电平: 3.3V
   - 功率供电: 5V推荐(3.3V也可工作但功率较小)
   - 最大功耗: 约3W

### 音频质量优化
1. **采样率设置**: 推荐16kHz用于语音识别
2. **位深度**: INMP441输出24位，ESP32-S3处理16位
3. **缓冲区大小**: 根据实时性要求调整DMA缓冲区
4. **电源滤波**: 在VIN引脚添加100μF电容减少噪声

### 布线建议
1. **I2S信号线**: 尽量短，避免与高频信号并行
2. **电源线**: 使用较粗的线，减少压降
3. **地线**: 保持良好的接地，减少噪声干扰
4. **屏蔽**: 如有条件，对音频线路进行屏蔽

### 扬声器选择
1. **阻抗**: 4Ω-8Ω
2. **功率**: 2-5W
3. **频响**: 300Hz-8kHz (语音频段)
4. **尺寸**: 根据项目需求选择

## 测试验证

### 硬件连接检查
```bash
# 编译并烧录测试程序
./build_s3.sh
./flash_s3.sh

# 检查串口输出
idf.py monitor
```

### 功能测试步骤
1. **上电检查**: LED指示灯应该亮起
2. **WiFi连接**: 查看串口日志确认WiFi连接成功
3. **麦克风测试**: 对着麦克风说话，观察音频数据
4. **扬声器测试**: 通过Web界面发送TTS请求
5. **完整流程**: 语音识别→AI处理→语音合成→播放

### 常见问题排查
1. **无声音输出**: 检查MAX98357A的SD引脚是否接3.3V
2. **音质差**: 检查电源质量和接地
3. **无法识别语音**: 检查INMP441接线和I2S配置
4. **系统重启**: 检查电源供应能力

## 扩展功能

### 可选硬件扩展
1. **LCD显示屏**: SPI接口，显示状态信息
2. **SD卡模块**: 存储音频文件和配置
3. **蓝牙模块**: 无线音频传输
4. **传感器**: 温湿度、光照等环境传感器

### PCB设计建议
如需制作PCB，建议考虑：
1. 4层板设计，独立的电源和地平面
2. 音频部分与数字部分分离
3. 添加测试点便于调试
4. 预留扩展接口

---

**注意**: 请确保所有连接正确后再上电，避免损坏硬件。如有疑问，请参考各模块的官方文档。
