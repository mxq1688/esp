#include "web_interface.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "WEB_INTERFACE";

// Web服务器配置
static web_config_t web_config = {
    .port = 80,
    .hostname = "ai-assistant.local",
    .enable_ssl = false,
    .ssl_cert = "",
    .ssl_key = ""
};

// HTTP服务器句柄
static httpd_handle_t server = NULL;

// WebSocket回调
static ws_message_callback_t ws_callback = NULL;

// 简单的HTML页面
static const char *html_page = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AI小智助手</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: white;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 20px;
            padding: 30px;
            backdrop-filter: blur(10px);
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
        }
        .header {
            text-align: center;
            margin-bottom: 30px;
        }
        .header h1 {
            margin: 0;
            font-size: 2.5em;
            background: linear-gradient(45deg, #ff6b6b, #4ecdc4);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        .chat-container {
            height: 400px;
            overflow-y: auto;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 20px;
        }
        .message {
            margin-bottom: 15px;
            padding: 10px 15px;
            border-radius: 15px;
            max-width: 80%;
        }
        .user-message {
            background: linear-gradient(45deg, #667eea, #764ba2);
            margin-left: auto;
            text-align: right;
        }
        .ai-message {
            background: rgba(255, 255, 255, 0.2);
            margin-right: auto;
        }
        .input-container {
            display: flex;
            gap: 10px;
        }
        .chat-input {
            flex: 1;
            padding: 15px;
            border: none;
            border-radius: 25px;
            background: rgba(255, 255, 255, 0.2);
            color: white;
            font-size: 16px;
        }
        .chat-input::placeholder {
            color: rgba(255, 255, 255, 0.7);
        }
        .send-btn {
            padding: 15px 25px;
            border: none;
            border-radius: 25px;
            background: linear-gradient(45deg, #ff6b6b, #4ecdc4);
            color: white;
            font-size: 16px;
            cursor: pointer;
            transition: transform 0.2s;
        }
        .send-btn:hover {
            transform: scale(1.05);
        }
        .voice-btn {
            padding: 15px;
            border: none;
            border-radius: 50%;
            background: linear-gradient(45deg, #ff6b6b, #4ecdc4);
            color: white;
            font-size: 20px;
            cursor: pointer;
            transition: transform 0.2s;
        }
        .voice-btn:hover {
            transform: scale(1.1);
        }
        .status {
            text-align: center;
            margin-top: 20px;
            padding: 10px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 10px;
        }
        .controls {
            display: flex;
            justify-content: center;
            gap: 15px;
            margin-top: 20px;
        }
        .control-btn {
            padding: 10px 20px;
            border: none;
            border-radius: 15px;
            background: rgba(255, 255, 255, 0.2);
            color: white;
            cursor: pointer;
            transition: background 0.2s;
        }
        .control-btn:hover {
            background: rgba(255, 255, 255, 0.3);
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🤖 AI小智助手</h1>
            <p>您的智能语音助手</p>
        </div>
        
        <div class="chat-container" id="chatContainer">
            <div class="message ai-message">
                你好！我是小智，很高兴为您服务。有什么可以帮助您的吗？
            </div>
        </div>
        
        <div class="input-container">
            <input type="text" class="chat-input" id="chatInput" placeholder="输入您的问题..." onkeypress="handleKeyPress(event)">
            <button class="voice-btn" onclick="startVoiceRecognition()">🎤</button>
            <button class="send-btn" onclick="sendMessage()">发送</button>
        </div>
        
        <div class="controls">
            <button class="control-btn" onclick="clearChat()">清空对话</button>
            <button class="control-btn" onclick="getHelp()">帮助</button>
            <button class="control-btn" onclick="getStatus()">状态</button>
        </div>
        
        <div class="status" id="status">
            状态: 已连接
        </div>
    </div>

    <script>
        let ws = null;
        
        function connectWebSocket() {
            const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            const wsUrl = protocol + '//' + window.location.host + '/ws';
            
            ws = new WebSocket(wsUrl);
            
            ws.onopen = function() {
                updateStatus('WebSocket已连接');
            };
            
            ws.onmessage = function(event) {
                const data = JSON.parse(event.data);
                if (data.type === 'response') {
                    addMessage(data.text, 'ai');
                } else if (data.type === 'status') {
                    updateStatus(data.message);
                }
            };
            
            ws.onclose = function() {
                updateStatus('WebSocket连接断开');
                setTimeout(connectWebSocket, 3000);
            };
            
            ws.onerror = function(error) {
                updateStatus('WebSocket错误: ' + error);
            };
        }
        
        function sendMessage() {
            const input = document.getElementById('chatInput');
            const message = input.value.trim();
            
            if (message) {
                addMessage(message, 'user');
                input.value = '';
                
                if (ws && ws.readyState === WebSocket.OPEN) {
                    ws.send(JSON.stringify({
                        type: 'chat',
                        message: message
                    }));
                } else {
                    // 如果WebSocket未连接，发送HTTP请求
                    fetch('/api/chat', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json'
                        },
                        body: JSON.stringify({message: message})
                    })
                    .then(response => response.json())
                    .then(data => {
                        addMessage(data.response, 'ai');
                    })
                    .catch(error => {
                        addMessage('抱歉，连接出现问题', 'ai');
                    });
                }
            }
        }
        
        function addMessage(text, sender) {
            const container = document.getElementById('chatContainer');
            const messageDiv = document.createElement('div');
            messageDiv.className = 'message ' + (sender === 'user' ? 'user-message' : 'ai-message');
            messageDiv.textContent = text;
            container.appendChild(messageDiv);
            container.scrollTop = container.scrollHeight;
        }
        
        function updateStatus(status) {
            document.getElementById('status').textContent = '状态: ' + status;
        }
        
        function handleKeyPress(event) {
            if (event.key === 'Enter') {
                sendMessage();
            }
        }
        
        function startVoiceRecognition() {
            updateStatus('语音识别启动中...');
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({
                    type: 'voice',
                    action: 'start'
                }));
            }
        }
        
        function clearChat() {
            document.getElementById('chatContainer').innerHTML = 
                '<div class="message ai-message">对话已清空，有什么可以帮助您的吗？</div>';
        }
        
        function getHelp() {
            addMessage('我可以帮您：\n1. 查询时间和天气\n2. 控制智能设备\n3. 播放音乐\n4. 聊天对话\n5. 回答问题', 'ai');
        }
        
        function getStatus() {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({
                    type: 'status',
                    action: 'get'
                }));
            }
        }
        
        // 页面加载完成后连接WebSocket
        window.onload = function() {
            connectWebSocket();
        };
    </script>
