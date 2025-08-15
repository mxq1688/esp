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
    <style id="embedded-style">
/* 现代化LED控制器样式 - 重新设计 */
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700;800;900&display=swap');

/* CSS变量定义 */
:root {
    --primary-color: #6366f1;
    --primary-dark: #4f46e5;
    --primary-light: #a5b4fc;
    --secondary-color: #ec4899;
    --accent-color: #06b6d4;
    --success-color: #10b981;
    --warning-color: #f59e0b;
    --error-color: #ef4444;
    
    --bg-primary: #0f172a;
    --bg-secondary: #1e293b;
    --bg-tertiary: #334155;
    --bg-card: rgba(30, 41, 59, 0.8);
    --bg-glass: rgba(255, 255, 255, 0.1);
    
    --text-primary: #f8fafc;
    --text-secondary: #cbd5e1;
    --text-muted: #94a3b8;
    
    --border-color: rgba(148, 163, 184, 0.2);
    --shadow-light: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
    --shadow-medium: 0 10px 15px -3px rgba(0, 0, 0, 0.1);
    --shadow-heavy: 0 25px 50px -12px rgba(0, 0, 0, 0.25);
    
    --gradient-primary: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    --gradient-secondary: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
    --gradient-accent: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
    --gradient-success: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%);
    
    --border-radius: 16px;
    --border-radius-lg: 24px;
    --border-radius-xl: 32px;
    
    --transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    --transition-fast: all 0.15s cubic-bezier(0.4, 0, 0.2, 1);
}

/* 全局样式重置 */
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

*::before,
*::after {
    box-sizing: border-box;
}

/* 基础样式 */
html {
    font-size: 16px;
    scroll-behavior: smooth;
}

body {
    font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
    background: var(--bg-primary);
    color: var(--text-primary);
    line-height: 1.6;
    font-weight: 400;
    overflow-x: hidden;
    position: relative;
    min-height: 100vh;
}

/* 背景装饰 */
body::before {
    content: '';
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: 
        radial-gradient(circle at 20% 80%, rgba(99, 102, 241, 0.15) 0%, transparent 50%),
        radial-gradient(circle at 80% 20%, rgba(236, 72, 153, 0.15) 0%, transparent 50%),
        radial-gradient(circle at 40% 40%, rgba(6, 182, 212, 0.1) 0%, transparent 50%);
    pointer-events: none;
    z-index: -1;
}

/* 滚动条美化 */
::-webkit-scrollbar {
    width: 8px;
}

::-webkit-scrollbar-track {
    background: var(--bg-secondary);
    border-radius: 4px;
}

::-webkit-scrollbar-thumb {
    background: var(--bg-tertiary);
    border-radius: 4px;
}

::-webkit-scrollbar-thumb:hover {
    background: var(--primary-color);
}

/* 应用容器 */
.app {
    min-height: 100vh;
    display: flex;
    flex-direction: column;
    padding: 1rem;
    gap: 1.5rem;
}

/* 头部样式 */
.header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 1rem 0;
    border-bottom: 1px solid var(--border-color);
}

.header h1 {
    font-size: 1.5rem;
    font-weight: 700;
    background: var(--gradient-primary);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
}

/* 状态指示器 */
.status-indicator {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    padding: 0.5rem 1rem;
    background: var(--bg-card);
    border-radius: var(--border-radius);
    border: 1px solid var(--border-color);
    backdrop-filter: blur(10px);
}

.status-dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: var(--success-color);
    animation: pulse 2s ease-in-out infinite;
}

.status-dot.disconnected {
    background: var(--error-color);
}

@keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.5; }
}

.status-text {
    font-size: 0.875rem;
    font-weight: 500;
    color: var(--text-secondary);
}

/* 主内容区域 */
.main {
    flex: 1;
    display: flex;
    gap: 2rem;
    max-width: 1200px;
    margin: 0 auto;
    width: 100%;
}

