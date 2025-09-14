# ESP32 MicroPython 固件集合与代码上传指南

## 📁 目录结构

```
microPython/bin/
├── esp32/                    # ESP32 经典款固件
│   └── esp32-micropython.bin
├── esp32s3/                  # ESP32-S3 固件  
│   └── esp32s3-micropython.bin
├── esp32c3/                  # ESP32-C3 固件
│   └── esp32c3-micropython.bin
├── esp32c6/                  # ESP32-C6 固件 (需手动下载)
├── flash_esp32.sh            # ESP32 经典款烧录脚本
├── flash_esp32s3.sh          # ESP32-S3 烧录脚本
├── 代码上传指南.md           # 完整的代码上传教程
└── README.md                 # 本文件
```

## 🔧 固件版本信息

| 型号 | 版本 | 文件大小 | 状态 |
|------|------|----------|------|
| ESP32 经典款 | v1.23.0 | 1.6MB | ✅ 已下载 |
| ESP32-S3 | v1.23.0 | 1.6MB | ✅ 已下载 |
| ESP32-C3 | v1.23.0 | 1.6MB | ✅ 已下载 |
| ESP32-C6 | - | - | ❌ 需手动下载 |

## 📥 ESP32-C6 手动下载

ESP32-C6 固件需要手动下载：

1. 访问：https://micropython.org/download/esp32c6/
2. 下载最新的 `.bin` 文件
3. 重命名为 `esp32c6-micropython.bin`
4. 放入 `esp32c6/` 目录

## 🚀 烧录命令

### ESP32 经典款
```bash
python3 -m esptool --chip esp32 --port /dev/tty.usbserial-* erase_flash
python3 -m esptool --chip esp32 --port /dev/tty.usbserial-* write_flash -z 0x1000 esp32/esp32-micropython.bin
```

### ESP32-S3
```bash
python3 -m esptool --chip esp32s3 --port /dev/tty.usbserial-* erase_flash
python3 -m esptool --chip esp32s3 --port /dev/tty.usbserial-* write_flash -z 0x0 esp32s3/esp32s3-micropython.bin
```

### ESP32-C3
```bash
python3 -m esptool --chip esp32c3 --port /dev/tty.usbserial-* erase_flash
python3 -m esptool --chip esp32c3 --port /dev/tty.usbserial-* write_flash -z 0x0 esp32c3/esp32c3-micropython.bin
```

### ESP32-C6
```bash
python3 -m esptool --chip esp32c6 --port /dev/tty.usbserial-* erase_flash
python3 -m esptool --chip esp32c6 --port /dev/tty.usbserial-* write_flash -z 0x0 esp32c6/esp32c6-micropython.bin
```

## 🚀 快速开始

### 1. 固件烧录 (首次使用)
```bash
# ESP32 经典款
./flash_esp32.sh

# ESP32-S3  
./flash_esp32s3.sh
```

### 2. 代码上传
固件烧录完成后，参考 [代码上传指南.md](./代码上传指南.md) 了解如何上传Python代码到设备。

## ⚠️ 重要提醒

1. **绝对不能混用固件** - 每个型号必须使用对应的固件
2. **替换端口号** - 将 `/dev/tty.usbserial-*` 替换为实际端口
3. **确认芯片型号** - 烧录前务必确认开发板型号
4. **备份重要数据** - 烧录会清除所有数据

## 🔍 识别芯片型号

```bash
python3 -m esptool chip_id
```

## 📚 相关文档

- [代码上传指南.md](./代码上传指南.md) - 详细的代码上传教程
- [MicroPython 官方文档](https://docs.micropython.org/)
- [ESP32 开发指南](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
