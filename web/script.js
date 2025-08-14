// ESP32 LED控制器 Web客户端
class ESP32Controller {
    constructor() {
        this.deviceIP = '10.30.6.226';
        this.deviceType = 'esp32c3';
        this.isConnected = false;
        this.currentEffect = 'static';
        this.updateInterval = null;
        
        // 设备配置
        this.deviceConfigs = {
            esp32c3: {
                name: 'ESP32-C3',
                description: 'RISC-V 160MHz',
                defaultIP: '10.30.6.226',
                features: ['WiFi', 'RGB LED', 'Web API', 'PWM控制']
            },
            esp32s3: {
                name: 'ESP32-S3',
                description: 'Xtensa 240MHz',
                defaultIP: '10.30.6.226',
                features: ['WiFi', 'RGB LED', 'Web API', 'PWM控制', '摄像头']
            }
        };
        
        this.init();
    }
    
    init() {
        this.updateDeviceInfo();
        this.updateColorPreview();
        this.addLogEntry('Web控制器初始化完成');
        
        // 自动尝试连接
        setTimeout(() => {
            this.connectDevice();
        }, 1000);
    }
    
    // 设备连接
    async connectDevice() {
        const connectBtn = document.getElementById('connectBtn');
        const statusIndicator = document.getElementById('connectionStatus');
        
        try {
            connectBtn.textContent = '🔄 连接中...';
            connectBtn.disabled = true;
            
            this.deviceIP = document.getElementById('deviceIP').value;
            this.addLogEntry(`尝试连接到 ${this.deviceIP}`);
            
            // 获取设备状态
            const response = await this.apiRequest('GET', '/api/status');
            
            if (response.ok) {
                const data = await response.json();
                this.isConnected = true;
                
                // 更新连接状态
                statusIndicator.querySelector('.status-dot').classList.add('connected');
                statusIndicator.querySelector('.status-text').textContent = '已连接';
                
                // 显示设备信息
                this.displayDeviceInfo(data);
                
                // 开始定期更新
                this.startStatusUpdate();
                
                connectBtn.textContent = '✅ 已连接';
                this.addLogEntry(`成功连接到 ${this.deviceIP}`);
                this.showToast('设备连接成功', 'success');
                
                // 获取当前LED状态
                await this.updateLEDStatus();
                
            } else {
                throw new Error('连接失败');
            }
        } catch (error) {
            this.isConnected = false;
            statusIndicator.querySelector('.status-dot').classList.remove('connected');
            statusIndicator.querySelector('.status-text').textContent = '连接失败';
            
            connectBtn.textContent = '🔗 重新连接';
            this.addLogEntry(`连接失败: ${error.message}`, 'error');
            this.showToast('设备连接失败，请检查IP地址和网络', 'error');
        } finally {
            connectBtn.disabled = false;
        }
    }
    
    // 扫描设备
    async scanDevices() {
        const scanBtn = document.getElementById('scanBtn');
        scanBtn.textContent = '🔍 扫描中...';
        scanBtn.disabled = true;
        
        this.addLogEntry('开始扫描本地网络设备...');
        
        // 扫描常见IP地址
        const commonIPs = [
            '192.168.4.1',    // AP模式默认IP
            '192.168.1.100',  // STA模式常见IP
            '192.168.1.101',
            '192.168.0.100',
            '192.168.0.101'
        ];
        
        let foundDevices = [];
        
        for (const ip of commonIPs) {
            try {
                const response = await fetch(`http://${ip}/api/status`, {
                    method: 'GET',
                    timeout: 3000
                });
                
                if (response.ok) {
                    const data = await response.json();
                    foundDevices.push({
                        ip: ip,
                        data: data
                    });
                    this.addLogEntry(`发现设备: ${ip}`);
                }
            } catch (error) {
                // 忽略连接失败的IP
            }
        }
        
        if (foundDevices.length > 0) {
            // 使用第一个找到的设备
            const device = foundDevices[0];
            document.getElementById('deviceIP').value = device.ip;
            this.showToast(`发现 ${foundDevices.length} 个设备，已选择 ${device.ip}`, 'success');
            
            // 自动连接
            setTimeout(() => {
                this.connectDevice();
            }, 1000);
        } else {
            this.addLogEntry('未发现任何设备');
            this.showToast('未发现任何设备，请检查网络连接', 'warning');
        }
        
        scanBtn.textContent = '🔍 扫描设备';
        scanBtn.disabled = false;
    }
    
