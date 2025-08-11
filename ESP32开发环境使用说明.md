# ESP32 开发环境完整指南

> 官方文档：https://github.com/espressif/esp-idf
> 
> 本指南基于 ESP-IDF v5.4，适用于 ESP32-C3-DevKitM-1 和 ESP32-S3 开发板

## 🔧 环境准备
### 安装esp-idf
```bash
./install.sh esp32s3
```
### 1. 激活 ESP-IDF 环境
```bash
source /Users/meng/stu/esp/esp-idf/export.sh
```
### 2. 验证环境
```bash
# 检查 idf.py 是否可用
idf.py --version

# 检查 Python 环境
python --version
```
## ⚙️ 编译烧录流程
### 1. ESP32-C3 开发流程
#### 基本步骤
```bash
# 1. 进入项目目录
cd ~/stu/esp/projects/esp32-c3/your_project

# 2. 激活环境（如果尚未激活）
source /Users/meng/stu/esp/esp-idf/export.sh

# 3. 设置目标芯片
idf.py set-target esp32c3

# 4. 编译项目
idf.py build

# 5. 烧录并监控
idf.py -p /dev/tty.usbmodem1201 flash monitor
```

#### 添加组件依赖
```bash
# 如果需要添加外部组件（如 LED 控制）
idf.py add-dependency "espressif/led_strip"
```

#### 串口查找
```bash
# 查看可用串口
ls /dev/tty.*

# 检查设备连接
ls /dev/cu.*

# ESP32-C3 常见串口名称
# /dev/tty.usbmodem1201
# /dev/tty.usbserial-*
```

### 2. ESP32-S3 开发流程

#### 方法一：常规烧录
```bash
cd ~/stu/esp/projects/esp32-s3/hello_world
idf.py set-target esp32s3
idf.py build

# 进入下载模式：按住 BOOT，按一下 RST，松开 BOOT
idf.py -p /dev/tty.usbmodem58960267321 flash monitor
```

#### 方法二：DFU 烧录（推荐）
```bash
cd ~/stu/esp/projects/esp32-s3/hello_world
idf.py set-target esp32s3
idf.py build

# 不需要按按钮，直接烧录
idf.py dfu-flash

# 烧录完成后查看输出
idf.py monitor
```

---

## 🚨 常见问题解决
### 1. 烧录连接失败
```bash
# 错误：Failed to connect to ESP32-C3: No serial data received

# 解决方案：手动进入下载模式
# 1. 同时按住 BOOT + RESET 按钮
# 2. 先松开 RESET 按钮，继续按住 BOOT
# 3. 继续按住 BOOT 按钮 2-3 秒
# 4. 松开 BOOT 按钮
# 5. 立即执行烧录命令
```

## 💡 实用技巧
### 1. 常用命令速查
```bash
# 项目管理
idf.py clean                    # 清理构建目录
idf.py fullclean               # 完全清理
idf.py build                   # 编译项目
idf.py flash                   # 仅烧录
idf.py monitor                 # 仅监控
idf.py flash monitor           # 烧录并监控

# 配置管理
idf.py menuconfig              # 打开配置菜单
idf.py set-target esp32c3      # 设置目标芯片

# 组件管理
idf.py add-dependency "组件名"  # 添加依赖
idf.py reconfigure             # 重新配置

# 监控控制
# 按 Ctrl+] 退出监控
# 按 Ctrl+T 打开菜单
```

### 2. 项目结构模板
```
your_project/
├── CMakeLists.txt              # 根构建文件
├── main/                       # 主要源代码目录
│   ├── CMakeLists.txt          # 主模块构建文件
│   ├── main.c                  # 主程序文件
│   ├── Kconfig.projbuild       # 配置选项（可选）
│   └── idf_component.yml       # 组件依赖（自动生成）
├── build/                      # 构建输出目录（自动生成）
├── sdkconfig                   # 项目配置文件（自动生成）
└── README.md                   # 项目说明
```

## 🔌 USB 转串口方案对比
### ESP32-C3-DevKitM-1（当前使用）
- **连接方式**：USB-Serial/JTAG 内置
- **设备名**：`/dev/tty.usbmodem1201`
- **特点**：
  - ✅ 免驱动，即插即用
  - ✅ 支持 USB-Serial 和 JTAG 调试
  - ✅ 稳定可靠
  - ❌ 有时需要手动进入下载模式

**下载模式操作**：
```bash
# 如果自动下载失败，手动进入下载模式：
# 1. 同时按住 BOOT + RESET 按钮
# 2. 先松开 RESET，继续按住 BOOT 2-3秒
# 3. 松开 BOOT 按钮
# 4. 立即执行烧录命令
```

### ESP32-S3 内置 USB-Serial-JTAG
- **设备名**：`/dev/cu.usbmodem*`
- **优势**：
  - ✅ 支持 DFU 下载（无需按键）
  - ✅ 完整 JTAG 调试功能
  - ✅ 最稳定的解决方案

### 传统 CH340/CP2102 串口芯片
- **设备名**：`/dev/cu.usbserial*`
- **特点**：
  - ❌ 需要安装驱动
  - ❌ 仅支持串口通信
  - ❌ 经常需要手动进入下载模式
