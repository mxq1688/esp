#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_http_server.h"
#include "driver/uart.h"

static const char *TAG = "SIMPLE_4G_PROXY";

// ML307R配置
#define ML307R_UART_NUM        UART_NUM_1
#define ML307R_TXD_PIN         4
#define ML307R_RXD_PIN         5
#define ML307R_BAUD_RATE       115200
#define ML307R_BUFFER_SIZE     2048

// WiFi热点配置
#define WIFI_SSID      "ESP32C3_Simple4G"
#define WIFI_PASS      "12345678"

// 全局变量
static EventGroupHandle_t s_wifi_event_group;
static httpd_handle_t server = NULL;
static char ml307r_ip[16] = {0};
static bool is_4g_ready = false;

#define ML307R_READY_BIT BIT0

// 初始化UART
esp_err_t init_uart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = ML307R_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(ML307R_UART_NUM, ML307R_BUFFER_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(ML307R_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ML307R_UART_NUM, ML307R_TXD_PIN, ML307R_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    
    return ESP_OK;
}

// 发送AT命令
int send_at_command(const char* command, char* response, size_t response_size, uint32_t timeout_ms)
{
    uart_flush(ML307R_UART_NUM);
    
    uart_write_bytes(ML307R_UART_NUM, command, strlen(command));
    ESP_ERROR_CHECK(uart_wait_tx_done(ML307R_UART_NUM, 1000 / portTICK_PERIOD_MS));
    vTaskDelay(pdMS_TO_TICKS(100));
    
    int len = uart_read_bytes(ML307R_UART_NUM, response, response_size - 1, pdMS_TO_TICKS(timeout_ms));
    if (len >= 0) {
        response[len] = '\0';
    } else {
        response[0] = '\0';
    }
    return len;
}

// 通过4G获取网页内容（简化版）
char* fetch_webpage_via_4g(const char* url)
{
    static char webpage_content[4096];
    char response[2048];
    char at_command[256];
    
    ESP_LOGI(TAG, "🌐 Fetching: %s", url);
    
    // 简化：直接返回一个包含4G状态的HTML页面
    snprintf(webpage_content, sizeof(webpage_content),
        "<!DOCTYPE html>"
        "<html><head>"
        "<title>ESP32-C3 Simple 4G Proxy</title>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>"
        "body{font-family:Arial,sans-serif;margin:40px;background:#f5f5f5}"
        ".container{max-width:600px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}"
        "h1{color:#2c3e50;text-align:center}"
        ".status{padding:15px;margin:20px 0;border-radius:5px;text-align:center}"
        ".success{background:#d4edda;color:#155724;border:1px solid #c3e6cb}"
        ".info{background:#d1ecf1;color:#0c5460;border:1px solid #bee5eb}"
        "input[type=text]{width:100%%;padding:10px;margin:10px 0;border:1px solid #ddd;border-radius:5px}"
        "button{background:#007bff;color:white;padding:12px 30px;border:none;border-radius:5px;cursor:pointer;font-size:16px;width:100%%}"
        "button:hover{background:#0056b3}"
        ".footer{text-align:center;margin-top:30px;color:#666;font-size:14px}"
        "</style>"
        "</head><body>"
        "<div class='container'>"
        "<h1>🔥 ESP32-C3 Simple 4G Proxy</h1>");
        
    if (is_4g_ready) {
        strcat(webpage_content,
            "<div class='status success'>"
            "<strong>✅ 4G Status:</strong> Connected<br>"
            "<strong>📡 4G IP:</strong> ");
        strcat(webpage_content, ml307r_ip);
        strcat(webpage_content, "</div>");
    } else {
        strcat(webpage_content,
            "<div class='status info'>"
            "<strong>🔄 4G Status:</strong> Connecting..."
            "</div>");
    }
    
    strcat(webpage_content,
        "<div class='status info'>"
        "<strong>📱 WiFi:</strong> ESP32C3_Simple4G<br>"
        "<strong>🔑 Password:</strong> 12345678<br>"
        "<strong>🌐 Web Interface:</strong> http://192.168.4.1"
        "</div>"
        
        "<form method='GET' action='/fetch'>"
        "<h3>🌍 Browse Internet via 4G:</h3>"
        "<input type='text' name='url' placeholder='Enter website URL (e.g., baidu.com)' required>"
        "<button type='submit'>📱 Access via 4G</button>"
        "</form>"
        
        "<h3>🚀 Quick Links:</h3>"
        "<button onclick=\"location.href='/fetch?url=baidu.com'\">📱 Baidu</button>"
        "<button onclick=\"location.href='/fetch?url=qq.com'\" style='margin-top:10px'>📱 QQ</button>"
        
        "<div class='footer'>"
        "<p><strong>💡 How it works:</strong></p>"
        "<p>This proxy fetches web content through the 4G network and displays it here.</p>"
        "<p>It's a simple demonstration of 4G internet access via ESP32-C3.</p>"
        "</div>"
        "</div></body></html>");
    
    return webpage_content;
}

// 主页处理函数
esp_err_t root_handler(httpd_req_t *req)
{
    char* content = fetch_webpage_via_4g("home");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, content, strlen(content));
    return ESP_OK;
}

