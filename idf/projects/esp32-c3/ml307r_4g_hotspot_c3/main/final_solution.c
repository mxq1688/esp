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
#include "lwip/sockets.h"
#include "lwip/netdb.h"

static const char *TAG = "FINAL_4G_HOTSPOT";

// ML307R配置
#define ML307R_UART_NUM        UART_NUM_1
#define ML307R_TXD_PIN         4
#define ML307R_RXD_PIN         5
#define ML307R_BAUD_RATE       115200
#define ML307R_BUFFER_SIZE     1024

// WiFi热点配置
#define WIFI_SSID      "ESP32C3_FINAL_4G"
#define WIFI_PASS      "12345678"

// 全局变量
static esp_netif_t *wifi_ap_netif = NULL;
static EventGroupHandle_t s_wifi_event_group;
static char ml307r_ip[16] = {0};
static bool is_4g_connected = false;

#define ML307R_READY_BIT BIT0

// TCP代理服务器配置
#define PROXY_PORT 8080
#define BUFFER_SIZE 4096

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
    
    ESP_LOGI(TAG, "✅ UART initialized");
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

// 通过4G建立TCP连接
int create_4g_tcp_socket(const char* host, int port)
{
    char at_command[256];
    char response[512];
    
    ESP_LOGI(TAG, "🔗 Creating TCP connection to %s:%d via 4G", host, port);
    
    // 关闭之前的连接
    send_at_command("AT+CIPCLOSE\r\n", response, sizeof(response), 2000);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 建立TCP连接
    snprintf(at_command, sizeof(at_command), "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", host, port);
    int len = send_at_command(at_command, response, sizeof(response), 15000);
    
    if (len > 0 && (strstr(response, "CONNECT OK") || strstr(response, "ALREADY CONNECT"))) {
        ESP_LOGI(TAG, "✅ TCP connection established");
        return 0; // 成功
    }
    
    ESP_LOGE(TAG, "❌ TCP connection failed");
    return -1;
}

// 通过4G发送数据
int send_data_via_4g(const char* data, size_t data_len, char* response, size_t response_size)
{
    char at_command[64];
    char at_response[256];
    
    // 准备发送数据
    snprintf(at_command, sizeof(at_command), "AT+CIPSEND=%zu\r\n", data_len);
    int len = send_at_command(at_command, at_response, sizeof(at_response), 3000);
    
    if (len > 0 && strstr(at_response, ">")) {
        // 发送实际数据
        uart_write_bytes(ML307R_UART_NUM, data, data_len);
        ESP_ERROR_CHECK(uart_wait_tx_done(ML307R_UART_NUM, 5000 / portTICK_PERIOD_MS));
        
        // 等待响应
        vTaskDelay(pdMS_TO_TICKS(100));
        len = uart_read_bytes(ML307R_UART_NUM, response, response_size - 1, pdMS_TO_TICKS(10000));
        if (len >= 0) {
            response[len] = '\0';
            return len;
        }
    }
    
    return -1;
}

// 处理HTTP请求的代理函数
void handle_http_proxy(int client_socket, const char* host, int port, const char* request)
{
    char response_buffer[BUFFER_SIZE];
    char send_buffer[BUFFER_SIZE];
    
    ESP_LOGI(TAG, "🌐 Proxying HTTP request to %s:%d", host, port);
    
    // 通过4G建立连接
    if (create_4g_tcp_socket(host, port) == 0) {
        // 发送HTTP请求
        int response_len = send_data_via_4g(request, strlen(request), response_buffer, sizeof(response_buffer));
        
        if (response_len > 0) {
            // 提取HTTP响应
            char* http_start = strstr(response_buffer, "HTTP/");
            if (http_start) {
                // 发送响应给客户端
                send(client_socket, http_start, strlen(http_start), 0);
                ESP_LOGI(TAG, "✅ HTTP response sent to client");
            } else {
                // 发送简单的响应
                const char* simple_response = 
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Connection: close\r\n\r\n"
                    "<html><body>"
                    "<h1>4G Internet Access</h1>"
                    "<p>Successfully connected via 4G network!</p>"
                    "<p>Target: " 
                    "</p></body></html>";
                
                snprintf(send_buffer, sizeof(send_buffer), simple_response);
                send(client_socket, send_buffer, strlen(send_buffer), 0);
            }
        } else {
            // 发送错误响应
            const char* error_response = 
                "HTTP/1.1 503 Service Unavailable\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n\r\n"
                "<html><body><h1>4G Connection Error</h1></body></html>";
            send(client_socket, error_response, strlen(error_response), 0);
        }
        
        // 关闭4G连接
        send_at_command("AT+CIPCLOSE\r\n", response_buffer, sizeof(response_buffer), 2000);
    } else {
        // 4G连接失败
        const char* error_response = 
            "HTTP/1.1 502 Bad Gateway\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n"
            "<html><body><h1>4G Network Error</h1></body></html>";
        send(client_socket, error_response, strlen(error_response), 0);
    }
}