/* 控制面板 */
.control-panel {
    flex: 1;
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
}

/* 卡片样式 */
.card {
    background: var(--bg-card);
    border-radius: var(--border-radius-lg);
    padding: 1.5rem;
    border: 1px solid var(--border-color);
    backdrop-filter: blur(10px);
    box-shadow: var(--shadow-medium);
    transition: var(--transition);
}

.card:hover {
    transform: translateY(-2px);
    box-shadow: var(--shadow-heavy);
}

.card h2 {
    font-size: 1.25rem;
    font-weight: 600;
    margin-bottom: 1rem;
    color: var(--text-primary);
    display: flex;
    align-items: center;
    gap: 0.5rem;
}

/* 颜色控制 */
.color-controls {
    display: flex;
    flex-direction: column;
    gap: 1rem;
}

.color-slider {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
}

.color-label {
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-size: 0.875rem;
    font-weight: 500;
    color: var(--text-secondary);
}

.color-value-display {
    font-family: 'Courier New', monospace;
    font-weight: 600;
    color: var(--text-primary);
    background: var(--bg-tertiary);
    padding: 0.25rem 0.5rem;
    border-radius: 4px;
    min-width: 3rem;
    text-align: center;
}

/* 滑块样式 */
.slider {
    -webkit-appearance: none;
    appearance: none;
    width: 100%;
    height: 8px;
    border-radius: 4px;
    background: var(--bg-tertiary);
    outline: none;
    cursor: pointer;
    transition: var(--transition);
}

.slider::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 20px;
    height: 20px;
    border-radius: 50%;
    background: var(--primary-color);
    cursor: pointer;
    box-shadow: var(--shadow-light);
    transition: var(--transition);
}

.slider::-webkit-slider-thumb:hover {
    transform: scale(1.1);
    box-shadow: var(--shadow-medium);
}

.slider::-moz-range-thumb {
    width: 20px;
    height: 20px;
    border-radius: 50%;
    background: var(--primary-color);
    cursor: pointer;
    border: none;
    box-shadow: var(--shadow-light);
}

/* 红色滑块 */
.slider[data-color="red"]::-webkit-slider-thumb {
    background: #ef4444;
}

.slider[data-color="green"]::-webkit-slider-thumb {
    background: #10b981;
}

.slider[data-color="blue"]::-webkit-slider-thumb {
    background: #3b82f6;
}

/* 按钮样式 */
.btn {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 0.5rem;
    padding: 0.75rem 1.5rem;
    border: none;
    border-radius: var(--border-radius);
    font-size: 0.875rem;
    font-weight: 600;
    cursor: pointer;
    transition: var(--transition);
    text-decoration: none;
    background: var(--bg-tertiary);
    color: var(--text-primary);
    border: 1px solid var(--border-color);
}

.btn:hover {
    transform: translateY(-1px);
    box-shadow: var(--shadow-medium);
}

.btn:active {
    transform: translateY(0);
}

.btn.active {
    background: var(--primary-color);
    color: white;
    border-color: var(--primary-color);
}

.btn-power {
    background: var(--gradient-success);
    color: white;
    border: none;
    font-size: 1rem;
    padding: 1rem 2rem;
}

.btn-power.off {
    background: var(--gradient-secondary);
}

/* 特效控制 */
.effect-controls {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
    gap: 0.75rem;
}

.btn-effect {
    flex-direction: column;
    padding: 1rem;
    min-height: 80px;
}

.btn-effect .btn-icon {
    font-size: 1.5rem;
}

/* 预设颜色 */
.preset-colors {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(60px, 1fr));
    gap: 0.75rem;
}

.preset-btn {
    width: 60px;
    height: 60px;
    border-radius: var(--border-radius);
    border: 2px solid var(--border-color);
    cursor: pointer;
    transition: var(--transition);
    position: relative;
    overflow: hidden;
}

.preset-btn:hover {
    transform: scale(1.05);
    border-color: var(--primary-color);
}

