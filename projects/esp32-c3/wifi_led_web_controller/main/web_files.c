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
    <meta name="theme-color" content="#6366f1">
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
    font-size: 2.5rem;
    font-weight: 900;
    background: var(--gradient-primary);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
    letter-spacing: -0.02em;
    line-height: 1.2;
}

.header h1::before {
    content: '🌈';
    margin-right: 0.5rem;
    font-size: 2rem;
}

/* 状态指示器 */
.status-indicator {
    display: flex;
    align-items: center;
    gap: 0.75rem;
    background: var(--bg-glass);
    border: 1px solid var(--border-color);
    border-radius: 50px;
    padding: 0.75rem 1.5rem;
    font-size: 0.875rem;
    font-weight: 500;
    color: var(--text-secondary);
    backdrop-filter: blur(10px);
    -webkit-backdrop-filter: blur(10px);
    transition: var(--transition);
}

.status-indicator:hover {
    background: rgba(255, 255, 255, 0.15);
    transform: translateY(-2px);
    box-shadow: var(--shadow-medium);
}

.status-dot {
    width: 10px;
    height: 10px;
    border-radius: 50%;
    background: var(--success-color);
    position: relative;
    animation: pulse 2s infinite;
}

.status-dot::after {
    content: '';
    position: absolute;
    top: -4px;
    left: -4px;
    right: -4px;
    bottom: -4px;
    border-radius: 50%;
    background: rgba(16, 185, 129, 0.3);
    animation: ripple 2s infinite;
}

@keyframes pulse {
    0%, 100% { opacity: 1; transform: scale(1); }
    50% { opacity: 0.7; transform: scale(1.1); }
}

@keyframes ripple {
    0% { transform: scale(1); opacity: 1; }
    100% { transform: scale(2.5); opacity: 0; }
}

/* 主内容区域 */
.main {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 2rem;
    flex: 1;
}

/* 左侧控制区域 */
.control-panel {
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
}

/* 右侧预览区域 */
.preview-panel {
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
}

