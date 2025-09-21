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
#include "ml307r.h"

static const char *TAG = "ML307R_4G_HOTSPOT";

// WiFi热点配置
#define WIFI_SSID      "ESP32_4G_Hotspot"
#define WIFI_PASS      "12345678"
#define WIFI_CHANNEL   1
#define MAX_STA_CONN   4

// ML307R实例
static ml307r_t ml307r_instance;

// 事件组位
#define WIFI_CONNECTED_BIT BIT0
#define ML307R_READY_BIT   BIT1

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

// WiFi事件处理函数
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

// 初始化WiFi热点
void wifi_init_softap(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
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
    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);
}

// 初始化ML307R
esp_err_t ml307r_init_hardware(void)
{
    // 配置ML307R参数
    ml307r_instance.uart_num = ML307R_UART_NUM;
    ml307r_instance.txd_pin = ML307R_TXD_PIN;
    ml307r_instance.rxd_pin = ML307R_RXD_PIN;
    ml307r_instance.baud_rate = ML307R_BAUD_RATE;
    ml307r_instance.initialized = false;
    
    // 初始化ML307R
    esp_err_t ret = ml307r_init(&ml307r_instance);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ML307R: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "ML307R hardware initialized");
    return ESP_OK;
}


// ML307R连接任务
void ml307r_connect_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Starting ML307R 4G connection...");
    
    vTaskDelay(3000 / portTICK_PERIOD_MS); // 等待模块启动
    
    // 执行完整的4G连接流程
    ml307r_status_t status = ml307r_connect_4g(&ml307r_instance);
    
    if (status == ML307R_OK) {
        ESP_LOGI(TAG, "ML307R 4G connection successful!");
        ESP_LOGI(TAG, "IP Address: %s", ml307r_instance.ip_address);
        xEventGroupSetBits(s_wifi_event_group, ML307R_READY_BIT);
    } else {
        ESP_LOGE(TAG, "ML307R 4G connection failed with status: %d", status);
        // 即使4G连接失败，也继续启动WiFi热点
        xEventGroupSetBits(s_wifi_event_group, ML307R_READY_BIT);
    }
    
    vTaskDelete(NULL);
}

// 主任务
void app_main(void)
{
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP32 ML307R 4G Hotspot Project Starting...");
    
    // 初始化ML307R硬件
    if (ml307r_init_hardware() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ML307R hardware");
        return;
    }
    
    // 初始化WiFi热点
    wifi_init_softap();
    
    // 创建ML307R连接任务
    xTaskCreate(ml307r_connect_task, "ml307r_connect", 8192, NULL, 5, NULL);
    
    // 等待ML307R就绪
    xEventGroupWaitBits(s_wifi_event_group, ML307R_READY_BIT, false, true, portMAX_DELAY);
    
    ESP_LOGI(TAG, "System ready! WiFi hotspot: %s, Password: %s", WIFI_SSID, WIFI_PASS);
    ESP_LOGI(TAG, "Connect your devices to the hotspot to access 4G internet");
    
    // 主循环
    while (1) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "System running...");
    }
}