.preset-btn::before {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background: linear-gradient(45deg, transparent 30%, rgba(255,255,255,0.1) 50%, transparent 70%);
    transform: translateX(-100%);
    transition: var(--transition);
}

.preset-btn:hover::before {
    transform: translateX(100%);
}

/* 颜色预览 */
.color-preview {
    flex: 1;
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
}

.preview-card {
    background: var(--bg-card);
    border-radius: var(--border-radius-lg);
    padding: 2rem;
    border: 1px solid var(--border-color);
    backdrop-filter: blur(10px);
    box-shadow: var(--shadow-medium);
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    min-height: 300px;
    position: relative;
    overflow: hidden;
}

.preview-card::before {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background: radial-gradient(circle at center, currentColor 0%, transparent 70%);
    opacity: 0.1;
    pointer-events: none;
}

.color-display {
    width: 200px;
    height: 200px;
    border-radius: 50%;
    border: 4px solid var(--border-color);
    box-shadow: var(--shadow-heavy);
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 2rem;
    font-weight: 700;
    color: white;
    text-shadow: 0 2px 4px rgba(0,0,0,0.3);
    transition: var(--transition);
    position: relative;
    z-index: 1;
}

.color-display.off {
    background: #374151 !important;
    color: var(--text-muted);
}

.color-info {
    margin-top: 1rem;
    text-align: center;
}

.color-hex {
    font-family: 'Courier New', monospace;
    font-size: 1.25rem;
    font-weight: 700;
    color: var(--text-primary);
    margin-bottom: 0.5rem;
}

.color-rgb {
    font-size: 0.875rem;
    color: var(--text-secondary);
}

/* 响应式设计 */
@media (max-width: 768px) {
    .main {
        flex-direction: column;
        gap: 1rem;
    }
    
    .header {
        flex-direction: column;
        gap: 1rem;
        text-align: center;
    }
    
    .header h1 {
        font-size: 1.25rem;
    }
    
    .effect-controls {
        grid-template-columns: repeat(2, 1fr);
    }
    
    .preset-colors {
        grid-template-columns: repeat(4, 1fr);
    }
    
    .color-display {
        width: 150px;
        height: 150px;
        font-size: 1.5rem;
    }
    
    .card {
        padding: 1rem;
    }
}

@media (max-width: 480px) {
    .app {
        padding: 0.5rem;
    }
    
    .effect-controls {
        grid-template-columns: 1fr;
    }
    
    .preset-colors {
        grid-template-columns: repeat(3, 1fr);
    }
    
    .btn-effect {
        min-height: 60px;
        padding: 0.75rem;
    }
    
    .btn-effect .btn-icon {
        font-size: 1.25rem;
    }
}

/* 触摸设备优化 */
.touch-device .btn {
    min-height: 44px;
}

.touch-device .slider {
    height: 12px;
}

.touch-device .slider::-webkit-slider-thumb {
    width: 24px;
    height: 24px;
}

/* 动画效果 */
@keyframes fadeIn {
    from { opacity: 0; transform: translateY(20px); }
    to { opacity: 1; transform: translateY(0); }
}

.card {
    animation: fadeIn 0.6s ease-out;
}

.card:nth-child(1) { animation-delay: 0.1s; }
.card:nth-child(2) { animation-delay: 0.2s; }
.card:nth-child(3) { animation-delay: 0.3s; }
.card:nth-child(4) { animation-delay: 0.4s; }

/* 通知样式 */
.notification {
    position: fixed;
    top: 20px;
    right: 20px;
    padding: 1rem 1.5rem;
    border-radius: var(--border-radius);
    color: white;
    font-weight: 500;
    z-index: 1000;
    transform: translateX(100%);
    transition: var(--transition);
    box-shadow: var(--shadow-medium);
}

