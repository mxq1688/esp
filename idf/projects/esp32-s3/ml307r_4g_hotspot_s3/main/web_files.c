/*
 * 嵌入的Web文件
 * 这些文件在编译时被嵌入到固件中
 */

#include "web_files.h"

// HTML文件内容
const char web_index_html_start[] = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-S3 ML307R 4G热点控制器</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>🌐 ESP32-S3 ML307R 4G热点控制器</h1>
            <div class="status-bar">
                <div class="status-item">
                    <span class="label">系统状态:</span>
                    <span id="system-status" class="status-value">连接中...</span>
                </div>
                <div class="status-item">
                    <span class="label">ML307R:</span>
                    <span id="ml307r-status" class="status-value">未知</span>
                </div>
                <div class="status-item">
                    <span class="label">WiFi:</span>
                    <span id="wifi-status" class="status-value">未连接</span>
                </div>
            </div>
        </header>

        <main>
            <!-- 网络信息卡片 -->
            <div class="card">
                <h2>📶 网络信息</h2>
                <div class="info-grid">
                    <div class="info-item">
                        <span class="info-label">运营商:</span>
                        <span id="operator" class="info-value">--</span>
                    </div>
                    <div class="info-item">
                        <span class="info-label">信号强度:</span>
                        <span id="signal-strength" class="info-value">--</span>
                    </div>
                    <div class="info-item">
                        <span class="info-label">网络类型:</span>
                        <span id="network-type" class="info-value">--</span>
                    </div>
                    <div class="info-item">
                        <span class="info-label">IP地址:</span>
                        <span id="ip-address" class="info-value">--</span>
                    </div>
                </div>
                <button id="refresh-network" class="btn btn-secondary">刷新网络信息</button>
            </div>

            <!-- 4G热点控制卡片 -->
            <div class="card">
                <h2>📡 4G热点控制</h2>
                <div class="hotspot-controls">
                    <div class="control-group">
                        <label for="hotspot-ssid">热点名称 (SSID):</label>
                        <input type="text" id="hotspot-ssid" value="ESP32-ML307R-Hotspot" maxlength="31">
                    </div>
                    <div class="control-group">
                        <label for="hotspot-password">密码:</label>
                        <input type="password" id="hotspot-password" value="12345678" maxlength="63">
                    </div>
                    <div class="control-group">
                        <label for="max-connections">最大连接数:</label>
                        <input type="number" id="max-connections" value="5" min="1" max="10">
                    </div>
                    <div class="button-group">
                        <button id="enable-hotspot" class="btn btn-primary">启用热点</button>
                        <button id="disable-hotspot" class="btn btn-danger">禁用热点</button>
                    </div>
                </div>
                <div id="hotspot-status" class="status-message"></div>
            </div>

            <!-- WiFi连接卡片 -->
            <div class="card">
                <h2>📶 WiFi连接</h2>
                <div class="wifi-controls">
                    <div class="control-group">
                        <label for="wifi-ssid">WiFi名称:</label>
                        <input type="text" id="wifi-ssid" placeholder="输入WiFi名称">
                    </div>
                    <div class="control-group">
                        <label for="wifi-password">WiFi密码:</label>
                        <input type="password" id="wifi-password" placeholder="输入WiFi密码">
                    </div>
                    <button id="connect-wifi" class="btn btn-primary">连接WiFi</button>
                </div>
                <div id="wifi-info">
                    <div class="info-item">
                        <span class="info-label">当前连接:</span>
                        <span id="current-wifi-ssid" class="info-value">--</span>
                    </div>
                    <div class="info-item">
                        <span class="info-label">IP地址:</span>
                        <span id="wifi-ip" class="info-value">--</span>
                    </div>
                </div>
            </div>

            <!-- 系统控制卡片 -->
            <div class="card">
                <h2>⚙️ 系统控制</h2>
                <div class="system-info">
                    <div class="info-item">
                        <span class="info-label">芯片型号:</span>
                        <span id="chip-model" class="info-value">--</span>
                    </div>
                    <div class="info-item">
                        <span class="info-label">可用内存:</span>
                        <span id="free-heap" class="info-value">--</span>
                    </div>
                    <div class="info-item">
                        <span class="info-label">运行时间:</span>
                        <span id="uptime" class="info-value">--</span>
                    </div>
                </div>
                <div class="button-group">
                    <button id="reset-ml307r" class="btn btn-warning">重启ML307R</button>
                    <button id="refresh-status" class="btn btn-secondary">刷新状态</button>
                </div>
            </div>
        </main>

        <footer>
            <p>© 2024 ESP32-S3 ML307R 4G热点控制器 v1.0.0</p>
        </footer>
    </div>

    <script src="/script.js"></script>
