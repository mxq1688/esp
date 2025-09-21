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
#include "driver/uart.h"
#include "esp_http_server.h"

static const char *TAG = "WORKING_4G_SOLUTION";

// ML307R配置
#define ML307R_UART_NUM        UART_NUM_1
#define ML307R_TXD_PIN         4
#define ML307R_RXD_PIN         5
#define ML307R_BAUD_RATE       115200
#define ML307R_BUFFER_SIZE     1024

// WiFi热点配置  
#define WIFI_SSID      "ESP32C3_4G_Working"
#define WIFI_PASS      "12345678"
#define WIFI_CHANNEL   1
#define MAX_STA_CONN   4

// 全局变量
static esp_netif_t *wifi_ap_netif = NULL;
static EventGroupHandle_t s_wifi_event_group;
static char ml307r_ip[16] = {0};
static bool is_4g_connected = false;
static httpd_handle_t server = NULL;

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
    
    ESP_LOGI(TAG, "UART initialized successfully");
    return ESP_OK;
}

// 发送AT命令
int send_at_command(const char* command, char* response, size_t response_size, uint32_t timeout_ms)
{
    uart_flush(ML307R_UART_NUM);
    
    int written = uart_write_bytes(ML307R_UART_NUM, command, strlen(command));
    if (written < 0) return -1;
    
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

// 通过4G获取网页内容
esp_err_t get_url_via_4g(const char* url, char* result, size_t max_len)
{
    char at_command[256];
    char response[2048];
    
    // 开启HTTP功能
    send_at_command("AT+HTTPINIT\r\n", response, sizeof(response), 3000);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 设置URL
    snprintf(at_command, sizeof(at_command), "AT+HTTPPARA=\"URL\",\"%s\"\r\n", url);
    send_at_command(at_command, response, sizeof(response), 3000);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 发起HTTP GET请求
    int len = send_at_command("AT+HTTPACTION=0\r\n", response, sizeof(response), 15000);
    
    if (len > 0 && strstr(response, "200")) {
        // 读取HTTP内容
        len = send_at_command("AT+HTTPREAD\r\n", response, sizeof(response), 10000);
        if (len > 0) {
            // 查找内容起始位置
            char *content_start = strstr(response, "\r\n\r\n");
            if (content_start) {
                content_start += 4;
                strncpy(result, content_start, max_len - 1);
                result[max_len - 1] = '\0';
                
                send_at_command("AT+HTTPTERM\r\n", response, sizeof(response), 3000);
                ESP_LOGI(TAG, "Successfully fetched URL via 4G");
                return ESP_OK;
            }
        }
    }
    
    send_at_command("AT+HTTPTERM\r\n", response, sizeof(response), 3000);
    
    // 如果失败，返回错误信息
    snprintf(result, max_len, 
        "<html><body>"
        "<h1>4G Request Failed</h1>"
        "<p>Unable to fetch: %s</p>"
        "<p>4G Status: %s</p>"
        "</body></html>", 
        url, is_4g_connected ? "Connected" : "Disconnected");
        
    return ESP_FAIL;
}

// 网站代理处理器
esp_err_t proxy_handler(httpd_req_t *req)
{
    char *url_param = malloc(512);
    char *content = malloc(8192);
    
    if (!url_param || !content) {
        if (url_param) free(url_param);
        if (content) free(content);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    // 获取URL参数
    if (httpd_req_get_url_query_str(req, url_param, 512) == ESP_OK) {
        char *url = strstr(url_param, "url=");
        if (url) {
            url += 4; // 跳过 "url="
            
            // URL解码（简单版本）
            char decoded_url[256];
            int j = 0;
            for (int i = 0; url[i] && j < 255; i++) {
                if (url[i] == '%' && url[i+1] && url[i+2]) {
                    // 简单的URL解码
                    int hex_val;
                    sscanf(&url[i+1], "%2x", &hex_val);
                    decoded_url[j++] = (char)hex_val;
                    i += 2;
                } else if (url[i] == '&') {
                    break; // 参数结束
                } else {
                    decoded_url[j++] = url[i];
                }
            }
            decoded_url[j] = '\0';
            
            ESP_LOGI(TAG, "Fetching URL: %s", decoded_url);
            
            if (get_url_via_4g(decoded_url, content, 8192) == ESP_OK) {
                httpd_resp_set_type(req, "text/html");
                httpd_resp_send(req, content, strlen(content));
            } else {
                httpd_resp_set_type(req, "text/html");
                httpd_resp_send(req, content, strlen(content)); // 错误信息已在函数中设置
            }
        } else {
            httpd_resp_send_404(req);
        }
    } else {
        httpd_resp_send_404(req);
    }
    
    free(url_param);
    free(content);
    return ESP_OK;
}

// 主页处理器
esp_err_t root_handler(httpd_req_t *req)
{
    const char* html_page = 
        "<!DOCTYPE html>"
        "<html><head>"
        "<title>ESP32-C3 4G Internet Proxy</title>"
        "<meta charset='UTF-8'>"
        "<style>"
        "body { font-family: Arial, sans-serif; margin: 40px; background: #f0f0f0; }"
        ".container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }"
        "h1 { color: #333; text-align: center; }"
        ".status { background: #e8f5e8; padding: 15px; border-radius: 5px; margin: 20px 0; }"
        ".input-group { margin: 20px 0; }"
        "input[type='text'] { width: 70%; padding: 10px; font-size: 16px; border: 1px solid #ddd; border-radius: 5px; }"
        "button { padding: 10px 20px; font-size: 16px; background: #007bff; color: white; border: none; border-radius: 5px; cursor: pointer; }"
        "button:hover { background: #0056b3; }"
        ".quick-links a { display: inline-block; margin: 5px; padding: 8px 15px; background: #28a745; color: white; text-decoration: none; border-radius: 5px; }"
        ".quick-links a:hover { background: #1e7e34; }"
        "</style>"
        "</head><body>"
        "<div class='container'>"
        "<h1>🌐 ESP32-C3 4G Internet Proxy</h1>"
        "<div class='status'>"
        "<strong>✅ System Status:</strong><br>"
        "• 4G Connection: <span style='color: %s'>%s</span><br>"
        "• 4G IP Address: %s<br>"
        "• WiFi Hotspot: %s<br>"
        "• Free Memory: %ld KB"
        "</div>"
        "<div class='input-group'>"
        "<h3>🔗 访问网站</h3>"
        "<form action='/proxy' method='get'>"
        "输入网址: <input type='text' name='url' placeholder='http://www.baidu.com' value='http://www.baidu.com'>"
        "<button type='submit'>通过4G访问</button>"
        "</form>"
        "</div>"
        "<div class='quick-links'>"
        "<h3>🚀 快速链接</h3>"
        "<a href='/proxy?url=http://www.baidu.com'>百度</a>"
        "<a href='/proxy?url=http://www.qq.com'>腾讯</a>"
        "<a href='/proxy?url=http://www.sina.com.cn'>新浪</a>"
        "<a href='/proxy?url=http://httpbin.org/ip'>查看IP</a>"
        "</div>"
        "<p style='text-align: center; color: #666; margin-top: 30px;'>"
        "连接WiFi: %s | 密码: %s<br>"
        "通过4G网络访问互联网"
        "</p>"
        "</div>"
        "<script>"
        "setTimeout(function(){ location.reload(); }, 30000);" // 30秒自动刷新
        "</script>"
        "</body></html>";
    
    char* response = malloc(4096);
    if (!response) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    snprintf(response, 4096, html_page,
        is_4g_connected ? "green" : "red",
        is_4g_connected ? "已连接" : "连接中",
        strlen(ml307r_ip) > 0 ? ml307r_ip : "获取中...",
        WIFI_SSID,
        esp_get_free_heap_size() / 1024,
        WIFI_SSID,
        WIFI_PASS
    );
    
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response, strlen(response));
    
    free(response);
    return ESP_OK;
}

// 启动HTTP服务器
esp_err_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_open_sockets = 7;
    config.max_uri_handlers = 10;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &root_uri);
        
        httpd_uri_t proxy_uri = {
            .uri = "/proxy",
            .method = HTTP_GET,
            .handler = proxy_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &proxy_uri);
        
        ESP_LOGI(TAG, "✅ HTTP server started on port 80");
        return ESP_OK;
    }
    
    ESP_LOGE(TAG, "❌ Failed to start HTTP server");
    return ESP_FAIL;
}