/* 加载动画 */
.loading {
    display: inline-block;
    width: 20px;
    height: 20px;
    border: 3px solid var(--border-color);
    border-radius: 50%;
    border-top-color: var(--primary-color);
    animation: spin 1s ease-in-out infinite;
}

@keyframes spin {
    to { transform: rotate(360deg); }
}
    </style>
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
            <div class="control-panel">
                <!-- 颜色控制区域 -->
                <div class="card">
                    <h2>🎨 颜色控制</h2>
                    <div class="color-controls">
                        <div class="color-slider">
                            <div class="color-label">
                                <span>红色</span>
                                <span class="color-value-display" id="r-display">255</span>
                            </div>
                            <input type="range" id="r" min="0" max="255" value="255" class="slider">
                        </div>
                        
                        <div class="color-slider">
                            <div class="color-label">
                                <span>绿色</span>
                                <span class="color-value-display" id="g-display">255</span>
                            </div>
                            <input type="range" id="g" min="0" max="255" value="255" class="slider">
                        </div>
                        
                        <div class="color-slider">
                            <div class="color-label">
                                <span>蓝色</span>
                                <span class="color-value-display" id="b-display">255</span>
                            </div>
                            <input type="range" id="b" min="0" max="255" value="255" class="slider">
                        </div>
                        
                        <div class="color-slider">
                            <div class="color-label">
                                <span>亮度</span>
                                <span class="color-value-display" id="brightness-display">50%</span>
                            </div>
                            <input type="range" id="brightness" min="0" max="100" value="50" class="slider">
                        </div>
                    </div>
                </div>

                <!-- 电源控制区域 -->
                <div class="card">
                    <h2>⚡ 电源控制</h2>
                    <div class="power-control">
                        <button class="btn btn-power" id="power-btn" onclick="togglePower()">
                            <span class="btn-icon">💡</span>
                            <span class="btn-text">开启LED</span>
                        </button>
                    </div>
                </div>

                <!-- 特效控制区域 -->
                <div class="card">
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
                </div>

                <!-- 预设颜色区域 -->
                <div class="card">
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
                </div>
            </div>

            <div class="preview-panel">
                <!-- 颜色预览区域 -->
                <div class="card">
                    <h2>🎨 颜色预览</h2>
                    <div class="preview-section">
                        <div class="color-preview" id="color-preview">
                            <div class="preview-circle" id="preview-circle"></div>
                            <div class="preview-info">
                                <div class="color-value">
                                    <h3>R</h3>
                                    <div class="value" id="r-val">255</div>
                                </div>
                                <div class="color-value">
                                    <h3>G</h3>
                                    <div class="value" id="g-val">255</div>
                                </div>
                                <div class="color-value">
                                    <h3>B</h3>
                                    <div class="value" id="b-val">255</div>
                                </div>
                                <div class="color-value">
                                    <h3>亮度</h3>
                                    <div class="value" id="brightness-val">50%</div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
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

// 嵌入的CSS文件内容 - 现代化设计，深色主题
const char style_css[] = R"rawliteral(
/* 现代化LED控制器样式 - 重新设计 */
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700;800;900&display=swap');

/* CSS变量定义 */
:root {
    --primary-color: #6366f1;
    --primary-dark: #4f46e5;
    --primary-light: #a5b4fc;
    --secondary-color: #ec4899;
    --accent-color: #06b6d4;
    --success-color: #10b981;
    --warning-color: #f59e0b;
    --error-color: #ef4444;
    
    --bg-primary: #0f172a;
    --bg-secondary: #1e293b;
    --bg-tertiary: #334155;
    --bg-card: rgba(30, 41, 59, 0.8);
    --bg-glass: rgba(255, 255, 255, 0.1);
    
    --text-primary: #f8fafc;
    --text-secondary: #cbd5e1;
    --text-muted: #94a3b8;
    
    --border-color: rgba(148, 163, 184, 0.2);
    --shadow-light: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
    --shadow-medium: 0 10px 15px -3px rgba(0, 0, 0, 0.1);
    --shadow-heavy: 0 25px 50px -12px rgba(0, 0, 0, 0.25);
    
    --gradient-primary: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    --gradient-secondary: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
    --gradient-accent: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
    --gradient-success: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%);
    
    --border-radius: 16px;
    --border-radius-lg: 24px;
    --border-radius-xl: 32px;
    
    --transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    --transition-fast: all 0.15s cubic-bezier(0.4, 0, 0.2, 1);
}