/* 卡片通用样式 */
.card {
    background: var(--bg-glass);
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

@keyframes gradientMove {
    0% { background-position: 0% 50%; }
    50% { background-position: 100% 50%; }
    100% { background-position: 0% 50%; }
}

.card:hover {
    transform: translateY(-4px);
    box-shadow: var(--shadow-heavy);
    border-color: rgba(99, 102, 241, 0.3);
}

.card h2 {
    font-size: 1.25rem;
    font-weight: 700;
    color: var(--text-primary);
    margin-bottom: 1.5rem;
    display: flex;
    align-items: center;
    gap: 0.5rem;
}

.card h2::before {
    font-size: 1.5rem;
}

/* 颜色预览区域 */
.preview-section {
    text-align: center;
}

.color-preview {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 2rem;
}

.preview-circle {
    width: 200px;
    height: 200px;
    border-radius: 50%;
    background: var(--gradient-primary);
    border: 4px solid var(--bg-glass);
    box-shadow: 
        0 20px 40px rgba(0, 0, 0, 0.3),
        inset 0 0 40px rgba(255, 255, 255, 0.1);
    transition: var(--transition);
    cursor: pointer;
    position: relative;
    overflow: hidden;
}

.preview-circle::before {
    content: '';
    position: absolute;
    top: -50%;
    left: -50%;
    width: 200%;
    height: 200%;
    background: conic-gradient(from 0deg, transparent, rgba(255, 255, 255, 0.1), transparent);
    animation: rotate 10s linear infinite;
}

@keyframes rotate {
    from { transform: rotate(0deg); }
    to { transform: rotate(360deg); }
}

.preview-circle:hover {
    transform: scale(1.05);
    box-shadow: 
        0 30px 60px rgba(0, 0, 0, 0.4),
        inset 0 0 60px rgba(255, 255, 255, 0.2);
}

.preview-info {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    gap: 1rem;
    width: 100%;
}

.color-value {
    background: var(--bg-secondary);
    border: 1px solid var(--border-color);
    border-radius: var(--border-radius);
    padding: 1rem;
    text-align: center;
    transition: var(--transition);
}

.color-value:hover {
    background: var(--bg-tertiary);
    border-color: var(--primary-color);
}

.color-value span {
    display: block;
    font-size: 0.875rem;
    color: var(--text-muted);
    margin-bottom: 0.25rem;
}

.color-value strong {
    font-size: 1.25rem;
    font-weight: 700;
    color: var(--text-primary);
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

.color-slider label {
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-weight: 500;
    color: var(--text-secondary);
}

.color-label {
    display: flex;
    align-items: center;
    gap: 0.5rem;
}

.color-label.red::before { content: "🔴"; }
.color-label.green::before { content: "🟢"; }
.color-label.blue::before { content: "🔵"; }

.color-value-display {
    font-weight: 600;
    color: var(--primary-color);
    min-width: 3rem;
    text-align: right;
}

/* 滑块样式 */
.slider {
    width: 100%;
    height: 8px;
    border-radius: 4px;
    outline: none;
    -webkit-appearance: none;
    appearance: none;
    background: var(--bg-tertiary);
    cursor: pointer;
    transition: var(--transition);
    border: 1px solid var(--border-color);
}

.slider:hover {
    background: var(--bg-secondary);
}

.slider:focus {
    box-shadow: 0 0 0 3px rgba(99, 102, 241, 0.3);
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
    transform: scale(1.2);
    box-shadow: var(--shadow-heavy);
}

.red-slider::-webkit-slider-thumb { background: var(--error-color); }
.green-slider::-webkit-slider-thumb { background: var(--success-color); }
.blue-slider::-webkit-slider-thumb { background: var(--accent-color); }
.brightness-slider::-webkit-slider-thumb { background: var(--warning-color); }

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
    min-height: 3.5rem;
    position: relative;
    overflow: hidden;
    letter-spacing: 0.025em;
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
    transform: translateY(-2px);
    box-shadow: var(--shadow-heavy);
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
}

.btn-power:hover {
    background: linear-gradient(135deg, #059669, #10b981);
}

.btn-effect {
    background: var(--bg-secondary);
    color: var(--text-primary);
    border: 1px solid var(--border-color);
    flex: 1;
    min-width: 120px;
}

.btn-effect:hover {
    background: var(--bg-tertiary);
    border-color: var(--primary-color);
}

.btn-effect.active {
    background: var(--gradient-primary);
    border-color: var(--primary-color);
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
    grid-template-columns: repeat(2, 1fr);
    gap: 1rem;
}

/* 预设颜色区域 */
.preset-colors {
    display: grid;
    grid-template-columns: repeat(4, 1fr);
    gap: 1rem;
}

.preset-btn {
    padding: 1rem;
    border: none;
    border-radius: var(--border-radius);
    color: var(--text-primary);
    font-weight: 600;
    cursor: pointer;
    transition: var(--transition);
    text-shadow: 0 1px 2px rgba(0, 0, 0, 0.5);
    min-height: 3.5rem;
    display: flex;
    align-items: center;
    justify-content: center;
    position: relative;
    overflow: hidden;
    font-size: 0.875rem;
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
    box-shadow: var(--shadow-heavy);
}

.preset-btn:active {
    transform: translateY(-1px) scale(1.02);
    transition: var(--transition-fast);
}

/* 电源控制区域 */
.power-control {
    display: flex;
    justify-content: center;
}

/* 底部样式 */
.footer {
    text-align: center;
    padding: 1.5rem;
    color: var(--text-muted);
    font-size: 0.875rem;
    border-top: 1px solid var(--border-color);
    margin-top: auto;
}

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

.notification.success { background: rgba(16, 185, 129, 0.9); }
.notification.error { background: rgba(239, 68, 68, 0.9); }
.notification.info { background: rgba(99, 102, 241, 0.9); }
.notification.warning { background: rgba(245, 158, 11, 0.9); }

/* WiFi控制样式 */
.wifi-control {
    display: flex;
    flex-direction: column;
    gap: 1rem;
}

.wifi-status {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 0.75rem;
    background: var(--bg-tertiary);
    border-radius: var(--border-radius);
    border: 1px solid var(--border-color);
}

.status-label {
    font-size: 0.875rem;
    color: var(--text-secondary);
}

.status-value {
    font-weight: 600;
    padding: 0.25rem 0.75rem;
    border-radius: 12px;
    font-size: 0.75rem;
}

.status-value.enabled {
    background: var(--success-color);
    color: white;
}

.status-value.disabled {
    background: var(--error-color);
    color: white;
}

.btn-wifi {
    background: var(--bg-tertiary);
    color: var(--text-primary);
    border: 1px solid var(--border-color);
}

.btn-wifi.active {
    background: var(--gradient-accent);
    color: white;
    border-color: var(--accent-color);
}

.wifi-info {
    background: var(--bg-tertiary);
    padding: 1rem;
    border-radius: var(--border-radius);
    border: 1px solid var(--border-color);
    font-size: 0.875rem;
}

.wifi-info p {
    margin: 0.5rem 0;
    color: var(--text-secondary);
}

.wifi-info strong {
    color: var(--text-primary);
}

/* 响应式设计 */
@media (max-width: 1024px) {
    .main {
        grid-template-columns: 1fr;
    }
    
    .app {
        padding: 1.5rem;
    }
    
    .header {
        padding: 1.5rem;
    }
    
    .header h1 {
        font-size: 2rem;
    }
    
    .card {
        padding: 1.5rem;
    }
}

@media (max-width: 768px) {
    .app {
        padding: 1rem;
        gap: 1.5rem;
    }
    
    .header {
        flex-direction: column;
        text-align: center;
        padding: 1.5rem;
    }
    
    .header h1 {
        font-size: 1.75rem;
    }
    
    .preview-circle {
        width: 150px;
        height: 150px;
    }
    
    .effect-controls {
        grid-template-columns: 1fr;
    }
    
    .preset-colors {
        grid-template-columns: repeat(3, 1fr);
    }
    
    .card {
        padding: 1.25rem;
    }
}

@media (max-width: 480px) {
    .app {
        padding: 0.75rem;
    }
    
    .header {
        padding: 1.25rem;
        border-radius: var(--border-radius);
    }
    
    .header h1 {
        font-size: 1.5rem;
    }
    
    .preview-circle {
        width: 120px;
        height: 120px;
    }
    
    .preset-colors {
        grid-template-columns: repeat(2, 1fr);
    }
    
    .btn {
        padding: 0.875rem 1.5rem;
        min-height: 3rem;
    }
    
    .notification {
        right: 1rem;
        left: 1rem;
        max-width: none;
    }
}

/* 触摸设备优化 */
@media (hover: none) and (pointer: coarse) {
    .btn {
        min-height: 3.5rem;
        padding: 1.125rem 2rem;
    }
    
    .slider::-webkit-slider-thumb {
        width: 28px;
        height: 28px;
    }
    
    .preset-btn {
        min-height: 3.5rem;
        padding: 1.125rem;
    }
    
    .color-slider {
        gap: 1rem;
    }
    
    .effect-controls {
        gap: 1.25rem;
    }
    
    .preset-colors {
        gap: 1.25rem;
    }
}

/* 加载动画 */
@keyframes fadeInUp {
    from {
        opacity: 0;
        transform: translateY(30px);
    }
    to {
        opacity: 1;
        transform: translateY(0);
    }
}

.card {
    animation: fadeInUp 0.6s ease-out;
}

.card:nth-child(2) { animation-delay: 0.1s; }
.card:nth-child(3) { animation-delay: 0.2s; }
.card:nth-child(4) { animation-delay: 0.3s; }

.header {
    animation: fadeInUp 0.6s ease-out;
}

/* 浮动动画 */
@keyframes float {
    0%, 100% { transform: translateY(0px); }
    50% { transform: translateY(-10px); }
}

.preview-circle {
    animation: float 6s ease-in-out infinite;
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
    </style>
</head>
<body>
    <div class="app">
        <header class="header">
            <h1>ESP32-C3 LED控制器</h1>
            <div class="status-indicator" id="status-indicator">
                <span class="status-dot"></span>
                <span class="status-text">已连接</span>
            </div>
        </header>

        <main class="main">
            <!-- 左侧控制区域 -->
            <div class="control-panel">
                <!-- 颜色控制区域 -->
                <div class="card">
                    <h2>🎨 颜色控制</h2>
                    <div class="color-controls">
                        <div class="color-slider">
                            <label for="r">
                                <span class="color-label red">红色</span>
                                <span class="color-value-display" id="r-display">255</span>
                            </label>
                            <input type="range" id="r" min="0" max="255" value="255" class="slider red-slider">
                        </div>
                        
                        <div class="color-slider">
                            <label for="g">
                                <span class="color-label green">绿色</span>
                                <span class="color-value-display" id="g-display">255</span>
                            </label>
                            <input type="range" id="g" min="0" max="255" value="255" class="slider green-slider">
                        </div>
                        
                        <div class="color-slider">
                            <label for="b">
                                <span class="color-label blue">蓝色</span>
                                <span class="color-value-display" id="b-display">255</span>
                            </label>
                            <input type="range" id="b" min="0" max="255" value="255" class="slider blue-slider">
                        </div>
                        
                        <div class="color-slider">
                            <label for="brightness">
                                <span class="color-label">亮度</span>
                                <span class="color-value-display" id="brightness-display">50%</span>
                            </label>
                            <input type="range" id="brightness" min="0" max="100" value="50" class="slider brightness-slider">
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

                <!-- WiFi热点控制区域 -->
                <div class="card">
                    <h2>📶 WiFi热点控制</h2>
                    <div class="wifi-control">
                        <div class="wifi-status">
                            <span class="status-label">热点状态:</span>
                            <span class="status-value" id="ap-status">已关闭</span>
                        </div>
                        <button class="btn btn-wifi" id="ap-btn" onclick="toggleAP()">
                            <span class="btn-icon">📡</span>
                            <span class="btn-text">开启热点</span>
                        </button>
                        <div class="wifi-info" id="wifi-info" style="display: none;">
                            <p><strong>热点名称:</strong> <span id="ap-ssid">ESP32C3-LED-Controller</span></p>
                            <p><strong>热点密码:</strong> <span id="ap-password">12345678</span></p>
                            <p><strong>热点IP:</strong> <span id="ap-ip">192.168.4.1</span></p>
                        </div>
                    </div>
                </div>

                <!-- 特效控制区域 -->
                <div class="card">
                    <h2>✨ 特效控制</h2>
                    <div class="effect-controls">
                        <button class="btn btn-effect active" onclick="setEffect('static')">
                            <span class="btn-icon">🌟</span>
                            <span class="btn-text">静态</span>
                        </button>
                        <button class="btn btn-effect" onclick="setEffect('rainbow')">
                            <span class="btn-icon">🌈</span>
                            <span class="btn-text">彩虹</span>
                        </button>
                        <button class="btn btn-effect" onclick="setEffect('breathing')">
                            <span class="btn-icon">💨</span>
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
                    <h2>🎯 预设颜色</h2>
                    <div class="preset-colors">
                        <button class="preset-btn" style="background: #ef4444;" onclick="setPresetColor(255, 0, 0)">红色</button>
                        <button class="preset-btn" style="background: #10b981;" onclick="setPresetColor(0, 255, 0)">绿色</button>
                        <button class="preset-btn" style="background: #06b6d4;" onclick="setPresetColor(0, 0, 255)">蓝色</button>
                        <button class="preset-btn" style="background: #f59e0b;" onclick="setPresetColor(255, 255, 0)">黄色</button>
                        <button class="preset-btn" style="background: #ec4899;" onclick="setPresetColor(255, 0, 255)">紫色</button>
                        <button class="preset-btn" style="background: #06b6d4;" onclick="setPresetColor(0, 255, 255)">青色</button>
                        <button class="preset-btn" style="background: #f97316;" onclick="setPresetColor(255, 128, 0)">橙色</button>
                        <button class="preset-btn" style="background: #f8fafc; color: #0f172a;" onclick="setPresetColor(255, 255, 255)">白色</button>
                    </div>
                </div>
            </div>

            <!-- 右侧预览区域 -->
            <div class="preview-panel">
                <!-- 颜色预览区域 -->
                <div class="card preview-section">
                    <h2>🎨 颜色预览</h2>
                    <div class="color-preview">
                        <div class="preview-circle" id="preview-circle"></div>
                        <div class="preview-info">
                            <div class="color-value">
                                <span>红色</span>
                                <strong id="r-val">255</strong>
                            </div>
                            <div class="color-value">
                                <span>绿色</span>
                                <strong id="g-val">255</strong>
                            </div>
                            <div class="color-value">
                                <span>蓝色</span>
                                <strong id="b-val">255</strong>
                            </div>
                            <div class="color-value">
                                <span>亮度</span>
                                <strong id="brightness-val">50%</strong>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </main>

        <footer class="footer">
            <p>ESP32-C3 WiFi LED Web控制器 v2.0 - 现代化设计</p>
        </footer>
    </div>

    <script>
// 全局变量
let powerState = false;
let currentColor = { r: 255, g: 255, b: 255 };
let currentBrightness = 50;
let isConnected = true;
let isTouchDevice = false;
let apEnabled = false;

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
    checkConnection();
    setInterval(checkConnection, 5000);
    setupPWA();
    getAPStatus();
    setInterval(getAPStatus, 10000);
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
    document.getElementById('r').addEventListener('input', updateColor);
    document.getElementById('g').addEventListener('input', updateColor);
    document.getElementById('b').addEventListener('input', updateColor);
    document.getElementById('brightness').addEventListener('input', updateColor);
    
    if (isTouchDevice) {
        setupTouchOptimizations();
    }
    setupKeyboardShortcuts();
}

// 设置触摸优化
function setupTouchOptimizations() {
    let lastTouchEnd = 0;
    document.addEventListener('touchend', function(event) {
        const now = (new Date()).getTime();
        if (now - lastTouchEnd <= 300) {
            event.preventDefault();
        }
        lastTouchEnd = now;
    }, false);
    
    document.addEventListener('touchstart', function(event) {
        if (event.touches.length > 1) {
            event.preventDefault();
        }
    }, { passive: false });
}

// 设置键盘快捷键
function setupKeyboardShortcuts() {
    document.addEventListener('keydown', function(event) {
        if (event.code === 'Space' && event.target.tagName !== 'INPUT') {
            event.preventDefault();
            togglePower();
        }
        
        const presetColors = [
            [255, 0, 0], [0, 255, 0], [0, 0, 255], [255, 255, 0],
            [255, 0, 255], [0, 255, 255], [255, 128, 0], [255, 255, 255]
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
    sendColorToDevice(r, g, b, brightness);
}

// 更新颜色预览
function updateColorPreview() {
    const previewCircle = document.getElementById('preview-circle');
    const { r, g, b } = currentColor;
    
    const brightnessMultiplier = currentBrightness / 100;
    const adjustedR = Math.round(r * brightnessMultiplier);
    const adjustedG = Math.round(g * brightnessMultiplier);
    const adjustedB = Math.round(b * brightnessMultiplier);
    
    previewCircle.style.background = `rgb(${adjustedR}, ${adjustedG}, ${adjustedB})`;
    
    if (currentBrightness > 50) {
        previewCircle.style.boxShadow = `0 0 20px rgba(${adjustedR}, ${adjustedG}, ${adjustedB}, 0.6)`;
    } else {
        previewCircle.style.boxShadow = '0 8px 24px rgba(0, 0, 0, 0.15)';
    }
}

// 更新所有显示
function updateAllDisplays() {
    document.getElementById('r-display').textContent = currentColor.r;
    document.getElementById('g-display').textContent = currentColor.g;
    document.getElementById('b-display').textContent = currentColor.b;
    document.getElementById('brightness-display').textContent = currentBrightness + '%';
    
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
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ r, g, b, brightness })
    })
    .then(response => {
        if (!response.ok) throw new Error('Network response was not ok');
        return response.json();
    })
    .then(data => console.log('Color updated successfully:', data))
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
    
    sendPowerToDevice(powerState);
}

// 发送电源状态到设备
function sendPowerToDevice(power) {
    if (!isConnected) return;
    
    fetch('/api/led/power', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ power })
    })
    .then(response => {
        if (!response.ok) throw new Error('Network response was not ok');
        return response.json();
    })
    .then(data => {
        console.log('Power updated successfully:', data);
        showNotification(power ? 'LED已开启' : 'LED已关闭', 'success');
    })
    .catch(error => {
        console.error('Error updating power:', error);
        showNotification('电源控制失败', 'error');
        powerState = !powerState;
        togglePower();
    });
}

