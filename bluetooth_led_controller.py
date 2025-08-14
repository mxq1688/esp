#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP32 LED蓝牙控制器
通过蓝牙BLE控制ESP32 LED
"""

import asyncio
import json
import struct
from typing import Optional
try:
    from bleak import BleakClient, BleakScanner
    BLEAK_AVAILABLE = True
except ImportError:
    BLEAK_AVAILABLE = False
    print("❌ 需要安装bleak库: pip install bleak")

class ESP32BluetoothController:
    def __init__(self, device_name: str = "ESP32-LED-Controller"):
        """
        初始化蓝牙控制器
        
        Args:
            device_name: ESP32设备名称
        """
        self.device_name = device_name
        self.client: Optional[BleakClient] = None
        self.device_address: Optional[str] = None
        
        # BLE服务和特征UUID (需要在ESP32固件中定义)
        self.service_uuid = "12345678-1234-1234-1234-123456789abc"
        self.command_char_uuid = "12345678-1234-1234-1234-123456789abd"
        self.response_char_uuid = "12345678-1234-1234-1234-123456789abe"
    
    async def scan_devices(self, timeout: int = 10) -> list:
        """扫描蓝牙设备"""
        if not BLEAK_AVAILABLE:
            return []
        
        print(f"🔍 扫描蓝牙设备... (超时: {timeout}秒)")
        devices = await BleakScanner.discover(timeout=timeout)
        
        esp32_devices = []
        for device in devices:
            if device.name and self.device_name in device.name:
                esp32_devices.append(device)
                print(f"📱 找到设备: {device.name} ({device.address})")
        
        return esp32_devices
    
    async def connect(self, address: Optional[str] = None) -> bool:
        """连接到ESP32设备"""
        if not BLEAK_AVAILABLE:
            return False
        
        try:
            if not address:
                # 自动搜索设备
                devices = await self.scan_devices()
                if not devices:
                    print("❌ 未找到ESP32设备")
                    return False
                address = devices[0].address
            
            self.device_address = address
            self.client = BleakClient(address)
            
            await self.client.connect()
            print(f"✅ 已连接到 {address}")
            
            # 启用通知
            await self.client.start_notify(self.response_char_uuid, self._notification_handler)
            
            return True
        except Exception as e:
            print(f"❌ 连接失败: {e}")
            return False
    
    async def disconnect(self):
        """断开连接"""
        if self.client and self.client.is_connected:
            await self.client.disconnect()
            print("🔌 已断开蓝牙连接")
    
    def _notification_handler(self, sender, data):
        """通知处理器"""
        try:
            response = data.decode('utf-8')
            print(f"📥 收到响应: {response}")
        except Exception as e:
            print(f"❌ 解析响应失败: {e}")
    
    async def _send_command(self, command: dict) -> bool:
        """发送命令"""
        if not self.client or not self.client.is_connected:
            print("❌ 设备未连接")
            return False
        
        try:
            message = json.dumps(command).encode('utf-8')
            await self.client.write_gatt_char(self.command_char_uuid, message)
            print(f"📤 命令已发送: {json.dumps(command, ensure_ascii=False)}")
            return True
        except Exception as e:
            print(f"❌ 发送失败: {e}")
            return False
    
    async def set_color(self, red: int, green: int, blue: int, brightness: int = 100):
        """设置LED颜色"""
        command = {
            "action": "set_color",
            "red": red,
            "green": green,
            "blue": blue,
            "brightness": brightness
        }
        return await self._send_command(command)
    
    async def set_power(self, on: bool):
        """设置LED电源"""
        command = {
            "action": "set_power",
            "power": on
        }
        return await self._send_command(command)
    
    async def set_effect(self, effect: str, speed: int = 50):
        """设置LED特效"""
        command = {
            "action": "set_effect",
            "effect": effect,
            "speed": speed
        }
        return await self._send_command(command)
    
    async def get_status(self):
        """获取设备状态"""
        command = {"action": "get_status"}
        return await self._send_command(command)

async def main():
    """主函数"""
    if not BLEAK_AVAILABLE:
        print("请先安装依赖: pip install bleak")
        return
    
    controller = ESP32BluetoothController()
    
    try:
        # 连接设备
        if not await controller.connect():
            return
        
        # 交互式控制
        print("\n🎮 蓝牙控制模式")
        print("📝 可用命令:")
        print("  1 - 红色LED")
        print("  2 - 绿色LED") 
        print("  3 - 蓝色LED")
        print("  4 - 白色LED")
        print("  0 - 关闭LED")
        print("  s - 获取状态")
        print("  q - 退出")
        print()
        
        while True:
            try:
                cmd = input("BLE> ").strip().lower()
                
                if cmd == 'q':
                    break
                elif cmd == '1':
                    await controller.set_color(255, 0, 0)
                elif cmd == '2':
                    await controller.set_color(0, 255, 0)
                elif cmd == '3':
                    await controller.set_color(0, 0, 255)
                elif cmd == '4':
                    await controller.set_color(255, 255, 255)
                elif cmd == '0':
                    await controller.set_power(False)
                elif cmd == 's':
                    await controller.get_status()
                else:
                    print("❌ 无效命令")
                
                await asyncio.sleep(0.1)  # 短暂延时
                
            except KeyboardInterrupt:
                break
            except Exception as e:
                print(f"❌ 错误: {e}")
    
    finally:
        await controller.disconnect()

if __name__ == '__main__':
    if BLEAK_AVAILABLE:
        asyncio.run(main())
    else:
        print("请安装蓝牙库: pip install bleak")