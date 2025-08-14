#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP32 LED UDP控制器
通过UDP广播控制局域网中的ESP32设备
"""

import socket
import json
import time
import threading
import argparse
from typing import Dict, List, Tuple

class ESP32UDPController:
    def __init__(self, broadcast_port: int = 9999, listen_port: int = 9998):
        """
        初始化UDP控制器
        
        Args:
            broadcast_port: 广播端口
            listen_port: 监听端口
        """
        self.broadcast_port = broadcast_port
        self.listen_port = listen_port
        self.broadcast_socket: socket.socket = None
        self.listen_socket: socket.socket = None
        self.devices: Dict[str, Tuple[str, int]] = {}  # device_id -> (ip, last_seen)
        self.running = False
        self.listen_thread: threading.Thread = None
    
    def start(self) -> bool:
        """启动UDP控制器"""
        try:
            # 创建广播socket
            self.broadcast_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            
            # 创建监听socket
            self.listen_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.listen_socket.bind(('', self.listen_port))
            self.listen_socket.settimeout(1)
            
            # 启动监听线程
            self.running = True
            self.listen_thread = threading.Thread(target=self._listen_responses)
            self.listen_thread.daemon = True
            self.listen_thread.start()
            
            print(f"✅ UDP控制器已启动 (广播:{self.broadcast_port}, 监听:{self.listen_port})")
            return True
            
        except Exception as e:
            print(f"❌ 启动失败: {e}")
            return False
    
    def stop(self):
        """停止UDP控制器"""
        self.running = False
        
        if self.broadcast_socket:
            self.broadcast_socket.close()
        if self.listen_socket:
            self.listen_socket.close()
        
        if self.listen_thread:
            self.listen_thread.join(timeout=2)
        
        print("🔌 UDP控制器已停止")
    
    def _listen_responses(self):
        """监听响应"""
        while self.running:
            try:
                data, addr = self.listen_socket.recvfrom(1024)
                response = data.decode('utf-8')
                self._handle_response(response, addr)
            except socket.timeout:
                continue
            except Exception as e:
                if self.running:
                    print(f"❌ 监听异常: {e}")
    
    def _handle_response(self, response: str, addr: Tuple[str, int]):
        """处理响应"""
        try:
            data = json.loads(response)
            device_id = data.get('device_id', f"unknown_{addr[0]}")
            
            # 更新设备列表
            self.devices[device_id] = (addr[0], time.time())
            
            print(f"📥 [{device_id}@{addr[0]}]: {json.dumps(data, ensure_ascii=False)}")
            
        except Exception as e:
            print(f"❌ 解析响应失败: {e}")
    
    def discover_devices(self, timeout: int = 3) -> List[str]:
        """发现设备"""
        print(f"🔍 搜索ESP32设备... (超时: {timeout}秒)")
        
        # 清空设备列表
        self.devices.clear()
        
        # 发送发现广播
        discovery_msg = {
            "cmd": "discover",
            "timestamp": time.time()
        }
        
        self._broadcast_command(discovery_msg)
        
        # 等待响应
        time.sleep(timeout)
        
        # 清理过期设备
        current_time = time.time()
        active_devices = []
        
        for device_id, (ip, last_seen) in self.devices.items():
            if current_time - last_seen < timeout + 1:
                active_devices.append(f"{device_id}@{ip}")
                print(f"📱 发现设备: {device_id} ({ip})")
        
        return active_devices
    
    def _broadcast_command(self, command: dict, target_ip: str = None):
        """广播命令"""
        try:
            message = json.dumps(command).encode('utf-8')
            
            if target_ip:
                # 单播到指定设备
                self.broadcast_socket.sendto(message, (target_ip, self.broadcast_port))
                print(f"📤 发送到 {target_ip}: {json.dumps(command, ensure_ascii=False)}")
            else:
                # 广播到所有设备
                self.broadcast_socket.sendto(message, ('<broadcast>', self.broadcast_port))
                print(f"📻 广播命令: {json.dumps(command, ensure_ascii=False)}")
            
            return True
        except Exception as e:
            print(f"❌ 发送失败: {e}")
            return False
    
    def set_color(self, red: int, green: int, blue: int, brightness: int = 100, target_ip: str = None):
        """设置LED颜色"""
        command = {
            "cmd": "set_color",
            "red": red,
            "green": green,
            "blue": blue,
            "brightness": brightness,
            "timestamp": time.time()
        }
        return self._broadcast_command(command, target_ip)
    
    def set_power(self, on: bool, target_ip: str = None):
        """设置LED电源"""
        command = {
            "cmd": "set_power",
            "power": on,
            "timestamp": time.time()
        }
        return self._broadcast_command(command, target_ip)
    
    def set_effect(self, effect: str, speed: int = 50, target_ip: str = None):
        """设置LED特效"""
        command = {
            "cmd": "set_effect",
            "effect": effect,
            "speed": speed,
            "timestamp": time.time()
        }
        return self._broadcast_command(command, target_ip)
    
    def get_status(self, target_ip: str = None):
        """获取设备状态"""
        command = {
            "cmd": "get_status",
            "timestamp": time.time()
        }
        return self._broadcast_command(command, target_ip)

def main():
    parser = argparse.ArgumentParser(description='ESP32 LED UDP控制器')
    parser.add_argument('-bp', '--broadcast-port', type=int, default=9999, help='广播端口')
    parser.add_argument('-lp', '--listen-port', type=int, default=9998, help='监听端口')
    parser.add_argument('-t', '--target', help='目标设备IP（可选）')
    
    subparsers = parser.add_subparsers(dest='command', help='控制命令')
    
    # 发现设备
    discover_parser = subparsers.add_parser('discover', help='发现设备')
    discover_parser.add_argument('--timeout', type=int, default=3, help='超时时间')
    
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
    
    # 监听模式
    subparsers.add_parser('monitor', help='监听模式')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return
    
    # 创建控制器
    controller = ESP32UDPController(args.broadcast_port, args.listen_port)
    
    if not controller.start():
        return
    
    try:
        target_ip = getattr(args, 'target', None)
        
        if args.command == 'discover':
            controller.discover_devices(args.timeout)
        elif args.command == 'color':
            controller.set_color(args.red, args.green, args.blue, args.brightness, target_ip)
            time.sleep(1)
        elif args.command == 'power':
            controller.set_power(args.state == 'on', target_ip)
            time.sleep(1)
        elif args.command == 'effect':
            controller.set_effect(args.type, args.speed, target_ip)
            time.sleep(1)
        elif args.command == 'status':
            controller.get_status(target_ip)
            time.sleep(1)
        elif args.command == 'monitor':
            print("📻 监听模式 (按Ctrl+C退出)")
            try:
                while True:
                    time.sleep(1)
            except KeyboardInterrupt:
                print("\n📻 停止监听")
    finally:
        controller.stop()

if __name__ == '__main__':
    main()