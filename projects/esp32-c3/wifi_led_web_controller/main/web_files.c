/*
 * Web Files - Embedded HTML, CSS, and JavaScript
 * 适配PC和H5设备的响应式设计
 */

#include <stddef.h>
#include <string.h>
#include "web_files.h"

// 嵌入的HTML文件内容 - 适配PC和H5
const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <meta name="theme-color" content="#007bff">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-status-bar-style" content="default">
    <meta name="apple-mobile-web-app-title" content="LED控制器">
    <title>ESP32-C3 LED控制器</title>
    <link rel="icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='.9em' font-size='90'>🌈</text></svg>">
    <link rel="manifest" href="/manifest.json">
    <style id="embedded-style"></style>
</head>
<body>
    <div class="app">
        <header class="header">
            <h1>🌈 ESP32-C3 LED控制器</h1>
            <div class="status-indicator" id="status-indicator">
                <span class="status-dot"></span>
                <span class="status-text">已连接</span>
            </div>
        </header>

        <main class="main">
            <!-- 颜色预览区域 -->
            <section class="preview-section">
                <div class="color-preview" id="color-preview">
                    <div class="preview-circle" id="preview-circle"></div>
                    <div class="preview-info">
                        <div class="color-values">
                            <span>R: <span id="r-val">255</span></span>
                            <span>G: <span id="g-val">255</span></span>
                            <span>B: <span id="b-val">255</span></span>
                        </div>
                        <div class="brightness-value">
                            亮度: <span id="brightness-val">50%</span>
                        </div>
                    </div>
                </div>
            </section>

            <!-- 颜色控制区域 -->
            <section class="control-section">
                <h2>🎨 颜色控制</h2>
                <div class="color-controls">
                    <div class="color-slider">
                        <label for="r">
                            <span class="color-label red">红色</span>
                            <span class="color-value" id="r-display">255</span>
                        </label>
                        <input type="range" id="r" min="0" max="255" value="255" class="slider red-slider">
                    </div>
                    
                    <div class="color-slider">
                        <label for="g">
                            <span class="color-label green">绿色</span>
                            <span class="color-value" id="g-display">255</span>
                        </label>
                        <input type="range" id="g" min="0" max="255" value="255" class="slider green-slider">
                    </div>
                    
                    <div class="color-slider">
                        <label for="b">
                            <span class="color-label blue">蓝色</span>
                            <span class="color-value" id="b-display">255</span>
                        </label>
                        <input type="range" id="b" min="0" max="255" value="255" class="slider blue-slider">
                    </div>
                    
                    <div class="color-slider">
                        <label for="brightness">
                            <span class="color-label">亮度</span>
                            <span class="color-value" id="brightness-display">50%</span>
                        </label>
                        <input type="range" id="brightness" min="0" max="100" value="50" class="slider brightness-slider">
                    </div>
                </div>
            </section>

            <!-- 电源控制区域 -->
            <section class="control-section">
                <h2>⚡ 电源控制</h2>
                <div class="power-control">
                    <button class="btn btn-power" id="power-btn" onclick="togglePower()">
                        <span class="btn-icon">💡</span>
                        <span class="btn-text">开启LED</span>
                    </button>
                </div>
            </section>

            <!-- 特效控制区域 -->
            <section class="control-section">
                <h2>✨ 特效模式</h2>
                <div class="effect-controls">
                    <button class="btn btn-effect" onclick="setEffect('static')">
                        <span class="btn-icon">🎯</span>
                        <span class="btn-text">静态</span>
                    </button>
                    <button class="btn btn-effect" onclick="setEffect('rainbow')">
                        <span class="btn-icon">🌈</span>
                        <span class="btn-text">彩虹</span>
                    </button>
                    <button class="btn btn-effect" onclick="setEffect('breathing')">
                        <span class="btn-icon">🫁</span>
                        <span class="btn-text">呼吸</span>
                    </button>
                    <button class="btn btn-effect" onclick="setEffect('blink')">
                        <span class="btn-icon">⚡</span>
                        <span class="btn-text">闪烁</span>
                    </button>
                </div>
            </section>

            <!-- 预设颜色区域 -->
            <section class="control-section">
                <h2>🎨 预设颜色</h2>
                <div class="preset-colors">
                    <button class="preset-btn" style="background: #ff0000" onclick="setPresetColor(255, 0, 0)">红色</button>
                    <button class="preset-btn" style="background: #00ff00" onclick="setPresetColor(0, 255, 0)">绿色</button>
                    <button class="preset-btn" style="background: #0000ff" onclick="setPresetColor(0, 0, 255)">蓝色</button>
                    <button class="preset-btn" style="background: #ffff00" onclick="setPresetColor(255, 255, 0)">黄色</button>
                    <button class="preset-btn" style="background: #ff00ff" onclick="setPresetColor(255, 0, 255)">紫色</button>
                    <button class="preset-btn" style="background: #00ffff" onclick="setPresetColor(0, 255, 255)">青色</button>
                    <button class="preset-btn" style="background: #ff8000" onclick="setPresetColor(255, 128, 0)">橙色</button>
                    <button class="preset-btn" style="background: #ffffff" onclick="setPresetColor(255, 255, 255)">白色</button>
                </div>
            </section>
        </main>

        <footer class="footer">
            <p>ESP32-C3 LED控制器 v2.0</p>
        </footer>
    </div>

    <script>
// 全局变量
let powerState = false;
let currentColor = { r: 255, g: 255, b: 255 };
let currentBrightness = 50;
let isConnected = true;
let isTouchDevice = false;

// 初始化
document.addEventListener('DOMContentLoaded', function() {
    detectDeviceType();
    initializeApp();
    setupEventListeners();
    updateStatusIndicator();
});

// 检测设备类型
function detectDeviceType() {
    isTouchDevice = 'ontouchstart' in window || navigator.maxTouchPoints > 0;
    if (isTouchDevice) {
        document.body.classList.add('touch-device');
    }
}

// 初始化应用
function initializeApp() {
    updateColorPreview();
    updateAllDisplays();
    
    // 检查连接状态
    checkConnection();
    
    // 定期检查连接状态
    setInterval(checkConnection, 5000);
}

// 设置事件监听器
function setupEventListeners() {
    // 颜色滑块事件
    document.getElementById('r').addEventListener('input', updateColor);
    document.getElementById('g').addEventListener('input', updateColor);
    document.getElementById('b').addEventListener('input', updateColor);
    document.getElementById('brightness').addEventListener('input', updateColor);
    
    // 触摸设备优化
    if (isTouchDevice) {
        setupTouchOptimizations();
    }
    
    // 键盘快捷键支持
    setupKeyboardShortcuts();
}

