#!/usr/bin/env python3
import requests
import json
import time

ESP_IP = "192.168.5.48"
BASE_URL = f"http://{ESP_IP}"

def test_options_request():
    """测试OPTIONS预检请求"""
    print("🔍 测试OPTIONS预检请求...")
    try:
        response = requests.options(f"{BASE_URL}/api/color", headers={
            'Origin': 'http://localhost:8080',
            'Access-Control-Request-Method': 'POST',
            'Access-Control-Request-Headers': 'content-type'
        })
        print(f"   状态码: {response.status_code}")
        print(f"   CORS头: {response.headers.get('Access-Control-Allow-Origin', '无')}")
        return response.status_code == 200
    except Exception as e:
        print(f"   ❌ 错误: {e}")
        return False

def test_get_status():
    """测试GET状态请求"""
    print("📊 测试GET状态请求...")
    try:
        response = requests.get(f"{BASE_URL}/api/status", headers={
            'Origin': 'http://localhost:8080'
        })
        if response.status_code == 200:
            data = response.json()
            color = data['color']
            print(f"   ✅ 当前颜色: R={color['red']} G={color['green']} B={color['blue']} 亮度={color['brightness']}%")
            return True
        else:
            print(f"   ❌ 状态码: {response.status_code}")
            return False
    except Exception as e:
        print(f"   ❌ 错误: {e}")
        return False

def test_post_color(red, green, blue, brightness=75):
    """测试POST颜色设置"""
    color_name = "红色" if red > 200 else "绿色" if green > 200 else "蓝色" if blue > 200 else "自定义"
    print(f"🎨 测试设置{color_name} (R={red} G={green} B={blue} 亮度={brightness}%)...")
    
    try:
        # 模拟浏览器的完整请求
        response = requests.post(f"{BASE_URL}/api/color", 
            headers={
                'Content-Type': 'application/json',
                'Origin': 'http://localhost:8080'
            },
            json={
                'red': red,
                'green': green,
                'blue': blue,
                'brightness': brightness
            }
        )
        
        if response.status_code == 200:
            print(f"   ✅ 设置成功")
            time.sleep(0.5)  # 等待ESP32-S3处理
            return test_get_status()  # 验证设置
        else:
            print(f"   ❌ 状态码: {response.status_code}")
            print(f"   响应: {response.text}")
            return False
    except Exception as e:
        print(f"   ❌ 错误: {e}")
        return False

def main():
    print("🚀 PWA API 完整测试")
    print("=" * 50)
    
    tests = [
        ("OPTIONS预检", test_options_request),
        ("GET状态", test_get_status),
        ("设置红色", lambda: test_post_color(255, 0, 0)),
        ("设置绿色", lambda: test_post_color(0, 255, 0)),
        ("设置蓝色", lambda: test_post_color(0, 0, 255)),
        ("设置紫色", lambda: test_post_color(128, 0, 128, 60)),
    ]
    
    passed = 0
    total = len(tests)
    
    for test_name, test_func in tests:
        print(f"\n🧪 {test_name}")
        if test_func():
            passed += 1
            print(f"   ✅ 通过")
        else:
            print(f"   ❌ 失败")
        time.sleep(1)
    
    print(f"\n📈 测试结果: {passed}/{total} 通过")
    
    if passed == total:
        print("🎉 所有测试通过！PWA应该能正常工作。")
        print(f"🌐 PWA地址: http://localhost:8080")
        print(f"🔧 调试页面: http://localhost:8080/debug_pwa.html")
        print(f"📱 在PWA中输入ESP32-S3 IP: {ESP_IP}")
    else:
        print("⚠️  部分测试失败，请检查ESP32-S3连接。")

if __name__ == "__main__":
    main() 