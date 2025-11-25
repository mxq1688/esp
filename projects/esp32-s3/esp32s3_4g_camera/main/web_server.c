#include "include/web_server.h"
#include "include/api_handlers.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <string.h>

static const char *TAG = "WEB_SERVER";

// 全局变量
static httpd_handle_t server = NULL;
static bool server_running = false;

// 根页面处理器
extern const char html_page_start[] asm("_binary_index_html_start");
extern const char html_page_end[] asm("_binary_index_html_end");

static esp_err_t root_handler(httpd_req_t *req)
{
    const char *html_content = 
        "<!DOCTYPE html>"
        "<html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>ESP32-S3 4G Camera</title>"
        "<style>"
        "body { font-family: Arial, sans-serif; max-width: 1200px; margin: 0 auto; padding: 20px; background: #f5f5f5; }"
        ".container { background: white; border-radius: 10px; padding: 20px; margin-bottom: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }"
        ".header { text-align: center; color: #333; margin-bottom: 30px; }"
        ".stream-container { text-align: center; background: #000; border-radius: 10px; overflow: hidden; }"
        ".stream-container img { width: 100%; max-width: 800px; height: auto; }"
        ".controls { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin-top: 20px; }"
        ".btn { padding: 12px 24px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; transition: all 0.3s; }"
        ".btn-primary { background: #007bff; color: white; }"
        ".btn-primary:hover { background: #0056b3; }"
        ".btn-success { background: #28a745; color: white; }"
        ".btn-success:hover { background: #1e7e34; }"
        ".btn-danger { background: #dc3545; color: white; }"
        ".btn-danger:hover { background: #c82333; }"
        ".info-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 15px; }"
        ".info-item { padding: 15px; background: #f8f9fa; border-radius: 5px; border-left: 4px solid #007bff; }"
        ".info-label { font-weight: bold; color: #666; font-size: 14px; }"
        ".info-value { font-size: 18px; color: #333; margin-top: 5px; }"
        ".quality-control { display: flex; align-items: center; gap: 10px; }"
        ".quality-control input { flex: 1; }"
        "</style>"
        "</head><body>"
        "<div class='header'><h1>📷 ESP32-S3 4G 远程摄像头</h1></div>"
        
        "<div class='container'>"
        "<h2>📡 实时视频流</h2>"
        "<div class='stream-container'>"
        "<img id='stream' src='/api/camera/stream' alt='Camera Stream'>"
        "</div>"
        "<div class='controls'>"
        "<button class='btn btn-primary' onclick='captureImage()'>📸 抓拍</button>"
        "<button class='btn btn-success' onclick='startStream()'>▶️ 开始</button>"
        "<button class='btn btn-danger' onclick='stopStream()'>⏸️ 暂停</button>"
        "</div>"
        "</div>"
        
        "<div class='container'>"
        "<h2>⚙️ 摄像头控制</h2>"
        "<div class='quality-control'>"
        "<label>图像质量:</label>"
        "<input type='range' id='quality' min='0' max='63' value='12' oninput='updateQuality(this.value)'>"
        "<span id='qualityValue'>12</span>"
        "</div>"
        "<div class='controls' style='margin-top: 15px;'>"
        "<button class='btn btn-primary' onclick='setResolution(\"QVGA\")'>QVGA (320x240)</button>"
        "<button class='btn btn-primary' onclick='setResolution(\"VGA\")'>VGA (640x480)</button>"
        "<button class='btn btn-primary' onclick='setResolution(\"SVGA\")'>SVGA (800x600)</button>"
        "</div>"
        "</div>"
        
        "<div class='container'>"
        "<h2>📊 系统状态</h2>"
        "<div class='info-grid' id='statusInfo'>"
        "<div class='info-item'><div class='info-label'>摄像头状态</div><div class='info-value' id='cameraState'>-</div></div>"
        "<div class='info-item'><div class='info-label'>4G 状态</div><div class='info-value' id='networkState'>-</div></div>"
        "<div class='info-item'><div class='info-label'>信号强度</div><div class='info-value' id='signalStrength'>-</div></div>"
        "<div class='info-item'><div class='info-label'>可用内存</div><div class='info-value' id='freeHeap'>-</div></div>"
        "</div>"
        "</div>"
        
        "<script>"
        "let streaming = true;"
        "function captureImage() {"
        "  window.open('/api/camera/capture', '_blank');"
        "}"
        "function startStream() {"
        "  document.getElementById('stream').src = '/api/camera/stream?' + new Date().getTime();"
        "  streaming = true;"
        "}"
        "function stopStream() {"
        "  document.getElementById('stream').src = '';"
        "  streaming = false;"
        "}"
        "function updateQuality(val) {"
        "  document.getElementById('qualityValue').innerText = val;"
        "  fetch('/api/camera/quality?value=' + val);"
        "}"
        "function setResolution(res) {"
        "  fetch('/api/camera/resolution?value=' + res)"
        "  .then(() => { if(streaming) startStream(); });"
        "}"
        "function updateStatus() {"
        "  fetch('/api/status')"
        "  .then(r => r.json())"
        "  .then(data => {"
        "    document.getElementById('cameraState').innerText = data.camera_ready ? '✅ 就绪' : '❌ 未就绪';"
        "    document.getElementById('networkState').innerText = data.network_connected ? '✅ 已连接' : '❌ 未连接';"
        "    document.getElementById('signalStrength').innerText = data.signal_strength + ' dBm';"
        "    document.getElementById('freeHeap').innerText = (data.free_heap / 1024).toFixed(1) + ' KB';"
        "  });"
        "}"
        "setInterval(updateStatus, 3000);"
        "updateStatus();"
        "</script>"
        "</body></html>";
    
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html_content, HTTPD_RESP_USE_STRLEN);
}

// 启动Web服务器
esp_err_t web_server_start(void)
{
    if (server_running) {
        ESP_LOGW(TAG, "Web server already running");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Starting web server...");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16;
    config.stack_size = 8192;
    config.core_id = 1;  // 使用核心1
    config.task_priority = 5;
    config.lru_purge_enable = true;

    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(ret));
        return ret;
    }

    // 注册根路径处理器
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &root_uri);

    // 注册API处理器
    ret = api_handlers_register(server);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register API handlers");
        httpd_stop(server);
        return ret;
    }

    server_running = true;
    ESP_LOGI(TAG, "✅ Web server started on port %d", config.server_port);

    return ESP_OK;
}

// 停止Web服务器
esp_err_t web_server_stop(void)
{
    if (!server_running) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Stopping web server...");

    esp_err_t ret = httpd_stop(server);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop HTTP server: %s", esp_err_to_name(ret));
        return ret;
    }

    server = NULL;
    server_running = false;
    ESP_LOGI(TAG, "Web server stopped");

    return ESP_OK;
}

// 检查Web服务器是否运行
bool web_server_is_running(void)
{
    return server_running;
}

// 获取Web服务器句柄
httpd_handle_t web_server_get_handle(void)
{
    return server;
}

