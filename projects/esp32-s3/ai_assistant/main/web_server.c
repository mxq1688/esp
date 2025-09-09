/**
 * @file web_server.c
 * @brief Web Server Implementation for ESP32-S3 AI Assistant
 * @author AI Assistant Team
 * @version 1.0.0
 */

#include "web_server.h"
#include "api_handlers.h"
#include "ai_assistant.h"
#include "esp_log.h"
#include "esp_http_server.h"

static const char *TAG = "WEB_SERVER";

// Web服务器状态和配置
static web_server_state_t s_server_state = WEB_SERVER_STATE_STOPPED;
static web_server_config_t s_server_config = {
    .port = 80,
    .max_uri_handlers = WEB_SERVER_MAX_HANDLERS,
    .stack_size = WEB_SERVER_STACK_SIZE,
    .recv_wait_timeout = 10,
    .send_wait_timeout = 10,
    .enable_cors = true,
    .cors_origin = "*"
};

static httpd_handle_t s_server_handle = NULL;

/**
 * @brief 初始化Web服务器
 */
esp_err_t web_server_init(void)
{
    ai_assistant_log_info(TAG, "Initializing Web Server");
    
    s_server_state = WEB_SERVER_STATE_STOPPED;
    ai_assistant_log_info(TAG, "Web Server initialized successfully");
    return ESP_OK;
}

/**
 * @brief 启动Web服务器
 */
esp_err_t web_server_start(void)
{
    ai_assistant_log_info(TAG, "Starting Web Server on port %d", s_server_config.port);
    
    if (s_server_state != WEB_SERVER_STATE_STOPPED) {
        ai_assistant_log_error(TAG, "Web server is not in stopped state");
        return ESP_ERR_INVALID_STATE;
    }
    
    s_server_state = WEB_SERVER_STATE_STARTING;
    
    // 配置HTTP服务器
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = s_server_config.port;
    config.max_uri_handlers = s_server_config.max_uri_handlers;
    config.stack_size = s_server_config.stack_size;
    config.recv_wait_timeout = s_server_config.recv_wait_timeout;
    config.send_wait_timeout = s_server_config.send_wait_timeout;
    
    // 启动HTTP服务器
    esp_err_t ret = httpd_start(&s_server_handle, &config);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to start HTTP server: %s", esp_err_to_name(ret));
        s_server_state = WEB_SERVER_STATE_ERROR;
        return ret;
    }
    
    // 注册URI处理器
    ret = api_handlers_register(s_server_handle);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to register API handlers");
        httpd_stop(s_server_handle);
        s_server_handle = NULL;
        s_server_state = WEB_SERVER_STATE_ERROR;
        return ret;
    }
    
    // 注册静态文件处理器
    httpd_uri_t files_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = web_server_files_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(s_server_handle, &files_uri);
    
    s_server_state = WEB_SERVER_STATE_RUNNING;
    ai_assistant_log_info(TAG, "Web Server started successfully");
    return ESP_OK;
}

/**
 * @brief 停止Web服务器
 */
esp_err_t web_server_stop(void)
{
    ai_assistant_log_info(TAG, "Stopping Web Server");
    
    if (s_server_state != WEB_SERVER_STATE_RUNNING) {
        ai_assistant_log_error(TAG, "Web server is not running");
        return ESP_ERR_INVALID_STATE;
    }
    
    s_server_state = WEB_SERVER_STATE_STOPPING;
    
    // 注销API处理器
    if (s_server_handle != NULL) {
        api_handlers_unregister(s_server_handle);
        
        // 停止HTTP服务器
        esp_err_t ret = httpd_stop(s_server_handle);
        if (ret != ESP_OK) {
            ai_assistant_log_error(TAG, "Failed to stop HTTP server: %s", esp_err_to_name(ret));
        }
        s_server_handle = NULL;
    }
    
    s_server_state = WEB_SERVER_STATE_STOPPED;
    ai_assistant_log_info(TAG, "Web Server stopped");
    return ESP_OK;
}

/**
 * @brief 设置Web服务器配置
 */
