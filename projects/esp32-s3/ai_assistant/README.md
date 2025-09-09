# ESP32-S3 AI小智助手

一个基于ESP32-S3的智能语音助手项目，支持语音识别、自然语言处理、AI对话和语音合成功能。

## 项目特性

### 🎯 核心功能
- **语音识别**: 支持中英文语音识别，实时转换语音为文本
- **AI对话**: 集成OpenAI GPT、百度AI、腾讯AI等多种AI引擎
- **语音合成**: 将AI回复转换为语音输出
- **自然语言处理**: 智能理解用户意图和实体识别
- **Web界面**: 提供现代化的Web控制界面
- **WiFi连接**: 支持WiFi网络连接和配置

### 🔧 技术特性
- **多任务处理**: 基于FreeRTOS的多任务架构
- **音频处理**: 支持噪声抑制、回声消除、自动增益控制
- **RESTful API**: 完整的HTTP API接口
- **实时通信**: WebSocket支持实时数据传输
- **OTA更新**: 支持无线固件更新
- **配置管理**: 灵活的配置系统

## 硬件要求

### 必需硬件
- ESP32-S3开发板 (推荐ESP32-S3-DevKitC-1)
- 麦克风模块 (I2S接口)
- 扬声器或耳机 (I2S/DAC输出)
- micro-USB数据线

### 可选硬件
- LED指示灯
- 按键开关
- LCD显示屏
- SD卡模块

### 引脚连接

#### INMP441麦克风模块
```
INMP441 -> ESP32-S3
VDD     -> 3.3V
GND     -> GND
L/R     -> GND (左声道配置)
WS      -> GPIO5 (Word Select)
SCK     -> GPIO4 (Serial Clock)
SD      -> GPIO6 (Serial Data)
```

#### MAX98357A音频放大器
```
MAX98357A -> ESP32-S3
VIN       -> 5V (推荐) 或 3.3V
GND     -> GND
DIN       -> GPIO16 (I2S数据输入)
BCLK      -> GPIO7 (位时钟)
LRC       -> GPIO15 (左右声道选择)
GAIN      -> 悬空 (默认9dB增益)
SD        -> 3.3V (启用放大器)
```

#### 控制引脚
```
LED指示灯: GPIO2
用户按键: GPIO0
状态LED:  GPIO48
```

#### 扬声器连接
```
MAX98357A -> 扬声器
+         -> 扬声器正极
-         -> 扬声器负极
(推荐使用4Ω-8Ω，3W扬声器)
```

## 软件环境

### 开发环境要求
- ESP-IDF v5.0+
- Python 3.8+
- Git

### 依赖库
- FreeRTOS
- ESP-IDF组件
- cJSON
- HTTP客户端
- WiFi管理

## 快速开始

### 1. 克隆项目
```bash
cd /Users/meng/stu/esp/projects/esp32-s3/
# 项目已创建在 ai_assistant 目录
```

### 2. 配置环境
```bash
# 设置ESP-IDF环境
. $HOME/esp/esp-idf/export.sh

# 进入项目目录
cd ai_assistant
```

### 3. 配置项目
```bash
# 配置目标芯片
idf.py set-target esp32s3

# 打开配置菜单
idf.py menuconfig
```

### 4. 编译和烧录
```bash
# 编译项目
idf.py build

# 烧录固件
idf.py -p /dev/ttyUSB0 flash

# 监控串口输出
idf.py -p /dev/ttyUSB0 monitor
```

## 配置说明

### WiFi配置
在`main.c`中修改WiFi配置：
```c
ai_assistant_config_t g_ai_config = {
    .wifi_ssid = "你的WiFi名称",
    .wifi_password = "你的WiFi密码",
    // 其他配置...
};
```

### AI引擎配置
支持多种AI引擎，在`ai_engine.c`中配置：
```c
static ai_engine_config_t s_ai_config = {
    .type = AI_ENGINE_OPENAI,  // 或 AI_ENGINE_BAIDU, AI_ENGINE_TENCENT
    .api_key = "你的API密钥",
    .server_url = "https://api.openai.com/v1/chat/completions",
    // 其他配置...
};
```

## API接口

### RESTful API

#### 系统状态
```http
GET /api/status
```

#### 配置管理
```http
GET /api/config
POST /api/config
```

#### AI对话
```http
POST /api/ai/chat
Content-Type: application/json

{
    "message": "你好，今天天气怎么样？"
}
```

#### 语音命令
```http
POST /api/voice/command
Content-Type: application/json

{
    "command": "开灯"
}
```

#### 系统信息
```http
GET /api/system/info
```

### Web界面
访问 `http://ESP32_IP_ADDRESS` 打开Web控制界面

## 项目结构