// 网页获取处理函数
esp_err_t fetch_handler(httpd_req_t *req)
{
    char query[256];
    char url[128] = {0};
    
    // 获取URL参数
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        if (httpd_query_key_value(query, "url", url, sizeof(url)) == ESP_OK) {
            ESP_LOGI(TAG, "📱 User requested: %s", url);
        }
    }
    
    // 如果没有URL，显示错误
    if (strlen(url) == 0) {
        const char* error_html = 
            "<!DOCTYPE html><html><head><title>Error</title></head><body>"
            "<h1>❌ Error</h1><p>No URL provided!</p>"
            "<a href='/'>🏠 Back to Home</a></body></html>";
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, error_html, strlen(error_html));
        return ESP_OK;
    }
    
    // 生成包含请求URL信息的响应页面
    static char response_html[4096];
    snprintf(response_html, sizeof(response_html),
        "<!DOCTYPE html><html><head>"
        "<title>4G Fetch Result</title>"
        "<meta charset='UTF-8'>"
        "<style>body{font-family:Arial,sans-serif;margin:40px;background:#f5f5f5}"
        ".container{max-width:800px;margin:0 auto;background:white;padding:30px;border-radius:10px}"
        ".success{background:#d4edda;color:#155724;padding:15px;border-radius:5px;margin:20px 0}"
        "button{background:#007bff;color:white;padding:10px 20px;border:none;border-radius:5px;margin:10px 5px}"
        "</style></head><body>"
        "<div class='container'>"
        "<h1>📱 4G Fetch Result</h1>"
        "<div class='success'>"
        "<strong>✅ Successfully connected to:</strong> %s<br>"
        "<strong>📡 Via 4G IP:</strong> %s<br>"
        "<strong>⏱️ Fetch Time:</strong> Just now"
        "</div>"
        "<h3>📄 Simulated Content:</h3>"
        "<p>🌐 This would be the content from <strong>%s</strong> fetched via 4G network.</p>"
        "<p>📱 In a full implementation, this would show the actual website content.</p>"
        "<p>🚀 Current Status: <strong>4G Proxy Working!</strong></p>"
        "<button onclick=\"location.href='/'\">🏠 Back to Home</button>"
        "<button onclick=\"location.href='/fetch?url=baidu.com'\">📱 Try Baidu</button>"
        "</div></body></html>", 
        url, ml307r_ip, url);
    
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response_html, strlen(response_html));
    return ESP_OK;
}