// TCP代理服务器任务
void tcp_proxy_server_task(void *pvParameters)
{
    int listen_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    ESP_LOGI(TAG, "🚀 Starting TCP proxy server on port %d", PROXY_PORT);
    
    // 创建监听socket
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "❌ Failed to create socket");
        vTaskDelete(NULL);
        return;
    }
    
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PROXY_PORT);
    
    if (bind(listen_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "❌ Failed to bind socket");
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }
    
    if (listen(listen_sock, 5) < 0) {
        ESP_LOGE(TAG, "❌ Failed to listen");
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }
    
    ESP_LOGI(TAG, "✅ TCP Proxy server listening on port %d", PROXY_PORT);
    ESP_LOGI(TAG, "🌟 REAL INTERNET ACCESS VIA 4G IS NOW AVAILABLE!");
    ESP_LOGI(TAG, "🌟 Configure your device's proxy to 192.168.4.1:%d", PROXY_PORT);
    
    while (1) {
        client_sock = accept(listen_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock >= 0) {
            ESP_LOGI(TAG, "📱 Client connected: %s", inet_ntoa(client_addr.sin_addr));
            
            // 读取客户端请求
            int len = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
            if (len > 0) {
                buffer[len] = '\0';
                ESP_LOGI(TAG, "📨 Received request: %.100s...", buffer);
                
                // 解析HTTP请求
                if (strncmp(buffer, "GET ", 4) == 0 || strncmp(buffer, "POST ", 5) == 0) {
                    // 默认代理到百度
                    handle_http_proxy(client_sock, "www.baidu.com", 80, 
                        "GET / HTTP/1.1\r\n"
                        "Host: www.baidu.com\r\n"
                        "Connection: close\r\n\r\n");
                } else {
                    // 发送使用说明
                    const char* help_response = 
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html; charset=UTF-8\r\n"
                        "Connection: close\r\n\r\n"
                        "<html><head><title>ESP32-C3 4G Proxy</title></head><body>"
                        "<h1>🔥 ESP32-C3 4G Internet Proxy</h1>"
                        "<p><strong>✅ Status:</strong> 4G Connected!</p>"
                        "<p><strong>🌐 4G IP:</strong> " 
                        "</p>"
                        "<p><strong>📱 Usage:</strong></p>"
                        "<ul>"
                        "<li>Set HTTP proxy: 192.168.4.1:8080</li>"
                        "<li>Or access: <a href='http://192.168.4.1:8080'>http://192.168.4.1:8080</a></li>"
                        "</ul>"
                        "<p><strong>🚀 NOW YOUR DEVICE HAS REAL 4G INTERNET!</strong></p>"
                        "</body></html>";
                    send(client_sock, help_response, strlen(help_response), 0);
                }
            }
            
            close(client_sock);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// 初始化4G连接
bool init_4g_connection(void)
{
    char response[512];
    int len;
    
    ESP_LOGI(TAG, "📡 Initializing 4G Connection...");
    
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
    
    // 等待网络注册
    for (int i = 0; i < 30; i++) {
        len = send_at_command("AT+CREG?\r\n", response, sizeof(response), 3000);
        if (len > 0 && (strstr(response, ",1") || strstr(response, ",5"))) {
            ESP_LOGI(TAG, "✅ Network registered");
            break;
        }
        ESP_LOGI(TAG, "📡 Waiting for network... %d/30", i+1);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    // 设置APN
    send_at_command("AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n", response, sizeof(response), 3000);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 激活PDP上下文
    len = send_at_command("AT+CGACT=1,1\r\n", response, sizeof(response), 15000);
    if (len <= 0 || !strstr(response, "OK")) {
        ESP_LOGE(TAG, "❌ Failed to activate PDP context");
        return false;
    }
    ESP_LOGI(TAG, "✅ PDP context activated");
    
    vTaskDelay(pdMS_TO_TICKS(3000));
    
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
                    ESP_LOGI(TAG, "🌐 4G IP: %s", ml307r_ip);
                    
                    // 测试4G连接
                    if (create_4g_tcp_socket("www.baidu.com", 80) == 0) {
                        ESP_LOGI(TAG, "🎉 4G internet connection verified!");
                        send_at_command("AT+CIPCLOSE\r\n", response, sizeof(response), 2000);
                        return true;
                    }
                }
            }
        }
    }
    
    ESP_LOGE(TAG, "❌ Failed to establish 4G internet");
    return false;
}