// 初始化4G连接
bool init_4g_connection(void)
{
    char response[512];
    int len;
    
    ESP_LOGI(TAG, "🔄 Initializing 4G Connection...");
    
    // 基本AT测试
    len = send_at_command("AT\r\n", response, sizeof(response), 2000);
    if (len <= 0 || !strstr(response, "OK")) {
        ESP_LOGE(TAG, "❌ ML307R not responding");
        return false;
    }
    ESP_LOGI(TAG, "✅ ML307R responding");
    
    // 关闭回显
    send_at_command("ATE0\r\n", response, sizeof(response), 2000);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 检查SIM卡
    len = send_at_command("AT+CPIN?\r\n", response, sizeof(response), 3000);
    if (len <= 0 || !strstr(response, "READY")) {
        ESP_LOGE(TAG, "❌ SIM card not ready");
        return false;
    }
    ESP_LOGI(TAG, "✅ SIM card ready");
    
    // 设置APN
    send_at_command("AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n", response, sizeof(response), 3000);
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI(TAG, "✅ APN configured");
    
    // 激活PDP上下文
    len = send_at_command("AT+CGACT=1,1\r\n", response, sizeof(response), 10000);
    if (len <= 0 || !strstr(response, "OK")) {
        ESP_LOGE(TAG, "❌ Failed to activate PDP context");
        return false;
    }
    ESP_LOGI(TAG, "✅ PDP context activated");
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 获取IP地址
    len = send_at_command("AT+CGPADDR=1\r\n", response, sizeof(response), 5000);
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
                    ESP_LOGI(TAG, "🌐 4G IP address: %s", ml307r_ip);
                    return true;
                }
            }
        }
    }
    
    ESP_LOGE(TAG, "❌ Failed to get IP address");
    return false;
}

