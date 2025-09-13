### 本项目采用官方SDK (esp-idf)

### 其他方式看esp32
1. arduino
2. MicroPython
3. PlatformIO
    1、编辑器插件PlatformIO IDE
    2、命令方式

4. NodeMCU（Lua RTOS 移植 / 第三方）
    ESP8266 上常用，ESP32 上也有人移植，但不活跃。
    现已被 MicroPython 取代，推荐后者。
    
5. 其它方式（小众 / 特殊需求）
    ESP32 + Rust（用 esp-idf-sys crate）
    ESP32 + JavaScript（如 Espruino，部分支持）
    ESP32 + CircuitPython（Adafruit 支持）