// 设置触摸优化
function setupTouchOptimizations() {
    // 防止双击缩放
    let lastTouchEnd = 0;
    document.addEventListener('touchend', function(event) {
        const now = (new Date()).getTime();
        if (now - lastTouchEnd <= 300) {
            event.preventDefault();
        }
        lastTouchEnd = now;
    }, false);
    
    // 防止多指触摸缩放
    document.addEventListener('touchstart', function(event) {
        if (event.touches.length > 1) {
            event.preventDefault();
        }
    }, { passive: false });
}

// 设置键盘快捷键
function setupKeyboardShortcuts() {
    document.addEventListener('keydown', function(event) {
        // 空格键切换电源
        if (event.code === 'Space' && event.target.tagName !== 'INPUT') {
            event.preventDefault();
            togglePower();
        }
        
        // 数字键设置预设颜色
        const presetColors = [
            [255, 0, 0],    // 1 - 红色
            [0, 255, 0],    // 2 - 绿色
            [0, 0, 255],    // 3 - 蓝色
            [255, 255, 0],  // 4 - 黄色
            [255, 0, 255],  // 5 - 紫色
            [0, 255, 255],  // 6 - 青色
            [255, 128, 0],  // 7 - 橙色
            [255, 255, 255] // 8 - 白色
        ];
        
        const keyIndex = parseInt(event.key) - 1;
        if (keyIndex >= 0 && keyIndex < presetColors.length) {
            event.preventDefault();
            setPresetColor(...presetColors[keyIndex]);
        }
    });
}

// 更新颜色
function updateColor() {
    const r = parseInt(document.getElementById('r').value);
    const g = parseInt(document.getElementById('g').value);
    const b = parseInt(document.getElementById('b').value);
    const brightness = parseInt(document.getElementById('brightness').value);
    
    currentColor = { r, g, b };
    currentBrightness = brightness;
    
    updateColorPreview();
    updateAllDisplays();
    
    // 发送到ESP32
    sendColorToDevice(r, g, b, brightness);
}

// 更新颜色预览
function updateColorPreview() {
    const previewCircle = document.getElementById('preview-circle');
    const { r, g, b } = currentColor;
    
    // 应用亮度
    const brightnessMultiplier = currentBrightness / 100;
    const adjustedR = Math.round(r * brightnessMultiplier);
    const adjustedG = Math.round(g * brightnessMultiplier);
    const adjustedB = Math.round(b * brightnessMultiplier);
    
    previewCircle.style.background = `rgb(${adjustedR}, ${adjustedG}, ${adjustedB})`;
    
    // 添加发光效果
    if (currentBrightness > 50) {
        previewCircle.style.boxShadow = `0 0 20px rgba(${adjustedR}, ${adjustedG}, ${adjustedB}, 0.6)`;
    } else {
        previewCircle.style.boxShadow = '0 8px 24px rgba(0, 0, 0, 0.15)';
    }
}

// 更新所有显示
function updateAllDisplays() {
    // 更新滑块旁边的数值显示
    document.getElementById('r-display').textContent = currentColor.r;
    document.getElementById('g-display').textContent = currentColor.g;
    document.getElementById('b-display').textContent = currentColor.b;
    document.getElementById('brightness-display').textContent = currentBrightness + '%';
    
    // 更新预览区域的数值显示
    document.getElementById('r-val').textContent = currentColor.r;
    document.getElementById('g-val').textContent = currentColor.g;
    document.getElementById('b-val').textContent = currentColor.b;
    document.getElementById('brightness-val').textContent = currentBrightness + '%';
}

// 发送颜色到设备
function sendColorToDevice(r, g, b, brightness) {
    if (!isConnected) return;
    
    fetch('/api/led/color', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ 
            r: r, 
            g: g, 
            b: b, 
            brightness: brightness 
        })
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        return response.json();
    })
    .then(data => {
        console.log('Color updated successfully:', data);
    })
    .catch(error => {
        console.error('Error updating color:', error);
        showNotification('颜色更新失败', 'error');
    });
}

// 切换电源
function togglePower() {
    powerState = !powerState;
    
    const powerBtn = document.getElementById('power-btn');
    const btnIcon = powerBtn.querySelector('.btn-icon');
    const btnText = powerBtn.querySelector('.btn-text');
    
    if (powerState) {
        btnIcon.textContent = '💡';
        btnText.textContent = '关闭LED';
        powerBtn.classList.add('active');
    } else {
        btnIcon.textContent = '💡';
        btnText.textContent = '开启LED';
        powerBtn.classList.remove('active');
    }
    
    // 发送到ESP32
    sendPowerToDevice(powerState);
}

// 发送电源状态到设备
function sendPowerToDevice(power) {
    if (!isConnected) return;
    
    fetch('/api/led/power', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ power: power })
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        return response.json();
    })
    .then(data => {
        console.log('Power updated successfully:', data);
        showNotification(power ? 'LED已开启' : 'LED已关闭', 'success');
    })
    .catch(error => {
        console.error('Error updating power:', error);
        showNotification('电源控制失败', 'error');
        // 恢复状态
        powerState = !powerState;
        togglePower();
    });
}

// 设置特效
function setEffect(effect) {
    if (!isConnected) return;
    
    // 更新按钮状态
    document.querySelectorAll('.btn-effect').forEach(btn => {
        btn.classList.remove('active');
    });
    event.target.closest('.btn-effect').classList.add('active');
    
    fetch('/api/led/effect', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ effect: effect })
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        return response.json();
    })
    .then(data => {
        console.log('Effect updated successfully:', data);
        showNotification(`特效已切换为: ${getEffectName(effect)}`, 'success');
    })
    .catch(error => {
        console.error('Error updating effect:', error);
        showNotification('特效切换失败', 'error');
    });
}

// 获取特效名称
function getEffectName(effect) {
    const effectNames = {
        'static': '静态',
        'rainbow': '彩虹',
        'breathing': '呼吸',
        'blink': '闪烁'
    };
    return effectNames[effect] || effect;
}

// 设置预设颜色
function setPresetColor(r, g, b) {
    // 更新滑块值
    document.getElementById('r').value = r;
    document.getElementById('g').value = g;
    document.getElementById('b').value = b;
    
    // 更新颜色
    updateColor();
    
    // 显示通知
    showNotification('预设颜色已应用', 'success');
}

// 检查连接状态
function checkConnection() {
    fetch('/api/status')
    .then(response => {
        if (response.ok) {
            isConnected = true;
            updateStatusIndicator();
        } else {
            throw new Error('Status check failed');
        }
    })
    .catch(error => {
        console.error('Connection check failed:', error);
        isConnected = false;
        updateStatusIndicator();
    });
}

