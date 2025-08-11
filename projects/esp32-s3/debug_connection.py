#!/usr/bin/env python3
import subprocess
import time
import requests
import json
import socket

def find_esp32_ip():
    """通过网络扫描查找ESP32-S3的IP地址"""
    print("🔍 正在扫描网络，查找ESP32-S3...")
    
    # 获取本机IP地址段
    hostname = socket.gethostname()
    local_ip = socket.gethostbyname(hostname)
    ip_base = '.'.join(local_ip.split('.')[:-1]) + '.'
    
    print(f"扫描网段: {ip_base}1-254")
    
    possible_ips = []
    for i in range(1, 255):
        ip = f"{ip_base}{i}"
        try:
            # 尝试连接ESP32-S3的API
            response = requests.get(f"http://{ip}/api/status", timeout=2)
            if response.status_code == 200:
                data = response.json()
                if 'color' in data:
                    print(f"✅ 找到ESP32-S3: {ip}")
                    possible_ips.append(ip)
        except:
            pass
    
    return possible_ips

def test_esp32_connection(ip):
    """测试ESP32-S3连接"""
    print(f"\n🧪 测试ESP32-S3连接: {ip}")
    
    try:
        # 测试GET请求
        response = requests.get(f"http://{ip}/api/status", timeout=5)
        print(f"GET /api/status - 状态码: {response.status_code}")
        if response.status_code == 200:
            data = response.json()
            print(f"当前颜色: {data.get('color', {})}")
        
        # 测试POST请求
        test_color = {"red": 255, "green": 0, "blue": 0, "brightness": 50}
        response = requests.post(
            f"http://{ip}/api/color", 
            json=test_color,
            headers={'Content-Type': 'application/json'},
            timeout=5
        )
        print(f"POST /api/color - 状态码: {response.status_code}")
        
        if response.status_code == 200:
            print("✅ ESP32-S3连接正常！")
            return True
        else:
            print(f"❌ POST请求失败: {response.text}")
    
    except requests.exceptions.ConnectTimeout:
        print("❌ 连接超时")
    except requests.exceptions.ConnectionError:
        print("❌ 连接错误 - ESP32-S3可能未运行")
    except Exception as e:
        print(f"❌ 测试失败: {e}")
    
    return False

def check_serial_monitor():
    """检查串口监视器输出"""
    print("\n📡 检查ESP32-S3串口输出...")
    
    # 查找串口设备
    try:
        result = subprocess.run(['ls', '/dev/cu.*'], capture_output=True, text=True, shell=True)
        usb_devices = [line for line in result.stdout.split('\n') if 'usbmodem' in line]
        
        if usb_devices:
            print(f"发现USB设备: {usb_devices}")
            
            # 提示用户查看串口输出
            print("\n💡 建议操作:")
            print("1. 在另一个终端运行以下命令查看ESP32-S3输出:")
            for device in usb_devices:
                print(f"   source /Users/meng/stu/esp/esp-idf/export.sh && idf.py -p {device} monitor")
            print("2. 查找类似 'got ip:192.168.x.x' 的IP地址信息")
            print("3. 使用该IP地址连接PWA")
        else:
            print("❌ 未找到USB串口设备")
            print("请检查ESP32-S3是否正确连接")
    
    except Exception as e:
        print(f"检查串口失败: {e}")

def main():
    print("🚀 ESP32-S3 PWA 连接诊断工具")
    print("=" * 50)
    
    # 检查串口
    check_serial_monitor()
    
    # 尝试查找ESP32-S3
    esp_ips = find_esp32_ip()
    
    if esp_ips:
        print(f"\n找到 {len(esp_ips)} 个可能的ESP32-S3设备:")
        for ip in esp_ips:
            if test_esp32_connection(ip):
                print(f"\n🎉 请在PWA中使用IP地址: {ip}")
                break
    else:
        print("\n❌ 未在网络中找到ESP32-S3")
        print("\n📋 故障排除步骤:")
        print("1. 确保ESP32-S3已烧录并正在运行")
        print("2. 检查WiFi连接（SSID: mem2）")
        print("3. 查看串口输出获取IP地址")
        print("4. 确保ESP32-S3和电脑在同一网络")
    
    print("\n🌐 PWA访问地址: http://localhost:8080")

if __name__ == "__main__":
    main() 