#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP32 LED TCP Socket控制器
通过TCP Socket直接控制ESP32 LED
"""

import socket
import json
import time
import threading
import argparse
from typing import Optional

class ESP32TCPController:
    def __init__(self, host: str = "10.30.6.226", port: int = 8888):
        """
        初始化TCP控制器
        
        Args:
            host: ESP32 IP地址
            port: TCP端口
        """
        self.host = host
        self.port = port
        self.socket: Optional[socket.socket] = None
        self.connected = False
        self.listen_thread: Optional[threading.Thread] = None
        self.running = False
    
    def connect(self) -> bool:
        """连接到ESP32设备"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(5)
            self.socket.connect((self.host, self.port))
            self.connected = True
            print(f"✅ 已连接到 {self.host}:{self.port}")
            
            # 启动监听线程
            self.running = True
            self.listen_thread = threading.Thread(target=self._listen_responses)
            self.listen_thread.daemon = True
            self.listen_thread.start()
            
            return True
        except Exception as e:
            print(f"❌ 连接失败: {e}")
            return False
    
    def disconnect(self):
        """断开连接"""
        self.running = False
        self.connected = False
        
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
        
        if self.listen_thread:
            self.listen_thread.join(timeout=1)
        
        print("🔌 已断开TCP连接")
    
    def _listen_responses(self):
        """监听响应"""
        while self.running and self.connected:
            try:
                if self.socket:
                    self.socket.settimeout(1)
                    data = self.socket.recv(1024)
                    if data:
                        response = data.decode('utf-8').strip()
                        if response:
                            print(f"📥 收到响应: {response}")
            except socket.timeout:
                continue
            except Exception as e:
                if self.running:
                    print(f"❌ 接收响应失败: {e}")
                break
    
    def _send_command(self, command: dict) -> bool:
        """发送命令"""
        if not self.connected or not self.socket:
            print("❌ 设备未连接")
            return False
        
        try:
            message = json.dumps(command) + '\n'
            self.socket.send(message.encode('utf-8'))
            print(f"📤 命令已发送: {json.dumps(command, ensure_ascii=False)}")
            return True
        except Exception as e:
            print(f"❌ 发送失败: {e}")
            self.connected = False
            return False
    
    def set_color(self, red: int, green: int, blue: int, brightness: int = 100):
        """设置LED颜色"""
        command = {
            "cmd": "set_color",
            "red": red,
            "green": green,
            "blue": blue,
            "brightness": brightness
        }
        return self._send_command(command)
    
    def set_power(self, on: bool):
        """设置LED电源"""
        command = {
            "cmd": "set_power",
            "power": on
        }
        return self._send_command(command)
    
    def set_effect(self, effect: str, speed: int = 50):
        """设置LED特效"""
        command = {
            "cmd": "set_effect",
            "effect": effect,
            "speed": speed
        }
        return self._send_command(command)
    
    def get_status(self):
        """获取设备状态"""
        command = {"cmd": "get_status"}
        return self._send_command(command)
    
    def set_brightness(self, brightness: int):
        """设置亮度"""
        command = {
            "cmd": "set_brightness",
            "brightness": brightness
        }
        return self._send_command(command)

def main():
    parser = argparse.ArgumentParser(description='ESP32 LED TCP控制器')
    parser.add_argument('-H', '--host', default='10.30.6.226', help='ESP32 IP地址')
    parser.add_argument('-p', '--port', type=int, default=8888, help='TCP端口')
    
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
    
    # 亮度命令
    brightness_parser = subparsers.add_parser('brightness', help='设置LED亮度')
    brightness_parser.add_argument('level', type=int, help='亮度等级 (0-100)')
    
    # 状态命令
    subparsers.add_parser('status', help='获取设备状态')
    
    # 交互模式
    subparsers.add_parser('interactive', help='进入交互模式')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return
    
    # 创建控制器
    controller = ESP32TCPController(args.host, args.port)
    
    if not controller.connect():
        return
    
    try:
        if args.command == 'color':
            controller.set_color(args.red, args.green, args.blue, args.brightness)
            time.sleep(0.5)
        elif args.command == 'power':
            controller.set_power(args.state == 'on')
            time.sleep(0.5)
        elif args.command == 'effect':
            controller.set_effect(args.type, args.speed)
            time.sleep(0.5)
        elif args.command == 'brightness':
            controller.set_brightness(args.level)
            time.sleep(0.5)
        elif args.command == 'status':
            controller.get_status()
            time.sleep(0.5)
        elif args.command == 'interactive':
            interactive_mode(controller)
    finally:
        controller.disconnect()

def interactive_mode(controller: ESP32TCPController):
    """交互模式"""
    print("\n🎮 TCP交互模式")
    print("📝 快捷键:")
    print("  r - 红色    g - 绿色    b - 蓝色    w - 白色")
    print("  1-9 - 亮度  0 - 关闭    s - 状态    q - 退出")
    print()
    
    while True:
        try:
            cmd = input("TCP> ").strip().lower()
            
            if cmd == 'q':
                break
            elif cmd == 'r':
                controller.set_color(255, 0, 0)
            elif cmd == 'g':
                controller.set_color(0, 255, 0)
            elif cmd == 'b':
                controller.set_color(0, 0, 255)
            elif cmd == 'w':
                controller.set_color(255, 255, 255)
            elif cmd == '0':
                controller.set_power(False)
            elif cmd.isdigit() and 1 <= int(cmd) <= 9:
                brightness = int(cmd) * 11  # 1-9 映射到 11-99
                controller.set_brightness(brightness)
            elif cmd == 's':
                controller.get_status()
            else:
                print("❌ 无效命令")
            
            time.sleep(0.1)
            
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"❌ 错误: {e}")
    
    print("\n👋 再见!")

if __name__ == '__main__':
    main()