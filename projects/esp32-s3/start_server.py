#!/usr/bin/env python3
"""
ESP32-S3 RGB LED PWA 本地服务器
用于在开发环境中提供PWA Web应用服务
"""

import http.server
import socketserver
import os
import sys
import webbrowser
from pathlib import Path

class CORSHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """添加CORS支持的HTTP请求处理器"""
    
    def end_headers(self):
        # 添加CORS头部
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()
    
    def do_OPTIONS(self):
        # 处理OPTIONS请求（CORS预检）
        self.send_response(200)
        self.end_headers()

def main():
    # 切换到web目录
    web_dir = Path(__file__).parent / 'web'
    if not web_dir.exists():
        print("❌ 错误: web目录不存在")
        print("请确保在正确的项目目录中运行此脚本")
        sys.exit(1)
    
    os.chdir(web_dir)
    
    # 设置服务器参数
    PORT = 8080
    HOST = 'localhost'
    
    try:
        with socketserver.TCPServer((HOST, PORT), CORSHTTPRequestHandler) as httpd:
            print("🌈 ESP32-S3 RGB LED PWA 控制器")
            print("=" * 50)
            print(f"🚀 服务器启动成功!")
            print(f"🌐 本地地址: http://{HOST}:{PORT}")
            print(f"📱 PWA应用已就绪")
            print("=" * 50)
            print("📖 使用说明:")
            print("1. 确保ESP32-S3已连接到WiFi")
            print("2. 在PWA应用中输入ESP32-S3的IP地址")
            print("3. 点击连接按钮建立连接")
            print("4. 开始控制RGB LED!")
            print()
            print("⌨️  按 Ctrl+C 停止服务器")
            print("=" * 50)
            
            # 自动打开浏览器
            try:
                webbrowser.open(f'http://{HOST}:{PORT}')
                print("🔗 已自动打开浏览器")
            except:
                print("⚠️  请手动在浏览器中打开上述地址")
            
            print()
            httpd.serve_forever()
            
    except OSError as e:
        if e.errno == 48:  # Address already in use
            print(f"❌ 错误: 端口 {PORT} 已被占用")
            print(f"请尝试使用其他端口或关闭占用该端口的程序")
        else:
            print(f"❌ 错误: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\n👋 服务器已停止")
        print("感谢使用 ESP32-S3 RGB LED PWA 控制器!")

if __name__ == '__main__':
    main() 