    // API请求
    async apiRequest(method, endpoint, data = null) {
        const url = `http://${this.deviceIP}${endpoint}`;
        const options = {
            method: method,
            headers: {
                'Content-Type': 'application/json',
            }
        };
        
        if (data) {
            options.body = JSON.stringify(data);
        }
        
        return fetch(url, options);
    }
    
    // 显示设备信息
    displayDeviceInfo(data) {
        const deviceInfo = document.getElementById('deviceInfo');
        deviceInfo.style.display = 'block';
        
        document.getElementById('chipModel').textContent = 
            this.deviceConfigs[this.deviceType].name;
        document.getElementById('firmwareVersion').textContent = 
            data.version || 'v2.0.0';
        document.getElementById('uptime').textContent = 
            this.formatUptime(data.uptime || 0);
        document.getElementById('memoryUsage').textContent = 
            `${data.memory_used || 0}KB / ${data.memory_total || 0}KB`;
        document.getElementById('wifiStatus').textContent = 
            data.wifi_connected ? '已连接' : 'AP模式';
        document.getElementById('currentIP').textContent = 
            data.ip_address || this.deviceIP;
    }
    
    // 格式化运行时间
    formatUptime(seconds) {
        const hours = Math.floor(seconds / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const secs = seconds % 60;
        return `${hours}h ${minutes}m ${secs}s`;
    }
    
    // 开始状态更新
    startStatusUpdate() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
        }
        
