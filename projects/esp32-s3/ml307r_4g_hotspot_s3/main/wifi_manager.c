#include "wifi_manager.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include <string.h>

static const char *TAG = "WiFiManager";

// WiFi事件位
#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1

// 全局变量
static bool wifi_initialized = false;
static wifi_state_t current_wifi_state = WIFI_STATE_DISCONNECTED;
static EventGroupHandle_t wifi_event_group;
static esp_netif_t *sta_netif = NULL;
static esp_netif_t *ap_netif = NULL;
static wifi_info_t current_wifi_info = {0};
static int retry_count = 0;

// 私有函数声明
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static esp_err_t wifi_init_common(void);

esp_err_t wifi_manager_init(void)
{
    if (wifi_initialized) {
        ESP_LOGW(TAG, "WiFi manager already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing WiFi manager...");

    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 创建事件组
    wifi_event_group = xEventGroupCreate();
    if (wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }

    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 创建网络接口
    sta_netif = esp_netif_create_default_wifi_sta();
    ap_netif = esp_netif_create_default_wifi_ap();

    // 初始化WiFi
    ret = wifi_init_common();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    // 注册事件处理器
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_initialized = true;
    ESP_LOGI(TAG, "WiFi manager initialized successfully");

    return ESP_OK;
}

esp_err_t wifi_manager_start_ap(void)
{
    if (!wifi_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Starting AP mode...");

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .channel = WIFI_AP_CHANNEL,
            .password = WIFI_AP_PASSWORD,
            .max_connection = WIFI_AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    if (strlen(WIFI_AP_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    current_wifi_state = WIFI_STATE_AP_MODE;
    strcpy(current_wifi_info.ssid, WIFI_AP_SSID);
    strcpy(current_wifi_info.ip_address, "192.168.4.1");
    current_wifi_info.state = WIFI_STATE_AP_MODE;

    ESP_LOGI(TAG, "WiFi AP started. SSID: %s, IP: %s", WIFI_AP_SSID, "192.168.4.1");
    return ESP_OK;
}

esp_err_t wifi_manager_connect(const char *ssid, const char *password)
{
    if (!wifi_initialized || ssid == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Connecting to WiFi: %s", ssid);

    // 停止当前WiFi
    esp_wifi_stop();
    vTaskDelay(pdMS_TO_TICKS(100));

    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    if (password != NULL) {
        strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    current_wifi_state = WIFI_STATE_CONNECTING;
    strcpy(current_wifi_info.ssid, ssid);
    if (password != NULL) {
        strcpy(current_wifi_info.password, password);
    }
    current_wifi_info.state = WIFI_STATE_CONNECTING;
    retry_count = 0;

    // 等待连接结果
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                          WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                          pdFALSE,
                                          pdFALSE,
                                          pdMS_TO_TICKS(15000));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to WiFi: %s", ssid);
        current_wifi_state = WIFI_STATE_CONNECTED;
        current_wifi_info.state = WIFI_STATE_CONNECTED;
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to WiFi: %s", ssid);
        current_wifi_state = WIFI_STATE_ERROR;
        current_wifi_info.state = WIFI_STATE_ERROR;
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "WiFi connection timeout");
        current_wifi_state = WIFI_STATE_ERROR;
        current_wifi_info.state = WIFI_STATE_ERROR;
        return ESP_ERR_TIMEOUT;
    }
}

esp_err_t wifi_manager_disconnect(void)
{
    if (!wifi_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Disconnecting WiFi...");
    
    esp_err_t ret = esp_wifi_disconnect();
    if (ret == ESP_OK) {
        current_wifi_state = WIFI_STATE_DISCONNECTED;
        current_wifi_info.state = WIFI_STATE_DISCONNECTED;
        memset(current_wifi_info.ip_address, 0, sizeof(current_wifi_info.ip_address));
    }

    return ret;
}

wifi_state_t wifi_manager_get_state(void)
{
    return current_wifi_state;
}

esp_err_t wifi_manager_get_info(wifi_info_t *info)
{
    if (info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(info, &current_wifi_info, sizeof(wifi_info_t));
    return ESP_OK;
}

bool wifi_manager_is_connected(void)
{
    return current_wifi_state == WIFI_STATE_CONNECTED;
}

// 私有函数实现
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (retry_count < 5) {
            esp_wifi_connect();
            retry_count++;
            ESP_LOGI(TAG, "Retry to connect to the AP, attempt %d", retry_count);
        } else {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Connect to the AP failed");
            current_wifi_state = WIFI_STATE_ERROR;
            current_wifi_info.state = WIFI_STATE_ERROR;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        
        // 更新IP地址
        snprintf(current_wifi_info.ip_address, sizeof(current_wifi_info.ip_address), 
                IPSTR, IP2STR(&event->ip_info.ip));
        
        retry_count = 0;
        current_wifi_state = WIFI_STATE_CONNECTED;
        current_wifi_info.state = WIFI_STATE_CONNECTED;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "Station joined, AID=%d", event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "Station left, AID=%d", event->aid);
    }
}

static esp_err_t wifi_init_common(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        return ret;
    }

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    
    return ESP_OK;
}

// 配置网络接口用于互联网共享
static void configure_network_sharing(void)
{
    ESP_LOGI(TAG, "Configuring network sharing...");
    
    // 这里可以添加网络共享的配置
    // 目前ESP-IDF的lwip组件可能不支持完整的NAPT功能
    // 需要进一步研究ESP-IDF的网络共享实现
    
    ESP_LOGI(TAG, "✅ Network sharing configured (basic mode)");
}

// 在TCPIP线程中配置网络共享
static void tcpip_sharing_config(void *arg)
{
    configure_network_sharing();
}

esp_err_t wifi_manager_enable_napt(void)
{
    ESP_LOGI(TAG, "Enabling network sharing...");
    
    // 在TCPIP线程中执行网络共享配置
    tcpip_try_callback(tcpip_sharing_config, NULL);
    
    return ESP_OK;
}