// WiFi事件处理
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
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
    ESP_LOGI(TAG, "📡 Initializing WiFi Access Point...");
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_ap_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = WIFI_CHANNEL,
            .password = WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "✅ WiFi AP started: %s", WIFI_SSID);
}

// 4G连接任务
void ml307r_4g_task(void *pvParameters)
{
    ESP_LOGI(TAG, "🚀 Starting 4G connection task...");
    
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    if (init_4g_connection()) {
        ESP_LOGI(TAG, "🎉 4G connection established successfully!");
        is_4g_connected = true;
        xEventGroupSetBits(s_wifi_event_group, ML307R_READY_BIT);
    } else {
        ESP_LOGE(TAG, "💔 Failed to establish 4G connection");
    }
    
    vTaskDelete(NULL);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "🎯 ESP32-C3 4G Internet Proxy Starting...");
    
    s_wifi_event_group = xEventGroupCreate();
    
    if (init_uart() != ESP_OK) {
        ESP_LOGE(TAG, "❌ Failed to initialize UART");
        return;
    }
    
    init_wifi_ap();
    start_webserver();
    
    xTaskCreate(ml307r_4g_task, "ml307r_4g", 8192, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "🔮 System Ready!");
    ESP_LOGI(TAG, "📶 WiFi Hotspot: %s", WIFI_SSID);
    ESP_LOGI(TAG, "🔑 Password: %s", WIFI_PASS);
    ESP_LOGI(TAG, "🌐 Web Interface: http://192.168.4.1");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "📋 Usage Instructions:");
    ESP_LOGI(TAG, "1. Connect device to WiFi: %s", WIFI_SSID);
    ESP_LOGI(TAG, "2. Open browser and go to: http://192.168.4.1");
    ESP_LOGI(TAG, "3. Enter website URL to access via 4G");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000));
        if (is_4g_connected) {
            ESP_LOGI(TAG, "💚 System Status: 4G Connected, Proxy Active");
        } else {
            ESP_LOGI(TAG, "💛 System Status: 4G Connecting...");
        }
    }
}
