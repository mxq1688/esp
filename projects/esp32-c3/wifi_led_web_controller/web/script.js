// 全局变量
let powerState = false;
let currentColor = { r: 255, g: 255, b: 255 };
let currentBrightness = 50;
let isConnected = true;
let isTouchDevice = false;

// AP模式控制 - 默认关闭
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
    
    // 检查连接状态
    checkConnection();
    
    // 定期检查连接状态
    setInterval(checkConnection, 5000);
    
    // 添加PWA支持
    setupPWA();
    
    // 获取AP状态
    getAPStatus();
    
    // 定期更新AP状态
    setInterval(getAPStatus, 10000); // 每10秒更新一次
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

// 添加一些额外的交互功能
document.addEventListener('DOMContentLoaded', function() {
    // 点击预览圆圈随机颜色
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
    
    // 双击重置为白色
    let clickCount = 0;
    let clickTimer;
    
    previewCircle.addEventListener('click', function() {
        clickCount++;
        if (clickCount === 1) {
            clickTimer = setTimeout(() => {
                clickCount = 0;
            }, 300);
        } else {
            clearTimeout(clickTimer);
            clickCount = 0;
            
            // 重置为白色
            document.getElementById('r').value = 255;
            document.getElementById('g').value = 255;
            document.getElementById('b').value = 255;
            document.getElementById('brightness').value = 100;
            
            updateColor();
            showNotification('已重置为白色', 'info');
        }
    });
});

// 添加键盘导航支持
document.addEventListener('keydown', function(event) {
    // Tab键导航优化
    if (event.key === 'Tab') {
        // 确保焦点可见
        document.body.classList.add('keyboard-navigation');
    }
});

// 鼠标导航时移除键盘导航样式
document.addEventListener('mousedown', function() {
    document.body.classList.remove('keyboard-navigation');
});

// 触摸导航时移除键盘导航样式
document.addEventListener('touchstart', function() {
    document.body.classList.remove('keyboard-navigation');
});

// AP模式控制
async function toggleAP() {
    try {
        const response = await fetch('/api/ap-mode', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                enable: !apEnabled
            })
        });

        const data = await response.json();
        
        if (data.status === 'success') {
            apEnabled = data.ap_enabled;
            updateAPUI();
            showNotification(
                apEnabled ? '热点已开启' : '热点已关闭', 
                'success'
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