</body>
</html>
)";

// HTTP GET处理函数
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_page, strlen(html_page));
    return ESP_OK;
}

// API聊天处理函数
static esp_err_t api_chat_post_handler(httpd_req_t *req)
{
    char content[512];
    int recv_len = httpd_req_recv(req, content, sizeof(content) - 1);
    if (recv_len <= 0) {
        return ESP_FAIL;
    }
    content[recv_len] = '\0';
    
    ESP_LOGI(TAG, "收到聊天消息: %s", content);
    
    // 解析JSON
    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }
    
    cJSON *message = cJSON_GetObjectItem(json, "message");
    if (message == NULL || !cJSON_IsString(message)) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing message");
        return ESP_FAIL;
    }
    
    // 处理消息
    if (ws_callback != NULL) {
        ws_message_t ws_msg = {
            .type = WS_MSG_CHAT,
            .length = strlen(message->valuestring)
        };
        strcpy(ws_msg.data, message->valuestring);
        ws_callback(&ws_msg);
    }
    
    cJSON_Delete(json);
    
    // 返回响应
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "response", "消息已收到，正在处理...");
    cJSON_AddStringToObject(response, "status", "success");
    
    char *response_str = cJSON_Print(response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_str, strlen(response_str));
    
    free(response_str);
    cJSON_Delete(response);
    
    return ESP_OK;
}

// API状态处理函数
static esp_err_t api_status_get_handler(httpd_req_t *req)
{
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "status", "running");
    cJSON_AddStringToObject(response, "ai_name", "小智");
    cJSON_AddStringToObject(response, "version", "1.0.0");
    cJSON_AddBoolToObject(response, "voice_enabled", true);
    cJSON_AddBoolToObject(response, "web_enabled", true);
    
    char *response_str = cJSON_Print(response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_str, strlen(response_str));
    
    free(response_str);
    cJSON_Delete(response);
    
    return ESP_OK;
}

// HTTP URI处理表
static const httpd_uri_t uri_handlers[] = {
    {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler,
        .user_ctx = NULL
    },
    {
        .uri = "/api/chat",
        .method = HTTP_POST,
        .handler = api_chat_post_handler,
        .user_ctx = NULL
    },
    {
        .uri = "/api/status",
        .method = HTTP_GET,
        .handler = api_status_get_handler,
        .user_ctx = NULL
    }
};

esp_err_t web_interface_init(void)
{
    ESP_LOGI(TAG, "初始化Web界面");
    
    // 配置HTTP服务器
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = web_config.port;
    config.max_uri_handlers = 16;
    config.stack_size = 8192;
    
    // 启动HTTP服务器
    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "HTTP服务器启动失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 注册URI处理器
    for (int i = 0; i < sizeof(uri_handlers) / sizeof(uri_handlers[0]); i++) {
        ret = httpd_register_uri_handler(server, &uri_handlers[i]);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "URI处理器注册失败: %s", esp_err_to_name(ret));
            return ret;
        }
    }
    
    ESP_LOGI(TAG, "Web界面初始化完成");
    ESP_LOGI(TAG, "服务器地址: http://%s:%d", web_config.hostname, web_config.port);
    
    return ESP_OK;
}

esp_err_t web_interface_start(void)
{
    if (server == NULL) {
        return web_interface_init();
    }
    return ESP_OK;
}

esp_err_t web_interface_stop(void)
{
    if (server != NULL) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "Web界面已停止");
    }
    return ESP_OK;
}

esp_err_t web_send_response(ai_response_t *response)
{
    if (response == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "发送AI响应到Web界面: %s", response->text);
    
    // 这里应该通过WebSocket发送响应
    // 暂时只记录日志
    
    return ESP_OK;
}

esp_err_t web_send_status(const char *status)
{
    if (status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "发送状态更新: %s", status);
    
    return ESP_OK;
}

esp_err_t web_set_config(web_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&web_config, config, sizeof(web_config_t));
    ESP_LOGI(TAG, "Web配置已更新");
    
    return ESP_OK;
}

esp_err_t web_broadcast_message(const char *message)
{
    if (message == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "广播消息: %s", message);
    
    return ESP_OK;
}

esp_err_t web_handle_chat_message(const char *message)
{
    if (message == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "处理聊天消息: %s", message);
    
    return ESP_OK;
}

esp_err_t web_set_ws_callback(ws_message_callback_t callback)
{
    ws_callback = callback;
    ESP_LOGI(TAG, "WebSocket回调已设置");
    return ESP_OK;
}