```
ai_assistant/
├── CMakeLists.txt              # 项目构建配置
├── partitions.csv              # 分区表
├── README.md                   # 项目说明
├── main/
│   ├── CMakeLists.txt          # 主组件构建配置
│   ├── main.c                  # 主程序入口
│   ├── wifi_manager.c          # WiFi管理
│   ├── ai_engine.c             # AI引擎
│   ├── voice_recognition.c     # 语音识别
│   ├── voice_synthesis.c       # 语音合成
│   ├── audio_processor.c       # 音频处理
│   ├── nlp_processor.c         # 自然语言处理
│   ├── web_server.c            # Web服务器
│   ├── api_handlers.c          # API处理器
│   ├── web_files.c             # Web文件处理
│   └── include/                # 头文件目录
│       ├── ai_assistant.h      # 主头文件
│       ├── wifi_manager.h      # WiFi管理头文件
│       ├── ai_engine.h         # AI引擎头文件
│       ├── voice_recognition.h # 语音识别头文件
│       ├── voice_synthesis.h   # 语音合成头文件
│       ├── audio_processor.h   # 音频处理头文件
│       ├── nlp_processor.h     # NLP处理头文件
│       ├── web_server.h        # Web服务器头文件
│       ├── api_handlers.h      # API处理器头文件
│       └── web_files.h         # Web文件头文件
└── build/                      # 构建输出目录
```

## 功能模块

### 1. 语音识别模块 (voice_recognition.c)
- 实时音频采集
- 语音端点检测
- 语音转文本
- 支持多种语音识别服务

### 2. AI引擎模块 (ai_engine.c)
- 多AI平台支持
- 智能对话处理
- 上下文管理
- 流式响应支持

### 3. 语音合成模块 (voice_synthesis.c)
- 文本转语音
- 多种音色选择
- 音频播放控制
- 音量和语速调节

### 4. 音频处理模块 (audio_processor.c)
- 音频录制和播放
- 噪声抑制
- 回声消除
- 自动增益控制

### 5. NLP处理模块 (nlp_processor.c)
- 意图识别
- 实体提取
- 情感分析
- 关键词提取

### 6. Web服务器模块 (web_server.c)
- HTTP服务器
- 静态文件服务
- WebSocket支持
- CORS跨域支持

## 使用说明

### 语音交互
1. 说出唤醒词："你好小智"
2. 等待提示音后说出指令
3. 系统会语音回复结果

### 支持的语音命令
- **天气查询**: "今天天气怎么样？"
- **时间查询**: "现在几点了？"
- **新闻查询**: "播报今天的新闻"
- **音乐控制**: "播放音乐"、"暂停音乐"
- **灯光控制**: "开灯"、"关灯"
- **温度控制**: "调高温度"、"开空调"
- **系统设置**: "调节音量"、"切换语言"
- **帮助信息**: "帮助"、"你能做什么？"

### Web界面操作
1. 连接到ESP32-S3的WiFi网络
2. 打开浏览器访问设备IP地址
3. 使用Web界面进行文本对话
4. 查看系统状态和配置设置

## 故障排除

### 常见问题

#### 1. 编译错误
```bash
# 清理构建缓存
idf.py fullclean

# 重新配置
idf.py set-target esp32s3
idf.py menuconfig

# 重新编译
idf.py build
```

#### 2. WiFi连接失败
- 检查WiFi SSID和密码配置
- 确认WiFi信号强度
- 检查路由器设置

#### 3. 语音识别不工作
- 检查麦克风连接
- 确认I2S引脚配置
- 检查音频采样率设置

#### 4. AI对话无响应
- 检查网络连接
- 验证API密钥配置
- 查看串口日志错误信息

### 调试方法
```bash
# 查看详细日志
idf.py monitor

# 设置日志级别
idf.py menuconfig
# Component config -> Log output -> Default log verbosity -> Debug
```

## 开发指南

### 添加新的语音命令
1. 在`nlp_processor.c`中添加关键词映射
2. 在`voice_command_type_t`枚举中添加新类型
3. 在AI引擎中添加对应的处理逻辑

### 集成新的AI服务
1. 在`ai_engine.h`中添加新的引擎类型
2. 在`ai_engine.c`中实现对应的处理函数
3. 添加配置选项和API调用逻辑

### 自定义Web界面
1. 修改`web_server.c`中的HTML内容
2. 添加新的API端点
3. 实现前后端数据交互

## 性能优化

### 内存优化
- 使用PSRAM存储大型数据
- 优化音频缓冲区大小
- 及时释放不用的内存

### 功耗优化
- 使用深度睡眠模式
- 优化CPU频率设置
- 关闭不必要的外设

### 网络优化
- 使用连接池
- 实现请求缓存
- 优化数据传输格式

## 版本历史

### v1.0.0 (当前版本)
- 基础语音识别功能
- AI对话引擎
- Web控制界面
- 多任务架构
- RESTful API

### 计划功能
- 离线语音识别
- 多用户支持
- 智能家居集成
- 移动APP
- 云端同步

## 贡献指南

欢迎提交Issue和Pull Request来改进项目！

### 开发流程
1. Fork项目
2. 创建功能分支
3. 提交代码
4. 创建Pull Request

### 代码规范
- 使用统一的代码风格
- 添加必要的注释
- 编写单元测试
- 更新文档

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 联系方式

- 项目地址: [GitHub仓库地址]
- 问题反馈: [Issues页面]
- 技术交流: [讨论区地址]

## 致谢

感谢以下开源项目和贡献者：
- ESP-IDF框架
- FreeRTOS实时操作系统
- cJSON库
- 各AI服务提供商

---

**注意**: 本项目仅供学习和研究使用，请遵守相关法律法规和服务条款。
