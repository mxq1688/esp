#!/usr/bin/env python3
"""
ESP32-C3 WiFi LED Controller - AP Mode Debug Script
"""

import requests
import json
import time

# 配置
BASE_URL = "http://192.168.5.139"

def test_web_interface():
    """测试Web界面是否可访问"""
    print("🌐 测试Web界面...")
    try:
        response = requests.get(f"{BASE_URL}/", timeout=5)
        print(f"✅ Web界面状态码: {response.status_code}")
        return True
    except Exception as e:
        print(f"❌ Web界面访问失败: {e}")
        return False

def test_ap_status():
    """测试AP状态API"""
    print("\n🔍 测试AP状态API...")
    try:
        response = requests.get(f"{BASE_URL}/api/ap-status", timeout=5)
        print(f"状态码: {response.status_code}")
        if response.status_code == 200:
            data = response.json()
            print(f"✅ AP状态: {json.dumps(data, indent=2, ensure_ascii=False)}")
            return data
        else:
            print(f"❌ AP状态请求失败: {response.text}")
            return None
    except Exception as e:
        print(f"❌ AP状态请求异常: {e}")
        return None

def test_ap_toggle(enable=True):
    """测试AP模式切换"""
    action = "开启" if enable else "关闭"
    print(f"\n🔄 测试{action}AP模式...")
    
    try:
        payload = {"enable": enable}
        print(f"发送数据: {json.dumps(payload, ensure_ascii=False)}")
        
        response = requests.post(
            f"{BASE_URL}/api/ap-mode",
            json=payload,
            headers={"Content-Type": "application/json"},
            timeout=10
        )
        
        print(f"状态码: {response.status_code}")
        print(f"响应头: {dict(response.headers)}")
        
        if response.status_code == 200:
            data = response.json()
            print(f"✅ {action}AP模式响应: {json.dumps(data, indent=2, ensure_ascii=False)}")
            return data
        else:
            print(f"❌ {action}AP模式失败: {response.text}")
            return None
    except Exception as e:
        print(f"❌ {action}AP模式请求异常: {e}")
        return None

def test_all_apis():
    """测试所有相关API"""
    print("🔧 测试所有API接口...")
    
    apis = [
        "/api/status",
        "/api/led/color",
        "/api/led/power", 
        "/api/led/effect"
    ]
    
    for api in apis:
        try:
            response = requests.get(f"{BASE_URL}{api}", timeout=5)
            print(f"✅ {api}: {response.status_code}")
        except Exception as e:
            print(f"❌ {api}: {e}")

def main():
    """主测试函数"""
    print("🚀 ESP32-C3 WiFi LED Controller - AP Mode Debug")
    print("=" * 60)
    
    # 1. 测试Web界面
    if not test_web_interface():
        print("❌ Web界面不可访问，请检查设备连接")
        return
    
    # 2. 测试所有API
    test_all_apis()
    
    # 3. 检查当前AP状态
    current_status = test_ap_status()
    if current_status is None:
        print("❌ 无法获取AP状态")
        return
    
    # 4. 测试开启AP模式
    print("\n" + "="*60)
    print("🧪 开始AP模式切换测试")
    print("="*60)
    
    # 如果AP已开启，先关闭
    if current_status.get('ap_enabled', False):
        print("📱 检测到AP已开启，先关闭...")
        result = test_ap_toggle(False)
        if result is None:
            print("❌ 关闭AP模式失败")
            return
        time.sleep(3)
    
    # 测试开启AP模式
    print("\n🔄 测试开启AP模式...")
    result = test_ap_toggle(True)
    if result is None:
        print("❌ 开启AP模式失败")
        return
    
    # 等待并检查状态
    print("\n⏳ 等待3秒后检查状态...")
    time.sleep(3)
    
    new_status = test_ap_status()
    if new_status:
        print(f"📱 切换后AP状态: {'开启' if new_status.get('ap_enabled') else '关闭'}")
    
    print("\n✅ 调试测试完成!")

if __name__ == "__main__":
    main()