        this.updateInterval = setInterval(async () => {
            if (this.isConnected) {
                try {
                    const response = await this.apiRequest('GET', '/api/status');
                    if (response.ok) {
                        const data = await response.json();
                        this.displayDeviceInfo(data);
                    }
                } catch (error) {
                    console.warn('状态更新失败:', error);
                }
            }
        }, 5000);
    }
    
    // 更新设备信息
    updateDeviceInfo() {
        const deviceType = document.getElementById('deviceType').value;
        const deviceIP = document.getElementById('deviceIP');
        
        this.deviceType = deviceType;
        const config = this.deviceConfigs[deviceType];
        deviceIP.value = config.defaultIP;
        
        this.addLogEntry(`切换到设备类型: ${config.name}`);
    }
    
    // LED控制方法
    async togglePower() {
        const powerSwitch = document.getElementById('powerSwitch');
        const power = powerSwitch.checked;
        
        try {
            const response = await this.apiRequest('POST', '/api/led/power', {
                power: power
            });
            
            if (response.ok) {
                this.addLogEntry(`LED电源: ${power ? '开启' : '关闭'}`);
                this.showToast(`LED已${power ? '开启' : '关闭'}`, 'success');
            } else {
                throw new Error('电源控制失败');
            }
        } catch (error) {
            this.addLogEntry(`电源控制失败: ${error.message}`, 'error');
            this.showToast('电源控制失败', 'error');
            powerSwitch.checked = !power; // 回滚状态
        }
    }
    
    async updateColor() {
        const colorPicker = document.getElementById('colorPicker');
        const hex = colorPicker.value;
        
        // 转换HEX到RGB
        const rgb = this.hexToRgb(hex);
        
        // 更新滑块
        document.getElementById('redSlider').value = rgb.r;
        document.getElementById('greenSlider').value = rgb.g;
        document.getElementById('blueSlider').value = rgb.b;
        
        // 更新显示值
        document.getElementById('redValue').textContent = rgb.r;
        document.getElementById('greenValue').textContent = rgb.g;
        document.getElementById('blueValue').textContent = rgb.b;
        
        this.updateColorPreview();
        await this.sendColorUpdate();
    }
    
    async updateRGB() {
        const red = parseInt(document.getElementById('redSlider').value);
        const green = parseInt(document.getElementById('greenSlider').value);
        const blue = parseInt(document.getElementById('blueSlider').value);
        
        // 更新显示值
        document.getElementById('redValue').textContent = red;
        document.getElementById('greenValue').textContent = green;
        document.getElementById('blueValue').textContent = blue;
        
        // 更新颜色选择器
        const hex = this.rgbToHex(red, green, blue);
        document.getElementById('colorPicker').value = hex;
        
        this.updateColorPreview();
        await this.sendColorUpdate();
    }
    
    async updateBrightness() {
        const brightness = parseInt(document.getElementById('brightnessSlider').value);
        document.getElementById('brightnessValue').textContent = brightness + '%';
        
        await this.sendColorUpdate();
    }
    
    async sendColorUpdate() {
        if (!this.isConnected) return;
        
        const red = parseInt(document.getElementById('redSlider').value);
        const green = parseInt(document.getElementById('greenSlider').value);
        const blue = parseInt(document.getElementById('blueSlider').value);
        const brightness = parseInt(document.getElementById('brightnessSlider').value);
        
        try {
            const response = await this.apiRequest('POST', '/api/led/color', {
                red: red,
                green: green,
                blue: blue,
                brightness: brightness
            });
            
            if (response.ok) {
                this.addLogEntry(`颜色更新: RGB(${red}, ${green}, ${blue}) 亮度: ${brightness}%`);
            }
        } catch (error) {
            this.addLogEntry(`颜色更新失败: ${error.message}`, 'error');
        }
    }
    
    updateColorPreview() {
        const red = parseInt(document.getElementById('redSlider').value);
        const green = parseInt(document.getElementById('greenSlider').value);
        const blue = parseInt(document.getElementById('blueSlider').value);
        
        const color = `rgb(${red}, ${green}, ${blue})`;
        document.getElementById('colorPreview').style.background = color;
    }
    
    async setEffect(effect) {
        // 更新按钮状态
        document.querySelectorAll('.effect-btn').forEach(btn => {
            btn.classList.remove('active');
        });
        document.querySelector(`[data-effect="${effect}"]`).classList.add('active');
        
        this.currentEffect = effect;
        
        // 显示/隐藏速度控制
        const speedControl = document.getElementById('effectSpeedControl');
        if (effect === 'static') {
            speedControl.style.display = 'none';
        } else {
            speedControl.style.display = 'block';
        }
        
        try {
            const speed = parseInt(document.getElementById('speedSlider').value);
            const response = await this.apiRequest('POST', '/api/led/effect', {
                effect: effect,
                speed: speed
            });
            
            if (response.ok) {
                this.addLogEntry(`特效切换: ${effect}`);
                this.showToast(`已切换到${this.getEffectName(effect)}模式`, 'success');
            }
        } catch (error) {
            this.addLogEntry(`特效设置失败: ${error.message}`, 'error');
            this.showToast('特效设置失败', 'error');
        }
    }
    
    async updateEffectSpeed() {
        const speed = parseInt(document.getElementById('speedSlider').value);
        document.getElementById('speedValue').textContent = speed;
        
        if (this.currentEffect !== 'static') {
            try {
                await this.apiRequest('POST', '/api/led/effect', {
                    effect: this.currentEffect,
                    speed: speed
                });
                
                this.addLogEntry(`特效速度: ${speed}`);
            } catch (error) {
                this.addLogEntry(`特效速度设置失败: ${error.message}`, 'error');
            }
        }
    }
    
    getEffectName(effect) {
        const names = {
            'static': '静态',
            'rainbow': '彩虹',
            'breathing': '呼吸',
            'blink': '闪烁'
        };
        return names[effect] || effect;
    }
    
    async setPresetColor(red, green, blue) {
        // 更新滑块
        document.getElementById('redSlider').value = red;
        document.getElementById('greenSlider').value = green;
        document.getElementById('blueSlider').value = blue;
        
        // 更新显示
        await this.updateRGB();
        
        this.addLogEntry(`预设颜色: RGB(${red}, ${green}, ${blue})`);
    }
    
    // 获取当前LED状态
    async updateLEDStatus() {
        try {
            const response = await this.apiRequest('GET', '/api/led/status');
            if (response.ok) {
                const data = await response.json();
                
                // 更新界面状态
                document.getElementById('powerSwitch').checked = data.power || false;
                document.getElementById('redSlider').value = data.red || 0;
                document.getElementById('greenSlider').value = data.green || 0;
                document.getElementById('blueSlider').value = data.blue || 0;
                document.getElementById('brightnessSlider').value = data.brightness || 100;
                
                // 更新显示
                this.updateRGB();
                document.getElementById('brightnessValue').textContent = 
                    (data.brightness || 100) + '%';
                
                // 更新特效状态
                if (data.effect) {
                    this.setEffect(data.effect);
                }
            }
        } catch (error) {
            console.warn('获取LED状态失败:', error);
        }
    }
    
    // 工具方法
    hexToRgb(hex) {
        const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
        return result ? {
            r: parseInt(result[1], 16),
            g: parseInt(result[2], 16),
            b: parseInt(result[3], 16)
        } : null;
    }
    
    rgbToHex(r, g, b) {
        return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
    }
    
    // 日志管理
    addLogEntry(message, type = 'info') {
        const logContent = document.getElementById('logContent');
        const timestamp = new Date().toLocaleTimeString();
        
        const logEntry = document.createElement('div');
        logEntry.className = 'log-entry';
        logEntry.innerHTML = `
            <span class="timestamp">[${timestamp}]</span>
            <span class="message">${message}</span>
        `;
        
        logContent.appendChild(logEntry);
        logContent.scrollTop = logContent.scrollHeight;
        
        // 限制日志条数
        while (logContent.children.length > 100) {
            logContent.removeChild(logContent.firstChild);
        }
    }
    
    clearLog() {
        document.getElementById('logContent').innerHTML = '';
        this.addLogEntry('日志已清空');
    }
    
    exportLog() {
        const logContent = document.getElementById('logContent');
        const entries = Array.from(logContent.children).map(entry => 
            entry.textContent
        ).join('\n');
        
        const blob = new Blob([entries], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        
        const a = document.createElement('a');
        a.href = url;
        a.download = `esp32-log-${new Date().toISOString().slice(0, 19)}.txt`;
        a.click();
        
        URL.revokeObjectURL(url);
        this.addLogEntry('日志已导出');
    }
    
    // 提示消息
    showToast(message, type = 'success') {
        const toast = document.getElementById('toast');
        toast.textContent = message;
        toast.className = `toast ${type} show`;
        
        setTimeout(() => {
            toast.classList.remove('show');
        }, 3000);
    }
}

// 全局函数（供HTML调用）
let controller;

function init() {
    controller = new ESP32Controller();
}

function updateDeviceInfo() {
    controller.updateDeviceInfo();
}

function connectDevice() {
    controller.connectDevice();
}

function scanDevices() {
    controller.scanDevices();
}

function togglePower() {
    controller.togglePower();
}

function updateColor() {
    controller.updateColor();
}

function updateRGB() {
    controller.updateRGB();
}

function updateBrightness() {
    controller.updateBrightness();
}

function setEffect(effect) {
    controller.setEffect(effect);
}

function updateEffectSpeed() {
    controller.updateEffectSpeed();
}

function setPresetColor(red, green, blue) {
    controller.setPresetColor(red, green, blue);
}

function clearLog() {
    controller.clearLog();
}

function exportLog() {
    controller.exportLog();
}

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', init);