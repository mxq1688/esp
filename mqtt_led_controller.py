#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP32 LED MQTT控制器
通过MQTT协议控制ESP32 LED
"""

import paho.mqtt.client as mqtt
import json
import time
import argparse
from typing import Dict, Any

class ESP32MQTTController:
    def __init__(self, broker: str = "localhost", port: int = 1883, 
                 client_id: str = "esp32_controller"):
        """
        初始化MQTT控制器
        
        Args:
            broker: MQTT代理服务器地址
            port: MQTT端口
            client_id: 客户端ID
        """
        self.broker = broker
        self.port = port
        self.client_id = client_id
        self.client = mqtt.Client(client_id=client_id)
        
        # MQTT主题
        self.topics = {
            'command': 'esp32/led/command',
            'status': 'esp32/led/status',
            'response': 'esp32/led/response'
        }
        
        # 设置回调函数
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message
        
    def _on_connect(self, client, userdata, flags, rc):
        """连接回调"""
        if rc == 0:
            print(f"✅ 已连接到MQTT代理: {self.broker}:{self.port}")
            # 订阅响应主题
            client.subscribe(self.topics['response'])
            client.subscribe(self.topics['status'])
        else:
            print(f"❌ MQTT连接失败，代码: {rc}")
    
    def _on_message(self, client, userdata, msg):
        """消息回调"""
        try:
            topic = msg.topic
            payload = json.loads(msg.payload.decode())
            print(f"📥 收到消息 [{topic}]: {json.dumps(payload, ensure_ascii=False)}")
        except Exception as e:
            print(f"❌ 解析消息失败: {e}")
    
    def connect(self) -> bool:
        """连接到MQTT代理"""
        try:
            self.client.connect(self.broker, self.port, 60)
            self.client.loop_start()
            time.sleep(1)
            return True
        except Exception as e:
            print(f"❌ 连接失败: {e}")
            return False
    
    def disconnect(self):
        """断开连接"""
        self.client.loop_stop()
        self.client.disconnect()
        print("🔌 已断开MQTT连接")
    
    def _send_command(self, command: Dict[str, Any]):
        """发送命令"""
        try:
            message = json.dumps(command)
            result = self.client.publish(self.topics['command'], message)
            
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                print(f"📤 命令已发送: {json.dumps(command, ensure_ascii=False)}")
                return True
            else:
                print(f"❌ 发送失败，错误码: {result.rc}")
                return False
        except Exception as e:
            print(f"❌ 发送异常: {e}")
            return False
    
    def set_color(self, red: int, green: int, blue: int, brightness: int = 100):
        """设置LED颜色"""
        command = {
            "action": "set_color",
            "params": {
                "red": red,
                "green": green,
                "blue": blue,
                "brightness": brightness
            },
            "timestamp": time.time()
        }
        return self._send_command(command)
    
    def set_power(self, on: bool):
        """设置LED电源"""
        command = {
            "action": "set_power",
            "params": {"power": on},
            "timestamp": time.time()
        }
        return self._send_command(command)
    
    def set_effect(self, effect: str, speed: int = 50):
        """设置LED特效"""
        command = {
            "action": "set_effect",
            "params": {
                "effect": effect,
                "speed": speed
            },
            "timestamp": time.time()
        }
        return self._send_command(command)
    
    def get_status(self):
        """获取设备状态"""
        command = {
            "action": "get_status",
            "timestamp": time.time()
        }
        return self._send_command(command)
    
    def start_listening(self):
        """开始监听消息"""
        print("👂 开始监听MQTT消息... (按Ctrl+C退出)")
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\n📻 停止监听")

def main():
    parser = argparse.ArgumentParser(description='ESP32 LED MQTT控制器')
    parser.add_argument('-b', '--broker', default='localhost', help='MQTT代理地址')
    parser.add_argument('-p', '--port', type=int, default=1883, help='MQTT端口')
    parser.add_argument('-c', '--client-id', default='esp32_controller', help='客户端ID')
    
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
    
    # 监听模式
    subparsers.add_parser('listen', help='监听模式')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return
    
    # 创建控制器
    controller = ESP32MQTTController(args.broker, args.port, args.client_id)
    
    if not controller.connect():
        return
    
    try:
        if args.command == 'color':
            controller.set_color(args.red, args.green, args.blue, args.brightness)
            time.sleep(1)  # 等待响应
        elif args.command == 'power':
            controller.set_power(args.state == 'on')
            time.sleep(1)
        elif args.command == 'effect':
            controller.set_effect(args.type, args.speed)
            time.sleep(1)
        elif args.command == 'status':
            controller.get_status()
            time.sleep(1)
        elif args.command == 'listen':
            controller.start_listening()
    finally:
        controller.disconnect()

if __name__ == '__main__':
    main()