/* 全局样式重置 */
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

*::before,
*::after {
    box-sizing: border-box;
}

/* 基础样式 */
html {
    font-size: 16px;
    scroll-behavior: smooth;
}

body {
    font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
    background: var(--bg-primary);
    color: var(--text-primary);
    line-height: 1.6;
    font-weight: 400;
    overflow-x: hidden;
    position: relative;
    min-height: 100vh;
}

/* 背景装饰 */
body::before {
    content: '';
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: 
        radial-gradient(circle at 20% 80%, rgba(99, 102, 241, 0.15) 0%, transparent 50%),
        radial-gradient(circle at 80% 20%, rgba(236, 72, 153, 0.15) 0%, transparent 50%),
        radial-gradient(circle at 40% 40%, rgba(6, 182, 212, 0.1) 0%, transparent 50%);
    pointer-events: none;
    z-index: -1;
}

/* 滚动条美化 */
::-webkit-scrollbar {
    width: 8px;
}

::-webkit-scrollbar-track {
    background: var(--bg-secondary);
    border-radius: 4px;
}

::-webkit-scrollbar-thumb {
    background: var(--bg-tertiary);
    border-radius: 4px;
    transition: var(--transition);
}

::-webkit-scrollbar-thumb:hover {
    background: var(--primary-color);
}

/* 主容器 */
.app {
    max-width: 1200px;
    margin: 0 auto;
    padding: 2rem;
    min-height: 100vh;
    display: flex;
    flex-direction: column;
    gap: 2rem;
}

/* 头部样式 */
.header {
    background: var(--bg-glass);
    backdrop-filter: blur(20px);
    -webkit-backdrop-filter: blur(20px);
    border: 1px solid var(--border-color);
    border-radius: var(--border-radius-xl);
    padding: 2rem;
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 1.5rem;
    box-shadow: var(--shadow-medium);
    position: relative;
    overflow: hidden;
}

.header::before {
    content: '';
    position: absolute;
    top: 0;
    left: -100%;
    width: 100%;
    height: 100%;
    background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.05), transparent);
    transition: left 0.6s ease;
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
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 2rem;
    align-items: start;
}

/* 控制面板 */
.control-panel {
    display: flex;
    flex-direction: column;
    gap: 2rem;
}

/* 预览面板 */
.preview-panel {
    position: sticky;
    top: 2rem;
}

/* 卡片通用样式 */
.card {
    background: var(--bg-card);
    backdrop-filter: blur(20px);
    -webkit-backdrop-filter: blur(20px);
    border: 1px solid var(--border-color);
    border-radius: var(--border-radius-lg);
    padding: 2rem;
    box-shadow: var(--shadow-medium);
    transition: var(--transition);
    position: relative;
    overflow: hidden;
}

.card:hover {
    transform: translateY(-4px);
    box-shadow: var(--shadow-heavy);
    border-color: var(--primary-color);
}

.card::before {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    height: 3px;
    background: var(--gradient-primary);
    background-size: 200% 100%;
    animation: gradientMove 3s ease-in-out infinite;
}

.card h2 {
    font-size: 1.5rem;
    font-weight: 700;
    color: var(--text-primary);
    margin-bottom: 1.5rem;
    display: flex;
    align-items: center;
    gap: 0.75rem;
}

.card h2::before {
    content: '';
    width: 4px;
    height: 1.5rem;
    background: var(--gradient-primary);
    border-radius: 2px;
}

