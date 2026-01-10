# ESP32-S3 音频播放器

通过 I2S 接口驱动喇叭播放音频的 ESP32-S3 项目。

## 功能特性

- 使用 I2S 标准模式输出音频
- 支持 16 位 PCM 音频，16kHz 采样率
- 内置正弦波生成器（播放指定频率的测试音）
- 内置《小星星》旋律演示
- 软件音量控制（0-100%）
- 支持 MAX98357A、PCM5102A 等 I2S DAC/功放模块

## 硬件需求

### 必需硬件

- ESP32-S3 开发板
- I2S DAC 或功放模块（推荐以下任一）:
  - **MAX98357A** - I2S 功放模块（直接驱动喇叭）
  - **PCM5102A** - I2S DAC 模块（需外接功放）
- 喇叭（4Ω 或 8Ω，3W 以内）

### 接线说明

#### 使用 MAX98357A 模块（推荐）

| ESP32-S3 | MAX98357A |
|----------|-----------|
| GPIO 15  | BCLK      |
| GPIO 16  | LRC       |
| GPIO 17  | DIN       |
| 3.3V     | VIN       |
| GND      | GND       |

喇叭连接到 MAX98357A 的 `+` 和 `-` 端子。

#### 使用 PCM5102A 模块

| ESP32-S3 | PCM5102A |
|----------|----------|
| GPIO 15  | BCK      |
| GPIO 16  | LCK      |
| GPIO 17  | DIN      |
| 3.3V     | VCC      |
| GND      | GND      |

PCM5102A 为 DAC 模块，输出线路电平信号，需外接功放才能驱动喇叭。

### 修改引脚配置

如需使用其他 GPIO 引脚，编辑 `main/include/audio_player.h`:

```c
#define I2S_BCLK_PIN    15  // 位时钟
#define I2S_WS_PIN      16  // 字选择/LRCK
#define I2S_DOUT_PIN    17  // 数据输出
```

## 编译与烧写

### 前置条件

确保已安装 ESP-IDF v5.0 或更高版本。

### 使用构建脚本

```bash
# 进入项目目录
cd /Users/meng/stu/esp/idf/projects/esp32-s3/audio_speaker

# 添加执行权限
chmod +x build_s3.sh

# 仅编译
./build_s3.sh

# 编译并烧写
./build_s3.sh flash

# 清理后重新编译
./build_s3.sh clean

# 清理、编译并烧写
./build_s3.sh clean flash
```

### 使用 idf.py 命令

```bash
# 激活 ESP-IDF 环境
source /Users/meng/stu/esp/idf/esp-idf/export.sh

# 设置目标芯片
idf.py set-target esp32s3

# 编译
idf.py build

# 烧写
idf.py flash

# 监视器
idf.py monitor

# 烧写并打开监视器
idf.py flash monitor
```

## 运行效果

程序启动后会：

1. 打印系统信息和接线指南
2. 初始化 I2S 音频输出
3. 等待 3 秒后开始演示
4. 依次播放：
   - 440Hz 测试音（A4 音符，1 秒）
   - C 大调音阶（C4-C5）
   - 《小星星》旋律
5. 演示完成后等待 30 秒，然后循环播放

## API 使用说明

### 初始化

```c
#include "include/audio_player.h"

// 初始化音频播放器
esp_err_t ret = audio_player_init();
```

### 播放正弦波

```c
// 播放 440Hz 音调，持续 1 秒
audio_player_play_tone(440, 1000);
```

### 播放 PCM 数据

```c
// 播放自定义 PCM 数据
int16_t pcm_data[1024];
// ... 填充 PCM 数据 ...
audio_player_play(pcm_data, sizeof(pcm_data), 1000);
```

### 音量控制

```c
// 设置音量 (0-100)
audio_player_set_volume(70);

// 获取当前音量
uint8_t vol = audio_player_get_volume();
```

### 播放示例旋律

```c
// 播放内置的《小星星》旋律
audio_player_play_sample();
```

## 故障排除

### 无声音输出

1. 检查接线是否正确
2. 确认使用正确的 GPIO 引脚
3. 检查喇叭是否正常工作
4. 尝试增大音量 `audio_player_set_volume(100)`

### 声音失真

1. 降低音量避免削波
2. 检查电源是否稳定
3. 确保 GND 连接良好

### I2S 初始化失败

1. 检查是否有其他程序占用 I2S
2. 确认 GPIO 引脚未被其他功能占用

## 扩展开发

### 播放 WAV 文件

可以将 WAV 文件转换为 C 数组，然后使用 `audio_player_play()` 播放：

```bash
# 使用 xxd 工具转换
xxd -i audio.raw > audio_data.h
```

### 支持更多采样率

修改 `audio_player.h` 中的定义：

```c
#define AUDIO_SAMPLE_RATE   44100  // 44.1kHz
```

## 项目结构

```
audio_speaker/
├── CMakeLists.txt          # 项目 CMake 配置
├── partitions.csv          # 分区表
├── sdkconfig.defaults      # SDK 默认配置
├── build_s3.sh             # 构建脚本
├── README.md               # 项目说明
└── main/
    ├── CMakeLists.txt      # 组件 CMake 配置
    ├── main.c              # 主程序
    ├── audio_player.c      # 音频播放器实现
    └── include/
        └── audio_player.h  # 音频播放器头文件
```

## 许可证

MIT License