// 设置特效
function setEffect(effect) {
    if (!isConnected) return;
    
    document.querySelectorAll('.btn-effect').forEach(btn => {
        btn.classList.remove('active');
    });
    event.target.closest('.btn-effect').classList.add('active');
    
    fetch('/api/led/effect', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ effect })
    })
    .then(response => {
        if (!response.ok) throw new Error('Network response was not ok');
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
        'static': '静态', 'rainbow': '彩虹', 'breathing': '呼吸', 'blink': '闪烁'
    };
    return effectNames[effect] || effect;
}

// 设置预设颜色
function setPresetColor(r, g, b) {
    document.getElementById('r').value = r;
    document.getElementById('g').value = g;
    document.getElementById('b').value = b;
    updateColor();
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
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.textContent = message;
    
    const colors = {
        success: '#28a745', error: '#dc3545', info: '#007bff', warning: '#ffc107'
    };
    notification.style.background = colors[type] || colors.info;
    
    document.body.appendChild(notification);
    
    setTimeout(() => {
        notification.style.transform = 'translateX(0)';
    }, 100);
    
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

// 添加交互功能
document.addEventListener('DOMContentLoaded', function() {
    const previewCircle = document.getElementById('preview-circle');
    
    previewCircle.addEventListener('click', function() {
        const randomR = Math.floor(Math.random() * 256);
        const randomG = Math.floor(Math.random() * 256);
        const randomB = Math.floor(Math.random() * 256);
        
        document.getElementById('r').value = randomR;
        document.getElementById('g').value = randomG;
        document.getElementById('b').value = randomB;
        
        updateColor();
        showNotification('随机颜色已应用', 'success');
    });
    
    let clickCount = 0;
    let clickTimer;
    
    previewCircle.addEventListener('click', function() {
        clickCount++;
        if (clickCount === 1) {
            clickTimer = setTimeout(() => { clickCount = 0; }, 300);
        } else {
            clearTimeout(clickTimer);
            clickCount = 0;
            
            document.getElementById('r').value = 255;
            document.getElementById('g').value = 255;
            document.getElementById('b').value = 255;
            document.getElementById('brightness').value = 100;
            
            updateColor();
            showNotification('已重置为白色', 'info');
        }
    });
});

// 键盘导航支持
document.addEventListener('keydown', function(event) {
    if (event.key === 'Tab') {
        document.body.classList.add('keyboard-navigation');
    }
});

document.addEventListener('mousedown', function() {
    document.body.classList.remove('keyboard-navigation');
});

document.addEventListener('touchstart', function() {
    document.body.classList.remove('keyboard-navigation');
});

// AP模式控制
async function toggleAP() {
    try {
        const response = await fetch('/api/ap-mode', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ enable: !apEnabled })
        });

        const data = await response.json();
        
        if (data.status === 'success') {
            apEnabled = data.ap_enabled;
            updateAPUI();
            showNotification(
                apEnabled ? '热点已开启' : '热点已关闭', 'success'
            );
        } else {
            showNotification('操作失败: ' + data.message, 'error');
        }
    } catch (error) {
        console.error('AP控制错误:', error);
        showNotification('网络错误，请重试', 'error');
    }
}