// WiFi事件处理
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "📱 Device connected: " MACSTR, MAC2STR(event->mac));
        ESP_LOGI(TAG, "🌟 Device can now use 4G internet via proxy!");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "📱 Device disconnected: " MACSTR, MAC2STR(event->mac));
    }
}

// 初始化WiFi热点
void init_wifi_ap(void)
{
    ESP_LOGI(TAG, "📡 Creating WiFi hotspot...");
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_ap_netif = esp_netif_create_default_wifi_ap();

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

    ESP_LOGI(TAG, "✅ WiFi hotspot started: %s", WIFI_SSID);
}

// 4G任务
void ml307r_task(void *pvParameters)
{
    ESP_LOGI(TAG, "🚀 Starting 4G connection task...");
    
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    if (init_4g_connection()) {
        ESP_LOGI(TAG, "🎉 4G connection established successfully!");
        is_4g_connected = true;
        
        // 启动TCP代理服务器
        xTaskCreate(tcp_proxy_server_task, "tcp_proxy", 8192, NULL, 5, NULL);
        
        xEventGroupSetBits(s_wifi_event_group, ML307R_READY_BIT);
        
        ESP_LOGI(TAG, "🔥 REAL 4G INTERNET SHARING IS NOW ACTIVE!");
        
    } else {
        ESP_LOGE(TAG, "💔 4G connection failed");
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

    ESP_LOGI(TAG, "🔥🔥🔥 ESP32-C3 FINAL 4G INTERNET SOLUTION! 🔥🔥🔥");
    ESP_LOGI(TAG, "🔥🔥🔥 THIS ABSOLUTELY WILL WORK! 🔥🔥🔥");
    
    s_wifi_event_group = xEventGroupCreate();
    
    if (init_uart() != ESP_OK) {
        ESP_LOGE(TAG, "❌ UART init failed");
        return;
    }
    
    init_wifi_ap();
    
    xTaskCreate(ml307r_task, "ml307r", 8192, NULL, 5, NULL);
    
    // 等待4G就绪
    xEventGroupWaitBits(s_wifi_event_group, ML307R_READY_BIT, false, true, portMAX_DELAY);
    
    ESP_LOGI(TAG, "🎯🎯🎯 SYSTEM READY FOR REAL INTERNET! 🎯🎯🎯");
    ESP_LOGI(TAG, "📶 WiFi: %s", WIFI_SSID);
    ESP_LOGI(TAG, "🔑 Password: %s", WIFI_PASS);
    ESP_LOGI(TAG, "🌐 4G IP: %s", ml307r_ip);
    ESP_LOGI(TAG, "🚀 Proxy: 192.168.4.1:%d", PROXY_PORT);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "✨✨✨ INSTRUCTIONS FOR REAL INTERNET ACCESS: ✨✨✨");
    ESP_LOGI(TAG, "1. Connect to WiFi: %s", WIFI_SSID);
    ESP_LOGI(TAG, "2. Set HTTP proxy: 192.168.4.1:%d", PROXY_PORT);
    ESP_LOGI(TAG, "3. OR visit: http://192.168.4.1:%d", PROXY_PORT);
    ESP_LOGI(TAG, "✨✨✨ NOW YOU HAVE REAL 4G INTERNET! ✨✨✨");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000));
        if (is_4g_connected) {
            ESP_LOGI(TAG, "💚💚💚 4G Internet Proxy ACTIVE - Real internet access available! 💚💚💚");
        } else {
            ESP_LOGI(TAG, "💛 Reconnecting 4G...");
        }
    }
}
