#!/usr/bin/env node
/**
 * ESP32 LED控制器 Node.js Web服务器
 * 支持本地Web界面托管和跨域请求
 */

const http = require('http');
const fs = require('fs');
const path = require('path');
const url = require('url');
const { exec } = require('child_process');

class ESP32WebServer {
    constructor(port = 8080) {
        this.port = port;
        this.webDir = __dirname;
        this.mimeTypes = {
            '.html': 'text/html',
            '.css': 'text/css',
            '.js': 'application/javascript',
            '.json': 'application/json',
            '.png': 'image/png',
            '.jpg': 'image/jpeg',
            '.gif': 'image/gif',
            '.svg': 'image/svg+xml',
            '.ico': 'image/x-icon'
        };
    }

    /**
     * 获取文件的MIME类型
     */
    getMimeType(filePath) {
        const ext = path.extname(filePath).toLowerCase();
        return this.mimeTypes[ext] || 'application/octet-stream';
    }

    /**
     * 设置CORS头
     */
    setCORSHeaders(res) {
        res.setHeader('Access-Control-Allow-Origin', '*');
        res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
        res.setHeader('Access-Control-Allow-Headers', 'Content-Type, Authorization');
        res.setHeader('Cache-Control', 'no-store, no-cache, must-revalidate');
    }

    /**
     * 处理文件请求
     */
    serveFile(req, res, filePath) {
        fs.readFile(filePath, (err, data) => {
            if (err) {
                res.writeHead(404, {'Content-Type': 'text/plain'});
                res.end('File not found');
                return;
            }

            const mimeType = this.getMimeType(filePath);
            this.setCORSHeaders(res);
            res.writeHead(200, {'Content-Type': mimeType});
            res.end(data);
        });
    }

    /**
     * 处理API代理请求（可选功能）
     */
    handleProxyRequest(req, res, pathname) {
        // 解析代理URL: /proxy/192.168.4.1/api/status
        const parts = pathname.split('/').filter(part => part);
        
        if (parts.length >= 3 && parts[0] === 'proxy') {
            const espIP = parts[1];
            const apiPath = '/' + parts.slice(2).join('/');
            
            // 这里可以添加实际的代理逻辑
            this.setCORSHeaders(res);
            res.writeHead(200, {'Content-Type': 'application/json'});
            res.end(JSON.stringify({
                status: 'proxy_not_implemented',
                message: `代理请求到 ${espIP}${apiPath}`,
                note: '直接连接ESP32设备即可，无需代理'
            }, null, 2));
            
            return;
        }

        res.writeHead(400, {'Content-Type': 'text/plain'});
        res.end('Invalid proxy URL');
    }

    /**
     * 处理HTTP请求
     */
    handleRequest(req, res) {
        const parsedUrl = url.parse(req.url);
        let pathname = parsedUrl.pathname;

        // 记录请求
        const timestamp = new Date().toLocaleString();
        console.log(`[${timestamp}] ${req.method} ${pathname}`);

        // 处理OPTIONS预检请求
        if (req.method === 'OPTIONS') {
            this.setCORSHeaders(res);
            res.writeHead(200);
            res.end();
            return;
        }

        // 处理代理请求
        if (pathname.startsWith('/proxy/')) {
            this.handleProxyRequest(req, res, pathname);
            return;
        }

        // 默认重定向到index.html
        if (pathname === '/') {
            pathname = '/index.html';
        }

        // 构建文件路径
        const filePath = path.join(this.webDir, pathname);

        // 安全检查：防止路径遍历攻击
        if (!filePath.startsWith(this.webDir)) {
            res.writeHead(403, {'Content-Type': 'text/plain'});
            res.end('Forbidden');
            return;
        }

        // 检查文件是否存在
        fs.stat(filePath, (err, stats) => {
            if (err || !stats.isFile()) {
                res.writeHead(404, {'Content-Type': 'text/plain'});
                res.end('File not found');
                return;
            }

            this.serveFile(req, res, filePath);
        });
    }

    /**
     * 查找可用端口
     */
    async findFreePort(startPort = 8080, maxAttempts = 10) {
        const net = require('net');
        
        for (let port = startPort; port < startPort + maxAttempts; port++) {
            if (await this.isPortFree(port)) {
                return port;
            }
        }
        
        throw new Error(`无法找到可用端口 (尝试了 ${startPort}-${startPort + maxAttempts - 1})`);
    }

    /**
     * 检查端口是否可用
     */
    isPortFree(port) {
        return new Promise((resolve) => {
            const server = require('net').createServer();
            server.on('error', () => resolve(false));
            server.on('listening', () => {
                server.close();
                resolve(true);
            });
            server.listen(port, '127.0.0.1');
        });
    }

    /**
     * 打开浏览器
     */
    openBrowser(url) {
        const start = (process.platform === 'darwin' ? 'open' : 
                      process.platform === 'win32' ? 'start' : 'xdg-open');
        
        setTimeout(() => {
            console.log(`\n🌐 正在打开浏览器: ${url}`);
            exec(`${start} ${url}`, (err) => {
                if (err) {
                    console.log(`❌ 无法自动打开浏览器: ${err.message}`);
                    console.log(`   请手动访问: ${url}`);
                }
            });
        }, 2000);
    }

    /**
     * 启动服务器
     */
    async start() {
        try {
            // 查找可用端口
            this.port = await this.findFreePort(this.port);
            
            // 创建HTTP服务器
            const server = http.createServer((req, res) => {
                this.handleRequest(req, res);
            });

            // 启动服务器
            server.listen(this.port, () => {
                const serverUrl = `http://localhost:${this.port}`;
                
                console.log('🚀 ESP32 LED控制器 Web服务器');
                console.log('='.repeat(50));
                console.log(`✅ Web服务器已启动`);
                console.log(`📍 服务地址: ${serverUrl}`);
                console.log(`📁 服务目录: ${this.webDir}`);
                console.log(`🌐 访问界面: ${serverUrl}/index.html`);
                console.log();
                console.log('💡 使用说明:');
                console.log('   1. 确保ESP32设备已连接并运行');
                console.log('   2. 在界面中选择设备类型 (ESP32-C3/ESP32-S3)');
                console.log('   3. 输入ESP32设备的IP地址');
                console.log('   4. 点击\'连接设备\'开始控制LED');
                console.log();
                console.log('🔧 ESP32设备常见IP地址:');
                console.log('   - AP模式: 192.168.4.1');
                console.log('   - STA模式: 查看串口输出获取实际IP');
                console.log();
                console.log('⏹️  按 Ctrl+C 停止服务器');
                console.log('='.repeat(50));

                // 自动打开浏览器
                this.openBrowser(`${serverUrl}/index.html`);
            });

            // 处理服务器错误
            server.on('error', (err) => {
                console.error(`❌ 服务器错误: ${err.message}`);
                process.exit(1);
            });

            // 处理进程退出
            process.on('SIGINT', () => {
                console.log('\n\n⏹️  服务器已停止');
                server.close();
                process.exit(0);
            });

        } catch (error) {
            console.error(`❌ 服务器启动失败: ${error.message}`);
            process.exit(1);
        }
    }
}

// 启动服务器
if (require.main === module) {
    const server = new ESP32WebServer();
    server.start();
}

module.exports = ESP32WebServer;