esp_err_t web_server_set_config(const web_server_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_server_config, config, sizeof(web_server_config_t));
    ai_assistant_log_info(TAG, "Web server configuration updated");
    return ESP_OK;
}

/**
 * @brief 获取Web服务器配置
 */
esp_err_t web_server_get_config(web_server_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &s_server_config, sizeof(web_server_config_t));
    return ESP_OK;
}

/**
 * @brief 获取Web服务器状态
 */
web_server_state_t web_server_get_state(void)
{
    return s_server_state;
}

/**
 * @brief 获取服务器句柄
 */
httpd_handle_t web_server_get_handle(void)
{
    return s_server_handle;
}

/**
 * @brief 注册URI处理器
 */
esp_err_t web_server_register_uri_handler(const httpd_uri_t *uri_handler)
{
    if (uri_handler == NULL) {
        ai_assistant_log_error(TAG, "URI handler cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_server_handle == NULL) {
        ai_assistant_log_error(TAG, "Web server is not running");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = httpd_register_uri_handler(s_server_handle, uri_handler);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to register URI handler: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ai_assistant_log_info(TAG, "URI handler registered: %s", uri_handler->uri);
    return ESP_OK;
}

/**
 * @brief 注销URI处理器
 */
esp_err_t web_server_unregister_uri_handler(const char *uri, httpd_method_t method)
{
    if (uri == NULL) {
        ai_assistant_log_error(TAG, "URI cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_server_handle == NULL) {
        ai_assistant_log_error(TAG, "Web server is not running");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = httpd_unregister_uri_handler(s_server_handle, uri, method);
    if (ret != ESP_OK) {
        ai_assistant_log_error(TAG, "Failed to unregister URI handler: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ai_assistant_log_info(TAG, "URI handler unregistered: %s", uri);
    return ESP_OK;
}

/**
 * @brief 静态文件处理器
 */
static esp_err_t web_server_files_handler(httpd_req_t *req)
{
    const char *filename = req->uri;
    
    // 如果请求根路径，返回index.html
    if (strcmp(filename, "/") == 0) {
        filename = "/index.html";
    }
    
    ai_assistant_log_info(TAG, "Serving file: %s", filename);
    
    // 设置CORS头
    if (s_server_config.enable_cors) {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", s_server_config.cors_origin);
        httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
    
    // 简单的HTML页面
    const char *html_content = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>ESP32-S3 AI Assistant</title>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <style>\n"
        "        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }\n"
        "        .container { max-width: 800px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n"
        "        h1 { color: #333; text-align: center; }\n"
        "        .status { padding: 15px; margin: 20px 0; border-radius: 5px; }\n"
        "        .status.online { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }\n"
        "        .chat-box { border: 1px solid #ddd; height: 300px; overflow-y: auto; padding: 15px; margin: 20px 0; background: #fafafa; }\n"
        "        .input-group { display: flex; gap: 10px; margin: 20px 0; }\n"
        "        input[type=\"text\"] { flex: 1; padding: 10px; border: 1px solid #ddd; border-radius: 5px; }\n"
        "        button { padding: 10px 20px; background: #007bff; color: white; border: none; border-radius: 5px; cursor: pointer; }\n"
        "        button:hover { background: #0056b3; }\n"
        "        .controls { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin: 20px 0; }\n"
        "        .control-item { padding: 15px; background: #f8f9fa; border-radius: 5px; text-align: center; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <div class=\"container\">\n"
        "        <h1>🤖 ESP32-S3 AI小智助手</h1>\n"
        "        <div class=\"status online\">✅ 系统在线 - 准备就绪</div>\n"
        "        \n"
        "        <div class=\"chat-box\" id=\"chatBox\">\n"
        "            <div><strong>AI小智:</strong> 您好！我是您的智能助手，有什么可以帮助您的吗？</div>\n"
        "        </div>\n"
        "        \n"
        "        <div class=\"input-group\">\n"
        "            <input type=\"text\" id=\"messageInput\" placeholder=\"请输入您的问题...\" onkeypress=\"handleKeyPress(event)\">\n"
        "            <button onclick=\"sendMessage()\">发送</button>\n"
        "            <button onclick=\"startVoice()\" id=\"voiceBtn\">🎤 语音</button>\n"
        "        </div>\n"
        "        \n"
        "        <div class=\"controls\">\n"
        "            <div class=\"control-item\">\n"
        "                <h3>🌤️ 天气查询</h3>\n"
        "                <button onclick=\"askWeather()\">查询天气</button>\n"
        "            </div>\n"
        "            <div class=\"control-item\">\n"
        "                <h3>🕐 时间日期</h3>\n"
        "                <button onclick=\"askTime()\">查询时间</button>\n"
        "            </div>\n"
        "            <div class=\"control-item\">\n"
        "                <h3>🎵 音乐控制</h3>\n"
        "                <button onclick=\"controlMusic()\">播放音乐</button>\n"
        "            </div>\n"
        "            <div class=\"control-item\">\n"
        "                <h3>💡 智能灯光</h3>\n"
        "                <button onclick=\"controlLight()\">控制灯光</button>\n"
        "            </div>\n"
        "        </div>\n"
        "    </div>\n"
        "    \n"
        "    <script>\n"
        "        function addMessage(sender, message) {\n"
        "            const chatBox = document.getElementById('chatBox');\n"
        "            const div = document.createElement('div');\n"
        "            div.innerHTML = '<strong>' + sender + ':</strong> ' + message;\n"
        "            div.style.margin = '10px 0';\n"
        "            chatBox.appendChild(div);\n"
        "            chatBox.scrollTop = chatBox.scrollHeight;\n"
        "        }\n"
        "        \n"
        "        function sendMessage() {\n"
        "            const input = document.getElementById('messageInput');\n"
        "            const message = input.value.trim();\n"
        "            if (message) {\n"
        "                addMessage('用户', message);\n"
        "                input.value = '';\n"
        "                \n"
        "                // 发送到AI引擎\n"
        "                fetch('/api/ai/chat', {\n"
        "                    method: 'POST',\n"
        "                    headers: {'Content-Type': 'application/json'},\n"
        "                    body: JSON.stringify({message: message})\n"
        "                })\n"
        "                .then(response => response.json())\n"
        "                .then(data => {\n"
        "                    if (data.error_code === 0) {\n"
        "                        addMessage('AI小智', data.data.response);\n"
        "                    } else {\n"
        "                        addMessage('系统', '抱歉，处理您的请求时出现错误');\n"
        "                    }\n"
        "                })\n"
        "                .catch(error => {\n"
        "                    addMessage('系统', '网络连接错误');\n"
        "                });\n"
        "            }\n"
        "        }\n"
        "        \n"
        "        function handleKeyPress(event) {\n"
        "            if (event.key === 'Enter') {\n"
        "                sendMessage();\n"
        "            }\n"
        "        }\n"
        "        \n"
        "        function startVoice() {\n"
        "            addMessage('系统', '语音功能开发中...');\n"
        "        }\n"
        "        \n"
        "        function askWeather() {\n"
        "            document.getElementById('messageInput').value = '今天天气怎么样？';\n"
        "            sendMessage();\n"
        "        }\n"
        "        \n"
        "        function askTime() {\n"
        "            document.getElementById('messageInput').value = '现在几点了？';\n"
        "            sendMessage();\n"
        "        }\n"
        "        \n"
        "        function controlMusic() {\n"
        "            document.getElementById('messageInput').value = '播放音乐';\n"
        "            sendMessage();\n"
        "        }\n"
        "        \n"
        "        function controlLight() {\n"
        "            document.getElementById('messageInput').value = '开灯';\n"
        "            sendMessage();\n"
        "        }\n"
        "    </script>\n"
        "</body>\n"
        "</html>";
    
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html_content, HTTPD_RESP_USE_STRLEN);
}

/**
 * @brief CORS预检请求处理器
 */
static esp_err_t web_server_cors_handler(httpd_req_t *req)
{
    if (s_server_config.enable_cors) {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", s_server_config.cors_origin);
        httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Authorization");
        httpd_resp_set_hdr(req, "Access-Control-Max-Age", "86400");
    }
    
    return httpd_resp_send(req, NULL, 0);
}