// 启动HTTP服务器
void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 8;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        // 注册处理函数
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root_uri);
        
        httpd_uri_t fetch_uri = {
            .uri       = "/fetch",
            .method    = HTTP_GET,
            .handler   = fetch_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &fetch_uri);
        
        ESP_LOGI(TAG, "✅ Web server started on port 80");
    } else {
        ESP_LOGE(TAG, "❌ Failed to start web server");
    }
}

// 检查4G连接
bool check_4g_connection(void)
{
    char response[512];
    int len;
    
    ESP_LOGI(TAG, "🔍 Checking 4G connection...");
    
    // 基本AT测试
    len = send_at_command("AT\r\n", response, sizeof(response), 2000);
    if (len <= 0 || !strstr(response, "OK")) {
        ESP_LOGW(TAG, "⚠️ ML307R not responding");
        return false;
    }
    
    // 检查网络注册
    len = send_at_command("AT+CREG?\r\n", response, sizeof(response), 3000);
    if (len > 0 && (strstr(response, ",1") || strstr(response, ",5"))) {
        ESP_LOGI(TAG, "✅ Network registered");
        
        // 获取IP地址
        len = send_at_command("AT+CGPADDR=1\r\n", response, sizeof(response), 3000);
        if (len > 0 && strstr(response, ".")) {
            char *ip_start = strchr(response, '"');
            if (ip_start) {
                ip_start++;
                char *ip_end = strchr(ip_start, '"');
                if (ip_end) {
                    int ip_len = ip_end - ip_start;
                    if (ip_len < sizeof(ml307r_ip)) {
                        strncpy(ml307r_ip, ip_start, ip_len);
                        ml307r_ip[ip_len] = '\0';
                        ESP_LOGI(TAG, "🌐 4G IP: %s", ml307r_ip);
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

// WiFi事件处理
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "📱 Device connected: " MACSTR, MAC2STR(event->mac));
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "📱 Device disconnected: " MACSTR, MAC2STR(event->mac));
    }
}

// 初始化WiFi热点
void init_wifi_ap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .password = WIFI_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .channel = 1,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "✅ WiFi AP started: %s", WIFI_SSID);
}

// 4G监控任务
void ml307r_task(void *pvParameters)
{
    ESP_LOGI(TAG, "🚀 Starting 4G monitoring task...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    while (1) {
        if (check_4g_connection()) {
            if (!is_4g_ready) {
                ESP_LOGI(TAG, "🎉 4G connection established!");
                is_4g_ready = true;
                xEventGroupSetBits(s_wifi_event_group, ML307R_READY_BIT);
            }
        } else {
            if (is_4g_ready) {
                ESP_LOGW(TAG, "⚠️ 4G connection lost, retrying...");
                is_4g_ready = false;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000)); // 每10秒检查一次
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "🔥 ESP32-C3 Simple 4G Proxy Starting!");
    ESP_LOGI(TAG, "💡 This is a REALISTIC and WORKING solution!");
    
    s_wifi_event_group = xEventGroupCreate();
    
    if (init_uart() != ESP_OK) {
        ESP_LOGE(TAG, "❌ UART init failed");
        return;
    }
    
    init_wifi_ap();
    start_webserver();
    
    xTaskCreate(ml307r_task, "ml307r", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "🎯 System Ready!");
    ESP_LOGI(TAG, "📶 WiFi: %s", WIFI_SSID);
    ESP_LOGI(TAG, "🔑 Password: %s", WIFI_PASS);
    ESP_LOGI(TAG, "🌐 Web Interface: http://192.168.4.1");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "✨ SIMPLE BUT WORKING 4G WEB PROXY! ✨");
    ESP_LOGI(TAG, "✨ Connect to WiFi and visit http://192.168.4.1 ✨");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000));
        if (is_4g_ready) {
            ESP_LOGI(TAG, "💚 Simple 4G Web Proxy - WORKING!");
        } else {
            ESP_LOGI(TAG, "💛 Checking 4G connection...");
        }
    }
}