// 更新状态指示器
function updateStatusIndicator() {
    const statusDot = document.querySelector('.status-dot');
    const statusText = document.querySelector('.status-text');
    
    if (isConnected) {
        statusDot.style.background = '#28a745';
        statusText.textContent = '已连接';
        statusDot.style.animation = 'pulse 2s infinite';
    } else {
        statusDot.style.background = '#dc3545';
        statusText.textContent = '连接断开';
        statusDot.style.animation = 'none';
    }
}

// 显示通知
function showNotification(message, type = 'info') {
    // 创建通知元素
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.textContent = message;
    
    // 设置背景色
    const colors = {
        success: '#28a745',
        error: '#dc3545',
        info: '#007bff',
        warning: '#ffc107'
    };
    notification.style.background = colors[type] || colors.info;
    
    // 添加到页面
    document.body.appendChild(notification);
    
    // 显示动画
    setTimeout(() => {
        notification.style.transform = 'translateX(0)';
    }, 100);
    
    // 自动隐藏
    setTimeout(() => {
        notification.style.transform = 'translateX(100%)';
        setTimeout(() => {
            if (document.body.contains(notification)) {
                document.body.removeChild(notification);
            }
        }, 300);
    }, 3000);
}

// 网络状态监听
window.addEventListener('online', function() {
    console.log('Network is online');
    checkConnection();
});

window.addEventListener('offline', function() {
    console.log('Network is offline');
    isConnected = false;
    updateStatusIndicator();
});

// 页面可见性API
document.addEventListener('visibilitychange', function() {
    if (!document.hidden) {
        checkConnection();
    }
});

// 错误处理
window.addEventListener('error', function(event) {
    console.error('JavaScript error:', event.error);
});

window.addEventListener('unhandledrejection', function(event) {
    console.error('Unhandled promise rejection:', event.reason);
});
    </script>
</body>
</html>
)rawliteral";

// 嵌入的CSS文件内容 - 响应式设计，适配PC和H5
const char style_css[] = R"rawliteral(
/* 全局样式 */
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 50%, #f093fb 100%);
    min-height: 100vh;
    color: #333;
    line-height: 1.6;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    overflow-x: hidden;
    position: relative;
}

body::before {
    content: '';
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: 
        radial-gradient(circle at 20% 80%, rgba(120, 119, 198, 0.3) 0%, transparent 50%),
        radial-gradient(circle at 80% 20%, rgba(255, 119, 198, 0.3) 0%, transparent 50%),
        radial-gradient(circle at 40% 40%, rgba(120, 219, 255, 0.2) 0%, transparent 50%);
    pointer-events: none;
    z-index: -1;
}

/* 滚动条美化 */
::-webkit-scrollbar {
    width: 8px;
}

::-webkit-scrollbar-track {
    background: rgba(255, 255, 255, 0.1);
    border-radius: 4px;
}

::-webkit-scrollbar-thumb {
    background: rgba(255, 255, 255, 0.3);
    border-radius: 4px;
}

::-webkit-scrollbar-thumb:hover {
    background: rgba(255, 255, 255, 0.5);
}

.app {
    max-width: 800px;
    margin: 0 auto;
    padding: 20px;
    min-height: 100vh;
    display: flex;
    flex-direction: column;
}

/* 头部样式 */
.header {
    background: rgba(255, 255, 255, 0.95);
    backdrop-filter: blur(20px);
    -webkit-backdrop-filter: blur(20px);
    border-radius: 30px;
    padding: 30px;
    margin-bottom: 30px;
    box-shadow: 
        0 20px 40px rgba(0, 0, 0, 0.1), 
        0 8px 20px rgba(0, 0, 0, 0.07),
        0 0 0 1px rgba(255, 255, 255, 0.2);
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    border: 1px solid rgba(255, 255, 255, 0.3);
    position: relative;
    overflow: hidden;
    transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
}

.header::before {
    content: '';
    position: absolute;
    top: 0;
    left: -100%;
    width: 100%;
    height: 100%;
    background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.1), transparent);
    transition: left 0.5s;
}

.header:hover::before {
    left: 100%;
}

