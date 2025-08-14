@echo off
REM ESP32 LED控制器 Web服务器启动脚本 (Windows)

echo 🚀 ESP32 LED控制器 Web服务器启动脚本
echo ========================================

REM 切换到脚本所在目录
cd /d "%~dp0"

echo 📁 当前目录: %CD%

REM 检查文件是否存在
if not exist "index.html" (
    echo ❌ 错误: 未找到 index.html 文件
    echo    请确保在正确的目录中运行此脚本
    pause
    exit /b 1
)

echo.
echo 🔍 检测可用的Web服务器...

REM 优先级1: 尝试使用Python3
where python3 >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo ✅ 找到 Python3，使用 Python Web服务器
    echo.
    echo 🌐 启动服务器...
    python3 server.py
    goto :eof
)

REM 优先级2: 尝试使用Python
where python >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo ✅ 找到 Python，使用 Python Web服务器
    echo.
    echo 🌐 启动服务器...
    python server.py
    goto :eof
)

REM 优先级3: 尝试使用Node.js
where node >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo ✅ 找到 Node.js，使用 Node.js Web服务器
    echo.
    echo 🌐 启动服务器...
    node server.js
    goto :eof
)

REM 优先级4: 尝试使用PHP内置服务器
where php >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo ✅ 找到 PHP，使用 PHP 内置服务器
    echo.
    echo 🌐 启动服务器...
    echo 📍 服务地址: http://localhost:8080
    echo 🌐 访问界面: http://localhost:8080/index.html
    echo.
    echo ⏹️  按 Ctrl+C 停止服务器
    echo ========================================
    
    REM 自动打开浏览器
    timeout /t 2 >nul
    start http://localhost:8080/index.html
    
    php -S localhost:8080
    goto :eof
)

REM 如果都没有找到，提供手动选项
echo.
echo ❌ 未找到可用的Web服务器 (Python3/Python/Node.js/PHP)
echo.
echo 💡 解决方案:
echo.
echo 1️⃣  安装 Python3 (推荐):
echo    - 访问 python.org 下载 Python 3.x
echo    - 安装时勾选 "Add Python to PATH"
echo.
echo 2️⃣  安装 Node.js:
echo    - 访问 nodejs.org 下载安装
echo.
echo 3️⃣  手动启动简单服务器:
echo    python3 -m http.server 8080
echo    # 或
echo    python -m http.server 8080
echo    # 或
echo    npx http-server -p 8080 --cors
echo.
echo 4️⃣  直接用浏览器打开 (可能有跨域限制):
echo    start index.html
echo.

pause
exit /b 1