@keyframes gradientMove {
    0% { background-position: 0% 50%; }
    50% { background-position: 100% 50%; }
    100% { background-position: 0% 50%; }
}

/* 预览区域 */
.preview-section {
    text-align: center;
}

.color-preview {
    margin-bottom: 2rem;
}

.preview-circle {
    width: 200px;
    height: 200px;
    border-radius: 50%;
    margin: 0 auto 1.5rem;
    box-shadow: 
        0 20px 40px rgba(0, 0, 0, 0.3),
        0 0 0 1px var(--border-color),
        inset 0 0 0 1px rgba(255, 255, 255, 0.1);
    cursor: pointer;
    transition: var(--transition);
    position: relative;
    overflow: hidden;
}

.preview-circle:hover {
    transform: scale(1.05);
    box-shadow: 
        0 25px 50px rgba(0, 0, 0, 0.4),
        0 0 0 1px var(--primary-color),
        inset 0 0 0 1px rgba(255, 255, 255, 0.2);
}

.preview-circle::before {
    content: '';
    position: absolute;
    top: -50%;
    left: -50%;
    width: 200%;
    height: 200%;
    background: radial-gradient(circle, rgba(255, 255, 255, 0.1) 0%, transparent 70%);
    animation: rotate 20s linear infinite;
}

.preview-info {
    display: grid;
    grid-template-columns: repeat(4, 1fr);
    gap: 1rem;
    margin-bottom: 2rem;
}

.color-value {
    background: var(--bg-tertiary);
    padding: 1rem;
    border-radius: var(--border-radius);
    border: 1px solid var(--border-color);
}

.color-value h3 {
    font-size: 0.875rem;
    color: var(--text-muted);
    margin-bottom: 0.5rem;
    text-transform: uppercase;
    letter-spacing: 0.05em;
}

.color-value .value {
    font-size: 1.25rem;
    font-weight: 700;
    color: var(--text-primary);
    font-family: 'Courier New', monospace;
}

/* 颜色控制区域 */
.color-controls {
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
}

.color-slider {
    display: flex;
    flex-direction: column;
    gap: 0.75rem;
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

.color-label {
    display: flex;
    justify-content: space-between;
    align-items: center;
    color: var(--text-secondary);
    font-weight: 500;
}

.color-value-display {
    font-family: 'Courier New', monospace;
    font-weight: 700;
    color: var(--text-primary);
    background: var(--bg-tertiary);
    padding: 0.25rem 0.5rem;
    border-radius: 4px;
    border: 1px solid var(--border-color);
}

.slider {
    -webkit-appearance: none;
    appearance: none;
    width: 100%;
    height: 8px;
    border-radius: 4px;
    background: var(--bg-tertiary);
    outline: none;
    border: 1px solid var(--border-color);
    transition: var(--transition);
}

.slider:hover {
    border-color: var(--primary-color);
}

.slider::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 24px;
    height: 24px;
    border-radius: 50%;
    background: var(--gradient-primary);
    cursor: pointer;
    box-shadow: var(--shadow-medium);
    border: 2px solid var(--text-primary);
    transition: var(--transition);
}

.slider::-webkit-slider-thumb:hover {
    transform: scale(1.1);
    box-shadow: var(--shadow-heavy);
}

.slider::-moz-range-thumb {
    width: 24px;
    height: 24px;
    border-radius: 50%;
    background: var(--gradient-primary);
    cursor: pointer;
    border: 2px solid var(--text-primary);
    box-shadow: var(--shadow-medium);
}

/* 按钮样式 */
.btn {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 0.75rem;
    padding: 1rem 2rem;
    border: none;
    border-radius: var(--border-radius);
    font-size: 1rem;
    font-weight: 600;
    cursor: pointer;
    transition: var(--transition);
    text-decoration: none;
    min-height: 3rem;
    -webkit-tap-highlight-color: transparent;
    position: relative;
    overflow: hidden;
    letter-spacing: 0.025em;
    box-shadow: var(--shadow-light);
}

