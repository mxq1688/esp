#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP32 LED串口控制器
通过串口发送命令控制ESP32 LED
"""

import serial
import time
import json
import argparse
from typing import Optional

class ESP32SerialController:
    def __init__(self, port: str = '/dev/tty.usbmodem1201', baudrate: int = 115200):
        """
        初始化串口控制器
        
        Args:
            port: 串口端口
            baudrate: 波特率
        """
        self.port = port
        self.baudrate = baudrate
        self.serial_conn: Optional[serial.Serial] = None
        
    def connect(self) -> bool:
        """连接到ESP32设备"""
        try:
            self.serial_conn = serial.Serial(self.port, self.baudrate, timeout=2)
            time.sleep(2)  # 等待ESP32复位
            print(f"✅ 已连接到 {self.port}")
            return True
        except Exception as e:
            print(f"❌ 连接失败: {e}")
            return False
    
    def disconnect(self):
        """断开连接"""
        if self.serial_conn:
            self.serial_conn.close()
            print("🔌 已断开连接")
    
    def send_command(self, command: str) -> bool:
        """发送命令到ESP32"""
        if not self.serial_conn:
            print("❌ 未连接设备")
            return False
        
        try:
            cmd = f"{command}\n"
            self.serial_conn.write(cmd.encode())
            print(f"📤 发送命令: {command}")
            
            # 读取响应
            time.sleep(0.1)
            if self.serial_conn.in_waiting:
                response = self.serial_conn.readline().decode().strip()
                print(f"📥 响应: {response}")
            
            return True
        except Exception as e:
            print(f"❌ 发送失败: {e}")
            return False
    
    def set_color(self, red: int, green: int, blue: int, brightness: int = 100):
        """设置LED颜色"""
        command = f"LED_COLOR {red} {green} {blue} {brightness}"
        return self.send_command(command)
    
    def set_power(self, on: bool):
        """设置LED电源"""
        command = f"LED_POWER {'ON' if on else 'OFF'}"
        return self.send_command(command)
    
    def set_effect(self, effect: str, speed: int = 50):
        """设置LED特效"""
        command = f"LED_EFFECT {effect} {speed}"
        return self.send_command(command)
    
    def get_status(self):
        """获取设备状态"""
        return self.send_command("STATUS")

def main():
    parser = argparse.ArgumentParser(description='ESP32 LED串口控制器')
    parser.add_argument('-p', '--port', default='/dev/tty.usbmodem1201', help='串口端口')
    parser.add_argument('-b', '--baudrate', type=int, default=115200, help='波特率')
    
    subparsers = parser.add_subparsers(dest='command', help='控制命令')
    
    # 颜色命令
    color_parser = subparsers.add_parser('color', help='设置LED颜色')
    color_parser.add_argument('red', type=int, help='红色 (0-255)')
    color_parser.add_argument('green', type=int, help='绿色 (0-255)')
    color_parser.add_argument('blue', type=int, help='蓝色 (0-255)')
    color_parser.add_argument('--brightness', type=int, default=100, help='亮度 (0-100)')
    
    # 电源命令
    power_parser = subparsers.add_parser('power', help='控制LED电源')
    power_parser.add_argument('state', choices=['on', 'off'], help='电源状态')
    
    # 特效命令
    effect_parser = subparsers.add_parser('effect', help='设置LED特效')
    effect_parser.add_argument('type', choices=['static', 'rainbow', 'breathing', 'blink'], help='特效类型')
    effect_parser.add_argument('--speed', type=int, default=50, help='特效速度 (0-100)')
    
    # 状态命令
    subparsers.add_parser('status', help='获取设备状态')
    
    # 交互模式
    subparsers.add_parser('interactive', help='进入交互模式')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return
    
    # 创建控制器
    controller = ESP32SerialController(args.port, args.baudrate)
    
    if not controller.connect():
        return
    
    try:
        if args.command == 'color':
            controller.set_color(args.red, args.green, args.blue, args.brightness)
        elif args.command == 'power':
            controller.set_power(args.state == 'on')
        elif args.command == 'effect':
            controller.set_effect(args.type, args.speed)
        elif args.command == 'status':
            controller.get_status()
        elif args.command == 'interactive':
            interactive_mode(controller)
    finally:
        controller.disconnect()

def interactive_mode(controller: ESP32SerialController):
    """交互模式"""
    print("\n🎮 进入交互模式")
    print("📝 可用命令:")
    print("  color <R> <G> <B> [brightness] - 设置颜色")
    print("  power <on|off>                - 控制电源")
    print("  effect <type> [speed]         - 设置特效")
    print("  status                        - 获取状态")
    print("  quit                          - 退出")
    print()
    
    while True:
        try:
            cmd = input("ESP32> ").strip()
            if not cmd:
                continue
            
            if cmd.lower() == 'quit':
                break
            
            parts = cmd.split()
            if len(parts) == 0:
                continue
            
            if parts[0] == 'color' and len(parts) >= 4:
                r, g, b = int(parts[1]), int(parts[2]), int(parts[3])
                brightness = int(parts[4]) if len(parts) > 4 else 100
                controller.set_color(r, g, b, brightness)
            elif parts[0] == 'power' and len(parts) >= 2:
                controller.set_power(parts[1].lower() == 'on')
            elif parts[0] == 'effect' and len(parts) >= 2:
                effect_type = parts[1]
                speed = int(parts[2]) if len(parts) > 2 else 50
                controller.set_effect(effect_type, speed)
            elif parts[0] == 'status':
                controller.get_status()
            else:
                print("❌ 无效命令")
                
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"❌ 错误: {e}")
    
    print("\n👋 再见!")

if __name__ == '__main__':
    main()