</body>
</html>
)";

// CSS文件内容
const char web_style_css_start[] = R"(
/* 现代化响应式CSS样式 */
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    min-height: 100vh;
    color: #333;
    line-height: 1.6;
}

.container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 20px;
}

header {
    text-align: center;
    margin-bottom: 30px;
    background: rgba(255, 255, 255, 0.95);
    padding: 20px;
    border-radius: 15px;
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
    backdrop-filter: blur(10px);
}

h1 {
    color: #2c3e50;
    font-size: 2.2em;
    margin-bottom: 15px;
    font-weight: 700;
}

.status-bar {
    display: flex;
    justify-content: center;
    gap: 20px;
    flex-wrap: wrap;
}

.status-item {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 8px 16px;
    background: rgba(52, 152, 219, 0.1);
    border-radius: 20px;
    border: 1px solid rgba(52, 152, 219, 0.3);
}

.label {
    font-weight: 600;
    color: #34495e;
}

.status-value {
    font-weight: 500;
    color: #2980b9;
}

main {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
    gap: 25px;
}

.card {
    background: rgba(255, 255, 255, 0.95);
    border-radius: 15px;
    padding: 25px;
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
    backdrop-filter: blur(10px);
    border: 1px solid rgba(255, 255, 255, 0.2);
    transition: transform 0.3s ease, box-shadow 0.3s ease;
}

.card:hover {
    transform: translateY(-5px);
    box-shadow: 0 12px 40px rgba(0, 0, 0, 0.15);
}

.card h2 {
    color: #2c3e50;
    margin-bottom: 20px;
    font-size: 1.4em;
    font-weight: 600;
    border-bottom: 2px solid #3498db;
    padding-bottom: 10px;
}

.info-grid, .info-item {
    display: grid;
    gap: 15px;
}

.info-grid {
    grid-template-columns: 1fr 1fr;
    margin-bottom: 20px;
}

.info-item {
    grid-template-columns: auto 1fr;
    align-items: center;
    padding: 10px;
    background: rgba(52, 152, 219, 0.05);
    border-radius: 8px;
    border-left: 4px solid #3498db;
}

.info-label {
    font-weight: 600;
    color: #34495e;
}

.info-value {
    font-weight: 500;
    color: #2980b9;
    text-align: right;
}

.control-group {
    margin-bottom: 20px;
}

.control-group label {
    display: block;
    margin-bottom: 8px;
    font-weight: 600;
    color: #34495e;
}

.control-group input {
    width: 100%;
    padding: 12px;
    border: 2px solid #e0e0e0;
    border-radius: 8px;
    font-size: 16px;
    transition: border-color 0.3s ease, box-shadow 0.3s ease;
}

.control-group input:focus {
    outline: none;
    border-color: #3498db;
    box-shadow: 0 0 0 3px rgba(52, 152, 219, 0.1);
}

.btn {
    padding: 12px 24px;
    border: none;
    border-radius: 8px;
    font-size: 16px;
    font-weight: 600;
    cursor: pointer;
    transition: all 0.3s ease;
    text-decoration: none;
    display: inline-block;
    text-align: center;
    min-width: 120px;
}