.btn::before {
    content: '';
    position: absolute;
    top: 0;
    left: -100%;
    width: 100%;
    height: 100%;
    background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.1), transparent);
    transition: left 0.5s;
}

.btn:hover::before {
    left: 100%;
}

.btn:hover {
    transform: translateY(-2px);
    box-shadow: var(--shadow-medium);
}

.btn:active {
    transform: translateY(0);
    transition: var(--transition-fast);
}

.btn-power {
    background: var(--gradient-success);
    color: var(--text-primary);
    width: 100%;
    font-size: 1.125rem;
    font-weight: 700;
}

.btn-power:hover {
    background: linear-gradient(135deg, #059669, #0d9488);
}

.btn-effect {
    background: var(--gradient-primary);
    color: var(--text-primary);
    flex: 1;
    min-width: 8rem;
}

.btn-effect:hover {
    background: linear-gradient(135deg, #4f46e5, #5b21b6);
}

.btn-effect.active {
    background: linear-gradient(135deg, #4f46e5, #5b21b6);
    transform: scale(0.98);
}

.btn-icon {
    font-size: 1.25rem;
}

.btn-text {
    font-size: 0.875rem;
}

/* 特效控制区域 */
.effect-controls {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(8rem, 1fr));
    gap: 1rem;
}

/* 预设颜色区域 */
.preset-colors {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(5rem, 1fr));
    gap: 1rem;
}

.preset-btn {
    padding: 1rem 0.75rem;
    border: none;
    border-radius: var(--border-radius);
    color: var(--text-primary);
    font-weight: 600;
    cursor: pointer;
    transition: var(--transition);
    text-shadow: 0 1px 2px rgba(0, 0, 0, 0.3);
    min-height: 3rem;
    display: flex;
    align-items: center;
    justify-content: center;
    -webkit-tap-highlight-color: transparent;
    position: relative;
    overflow: hidden;
    font-size: 0.875rem;
    letter-spacing: 0.025em;
}

.preset-btn::before {
    content: '';
    position: absolute;
    top: 0;
    left: -100%;
    width: 100%;
    height: 100%;
    background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.1), transparent);
    transition: left 0.5s;
}

.preset-btn:hover::before {
    left: 100%;
}

.preset-btn:hover {
    transform: translateY(-2px);
    box-shadow: var(--shadow-medium);
}

.preset-btn:active {
    transform: translateY(0);
    transition: var(--transition-fast);
}

/* 电源控制区域 */
.power-control {
    display: flex;
    justify-content: center;
    margin-top: 1rem;
}

/* 底部样式 */
.footer {
    text-align: center;
    padding: 2rem;
    color: var(--text-muted);
    font-size: 0.875rem;
    border-top: 1px solid var(--border-color);
    margin-top: 2rem;
}

/* 通知样式 */
.notification {
    position: fixed;
    top: 1.5rem;
    right: 1.5rem;
    padding: 1rem 1.5rem;
    border-radius: var(--border-radius);
    color: var(--text-primary);
    font-weight: 600;
    z-index: 1000;
    transform: translateX(100%);
    transition: var(--transition);
    max-width: 20rem;
    word-wrap: break-word;
    box-shadow: var(--shadow-medium);
    background: var(--bg-card);
    border: 1px solid var(--border-color);
}

/* 响应式设计 - 平板设备 */
@media (max-width: 1024px) {
    .main {
        grid-template-columns: 1fr;
        gap: 1.5rem;
    }
    
    .preview-panel {
        position: static;
    }
    
    .preview-circle {
        width: 180px;
        height: 180px;
    }
}

