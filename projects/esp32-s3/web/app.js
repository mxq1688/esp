// PWA RGB LED 控制器
class RGBController {
    constructor() {
        this.espIP = '';
        this.isConnected = false;
        this.currentColor = { red: 255, green: 255, blue: 255, brightness: 50 };
        this.effectRunning = null;
        this.updateInterval = null;
        
        this.init();
    }

    async init() {
        this.setupEventListeners();
        this.setupPWA();
        this.loadSettings();
        this.updateUI();
        
        // 尝试自动连接
        if (this.espIP) {
            await this.connect();
        }
    }

    setupEventListeners() {
        // 颜色选择器
        document.getElementById('colorPicker').addEventListener('change', (e) => {
            this.setColorFromHex(e.target.value);
        });

        // RGB滑块
        ['red', 'green', 'blue'].forEach(color => {
            const slider = document.getElementById(`${color}Slider`);
            const value = document.getElementById(`${color}Value`);
            
            slider.addEventListener('input', (e) => {
                const val = parseInt(e.target.value);
                value.textContent = val;
                this.currentColor[color] = val;
                this.updateColorPreview();
                this.sendColorToESP();
            });
        });

        // 亮度滑块
        const brightnessSlider = document.getElementById('brightnessSlider');
        const brightnessValue = document.getElementById('brightnessValue');
        
        brightnessSlider.addEventListener('input', (e) => {
            const val = parseInt(e.target.value);
            brightnessValue.textContent = val;
            this.currentColor.brightness = val;
            this.updateColorPreview();
            this.sendColorToESP();
        });

        // 预设颜色按钮
        document.querySelectorAll('.preset-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                this.setColorFromHex(e.target.dataset.color);
            });
        });

        // 特效按钮
        document.getElementById('rainbowBtn').addEventListener('click', () => this.startRainbowEffect());
        document.getElementById('breatheBtn').addEventListener('click', () => this.startBreatheEffect());
        document.getElementById('strobeBtn').addEventListener('click', () => this.startStrobeEffect());
        document.getElementById('stopEffectBtn').addEventListener('click', () => this.stopEffect());

        // 快速控制按钮
        document.getElementById('turnOffBtn').addEventListener('click', () => this.turnOff());
        document.getElementById('turnOnBtn').addEventListener('click', () => this.turnOn());
        document.getElementById('maxBrightnessBtn').addEventListener('click', () => this.maxBrightness());
        document.getElementById('dimBtn').addEventListener('click', () => this.dimMode());

        // 连接按钮
        document.getElementById('connectBtn').addEventListener('click', () => this.connectToESP());

        // 自动更新开关
        document.getElementById('autoUpdateCheckbox').addEventListener('change', (e) => {
            if (e.target.checked) {
                this.startAutoUpdate();
            } else {
                this.stopAutoUpdate();
            }
        });

        // 自动效果开关
        document.getElementById('autoEffectCheckbox').addEventListener('change', (e) => {
            this.toggleAutoEffect(e.target.checked);
        });
    }

    setupPWA() {
        // 注册 Service Worker
        if ('serviceWorker' in navigator) {
            navigator.serviceWorker.register('./sw.js')
                .then(registration => console.log('SW registered:', registration))
                .catch(error => console.log('SW registration failed:', error));
        }

        // PWA 安装提示
        let deferredPrompt;
        window.addEventListener('beforeinstallprompt', (e) => {
            e.preventDefault();
            deferredPrompt = e;
            
            const installPrompt = document.getElementById('installPrompt');
            installPrompt.style.display = 'block';

            document.getElementById('installBtn').addEventListener('click', async () => {
                if (deferredPrompt) {
                    deferredPrompt.prompt();
                    const { outcome } = await deferredPrompt.userChoice;
                    console.log('PWA install outcome:', outcome);
                    deferredPrompt = null;
                    installPrompt.style.display = 'none';
                }
            });

            document.getElementById('dismissBtn').addEventListener('click', () => {
                installPrompt.style.display = 'none';
            });
        });
    }

    loadSettings() {
        const savedIP = localStorage.getItem('espIP');
        if (savedIP) {
            this.espIP = savedIP;
            document.getElementById('espIpInput').value = savedIP;
        }

        const savedColor = localStorage.getItem('lastColor');
        if (savedColor) {
            this.currentColor = JSON.parse(savedColor);
        }
    }

    saveSettings() {
        localStorage.setItem('espIP', this.espIP);
        localStorage.setItem('lastColor', JSON.stringify(this.currentColor));
    }

    async connectToESP() {
        const ipInput = document.getElementById('espIpInput');
        this.espIP = ipInput.value.trim();
        
        if (!this.espIP) {
            this.showStatus('请输入ESP32-S3的IP地址', 'error');
            return;
        }

        await this.connect();
    }

    async connect() {
        if (!this.espIP) return;

        this.showStatus('正在连接到ESP32-S3...', 'connecting');
        
        try {
            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), 5000);
            
            const response = await fetch(`http://${this.espIP}/api/status`, {
                method: 'GET',
                signal: controller.signal,
                mode: 'cors',
                headers: {
                    'Content-Type': 'application/json',
                }
            });
            
            clearTimeout(timeoutId);

            if (response.ok) {
                const data = await response.json();
                this.isConnected = true;
                this.currentColor = data.color;
                this.updateUI();
                this.showStatus(`已连接到 ESP32-S3 (${this.espIP})`, 'connected');
                this.saveSettings();
                
                if (document.getElementById('autoUpdateCheckbox').checked) {
                    this.startAutoUpdate();
                }
                
                // 获取自动效果状态
                await this.getAutoEffectStatus();
            } else {
                throw new Error('连接失败');
            }
        } catch (error) {
            this.isConnected = false;
            this.showStatus(`连接失败: ${error.message}`, 'error');
            this.updateConnectionStatus();
        }
    }

    async sendColorToESP() {
        if (!this.isConnected || !this.espIP) return;

        try {
            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), 5000);
            
            const response = await fetch(`http://${this.espIP}/api/color`, {
                method: 'POST',
                signal: controller.signal,
                mode: 'cors',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(this.currentColor)
            });
            
            clearTimeout(timeoutId);

            if (!response.ok) {
                throw new Error('发送颜色数据失败');
            }

            this.saveSettings();
        } catch (error) {
            console.error('发送颜色失败:', error);
            this.showStatus('发送颜色失败', 'error');
        }
    }

    setColorFromHex(hex) {
        const r = parseInt(hex.substr(1, 2), 16);
        const g = parseInt(hex.substr(3, 2), 16);
        const b = parseInt(hex.substr(5, 2), 16);

        this.currentColor.red = r;
        this.currentColor.green = g;
        this.currentColor.blue = b;

        this.updateUI();
        this.sendColorToESP();
    }

    updateUI() {
        // 更新滑块值
        document.getElementById('redSlider').value = this.currentColor.red;
        document.getElementById('greenSlider').value = this.currentColor.green;
        document.getElementById('blueSlider').value = this.currentColor.blue;
        document.getElementById('brightnessSlider').value = this.currentColor.brightness;

        // 更新数值显示
        document.getElementById('redValue').textContent = this.currentColor.red;
        document.getElementById('greenValue').textContent = this.currentColor.green;
        document.getElementById('blueValue').textContent = this.currentColor.blue;
        document.getElementById('brightnessValue').textContent = this.currentColor.brightness;

        // 更新颜色选择器
        const hex = this.rgbToHex(this.currentColor.red, this.currentColor.green, this.currentColor.blue);
        document.getElementById('colorPicker').value = hex;

        // 更新颜色预览
        this.updateColorPreview();
        this.updateConnectionStatus();
    }

    updateColorPreview() {
        const preview = document.getElementById('colorPreview');
        const { red, green, blue, brightness } = this.currentColor;
        
        // 应用亮度
        const adjustedR = Math.floor((red * brightness) / 100);
        const adjustedG = Math.floor((green * brightness) / 100);
        const adjustedB = Math.floor((blue * brightness) / 100);
        
        preview.style.background = `rgb(${adjustedR}, ${adjustedG}, ${adjustedB})`;
    }

    updateConnectionStatus() {
        const statusIndicator = document.querySelector('.status-indicator');
        const statusText = document.querySelector('.status-text');

        if (this.isConnected) {
            statusIndicator.classList.add('connected');
            statusText.textContent = '已连接';
        } else {
            statusIndicator.classList.remove('connected');
            statusText.textContent = '未连接';
        }
    }

    showStatus(message, type = 'info') {
        const statusInfo = document.getElementById('statusInfo');
        statusInfo.innerHTML = `<p>${message}</p>`;
        
        // 添加类型样式
        statusInfo.className = `status-info ${type}`;
        
        // 自动清除消息
        setTimeout(() => {
            if (statusInfo.textContent === message) {
                statusInfo.innerHTML = '<p>等待操作...</p>';
                statusInfo.className = 'status-info';
            }
        }, 3000);
    }

    rgbToHex(r, g, b) {
        return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
    }

    // 特效方法
    startRainbowEffect() {
        this.stopEffect();
        this.effectRunning = 'rainbow';
        this.setActiveEffect('rainbowBtn');

        let hue = 0;
        this.effectInterval = setInterval(() => {
            const rgb = this.hslToRgb(hue / 360, 1, 0.5);
            this.currentColor.red = Math.floor(rgb.r * 255);
            this.currentColor.green = Math.floor(rgb.g * 255);
            this.currentColor.blue = Math.floor(rgb.b * 255);
            
            this.updateUI();
            this.sendColorToESP();
            
            hue = (hue + 5) % 360;
        }, 100);
    }

    startBreatheEffect() {
        this.stopEffect();
        this.effectRunning = 'breathe';
        this.setActiveEffect('breatheBtn');

        let direction = 1;
        this.effectInterval = setInterval(() => {
            this.currentColor.brightness += direction * 2;
            
            if (this.currentColor.brightness <= 5) {
                direction = 1;
                this.currentColor.brightness = 5;
            } else if (this.currentColor.brightness >= 100) {
                direction = -1;
                this.currentColor.brightness = 100;
            }
            
            this.updateUI();
            this.sendColorToESP();
        }, 50);
    }

    startStrobeEffect() {
        this.stopEffect();
        this.effectRunning = 'strobe';
        this.setActiveEffect('strobeBtn');

        let isOn = true;
        this.effectInterval = setInterval(() => {
            this.currentColor.brightness = isOn ? 100 : 0;
            isOn = !isOn;
            
            this.updateUI();
            this.sendColorToESP();
        }, 200);
    }

    stopEffect() {
        if (this.effectInterval) {
            clearInterval(this.effectInterval);
            this.effectInterval = null;
        }
        this.effectRunning = null;
        this.clearActiveEffect();
    }

    setActiveEffect(buttonId) {
        this.clearActiveEffect();
        document.getElementById(buttonId).classList.add('active');
    }

    clearActiveEffect() {
        document.querySelectorAll('.effect-btn').forEach(btn => {
            btn.classList.remove('active');
        });
    }

    // 快速控制方法
    turnOff() {
        this.stopEffect();
        this.currentColor.brightness = 0;
        this.updateUI();
        this.sendColorToESP();
    }

    turnOn() {
        this.stopEffect();
        this.currentColor.brightness = 50;
        this.updateUI();
        this.sendColorToESP();
    }

    maxBrightness() {
        this.stopEffect();
        this.currentColor.brightness = 100;
        this.updateUI();
        this.sendColorToESP();
    }

    dimMode() {
        this.stopEffect();
        this.currentColor.brightness = 10;
        this.currentColor.red = 255;
        this.currentColor.green = 200;
        this.currentColor.blue = 100;
        this.updateUI();
        this.sendColorToESP();
    }

    // 自动更新状态
    startAutoUpdate() {
        if (this.updateInterval) return;
        
        this.updateInterval = setInterval(async () => {
            if (this.isConnected && this.espIP && !this.effectRunning) {
                try {
                    const response = await fetch(`http://${this.espIP}/api/status`);
                    if (response.ok) {
                        const data = await response.json();
                        if (JSON.stringify(data.color) !== JSON.stringify(this.currentColor)) {
                            this.currentColor = data.color;
                            this.updateUI();
                        }
                    }
                } catch (error) {
                    console.error('自动更新失败:', error);
                }
            }
        }, 2000);
    }

    stopAutoUpdate() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
            this.updateInterval = null;
        }
    }

    // 切换自动效果
    async toggleAutoEffect(enabled) {
        if (!this.isConnected || !this.espIP) {
            this.showStatus('请先连接ESP32-S3', 'error');
            return;
        }

        try {
            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), 5000);
            
            const response = await fetch(`http://${this.espIP}/api/effect`, {
                method: 'POST',
                signal: controller.signal,
                mode: 'cors',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ enabled: enabled })
            });
            
            clearTimeout(timeoutId);

            if (response.ok) {
                this.updateAutoEffectStatus(enabled);
                this.showStatus(`自动效果已${enabled ? '启用' : '禁用'}`, 'success');
            } else {
                throw new Error('设置自动效果失败');
            }
        } catch (error) {
            console.error('切换自动效果失败:', error);
            this.showStatus('设置自动效果失败', 'error');
            // 恢复复选框状态
            document.getElementById('autoEffectCheckbox').checked = !enabled;
        }
    }

    // 更新自动效果状态显示
    updateAutoEffectStatus(enabled) {
        const statusElement = document.getElementById('autoEffectStatus');
        if (statusElement) {
            statusElement.textContent = enabled ? '已启用' : '已禁用';
            statusElement.style.color = enabled ? '#4CAF50' : '#ff6b35';
        }
    }

    // 获取自动效果状态
    async getAutoEffectStatus() {
        if (!this.isConnected || !this.espIP) return;

        try {
            const response = await fetch(`http://${this.espIP}/api/effect`);
            if (response.ok) {
                const data = await response.json();
                const checkbox = document.getElementById('autoEffectCheckbox');
                if (checkbox) {
                    checkbox.checked = data.enabled;
                    this.updateAutoEffectStatus(data.enabled);
                }
            }
        } catch (error) {
            console.error('获取自动效果状态失败:', error);
        }
    }

    // HSL 到 RGB 转换
    hslToRgb(h, s, l) {
        let r, g, b;

        if (s === 0) {
            r = g = b = l;
        } else {
            const hue2rgb = (p, q, t) => {
                if (t < 0) t += 1;
                if (t > 1) t -= 1;
                if (t < 1/6) return p + (q - p) * 6 * t;
                if (t < 1/2) return q;
                if (t < 2/3) return p + (q - p) * (2/3 - t) * 6;
                return p;
            };

            const q = l < 0.5 ? l * (1 + s) : l + s - l * s;
            const p = 2 * l - q;
            r = hue2rgb(p, q, h + 1/3);
            g = hue2rgb(p, q, h);
            b = hue2rgb(p, q, h - 1/3);
        }

        return { r, g, b };
    }
}

// 初始化应用
document.addEventListener('DOMContentLoaded', () => {
    window.rgbController = new RGBController();
});

// Service Worker 注册
if ('serviceWorker' in navigator) {
    window.addEventListener('load', () => {
        navigator.serviceWorker.register('./sw.js')
            .then(registration => {
                console.log('SW registered: ', registration);
            })
            .catch(registrationError => {
                console.log('SW registration failed: ', registrationError);
            });
    });
} 