.btn-primary {
    background: linear-gradient(135deg, #3498db, #2980b9);
    color: white;
}

.btn-primary:hover {
    background: linear-gradient(135deg, #2980b9, #1f618d);
    transform: translateY(-2px);
    box-shadow: 0 4px 12px rgba(52, 152, 219, 0.3);
}

.btn-danger {
    background: linear-gradient(135deg, #e74c3c, #c0392b);
    color: white;
}

.btn-danger:hover {
    background: linear-gradient(135deg, #c0392b, #a93226);
    transform: translateY(-2px);
    box-shadow: 0 4px 12px rgba(231, 76, 60, 0.3);
}

.btn-warning {
    background: linear-gradient(135deg, #f39c12, #e67e22);
    color: white;
}

.btn-warning:hover {
    background: linear-gradient(135deg, #e67e22, #d35400);
    transform: translateY(-2px);
    box-shadow: 0 4px 12px rgba(243, 156, 18, 0.3);
}

.btn-secondary {
    background: linear-gradient(135deg, #95a5a6, #7f8c8d);
    color: white;
}

.btn-secondary:hover {
    background: linear-gradient(135deg, #7f8c8d, #6c7b7d);
    transform: translateY(-2px);
    box-shadow: 0 4px 12px rgba(149, 165, 166, 0.3);
}

.button-group {
    display: flex;
    gap: 15px;
    flex-wrap: wrap;
    justify-content: center;
}

.status-message {
    margin-top: 15px;
    padding: 12px;
    border-radius: 8px;
    text-align: center;
    font-weight: 500;
}

.status-message.success {
    background: rgba(46, 204, 113, 0.1);
    color: #27ae60;
    border: 1px solid rgba(46, 204, 113, 0.3);
}

.status-message.error {
    background: rgba(231, 76, 60, 0.1);
    color: #e74c3c;
    border: 1px solid rgba(231, 76, 60, 0.3);
}

.status-message.info {
    background: rgba(52, 152, 219, 0.1);
    color: #3498db;
    border: 1px solid rgba(52, 152, 219, 0.3);
}

footer {
    text-align: center;
    margin-top: 40px;
    padding: 20px;
    background: rgba(255, 255, 255, 0.95);
    border-radius: 15px;
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
    backdrop-filter: blur(10px);
}

footer p {
    color: #7f8c8d;
    font-size: 14px;
}

/* 响应式设计 */
@media (max-width: 768px) {
    .container {
        padding: 15px;
    }
    
    h1 {
        font-size: 1.8em;
    }
    
    .status-bar {
        flex-direction: column;
        align-items: center;
    }
    
    main {
        grid-template-columns: 1fr;
    }
    
    .info-grid {
        grid-template-columns: 1fr;
    }
    
    .button-group {
        flex-direction: column;
    }
    
    .btn {
        width: 100%;
    }
}

@media (max-width: 480px) {
    .card {
        padding: 20px;
    }
    
    .info-item {
        grid-template-columns: 1fr;
        text-align: center;
    }
    
    .info-value {
        text-align: center;
    }
}

/* 动画效果 */
@keyframes fadeIn {
    from {
        opacity: 0;
        transform: translateY(20px);
    }
    to {
        opacity: 1;
        transform: translateY(0);
    }
}

.card {
    animation: fadeIn 0.6s ease-out;
}

/* 加载状态 */
.loading {
    opacity: 0.6;
    pointer-events: none;
}

.loading::after {
    content: '';
    position: absolute;
    top: 50%;
    left: 50%;
    width: 20px;
    height: 20px;
    margin: -10px 0 0 -10px;
    border: 2px solid #3498db;
    border-radius: 50%;
    border-top-color: transparent;
    animation: spin 1s linear infinite;
}

@keyframes spin {
    to {
        transform: rotate(360deg);
    }
}
)";

// JavaScript文件内容
const char web_script_js_start[] = R"(
// ESP32-S3 ML307R 4G热点控制器 JavaScript

class HotspotController {
    constructor() {
        this.init();
        this.bindEvents();
        this.startStatusUpdates();
    }

    init() {
        console.log('初始化4G热点控制器...');
        this.updateStatus();
    }

    bindEvents() {
        // 热点控制按钮
        document.getElementById('enable-hotspot').addEventListener('click', () => {
            this.enableHotspot();
        });

        document.getElementById('disable-hotspot').addEventListener('click', () => {
            this.disableHotspot();
        });

        // WiFi连接按钮
        document.getElementById('connect-wifi').addEventListener('click', () => {
            this.connectWiFi();
        });

        // 系统控制按钮
        document.getElementById('reset-ml307r').addEventListener('click', () => {
            this.resetML307R();
        });

        document.getElementById('refresh-status').addEventListener('click', () => {
            this.updateStatus();
        });

        document.getElementById('refresh-network').addEventListener('click', () => {
            this.updateNetworkInfo();
        });
    }

    async apiCall(url, method = 'GET', data = null) {
        try {
            const options = {
                method: method,
                headers: {
                    'Content-Type': 'application/json',
                }
            };

            if (data) {
                options.body = JSON.stringify(data);
            }

            const response = await fetch(url, options);
            return await response.json();
        } catch (error) {
            console.error('API调用失败:', error);
            return { success: false, error: error.message };
        }
    }

    async updateStatus() {
        const result = await this.apiCall('/api/status');
        
        if (result.success) {
            // 更新系统状态
            document.getElementById('system-status').textContent = '正常运行';
            document.getElementById('ml307r-status').textContent = result.ml307r.state;
            document.getElementById('wifi-status').textContent = result.wifi.state;

            // 更新系统信息
            document.getElementById('chip-model').textContent = result.system.chip_model;
            document.getElementById('free-heap').textContent = this.formatBytes(result.system.free_heap);
            document.getElementById('uptime').textContent = this.formatUptime(result.system.uptime);

            // 更新WiFi信息
            if (result.wifi.connected) {
                document.getElementById('current-wifi-ssid').textContent = result.wifi.ssid || '--';
                document.getElementById('wifi-ip').textContent = result.wifi.ip_address || '--';
            } else {
                document.getElementById('current-wifi-ssid').textContent = '--';
                document.getElementById('wifi-ip').textContent = '--';
            }
        } else {
            document.getElementById('system-status').textContent = '连接失败';
        }

        // 同时更新网络信息
        this.updateNetworkInfo();
    }

    async updateNetworkInfo() {
        const result = await this.apiCall('/api/network/info');
        
        if (result.success) {
            document.getElementById('operator').textContent = result.operator || '--';
            document.getElementById('signal-strength').textContent = 
                result.signal_strength ? `${result.signal_strength} dBm` : '--';
            document.getElementById('network-type').textContent = result.network_type || '--';
            document.getElementById('ip-address').textContent = result.ip_address || '--';
        }
    }

    async enableHotspot() {
        const ssid = document.getElementById('hotspot-ssid').value.trim();
        const password = document.getElementById('hotspot-password').value;
        const maxConnections = parseInt(document.getElementById('max-connections').value);

        if (!ssid) {
            this.showMessage('请输入热点名称', 'error');
            return;
        }

        if (password.length < 8) {
            this.showMessage('密码长度至少8位', 'error');
            return;
        }

        this.showMessage('正在启用热点...', 'info');

        const result = await this.apiCall('/api/hotspot/control', 'POST', {
            enable: true,
            ssid: ssid,
            password: password,
            max_connections: maxConnections
        });

        if (result.success) {
            this.showMessage('热点启用成功！', 'success');
        } else {
            this.showMessage(`启用热点失败: ${result.error || '未知错误'}`, 'error');
        }

        setTimeout(() => this.updateStatus(), 2000);
    }

    async disableHotspot() {
        this.showMessage('正在禁用热点...', 'info');

        const result = await this.apiCall('/api/hotspot/control', 'POST', {
            enable: false
        });

        if (result.success) {
            this.showMessage('热点已禁用', 'success');
        } else {
            this.showMessage(`禁用热点失败: ${result.error || '未知错误'}`, 'error');
        }

        setTimeout(() => this.updateStatus(), 2000);
    }

    async connectWiFi() {
        const ssid = document.getElementById('wifi-ssid').value.trim();
        const password = document.getElementById('wifi-password').value;

        if (!ssid) {
            alert('请输入WiFi名称');
            return;
        }

        const result = await this.apiCall('/api/wifi/connect', 'POST', {
            ssid: ssid,
            password: password
        });

        if (result.success) {
            alert('WiFi连接成功！');
        } else {
            alert(`WiFi连接失败: ${result.error || '未知错误'}`);
        }

        setTimeout(() => this.updateStatus(), 3000);
    }

    async resetML307R() {
        if (!confirm('确定要重启ML307R模块吗？')) {
            return;
        }

        const result = await this.apiCall('/api/ml307r/reset', 'POST');

        if (result.success) {
            alert('ML307R重启成功！');
        } else {
            alert(`ML307R重启失败: ${result.error || '未知错误'}`);
        }

        setTimeout(() => this.updateStatus(), 5000);
    }

    showMessage(message, type = 'info') {
        const statusDiv = document.getElementById('hotspot-status');
        statusDiv.textContent = message;
        statusDiv.className = `status-message ${type}`;
        
        if (type === 'success' || type === 'error') {
            setTimeout(() => {
                statusDiv.textContent = '';
                statusDiv.className = 'status-message';
            }, 5000);
        }
    }

    formatBytes(bytes) {
        if (bytes === 0) return '0 B';
        const k = 1024;
        const sizes = ['B', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }

    formatUptime(seconds) {
        const days = Math.floor(seconds / 86400);
        const hours = Math.floor((seconds % 86400) / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        
        if (days > 0) {
            return `${days}天 ${hours}小时 ${minutes}分钟`;
        } else if (hours > 0) {
            return `${hours}小时 ${minutes}分钟`;
        } else {
            return `${minutes}分钟`;
        }
    }

    startStatusUpdates() {
        // 每30秒更新一次状态
        setInterval(() => {
            this.updateStatus();
        }, 30000);
    }
}

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', () => {
    new HotspotController();
});
)";

// 文件结束标记（由链接器自动生成）
const char web_index_html_end[] = "";
const char web_style_css_end[] = "";
const char web_script_js_end[] = "";