// 更新AP界面
function updateAPUI() {
    const apBtn = document.getElementById('ap-btn');
    const apStatus = document.getElementById('ap-status');
    const wifiInfo = document.getElementById('wifi-info');
    
    if (apEnabled) {
        apBtn.innerHTML = '<span class="btn-icon">📡</span><span class="btn-text">关闭热点</span>';
        apBtn.className = 'btn btn-wifi active';
        apStatus.textContent = '已开启';
        apStatus.className = 'status-value enabled';
        wifiInfo.style.display = 'block';
    } else {
        apBtn.innerHTML = '<span class="btn-icon">📡</span><span class="btn-text">开启热点</span>';
        apBtn.className = 'btn btn-wifi';
        apStatus.textContent = '已关闭';
        apStatus.className = 'status-value disabled';
        wifiInfo.style.display = 'none';
    }
}

// 获取AP状态
async function getAPStatus() {
    try {
        const response = await fetch('/api/ap-status');
        const data = await response.json();
        
        if (data.status === 'ok') {
            apEnabled = data.ap_enabled;
            updateAPUI();
        }
    } catch (error) {
        console.error('获取AP状态错误:', error);
    }
}
    </script>
</body>
</html>
)rawliteral";

// 获取index_html大小的函数
size_t get_index_html_size(void)
{
    return strlen(index_html);
}