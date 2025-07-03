# ESP32 快速开发指南

## 1. 激活环境
```bash
source /Users/meng/stu/esp/esp-idf/export.sh
```

## 2. 创建项目
```bash
mkdir -p ~/stu/esp/projects/{esp32-c3,esp32-s3}
cp -r ~/stu/esp/esp-idf/examples/get-started/hello_world ~/stu/esp/projects/esp32-c3/
cp -r ~/stu/esp/esp-idf/examples/get-started/hello_world ~/stu/esp/projects/esp32-s3/
```

## 3. ESP32-C3 编译烧录
```bash
cd ~/stu/esp/projects/esp32-c3/hello_world
idf.py set-target esp32c3
idf.py build
idf.py -p /dev/cu.usbserial-XXX flash monitor
```

## 4. ESP32-S3 编译烧录

### 方法一：常规烧录（如果失败请用方法二）
```bash
cd ~/stu/esp/projects/esp32-s3/hello_world
idf.py set-target esp32s3
idf.py build
# ESP32-S3 使用内置 USB，进入下载模式：按住 BOOT，按一下 RST，松开 BOOT
idf.py -p /dev/cu.usbmodem* flash monitor
```

### 方法二：DFU 烧录（推荐）
```bash
cd ~/stu/esp/projects/esp32-s3/hello_world
idf.py set-target esp32s3
idf.py build
# 不需要按按钮，直接烧录
idf.py dfu-flash
# 烧录完成后查看输出
idf.py monitor
```

## 5. 查看串口
```bash
ls /dev/cu.*
# ESP32-S3 内置 USB: /dev/cu.usbmodem*
# ESP32-C3 外置芯片: /dev/cu.usbserial*
```

## 常用命令
- `idf.py clean` - 清理
- `idf.py build` - 编译
- `idf.py flash` - 烧录
- `idf.py monitor` - 监控
- 按 `Ctrl+]` 退出监控

## USB 转串口方案对比

### 1. ESP32-S3 内置 USB-Serial-JTAG
- **优点**：
  - 芯片内置，不需要额外器件
  - 免驱动，即插即用
  - 支持 JTAG 调试（断点、单步执行）
  - 支持 DFU 下载（无需进入下载模式）
  - 设备名：`/dev/cu.usbmodem*`

- **使用方法**：
  ```bash
  # 方法一：DFU 模式（推荐）
  idf.py dfu-flash    # 烧录，无需按键
  idf.py monitor      # 监控输出

  # 方法二：普通模式
  idf.py -p /dev/cu.usbmodem* flash monitor
  ```

### 2. CH340 串口芯片
- **特点**：
  - 需要外置串口芯片
  - 需要安装驱动
  - 仅支持串口通信
  - 设备名：`/dev/cu.usbserial*` 或 `/dev/cu.wchusbserial*`

- **使用方法**：
  ```bash
  # 需要进入下载模式：按住 BOOT，按一下 RST，松开 BOOT
  idf.py -p /dev/cu.usbserial* flash monitor
  ```

### 3. 调试功能对比
| 功能 | USB-Serial-JTAG | CH340 |
|-----|-----------------|-------|
| 串口通信 | ✅ | ✅ |
| 程序下载 | ✅ | ✅ |
| JTAG调试 | ✅ | ❌ |
| DFU下载 | ✅ | ❌ |
| 驱动安装 | 不需要 | 需要 |
| 稳定性 | 很好 | 一般 |

### 4. 使用建议
- ESP32-S3 推荐使用内置 USB-Serial-JTAG，功能更强大
- 如果用 CH340，记得安装驱动并经常备份代码
- 调试时 USB-Serial-JTAG 能提供更多帮助 