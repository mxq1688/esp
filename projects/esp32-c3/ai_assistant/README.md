projects/esp32-c3/ai_assistant/
├── CMakeLists.txt              # 项目构建配置
├── sdkconfig.defaults          # 默认系统配置
├── README.md                   # 详细项目文档
└── main/                       # 主程序目录
    ├── CMakeLists.txt          # 组件构建配置
    ├── idf_component.yml       # 依赖组件配置
    ├── main.c                  # 主程序入口
    ├── ai_engine.h/c           # AI引擎模块
    ├── voice_processor.h/c     # 语音处理模块
    ├── web_interface.h/c       # Web界面模块
    └── wifi_manager.h/c        # WiFi管理模块

AI对话引擎 - 支持自然语言处理和本地命令
语音识别 - I2S麦克风输入，实时语音转文字
语音合成 - 文字转语音，I2S扬声器输出
Web界面 - 现代化响应式UI，支持实时聊天
WiFi管理 - 自动连接和热点模式
硬件控制 - GPIO控制LED、继电器等设备

I2S音频: GPIO26(BCK), GPIO25(WS), GPIO22(DO), GPIO21(DI)
状态LED: GPIO2
功能按钮: GPIO3