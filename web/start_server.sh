#!/bin/bash

# ESP32 LED控制器 Web服务器启动脚本

echo "🚀 ESP32 LED控制器 Web服务器启动脚本"
echo "========================================"

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "📁 当前目录: $SCRIPT_DIR"

# 检查文件是否存在
if [ ! -f "index.html" ]; then
    echo "❌ 错误: 未找到 index.html 文件"
    echo "   请确保在正确的目录中运行此脚本"
    exit 1
fi

echo ""
echo "🔍 检测可用的Web服务器..."

# 优先级1: 尝试使用Python3
if command -v python3 &> /dev/null; then
    echo "✅ 找到 Python3，使用 Python Web服务器"
    echo ""
    echo "🌐 启动服务器..."
    python3 server.py
    exit 0
fi

# 优先级2: 尝试使用Python
if command -v python &> /dev/null; then
    echo "✅ 找到 Python，使用 Python Web服务器"
    echo ""
    echo "🌐 启动服务器..."
    python server.py
    exit 0
fi

# 优先级3: 尝试使用Node.js
if command -v node &> /dev/null; then
    echo "✅ 找到 Node.js，使用 Node.js Web服务器"
    echo ""
    echo "🌐 启动服务器..."
    node server.js
    exit 0
fi

# 优先级4: 尝试使用PHP内置服务器
if command -v php &> /dev/null; then
    echo "✅ 找到 PHP，使用 PHP 内置服务器"
    echo ""
    
    # 查找可用端口
    PORT=8080
    while lsof -i :$PORT &> /dev/null; do
        PORT=$((PORT + 1))
    done
    
    echo "🌐 启动服务器..."
    echo "📍 服务地址: http://localhost:$PORT"
    echo "🌐 访问界面: http://localhost:$PORT/index.html"
    echo ""
    echo "⏹️  按 Ctrl+C 停止服务器"
    echo "========================================"
    
    # 自动打开浏览器
    if command -v open &> /dev/null; then
        sleep 2 && open "http://localhost:$PORT/index.html" &
    elif command -v xdg-open &> /dev/null; then
        sleep 2 && xdg-open "http://localhost:$PORT/index.html" &
    fi
    
    php -S localhost:$PORT
    exit 0
fi

# 如果都没有找到，提供手动选项
echo ""
echo "❌ 未找到可用的Web服务器 (Python3/Python/Node.js/PHP)"
echo ""
echo "💡 解决方案:"
echo ""
echo "1️⃣  安装 Python3 (推荐):"
echo "   - macOS: brew install python3"
echo "   - Ubuntu/Debian: sudo apt install python3"
echo "   - Windows: 从 python.org 下载安装"
echo ""
echo "2️⃣  安装 Node.js:"
echo "   - 访问 nodejs.org 下载安装"
echo ""
echo "3️⃣  手动启动简单服务器:"
echo "   python3 -m http.server 8080"
echo "   # 或"
echo "   python -m http.server 8080"
echo "   # 或"
echo "   npx http-server -p 8080 --cors"
echo ""
echo "4️⃣  直接用浏览器打开 (可能有跨域限制):"
echo "   open index.html"
echo ""

exit 1