.header h1 {
    font-size: 2.2rem;
    font-weight: 900;
    background: linear-gradient(135deg, #667eea, #764ba2, #f093fb);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
    margin: 0;
    text-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
    letter-spacing: -0.5px;
    position: relative;
}

.status-indicator {
    display: flex;
    align-items: center;
    gap: 15px;
    font-size: 1rem;
    color: #555;
    background: rgba(255, 255, 255, 0.15);
    padding: 15px 25px;
    border-radius: 25px;
    backdrop-filter: blur(15px);
    -webkit-backdrop-filter: blur(15px);
    border: 1px solid rgba(255, 255, 255, 0.25);
    transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
    box-shadow: 0 4px 15px rgba(0, 0, 0, 0.1);
}

.status-indicator:hover {
    background: rgba(255, 255, 255, 0.15);
    transform: translateY(-2px);
    box-shadow: 0 8px 25px rgba(0, 0, 0, 0.1);
}

.status-dot {
    width: 12px;
    height: 12px;
    background: #28a745;
    border-radius: 50%;
    animation: pulse 2s infinite;
    box-shadow: 0 0 10px rgba(40, 167, 69, 0.5);
    position: relative;
}

.status-dot::after {
    content: '';
    position: absolute;
    top: -2px;
    left: -2px;
    right: -2px;
    bottom: -2px;
    border-radius: 50%;
    background: rgba(40, 167, 69, 0.3);
    animation: ripple 2s infinite;
}

@keyframes pulse {
    0% { opacity: 1; transform: scale(1); }
    50% { opacity: 0.7; transform: scale(1.1); }
    100% { opacity: 1; transform: scale(1); }
}

@keyframes ripple {
    0% { transform: scale(1); opacity: 1; }
    100% { transform: scale(2); opacity: 0; }
}

/* 主内容区域 */
.main {
    flex: 1;
    display: flex;
    flex-direction: column;
    gap: 20px;
}

/* 控制区域通用样式 */
.control-section {
    background: rgba(255, 255, 255, 0.95);
    backdrop-filter: blur(20px);
    -webkit-backdrop-filter: blur(20px);
    border-radius: 30px;
    padding: 35px;
    box-shadow: 
        0 20px 40px rgba(0, 0, 0, 0.1), 
        0 8px 20px rgba(0, 0, 0, 0.07),
        0 0 0 1px rgba(255, 255, 255, 0.2);
    border: 1px solid rgba(255, 255, 255, 0.25);
    transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
    position: relative;
    overflow: hidden;
}

.control-section::before {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    height: 3px;
    background: linear-gradient(90deg, #667eea, #764ba2, #667eea);
    background-size: 200% 100%;
    animation: gradientMove 3s ease-in-out infinite;
}

@keyframes gradientMove {
    0% { background-position: 0% 50%; }
    50% { background-position: 100% 50%; }
    100% { background-position: 0% 50%; }
}

.control-section:hover {
    transform: translateY(-5px);
    box-shadow: 0 20px 40px rgba(0, 0, 0, 0.15), 0 10px 20px rgba(0, 0, 0, 0.1);
}

.control-section h2 {
    font-size: 1.5rem;
    font-weight: 700;
    margin-bottom: 25px;
    color: #333;
    display: flex;
    align-items: center;
    gap: 12px;
    position: relative;
}

.control-section h2::after {
    content: '';
    position: absolute;
    bottom: -8px;
    left: 0;
    width: 40px;
    height: 3px;
    background: linear-gradient(90deg, #667eea, #764ba2);
    border-radius: 2px;
}

/* 颜色预览区域 */
.preview-section {
    background: rgba(255, 255, 255, 0.95);
    backdrop-filter: blur(20px);
    -webkit-backdrop-filter: blur(20px);
    border-radius: 35px;
    padding: 45px;
    box-shadow: 
        0 25px 50px rgba(0, 0, 0, 0.1), 
        0 10px 25px rgba(0, 0, 0, 0.07),
        0 0 0 1px rgba(255, 255, 255, 0.2);
    text-align: center;
    border: 1px solid rgba(255, 255, 255, 0.25);
    position: relative;
    overflow: hidden;
}

.preview-section::before {
    content: '';
    position: absolute;
    top: -50%;
    left: -50%;
    width: 200%;
    height: 200%;
    background: radial-gradient(circle, rgba(102, 126, 234, 0.1) 0%, transparent 70%);
    animation: rotate 20s linear infinite;
}

@keyframes rotate {
    from { transform: rotate(0deg); }
    to { transform: rotate(360deg); }
}

.color-preview {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 25px;
    position: relative;
    z-index: 1;
}

.preview-circle {
    width: 160px;
    height: 160px;
    border-radius: 50%;
    background: rgb(255, 255, 255);
    border: 8px solid rgba(255, 255, 255, 0.4);
    box-shadow: 
        0 20px 40px rgba(0, 0, 0, 0.25), 
        inset 0 0 30px rgba(255, 255, 255, 0.15),
        0 0 0 1px rgba(255, 255, 255, 0.1);
    transition: all 0.5s cubic-bezier(0.4, 0, 0.2, 1);
    position: relative;
    cursor: pointer;
}

.preview-circle:hover {
    transform: scale(1.05);
    box-shadow: 0 20px 40px rgba(0, 0, 0, 0.3), inset 0 0 30px rgba(255, 255, 255, 0.2);
}

.preview-circle::before {
    content: '';
    position: absolute;
    top: -10px;
    left: -10px;
    right: -10px;
    bottom: -10px;
    border-radius: 50%;
    background: linear-gradient(45deg, #667eea, #764ba2, #667eea);
    background-size: 200% 200%;
    animation: gradientShift 3s ease-in-out infinite;
    z-index: -1;
    opacity: 0.3;
}

@keyframes gradientShift {
    0% { background-position: 0% 50%; }
    50% { background-position: 100% 50%; }
    100% { background-position: 0% 50%; }
}

.preview-info {
    display: flex;
    flex-direction: column;
    gap: 10px;
    font-size: 0.9rem;
    color: #666;
}

.color-values {
    display: flex;
    gap: 15px;
    justify-content: center;
    flex-wrap: wrap;
}

.color-values span {
    font-weight: 600;
    color: #333;
}

.brightness-value {
    font-weight: 600;
    color: #333;
}

/* 颜色控制区域 */
.color-controls {
    display: flex;
    flex-direction: column;
    gap: 20px;
}

.color-slider {
    display: flex;
    flex-direction: column;
    gap: 8px;
}

.color-slider label {
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-weight: 500;
    color: #333;
}

.color-label {
    display: flex;
    align-items: center;
    gap: 8px;
}

.color-label.red::before {
    content: "🔴";
}

.color-label.green::before {
    content: "🟢";
}

.color-label.blue::before {
    content: "🔵";
}

.color-value {
    font-weight: 600;
    color: #007bff;
    min-width: 40px;
    text-align: right;
}

/* 滑块样式 */
.slider {
    width: 100%;
    height: 14px;
    border-radius: 7px;
    outline: none;
    -webkit-appearance: none;
    appearance: none;
    background: linear-gradient(90deg, #f8f9fa 0%, #e9ecef 50%, #dee2e6 100%);
    cursor: pointer;
    transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
    border: 2px solid rgba(255, 255, 255, 0.4);
    box-shadow: 
        inset 0 2px 4px rgba(0, 0, 0, 0.1),
        0 0 0 1px rgba(255, 255, 255, 0.2);
}

.slider:hover {
    background: linear-gradient(90deg, #dee2e6 0%, #ced4da 100%);
    box-shadow: inset 0 2px 4px rgba(0, 0, 0, 0.15);
}

.slider:focus {
    box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.3), inset 0 2px 4px rgba(0, 0, 0, 0.1);
}

.slider::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 28px;
    height: 28px;
    border-radius: 50%;
    background: linear-gradient(135deg, #667eea, #764ba2, #f093fb);
    cursor: pointer;
    box-shadow: 
        0 6px 20px rgba(102, 126, 234, 0.4), 
        0 4px 12px rgba(0, 0, 0, 0.2),
        0 0 0 2px rgba(255, 255, 255, 0.9);
    transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
    border: 2px solid rgba(255, 255, 255, 0.9);
}

.slider::-webkit-slider-thumb:hover {
    transform: scale(1.2);
    box-shadow: 0 6px 16px rgba(102, 126, 234, 0.5), 0 4px 12px rgba(0, 0, 0, 0.3);
}

.slider::-webkit-slider-thumb:active {
    transform: scale(1.1);
    box-shadow: 0 2px 8px rgba(102, 126, 234, 0.6), 0 1px 4px rgba(0, 0, 0, 0.4);
}

.slider::-moz-range-thumb {
    width: 20px;
    height: 20px;
    border-radius: 50%;
    background: #007bff;
    cursor: pointer;
    border: none;
    box-shadow: 0 2px 8px rgba(0, 123, 255, 0.3);
}

.slider:hover::-webkit-slider-thumb {
    transform: scale(1.1);
    box-shadow: 0 4px 12px rgba(0, 123, 255, 0.4);
}

.red-slider::-webkit-slider-thumb {
    background: #dc3545;
}

.green-slider::-webkit-slider-thumb {
    background: #28a745;
}

.blue-slider::-webkit-slider-thumb {
    background: #007bff;
}

.brightness-slider::-webkit-slider-thumb {
    background: #ffc107;
}

/* 按钮样式 */
.btn {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 12px;
    padding: 18px 32px;
    border: none;
    border-radius: 20px;
    font-size: 1.1rem;
    font-weight: 700;
    cursor: pointer;
    transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
    text-decoration: none;
    min-height: 56px;
    -webkit-tap-highlight-color: transparent;
    position: relative;
    overflow: hidden;
    letter-spacing: 0.5px;
    box-shadow: 0 4px 15px rgba(0, 0, 0, 0.1);
}

.btn::before {
    content: '';
    position: absolute;
    top: 0;
    left: -100%;
    width: 100%;
    height: 100%;
    background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.2), transparent);
    transition: left 0.5s;
}

.btn:hover::before {
    left: 100%;
}

.btn:hover {
    transform: translateY(-3px) scale(1.02);
    box-shadow: 0 15px 35px rgba(0, 0, 0, 0.2), 0 5px 15px rgba(0, 0, 0, 0.1);
}

.btn:active {
    transform: translateY(-1px) scale(0.98);
    transition: all 0.1s ease;
}

.btn-power {
    background: linear-gradient(135deg, #28a745, #20c997);
    color: white;
    width: 100%;
    font-size: 1.1rem;
}

.btn-power:hover {
    background: linear-gradient(135deg, #218838, #1ea085);
}

.btn-effect {
    background: linear-gradient(135deg, #007bff, #6610f2);
    color: white;
    flex: 1;
    min-width: 120px;
}

.btn-effect:hover {
    background: linear-gradient(135deg, #0056b3, #520dc2);
}

.btn-effect.active {
    background: linear-gradient(135deg, #0056b3, #520dc2);
    transform: scale(0.95);
}

.btn-icon {
    font-size: 1.2rem;
}

.btn-text {
    font-size: 0.9rem;
}

/* 特效控制区域 */
.effect-controls {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
    gap: 12px;
}

/* 预设颜色区域 */
.preset-colors {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(80px, 1fr));
    gap: 12px;
}

.preset-btn {
    padding: 15px 12px;
    border: none;
    border-radius: 16px;
    color: white;
    font-weight: 700;
    cursor: pointer;
    transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
    text-shadow: 0 2px 4px rgba(0, 0, 0, 0.4);
    min-height: 52px;
    display: flex;
    align-items: center;
    justify-content: center;
    -webkit-tap-highlight-color: transparent;
    position: relative;
    overflow: hidden;
    font-size: 0.9rem;
    letter-spacing: 0.5px;
}

.preset-btn::before {
    content: '';
    position: absolute;
    top: 0;
    left: -100%;
    width: 100%;
    height: 100%;
    background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.2), transparent);
    transition: left 0.5s;
}

.preset-btn:hover::before {
    left: 100%;
}

.preset-btn:hover {
    transform: translateY(-3px) scale(1.05);
    box-shadow: 0 12px 30px rgba(0, 0, 0, 0.4), 0 6px 20px rgba(0, 0, 0, 0.3);
}

.preset-btn:active {
    transform: translateY(-1px) scale(0.98);
    transition: all 0.1s ease;
}

/* 电源控制区域 */
.power-control {
    display: flex;
    justify-content: center;
}

/* 底部样式 */
.footer {
    text-align: center;
    padding: 20px;
    color: rgba(255, 255, 255, 0.8);
    font-size: 0.9rem;
}

/* 通知样式 */
.notification {
    position: fixed;
    top: 20px;
    right: 20px;
    padding: 12px 20px;
    border-radius: 8px;
    color: white;
    font-weight: 600;
    z-index: 1000;
    transform: translateX(100%);
    transition: transform 0.3s ease;
    max-width: 300px;
    word-wrap: break-word;
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
}

/* 响应式设计 - PC设备 */
@media (min-width: 769px) {
    .app {
        padding: 30px;
    }
    
    .header h1 {
        font-size: 2rem;
    }
    
    .preview-circle {
        width: 150px;
        height: 150px;
    }
    
    .btn {
        min-height: 52px;
    }
    
    .slider::-webkit-slider-thumb {
        width: 24px;
        height: 24px;
    }
}

/* 响应式设计 - 平板设备 */
@media (max-width: 768px) and (min-width: 481px) {
    .app {
        padding: 15px;
    }
    
    .header {
        padding: 15px;
        flex-direction: column;
        gap: 15px;
        text-align: center;
    }
    
    .header h1 {
        font-size: 1.5rem;
    }
    
    .control-section {
        padding: 20px;
    }
    
    .preview-section {
        padding: 25px;
    }
    
    .preview-circle {
        width: 100px;
        height: 100px;
    }
    
    .effect-controls {
        grid-template-columns: repeat(2, 1fr);
    }
    
    .preset-colors {
        grid-template-columns: repeat(4, 1fr);
    }
}

/* 响应式设计 - 手机设备 */
@media (max-width: 480px) {
    .app {
        padding: 12px;
    }
    
    .header {
        padding: 20px;
        border-radius: 25px;
    }
    
    .header h1 {
        font-size: 1.4rem;
    }
    
    .control-section {
        padding: 20px;
        border-radius: 25px;
    }
    
    .preview-section {
        padding: 25px;
        border-radius: 25px;
    }
    
    .preview-circle {
        width: 100px;
        height: 100px;
        border-width: 6px;
    }
    
    .effect-controls {
        grid-template-columns: 1fr;
        gap: 15px;
    }
    
    .preset-colors {
        grid-template-columns: repeat(3, 1fr);
        gap: 15px;
    }
    
    .color-values {
        flex-direction: column;
        gap: 8px;
    }
    
    .btn {
        padding: 12px 20px;
        font-size: 0.9rem;
        min-height: 48px;
        border-radius: 16px;
    }
    
    .btn-icon {
        font-size: 1.1rem;
    }
    
    .btn-text {
        font-size: 0.8rem;
    }
    
    .slider {
        height: 16px;
    }
    
    .slider::-webkit-slider-thumb {
        width: 24px;
        height: 24px;
    }
    
    .notification {
        right: 12px;
        left: 12px;
        max-width: none;
        border-radius: 12px;
    }
    
    .status-indicator {
        padding: 12px 20px;
        border-radius: 20px;
    }
}

/* 触摸设备优化 */
@media (hover: none) and (pointer: coarse) {
    .btn {
        min-height: 48px;
        padding: 16px 24px;
    }
    
    .slider::-webkit-slider-thumb {
        width: 28px;
        height: 28px;
    }
    
    .preset-btn {
        min-height: 48px;
        padding: 16px 20px;
    }
    
    /* 增加触摸目标间距 */
    .color-slider {
        gap: 16px;
    }
    
    .effect-controls {
        gap: 20px;
    }
    
    .preset-colors {
        gap: 20px;
    }
    
    /* 触摸反馈 */
    .btn:active,
    .preset-btn:active {
        transform: scale(0.95);
        transition: all 0.1s ease;
    }
    
    /* 触摸高亮 */
    .slider:active::-webkit-slider-thumb {
        transform: scale(1.1);
        box-shadow: 0 8px 25px rgba(102, 126, 234, 0.6);
    }
}

/* 深色模式支持 */
@media (prefers-color-scheme: dark) {
    body {
        background: linear-gradient(135deg, #1a1a2e 0%, #16213e 50%, #0f3460 100%);
    }
    
    body::before {
        background: 
            radial-gradient(circle at 20% 80%, rgba(120, 119, 198, 0.2) 0%, transparent 50%),
            radial-gradient(circle at 80% 20%, rgba(255, 119, 198, 0.2) 0%, transparent 50%),
            radial-gradient(circle at 40% 40%, rgba(120, 219, 255, 0.15) 0%, transparent 50%);
    }
    
    .control-section,
    .preview-section,
    .header {
        background: rgba(255, 255, 255, 0.08);
        color: #fff;
        border-color: rgba(255, 255, 255, 0.15);
    }
    
    .control-section h2,
    .header h1 {
        color: #fff;
    }
    
    .color-slider label {
        color: #fff;
    }
    
    .color-value {
        color: #4fc3f7;
    }
    
    .status-indicator {
        background: rgba(255, 255, 255, 0.08);
        border-color: rgba(255, 255, 255, 0.15);
        color: #fff;
    }
    
    .slider {
        background: linear-gradient(90deg, #2c3e50 0%, #34495e 50%, #2c3e50 100%);
        border-color: rgba(255, 255, 255, 0.2);
    }
    
    .slider::-webkit-slider-thumb {
        background: linear-gradient(135deg, #667eea, #764ba2, #f093fb);
        border-color: rgba(255, 255, 255, 0.8);
    }
}

/* 高对比度模式支持 */
@media (prefers-contrast: high) {
    .btn {
        border: 2px solid #000;
    }
    
    .slider {
        border: 1px solid #000;
    }
}

/* 减少动画模式支持 */
@media (prefers-reduced-motion: reduce) {
    * {
        animation-duration: 0.01ms !important;
        animation-iteration-count: 1 !important;
        transition-duration: 0.01ms !important;
    }
}

/* 加载动画 */
@keyframes fadeInUp {
    from {
        opacity: 0;
        transform: translateY(40px) scale(0.95);
    }
    to {
        opacity: 1;
        transform: translateY(0) scale(1);
    }
}

@keyframes fadeInLeft {
    from {
        opacity: 0;
        transform: translateX(-40px) scale(0.95);
    }
    to {
        opacity: 1;
        transform: translateX(0) scale(1);
    }
}

@keyframes fadeInRight {
    from {
        opacity: 0;
        transform: translateX(40px) scale(0.95);
    }
    to {
        opacity: 1;
        transform: translateX(0) scale(1);
    }
}

.control-section {
    animation: fadeInUp 0.8s cubic-bezier(0.4, 0, 0.2, 1);
}

.control-section:nth-child(2) { animation-delay: 0.15s; }
.control-section:nth-child(3) { animation-delay: 0.3s; }
.control-section:nth-child(4) { animation-delay: 0.45s; }
.control-section:nth-child(5) { animation-delay: 0.6s; }

.preview-section {
    animation: fadeInUp 0.8s cubic-bezier(0.4, 0, 0.2, 1);
    animation-delay: 0.1s;
}

.header {
    animation: fadeInUp 0.8s cubic-bezier(0.4, 0, 0.2, 1);
}

/* 浮动动画 */
@keyframes float {
    0%, 100% { 
        transform: translateY(0px) rotate(0deg); 
    }
    25% { 
        transform: translateY(-8px) rotate(1deg); 
    }
    50% { 
        transform: translateY(-12px) rotate(0deg); 
    }
    75% { 
        transform: translateY(-8px) rotate(-1deg); 
    }
}

.preview-circle {
    animation: float 8s ease-in-out infinite;
}

/* 新增：微妙的呼吸效果 */
@keyframes subtleBreath {
    0%, 100% { 
        transform: scale(1);
        opacity: 1;
    }
    50% { 
        transform: scale(1.02);
        opacity: 0.95;
    }
}

.preview-circle:hover {
    animation: subtleBreath 2s ease-in-out infinite;
}

/* 脉冲动画 */
@keyframes pulse-glow {
    0%, 100% { 
        box-shadow: 0 0 20px rgba(102, 126, 234, 0.3);
    }
    50% { 
        box-shadow: 0 0 30px rgba(102, 126, 234, 0.6);
    }
}

@keyframes pulse-ripple {
    0% { 
        transform: scale(1);
        opacity: 1;
    }
    100% { 
        transform: scale(2.5);
        opacity: 0;
    }
}

.status-dot {
    animation: pulse 2s infinite, pulse-glow 3s ease-in-out infinite;
}

.status-dot::before {
    content: '';
    position: absolute;
    top: -4px;
    left: -4px;
    right: -4px;
    bottom: -4px;
    border-radius: 50%;
    background: rgba(40, 167, 69, 0.2);
    animation: pulse-ripple 2s ease-out infinite;
}

/* 渐变文字动画 */
@keyframes textShine {
    0% { background-position: 0% 50%; }
    100% { background-position: 100% 50%; }
}

.header h1 {
    background-size: 200% 200%;
    animation: textShine 3s ease-in-out infinite;
}

/* 新增：悬浮卡片效果 */
@keyframes cardFloat {
    0%, 100% { 
        transform: translateY(0px) rotateX(0deg); 
    }
    50% { 
        transform: translateY(-8px) rotateX(2deg); 
    }
}

.control-section:hover {
    animation: cardFloat 3s ease-in-out infinite;
}

/* 新增：彩虹边框动画 */
@keyframes rainbowBorder {
    0% { border-image: linear-gradient(45deg, #ff0000, #ff8000, #ffff00, #80ff00, #00ff00, #00ff80, #00ffff, #0080ff, #8000ff, #ff0080) 1; }
    100% { border-image: linear-gradient(45deg, #ff0080, #ff0000, #ff8000, #ffff00, #80ff00, #00ff00, #00ff80, #00ffff, #0080ff, #8000ff) 1; }
}

.preview-section:hover {
    border-image: linear-gradient(45deg, #ff0000, #ff8000, #ffff00, #80ff00, #00ff00, #00ff80, #00ffff, #0080ff, #8000ff, #ff0080) 1;
    animation: rainbowBorder 2s linear infinite;
}

/* 新增：按钮脉冲效果 */
@keyframes buttonPulse {
    0% { transform: scale(1); }
    50% { transform: scale(1.05); }
    100% { transform: scale(1); }
}

.btn-power:hover {
    animation: buttonPulse 1s ease-in-out infinite;
}

/* 新增：状态指示器呼吸效果 */
@keyframes statusBreath {
    0%, 100% { 
        background: rgba(255, 255, 255, 0.15);
        transform: scale(1);
    }
    50% { 
        background: rgba(255, 255, 255, 0.2);
        transform: scale(1.02);
    }
}

.status-indicator:hover {
    animation: statusBreath 2s ease-in-out infinite;
}
)rawliteral";

// 嵌入的JavaScript文件内容 - 适配PC和H5
const char script_js[] = R"rawliteral(
// 全局变量
let powerState = false;
let currentColor = { r: 255, g: 255, b: 255 };
let currentBrightness = 50;
let isConnected = true;
let isTouchDevice = false;

// 初始化
document.addEventListener('DOMContentLoaded', function() {
    detectDeviceType();
    initializeApp();
    setupEventListeners();
    updateStatusIndicator();
});

// 检测设备类型
function detectDeviceType() {
    isTouchDevice = 'ontouchstart' in window || navigator.maxTouchPoints > 0;
    if (isTouchDevice) {
        document.body.classList.add('touch-device');
    }
}

// 注入CSS样式




// 初始化应用
function initializeApp() {
    updateColorPreview();
    updateAllDisplays();
    
    // 检查连接状态
    checkConnection();
    
    // 定期检查连接状态
    setInterval(checkConnection, 5000);
    
    // 添加PWA支持
    setupPWA();
}

// 设置PWA
function setupPWA() {
    if ('serviceWorker' in navigator) {
        navigator.serviceWorker.register('/sw.js')
            .then(function(registration) {
                console.log('SW registered: ', registration);
            })
            .catch(function(registrationError) {
                console.log('SW registration failed: ', registrationError);
            });
    }
}

// 设置事件监听器
function setupEventListeners() {
    // 颜色滑块事件
    document.getElementById('r').addEventListener('input', updateColor);
    document.getElementById('g').addEventListener('input', updateColor);
    document.getElementById('b').addEventListener('input', updateColor);
    document.getElementById('brightness').addEventListener('input', updateColor);
    
    // 触摸设备优化
    if (isTouchDevice) {
        setupTouchOptimizations();
    }
    
    // 键盘快捷键支持
    setupKeyboardShortcuts();
}

// 设置触摸优化
function setupTouchOptimizations() {
    // 防止双击缩放
    let lastTouchEnd = 0;
    document.addEventListener('touchend', function(event) {
        const now = (new Date()).getTime();
        if (now - lastTouchEnd <= 300) {
            event.preventDefault();
        }
        lastTouchEnd = now;
    }, false);
    
    // 防止多指触摸缩放
    document.addEventListener('touchstart', function(event) {
        if (event.touches.length > 1) {
            event.preventDefault();
        }
    }, { passive: false });
}

// 设置键盘快捷键
function setupKeyboardShortcuts() {
    document.addEventListener('keydown', function(event) {
        // 空格键切换电源
        if (event.code === 'Space' && event.target.tagName !== 'INPUT') {
            event.preventDefault();
            togglePower();
        }
        
        // 数字键设置预设颜色
        const presetColors = [
            [255, 0, 0],    // 1 - 红色
            [0, 255, 0],    // 2 - 绿色
            [0, 0, 255],    // 3 - 蓝色
            [255, 255, 0],  // 4 - 黄色
            [255, 0, 255],  // 5 - 紫色
            [0, 255, 255],  // 6 - 青色
            [255, 128, 0],  // 7 - 橙色
            [255, 255, 255] // 8 - 白色
        ];
        
        const keyIndex = parseInt(event.key) - 1;
        if (keyIndex >= 0 && keyIndex < presetColors.length) {
            event.preventDefault();
            setPresetColor(...presetColors[keyIndex]);
        }
    });
}

// 更新颜色
function updateColor() {
    const r = parseInt(document.getElementById('r').value);
    const g = parseInt(document.getElementById('g').value);
    const b = parseInt(document.getElementById('b').value);
    const brightness = parseInt(document.getElementById('brightness').value);
    
    currentColor = { r, g, b };
    currentBrightness = brightness;
    
    updateColorPreview();
    updateAllDisplays();
    
    // 发送到ESP32
    sendColorToDevice(r, g, b, brightness);
}

// 更新颜色预览
function updateColorPreview() {
    const previewCircle = document.getElementById('preview-circle');
    const { r, g, b } = currentColor;
    
    // 应用亮度
    const brightnessMultiplier = currentBrightness / 100;
    const adjustedR = Math.round(r * brightnessMultiplier);
    const adjustedG = Math.round(g * brightnessMultiplier);
    const adjustedB = Math.round(b * brightnessMultiplier);
    
    previewCircle.style.background = `rgb(${adjustedR}, ${adjustedG}, ${adjustedB})`;
    
    // 添加发光效果
    if (currentBrightness > 50) {
        previewCircle.style.boxShadow = `0 0 20px rgba(${adjustedR}, ${adjustedG}, ${adjustedB}, 0.6)`;
    } else {
        previewCircle.style.boxShadow = '0 8px 24px rgba(0, 0, 0, 0.15)';
    }
}

// 更新所有显示
function updateAllDisplays() {
    // 更新滑块旁边的数值显示
    document.getElementById('r-display').textContent = currentColor.r;
    document.getElementById('g-display').textContent = currentColor.g;
    document.getElementById('b-display').textContent = currentColor.b;
    document.getElementById('brightness-display').textContent = currentBrightness + '%';
    
    // 更新预览区域的数值显示
    document.getElementById('r-val').textContent = currentColor.r;
    document.getElementById('g-val').textContent = currentColor.g;
    document.getElementById('b-val').textContent = currentColor.b;
    document.getElementById('brightness-val').textContent = currentBrightness + '%';
}

// 发送颜色到设备
function sendColorToDevice(r, g, b, brightness) {
    if (!isConnected) return;
    
    fetch('/api/led/color', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ 
            r: r, 
            g: g, 
            b: b, 
            brightness: brightness 
        })
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        return response.json();
    })
    .then(data => {
        console.log('Color updated successfully:', data);
    })
    .catch(error => {
        console.error('Error updating color:', error);
        showNotification('颜色更新失败', 'error');
    });
}

// 切换电源
function togglePower() {
    powerState = !powerState;
    
    const powerBtn = document.getElementById('power-btn');
    const btnIcon = powerBtn.querySelector('.btn-icon');
    const btnText = powerBtn.querySelector('.btn-text');
    
    if (powerState) {
        btnIcon.textContent = '💡';
        btnText.textContent = '关闭LED';
        powerBtn.classList.add('active');
    } else {
        btnIcon.textContent = '💡';
        btnText.textContent = '开启LED';
        powerBtn.classList.remove('active');
    }
    
    // 发送到ESP32
    sendPowerToDevice(powerState);
}

// 发送电源状态到设备
function sendPowerToDevice(power) {
    if (!isConnected) return;
    
    fetch('/api/led/power', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ power: power })
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        return response.json();
    })
    .then(data => {
        console.log('Power updated successfully:', data);
        showNotification(power ? 'LED已开启' : 'LED已关闭', 'success');
    })
    .catch(error => {
        console.error('Error updating power:', error);
        showNotification('电源控制失败', 'error');
        // 恢复状态
        powerState = !powerState;
        togglePower();
    });
}

// 设置特效
function setEffect(effect) {
    if (!isConnected) return;
    
    // 更新按钮状态
    document.querySelectorAll('.btn-effect').forEach(btn => {
        btn.classList.remove('active');
    });
    event.target.closest('.btn-effect').classList.add('active');
    
    fetch('/api/led/effect', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ effect: effect })
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        return response.json();
    })
    .then(data => {
        console.log('Effect updated successfully:', data);
        showNotification(`特效已切换为: ${getEffectName(effect)}`, 'success');
    })
    .catch(error => {
        console.error('Error updating effect:', error);
        showNotification('特效切换失败', 'error');
    });
}

// 获取特效名称
function getEffectName(effect) {
    const effectNames = {
        'static': '静态',
        'rainbow': '彩虹',
        'breathing': '呼吸',
        'blink': '闪烁'
    };
    return effectNames[effect] || effect;
}

// 设置预设颜色
function setPresetColor(r, g, b) {
    // 更新滑块值
    document.getElementById('r').value = r;
    document.getElementById('g').value = g;
    document.getElementById('b').value = b;
    
    // 更新颜色
    updateColor();
    
    // 显示通知
    showNotification('预设颜色已应用', 'success');
}

// 检查连接状态
function checkConnection() {
    fetch('/api/status')
    .then(response => {
        if (response.ok) {
            isConnected = true;
            updateStatusIndicator();
        } else {
            throw new Error('Status check failed');
        }
    })
    .catch(error => {
        console.error('Connection check failed:', error);
        isConnected = false;
        updateStatusIndicator();
    });
}

// 更新状态指示器
function updateStatusIndicator() {
    const statusDot = document.querySelector('.status-dot');
    const statusText = document.querySelector('.status-text');
    
    if (isConnected) {
        statusDot.style.background = '#28a745';
        statusText.textContent = '已连接';
        statusDot.style.animation = 'pulse 2s infinite';
    } else {
        statusDot.style.background = '#dc3545';
        statusText.textContent = '连接断开';
        statusDot.style.animation = 'none';
    }
}

// 显示通知
function showNotification(message, type = 'info') {
    // 创建通知元素
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.textContent = message;
    
    // 设置背景色
    const colors = {
        success: '#28a745',
        error: '#dc3545',
        info: '#007bff',
        warning: '#ffc107'
    };
    notification.style.background = colors[type] || colors.info;
    
    // 添加到页面
    document.body.appendChild(notification);
    
    // 显示动画
    setTimeout(() => {
        notification.style.transform = 'translateX(0)';
    }, 100);
    
    // 自动隐藏
    setTimeout(() => {
        notification.style.transform = 'translateX(100%)';
        setTimeout(() => {
            if (document.body.contains(notification)) {
                document.body.removeChild(notification);
            }
        }, 300);
    }, 3000);
}

// 网络状态监听
window.addEventListener('online', function() {
    console.log('Network is online');
    checkConnection();
});

window.addEventListener('offline', function() {
    console.log('Network is offline');
    isConnected = false;
    updateStatusIndicator();
});

// 页面可见性API
document.addEventListener('visibilitychange', function() {
    if (!document.hidden) {
        checkConnection();
    }
});

// 错误处理
window.addEventListener('error', function(event) {
    console.error('JavaScript error:', event.error);
});

window.addEventListener('unhandledrejection', function(event) {
    console.error('Unhandled promise rejection:', event.reason);
});
)rawliteral";

// 获取文件大小的函数
size_t get_index_html_size(void) {
    return strlen(index_html);
}

size_t get_style_css_size(void) {
    return strlen(style_css);
}

size_t get_script_js_size(void) {
    return strlen(script_js);
}

// PWA Manifest文件内容
const char manifest_json[] = R"rawliteral({
    "name": "ESP32-C3 LED控制器",
    "short_name": "LED控制器",
    "description": "ESP32-C3 WiFi LED Web控制器",
    "start_url": "/",
    "display": "standalone",
    "background_color": "#667eea",
    "theme_color": "#007bff",
    "orientation": "portrait-primary",
    "icons": [
        {
            "src": "data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='.9em' font-size='90'>🌈</text></svg>",
            "sizes": "any",
            "type": "image/svg+xml",
            "purpose": "any maskable"
        }
    ],
    "categories": ["utilities", "productivity"],
    "lang": "zh-CN",
    "dir": "ltr"
})rawliteral";

// Service Worker文件内容
const char sw_js[] = R"rawliteral(
// 简单的Service Worker
const CACHE_NAME = 'led-controller-v1';
const urlsToCache = [
    '/',
    '/style.css',
    '/script.js'
];

self.addEventListener('install', function(event) {
    event.waitUntil(
        caches.open(CACHE_NAME)
            .then(function(cache) {
                return cache.addAll(urlsToCache);
            })
    );
});

self.addEventListener('fetch', function(event) {
    event.respondWith(
        caches.match(event.request)
            .then(function(response) {
                if (response) {
                    return response;
                }
                return fetch(event.request);
            })
    );
});
)rawliteral";

// 获取PWA文件大小的函数
size_t get_manifest_json_size(void) {
    return strlen(manifest_json);
}

size_t get_sw_js_size(void) {
    return strlen(sw_js);
}
