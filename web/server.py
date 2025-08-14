#!/usr/bin/env python3
"""
ESP32 LED控制器 Web服务器
支持本地Web界面托管和跨域请求
"""

import http.server
import socketserver
import os
import sys
import webbrowser
import threading
import time
from urllib.parse import urlparse, parse_qs
import json

class CORSHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """支持CORS的HTTP请求处理器"""
    
    def end_headers(self):
        """添加CORS头"""
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type, Authorization')
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate')
        super().end_headers()
    
    def do_OPTIONS(self):
        """处理预检请求"""
        self.send_response(200)
        self.end_headers()
    
    def do_GET(self):
        """处理GET请求"""
        # 如果请求根路径，重定向到index.html
        if self.path == '/':
            self.path = '/index.html'
        
        # 处理API代理请求（可选功能）
        if self.path.startswith('/proxy/'):
            self.handle_proxy_request()
            return
            
        super().do_GET()
    
    def handle_proxy_request(self):
        """处理代理请求到ESP32设备"""
        try:
            # 解析代理URL: /proxy/192.168.4.1/api/status
            parts = self.path.split('/')
            if len(parts) >= 4:
                esp_ip = parts[2]
                api_path = '/' + '/'.join(parts[3:])
                
                # 这里可以添加代理逻辑
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                
                response_data = {
                    "status": "proxy_not_implemented",
                    "message": f"代理请求到 {esp_ip}{api_path}",
                    "note": "直接连接ESP32设备即可，无需代理"
                }
                
                self.wfile.write(json.dumps(response_data, ensure_ascii=False).encode('utf-8'))
            else:
                self.send_error(400, "Invalid proxy URL")
        except Exception as e:
            self.send_error(500, f"Proxy error: {str(e)}")
    
    def log_message(self, format, *args):
        """自定义日志格式"""
        timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
        print(f"[{timestamp}] {format % args}")

def find_free_port(start_port=8080, max_attempts=10):
    """查找可用端口"""
    import socket
    
    for port in range(start_port, start_port + max_attempts):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.bind(('', port))
                return port
        except OSError:
            continue
    
    raise RuntimeError(f"无法找到可用端口 (尝试了 {start_port}-{start_port + max_attempts - 1})")

def open_browser(url, delay=2):
    """延迟打开浏览器"""
    time.sleep(delay)
    print(f"\n🌐 正在打开浏览器: {url}")
    try:
        webbrowser.open(url)
    except Exception as e:
        print(f"❌ 无法自动打开浏览器: {e}")
        print(f"   请手动访问: {url}")

def main():
    """主函数"""
    # 切换到web目录
    web_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(web_dir)
    
    print("🚀 ESP32 LED控制器 Web服务器")
    print("=" * 50)
    
    try:
        # 查找可用端口
        port = find_free_port(8080)
        
        # 创建服务器
        with socketserver.TCPServer(("", port), CORSHTTPRequestHandler) as httpd:
            server_url = f"http://localhost:{port}"
            
            print(f"✅ Web服务器已启动")
            print(f"📍 服务地址: {server_url}")
            print(f"📁 服务目录: {web_dir}")
            print(f"🌐 访问界面: {server_url}/index.html")
            print()
            print("💡 使用说明:")
            print("   1. 确保ESP32设备已连接并运行")
            print("   2. 在界面中选择设备类型 (ESP32-C3/ESP32-S3)")
            print("   3. 输入ESP32设备的IP地址")
            print("   4. 点击'连接设备'开始控制LED")
            print()
            print("🔧 ESP32设备常见IP地址:")
            print("   - AP模式: 192.168.4.1")
            print("   - STA模式: 查看串口输出获取实际IP")
            print()
            print("⏹️  按 Ctrl+C 停止服务器")
            print("=" * 50)
            
            # 启动浏览器（延迟2秒）
            browser_thread = threading.Thread(
                target=open_browser, 
                args=(f"{server_url}/index.html",)
            )
            browser_thread.daemon = True
            browser_thread.start()
            
            # 启动服务器
            httpd.serve_forever()
            
    except KeyboardInterrupt:
        print("\n\n⏹️  服务器已停止")
        sys.exit(0)
    except Exception as e:
        print(f"\n❌ 服务器启动失败: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()