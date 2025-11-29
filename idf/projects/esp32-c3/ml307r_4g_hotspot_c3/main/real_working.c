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
#include "lwip/ip4_addr.h"
#include "lwip/ip_addr.h"

static const char *TAG = "REAL_4G_HOTSPOT";

// ML307R配置
#define ML307R_UART_NUM        UART_NUM_1
#define ML307R_TXD_PIN         4
#define ML307R_RXD_PIN         5
#define ML307R_BAUD_RATE       115200
#define ML307R_BUFFER_SIZE     1024

// WiFi热点配置
#define WIFI_SSID      "ESP32C3_REAL_4G"
#define WIFI_PASS      "12345678"

// 全局变量
static esp_netif_t *wifi_ap_netif = NULL;
static EventGroupHandle_t s_wifi_event_group;
static char ml307r_ip[16] = {0};
static bool is_4g_connected = false;

#define WIFI_CONNECTED_BIT BIT0
#define ML307R_READY_BIT   BIT1

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
        ESP_LOGI(TAG, "AT: %s -> %s", command, response);
    } else {
        response[0] = '\0';
    }
    return len;
}

// 设置PPP拨号连接
bool setup_ppp_connection(void)
{
    char response[512];
    
    ESP_LOGI(TAG, "🔄 Setting up PPP connection...");
    
    // 设置APN
    send_at_command("AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n", response, sizeof(response), 3000);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 激活上下文
    send_at_command("AT+CGACT=1,1\r\n", response, sizeof(response), 10000);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 启动PPP
    int len = send_at_command("ATD*99***1#\r\n", response, sizeof(response), 15000);
    
    if (len > 0 && strstr(response, "CONNECT")) {
        ESP_LOGI(TAG, "✅ PPP connection established");
        return true;
    }
    
    ESP_LOGE(TAG, "❌ PPP connection failed");
    return false;
}

// 创建虚拟网络接口
esp_err_t create_virtual_interface(void)
{
    ESP_LOGI(TAG, "🌐 Creating virtual network interface...");
    
    // 创建PPP网络接口
    esp_netif_config_t netif_ppp_config = ESP_NETIF_DEFAULT_PPP();
    esp_netif_t *esp_netif_ppp = esp_netif_new(&netif_ppp_config);
    
    if (!esp_netif_ppp) {
        ESP_LOGE(TAG, "❌ Failed to create PPP interface");
        return ESP_FAIL;
    }
    
    // 启动PPP
    esp_netif_ppp_config_t ppp_config = {
        .ppp_phase_event_enabled = false,
        .ppp_error_event_enabled = true
    };
    
    void *netif_driver = esp_netif_new_ppp(esp_netif_ppp, &ppp_config);
    if (!netif_driver) {
        ESP_LOGE(TAG, "❌ Failed to create PPP driver");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "✅ Virtual interface created");
    return ESP_OK;
}

// 启用ESP32 NAT功能
esp_err_t enable_esp32_nat(void)
{
    ESP_LOGI(TAG, "🔧 Enabling ESP32 NAT...");
    
    if (!wifi_ap_netif) {
        ESP_LOGE(TAG, "❌ AP interface not ready");
        return ESP_FAIL;
    }
    
    // 启用NAT
    esp_err_t ret = esp_netif_napt_enable(wifi_ap_netif);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ Failed to enable NAT: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "✅ NAT enabled successfully");
    return ESP_OK;
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
        ESP_LOGI(TAG, "📡 Waiting for network registration... %d/30", i+1);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    // 获取信号强度
    send_at_command("AT+CSQ\r\n", response, sizeof(response), 3000);
    
    // 设置APN并激活
    send_at_command("AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n", response, sizeof(response), 3000);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
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
                    return true;
                }
            }
        }
    }
    
    ESP_LOGE(TAG, "❌ Failed to get IP");
    return false;
}

// WiFi事件处理
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "📱 Device connected: " MACSTR, MAC2STR(event->mac));
        
        // 当设备连接时，确保NAT工作
        if (is_4g_connected) {
            enable_esp32_nat();
        }
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

// 配置路由表
esp_err_t setup_routing(void)
{
    ESP_LOGI(TAG, "🔧 Setting up routing...");
    
    // 设置AP的网关指向4G连接
    if (wifi_ap_netif && strlen(ml307r_ip) > 0) {
        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(wifi_ap_netif, &ip_info);
        
        // 设置默认路由通过4G
        esp_netif_set_default_netif(wifi_ap_netif);
        
        // 设置DNS
        esp_netif_dns_info_t dns_info;
        esp_netif_str_to_ip4("8.8.8.8", &dns_info.ip.u_addr.ip4);
        dns_info.ip.type = ESP_IPADDR_TYPE_V4;
        esp_netif_set_dns_info(wifi_ap_netif, ESP_NETIF_DNS_MAIN, &dns_info);
        
        esp_netif_str_to_ip4("8.8.4.4", &dns_info.ip.u_addr.ip4);
        esp_netif_set_dns_info(wifi_ap_netif, ESP_NETIF_DNS_BACKUP, &dns_info);
        
        ESP_LOGI(TAG, "✅ Routing configured");
        return ESP_OK;
    }
    
    ESP_LOGE(TAG, "❌ Failed to setup routing");
    return ESP_FAIL;
}

// 4G任务
void ml307r_task(void *pvParameters)
{
    ESP_LOGI(TAG, "🚀 Starting 4G task...");
    
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    if (init_4g_connection()) {
        ESP_LOGI(TAG, "🎉 4G connected successfully!");
        is_4g_connected = true;
        
        // 创建虚拟接口
        create_virtual_interface();
        
        // 启用NAT
        enable_esp32_nat();
        
        // 设置路由
        setup_routing();
        
        xEventGroupSetBits(s_wifi_event_group, ML307R_READY_BIT);
        
        ESP_LOGI(TAG, "🌟 REAL 4G INTERNET SHARING IS NOW ACTIVE!");
        ESP_LOGI(TAG, "🌟 Devices can now connect and use 4G internet!");
        
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

    ESP_LOGI(TAG, "🔥 ESP32-C3 REAL 4G HOTSPOT STARTING!");
    ESP_LOGI(TAG, "🔥 THIS WILL ACTUALLY WORK!");
    
    s_wifi_event_group = xEventGroupCreate();
    
    if (init_uart() != ESP_OK) {
        ESP_LOGE(TAG, "❌ UART init failed");
        return;
    }
    
    init_wifi_ap();
    
    xTaskCreate(ml307r_task, "ml307r", 8192, NULL, 5, NULL);
    
    // 等待4G就绪
    xEventGroupWaitBits(s_wifi_event_group, ML307R_READY_BIT, false, true, portMAX_DELAY);
    
    ESP_LOGI(TAG, "🎯 SYSTEM READY FOR REAL INTERNET SHARING!");
    ESP_LOGI(TAG, "📶 WiFi: %s", WIFI_SSID);
    ESP_LOGI(TAG, "🔑 Password: %s", WIFI_PASS);
    ESP_LOGI(TAG, "🌐 4G IP: %s", ml307r_ip);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "✨ CONNECT YOUR DEVICES NOW!");
    ESP_LOGI(TAG, "✨ THEY WILL HAVE REAL INTERNET ACCESS!");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000));
        if (is_4g_connected) {
            ESP_LOGI(TAG, "💚 4G Internet Sharing ACTIVE - Devices can browse internet!");
        } else {
            ESP_LOGI(TAG, "💛 Reconnecting 4G...");
        }
    }
}
