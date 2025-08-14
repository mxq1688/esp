#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP32 LED统一控制器
支持多种控制方式：Web API、串口、MQTT、TCP、UDP
"""

import argparse
import requests
import json
import time
import sys
from typing import Optional

class ESP32LEDController:
    def __init__(self):
        self.esp32_ip = "10.30.6.226"
        self.web_port = 80
        self.tcp_port = 8888
        self.serial_port = "/dev/tty.usbmodem1201"
        self.mqtt_broker = "localhost"
    
    def web_api_control(self, endpoint: str, data: dict = None) -> bool:
        """通过Web API控制"""
        try:
            url = f"http://{self.esp32_ip}:{self.web_port}/api/{endpoint}"
            if data:
                response = requests.post(url, json=data, timeout=5)
            else:
                response = requests.get(url, timeout=5)
            
            if response.ok:
                result = response.json()
                print(f"✅ Web API: {json.dumps(result, ensure_ascii=False)}")
                return True
            else:
                print(f"❌ Web API错误: {response.status_code}")
                return False
        except Exception as e:
            print(f"❌ Web API失败: {e}")
            return False
    
    def set_color(self, red: int, green: int, blue: int, brightness: int = 100, method: str = "web"):
        """设置LED颜色"""
        print(f"🎨 设置颜色: RGB({red}, {green}, {blue}), 亮度: {brightness}%, 方式: {method}")
        
        if method == "web":
            return self.web_api_control("led/color", {
                "red": red, "green": green, "blue": blue
            })
        else:
            print(f"❌ 暂不支持方式: {method}")
            return False
    
    def set_power(self, on: bool, method: str = "web"):
        """设置LED电源"""
        state = "开启" if on else "关闭"
        print(f"🔆 {state}LED, 方式: {method}")
        
        if method == "web":
            return self.web_api_control("led/power", {"power": on})
        else:
            print(f"❌ 暂不支持方式: {method}")
            return False
    
    def set_brightness(self, brightness: int, method: str = "web"):
        """设置LED亮度"""
        print(f"💡 设置亮度: {brightness}%, 方式: {method}")
        
        if method == "web":
            return self.web_api_control("led/brightness", {"brightness": brightness})
        else:
            print(f"❌ 暂不支持方式: {method}")
            return False
    
    def get_status(self, method: str = "web"):
        """获取设备状态"""
        print(f"📊 获取状态, 方式: {method}")
        
        if method == "web":
            return self.web_api_control("status")
        else:
            print(f"❌ 暂不支持方式: {method}")
            return False
    
    def color_demo(self, method: str = "web"):
        """颜色演示"""
        print(f"🌈 开始颜色演示 (方式: {method})")
        
        colors = [
            (255, 0, 0, "红色"),
            (0, 255, 0, "绿色"),
            (0, 0, 255, "蓝色"),
            (255, 255, 0, "黄色"),
            (255, 0, 255, "紫色"),
            (0, 255, 255, "青色"),
            (255, 255, 255, "白色")
        ]
        
        self.set_power(True, method)
        time.sleep(0.5)
        
        for red, green, blue, name in colors:
            print(f"🎨 显示{name}...")
            self.set_color(red, green, blue, 80, method)
            time.sleep(1.5)
        
        print("🌈 演示完成")
    
    def brightness_demo(self, method: str = "web"):
        """亮度演示"""
        print(f"💡 开始亮度演示 (方式: {method})")
        
        self.set_color(255, 255, 255, 100, method)  # 白色
        time.sleep(0.5)
        
        # 渐亮
        for brightness in range(0, 101, 10):
            print(f"💡 亮度: {brightness}%")
            self.set_brightness(brightness, method)
            time.sleep(0.3)
        
        # 渐暗
        for brightness in range(100, -1, -10):
            print(f"💡 亮度: {brightness}%")
            self.set_brightness(brightness, method)
            time.sleep(0.3)
        
        print("💡 演示完成")

def main():
    parser = argparse.ArgumentParser(description='ESP32 LED统一控制器')
    parser.add_argument('--ip', default='10.30.6.226', help='ESP32 IP地址')
    parser.add_argument('--method', choices=['web', 'serial', 'mqtt', 'tcp', 'udp'], 
                       default='web', help='控制方式')
    
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
    
    # 亮度命令
    brightness_parser = subparsers.add_parser('brightness', help='设置LED亮度')
    brightness_parser.add_argument('level', type=int, help='亮度等级 (0-100)')
    
    # 状态命令
    subparsers.add_parser('status', help='获取设备状态')
    
    # 演示命令
    subparsers.add_parser('demo-color', help='颜色演示')
    subparsers.add_parser('demo-brightness', help='亮度演示')
    
    # 预设颜色
    preset_parser = subparsers.add_parser('preset', help='预设颜色')
    preset_parser.add_argument('color', choices=['red', 'green', 'blue', 'white', 'yellow', 
                                               'purple', 'cyan', 'orange', 'pink'], help='预设颜色')
    
    # 快速命令
    subparsers.add_parser('on', help='开启LED')
    subparsers.add_parser('off', help='关闭LED')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return
    
    # 创建控制器
    controller = ESP32LEDController()
    controller.esp32_ip = args.ip
    
    # 预设颜色映射
    preset_colors = {
        'red': (255, 0, 0),
        'green': (0, 255, 0),
        'blue': (0, 0, 255),
        'white': (255, 255, 255),
        'yellow': (255, 255, 0),
        'purple': (255, 0, 255),
        'cyan': (0, 255, 255),
        'orange': (255, 165, 0),
        'pink': (255, 192, 203)
    }
    
    # 执行命令
    try:
        if args.command == 'color':
            controller.set_color(args.red, args.green, args.blue, args.brightness, args.method)
        elif args.command == 'power':
            controller.set_power(args.state == 'on', args.method)
        elif args.command == 'brightness':
            controller.set_brightness(args.level, args.method)
        elif args.command == 'status':
            controller.get_status(args.method)
        elif args.command == 'demo-color':
            controller.color_demo(args.method)
        elif args.command == 'demo-brightness':
            controller.brightness_demo(args.method)
        elif args.command == 'preset':
            red, green, blue = preset_colors[args.color]
            controller.set_color(red, green, blue, 100, args.method)
        elif args.command == 'on':
            controller.set_power(True, args.method)
        elif args.command == 'off':
            controller.set_power(False, args.method)
        
    except KeyboardInterrupt:
        print("\n👋 用户中断")
    except Exception as e:
        print(f"❌ 执行错误: {e}")

if __name__ == '__main__':
    main()