/* 响应式设计 - 手机设备 */
@media (max-width: 768px) {
    .app {
        padding: 1rem;
    }
    
    .header {
        padding: 1.5rem;
        flex-direction: column;
        gap: 1rem;
        text-align: center;
    }
    
    .header h1 {
        font-size: 1.75rem;
    }
    
    .card {
        padding: 1.5rem;
    }
    
    .preview-circle {
        width: 150px;
        height: 150px;
    }
    
    .effect-controls {
        grid-template-columns: repeat(2, 1fr);
    }
    
    .preset-colors {
        grid-template-columns: repeat(3, 1fr);
    }
    
    .preview-info {
        grid-template-columns: repeat(2, 1fr);
    }
}

@media (max-width: 480px) {
    .app {
        padding: 0.75rem;
    }
    
    .header {
        padding: 1rem;
        border-radius: var(--border-radius-lg);
    }
    
    .header h1 {
        font-size: 1.5rem;
    }
    
    .card {
        padding: 1rem;
        border-radius: var(--border-radius);
    }
    
    .preview-circle {
        width: 120px;
        height: 120px;
    }
    
    .effect-controls {
        grid-template-columns: 1fr;
        gap: 0.75rem;
    }
    
    .preset-colors {
        grid-template-columns: repeat(2, 1fr);
        gap: 0.75rem;
    }
    
    .preview-info {
        grid-template-columns: 1fr;
        gap: 0.75rem;
    }
    
    .btn {
        padding: 0.75rem 1.5rem;
        font-size: 0.875rem;
        min-height: 2.5rem;
    }
    
    .slider::-webkit-slider-thumb {
        width: 20px;
        height: 20px;
    }
    
    .notification {
        right: 0.75rem;
        left: 0.75rem;
        max-width: none;
        border-radius: var(--border-radius);
    }
}

/* 触摸设备优化 */
@media (hover: none) and (pointer: coarse) {
    .btn {
        min-height: 3rem;
        padding: 1rem 1.5rem;
    }
    
    .slider::-webkit-slider-thumb {
        width: 28px;
        height: 28px;
    }
    
    .preset-btn {
        min-height: 3rem;
        padding: 1rem 1.25rem;
    }
    
    /* 增加触摸目标间距 */
    .color-slider {
        gap: 1rem;
    }
    
    .effect-controls {
        gap: 1.25rem;
    }
    
    .preset-colors {
        gap: 1.25rem;
    }
    
    /* 触摸反馈 */
    .btn:active,
    .preset-btn:active {
        transform: scale(0.98);
        transition: var(--transition-fast);
    }
    
    /* 触摸高亮 */
    .slider:active::-webkit-slider-thumb {
        transform: scale(1.1);
        box-shadow: var(--shadow-heavy);
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
        transform: translateY(2rem) scale(0.95);
    }
    to {
        opacity: 1;
        transform: translateY(0) scale(1);
    }
}

.card {
    animation: fadeInUp 0.6s cubic-bezier(0.4, 0, 0.2, 1);
}

.card:nth-child(2) { animation-delay: 0.1s; }
.card:nth-child(3) { animation-delay: 0.2s; }
.card:nth-child(4) { animation-delay: 0.3s; }

.header {
    animation: fadeInUp 0.6s cubic-bezier(0.4, 0, 0.2, 1);
}

/* 渐变移动动画 */
@keyframes gradientMove {
    0% { background-position: 0% 50%; }
    50% { background-position: 100% 50%; }
    100% { background-position: 0% 50%; }
}

/* 旋转动画 */
@keyframes rotate {
    from { transform: rotate(0deg); }
    to { transform: rotate(360deg); }
}

/* 脉冲动画 */
@keyframes pulse {
    0%, 100% { 
        opacity: 1; 
        transform: scale(1); 
    }
    50% { 
        opacity: 0.7; 
        transform: scale(1.1); 
    }
}

@keyframes ripple {
    0% { 
        transform: scale(1); 
        opacity: 1; 
    }
    100% { 
        transform: scale(2); 
        opacity: 0; 
    }
}

.status-dot {
    animation: pulse 2s infinite;
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
