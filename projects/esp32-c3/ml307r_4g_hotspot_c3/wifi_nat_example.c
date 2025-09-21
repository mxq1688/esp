// 这是一个ESP32 WiFi NAT的完整工作示例
// 展示为什么WiFi NAT这么简单

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"

static const char *TAG = "WIFI_NAT_EXAMPLE";

// WiFi配置
#define ROUTER_SSID "Your_Router"     // 上级路由器
#define ROUTER_PASS "router_password" 
#define HOTSPOT_SSID "ESP32_Hotspot"  // 创建的热点
#define HOTSPOT_PASS "12345678"

static esp_netif_t *sta_netif = NULL;  // Station接口（连路由器）
static esp_netif_t *ap_netif = NULL;   // AP接口（创建热点）

// WiFi事件处理
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "✅ Connected to router");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "🌐 Got IP from router: " IPSTR, IP2STR(&event->ip_info.ip));
        
        // 🔥 关键：启用NAT转发（就这一行！）
        esp_netif_napt_enable(ap_netif);
        ESP_LOGI(TAG, "🚀 NAT enabled! Devices can now use internet via hotspot!");
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "📱 Device connected to hotspot: " MACSTR, MAC2STR(event->mac));
        ESP_LOGI(TAG, "🌟 This device now has internet access through NAT!");
    }
}

// 初始化WiFi NAT热点
void init_wifi_nat_hotspot(void)
{
    ESP_LOGI(TAG, "🔥 Initializing WiFi NAT Hotspot...");
    
    // 1. 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // 2. 创建Station和AP接口
    sta_netif = esp_netif_create_default_wifi_sta();  // 连接路由器
    ap_netif = esp_netif_create_default_wifi_ap();    // 创建热点
    
    // 3. 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // 4. 注册事件处理
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    
    // 5. 配置Station（连接上级路由器）
    wifi_config_t sta_config = {
        .sta = {
            .ssid = ROUTER_SSID,
            .password = ROUTER_PASS,
        },
    };
    
    // 6. 配置AP（创建热点）
    wifi_config_t ap_config = {
        .ap = {
            .ssid = HOTSPOT_SSID,
            .ssid_len = strlen(HOTSPOT_SSID),
            .password = HOTSPOT_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .channel = 1,
        },
    };
    
    // 7. 启动WiFi（同时Station和AP模式）
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));  // 🔥 关键：同时模式
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "✅ WiFi NAT hotspot initialized!");
    ESP_LOGI(TAG, "📡 Connecting to router: %s", ROUTER_SSID);
    ESP_LOGI(TAG, "📱 Hotspot created: %s", HOTSPOT_SSID);
    ESP_LOGI(TAG, "🔑 Hotspot password: %s", HOTSPOT_PASS);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "💡 How it works:");
    ESP_LOGI(TAG, "   1. ESP32 connects to your router (%s)", ROUTER_SSID);
    ESP_LOGI(TAG, "   2. ESP32 creates its own hotspot (%s)", HOTSPOT_SSID);
    ESP_LOGI(TAG, "   3. WiFi chip automatically forwards packets between them");
    ESP_LOGI(TAG, "   4. Devices connecting to hotspot get internet via router");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "🚀 This is why WiFi NAT is so EASY!");
}

void app_main(void)
{
    ESP_LOGI(TAG, "🔥🔥🔥 ESP32 WiFi NAT Example 🔥🔥🔥");
    ESP_LOGI(TAG, "This shows why WiFi NAT is simple but 4G NAT is hard!");
    
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 启动WiFi NAT热点
    init_wifi_nat_hotspot();
    
    // 主循环
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "💚 WiFi NAT running smoothly!");
    }
}

/* 
🎯 为什么这个方案这么简单？

1. **硬件层面**：
   - ESP32的WiFi芯片本身支持Station+AP同时工作
   - 芯片内部有专门的包转发电路
   - 所有数据都在同一个WiFi芯片内处理

2. **软件层面**：
   - esp_netif_napt_enable() 一行代码启用NAT
   - LWIP协议栈自动处理所有转发逻辑
   - 不需要手动处理任何网络包

3. **协议层面**：
   - 上游（路由器）和下游（热点设备）都是标准WiFi/IP协议
   - 不需要协议转换
   - 内存可以直接共享

对比4G方案：
❌ 需要UART通信
❌ 需要AT指令转换
❌ 需要手动实现所有NAT逻辑
❌ 需要管理连接状态
❌ 性能和稳定性问题

这就是为什么ESP32做WiFi中继器轻松，做4G热点困难的原因！
*/
