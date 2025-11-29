#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nrf24l01_driver.h"
#include "nrf24l01_config.h"

static const char *TAG = "NRF24L01_MAIN";

// 全局变量
static nrf24l01_config_t nrf24l01_config;
static nrf24l01_app_config_t app_config;
static TaskHandle_t sender_task_handle = NULL;
static TaskHandle_t receiver_task_handle = NULL;
static QueueHandle_t data_queue = NULL;
static bool app_running = false;

// 数据包结构
typedef struct {
    uint8_t data[32];
    uint8_t length;
    uint32_t timestamp;
    uint16_t sequence;
} data_packet_t;

// 发送任务
static void sender_task(void *pvParameters)
{
    data_packet_t packet;
    uint16_t sequence = 0;
    esp_err_t ret;
    
    ESP_LOGI(TAG, "发送任务启动");
    
    while (app_running) {
        // 准备数据包
        packet.sequence = sequence++;
        packet.timestamp = esp_timer_get_time() / 1000; // 转换为毫秒
        packet.length = snprintf((char*)packet.data, sizeof(packet.data), 
                               "Hello NRF24L01! Seq:%d Time:%lu", 
                               packet.sequence, packet.timestamp);
        
        // 发送数据包
        ret = nrf24l01_send_packet(packet.data, packet.length);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "发送数据包 #%d: %s", packet.sequence, packet.data);
        } else {
            ESP_LOGE(TAG, "发送数据包失败: %s", esp_err_to_name(ret));
        }
        
        // 等待发送间隔
        vTaskDelay(pdMS_TO_TICKS(app_config.send_interval_ms));
    }
    
    ESP_LOGI(TAG, "发送任务结束");
    vTaskDelete(NULL);
}

// 接收任务
static void receiver_task(void *pvParameters)
{
    nrf24l01_packet_t rx_packet;
    esp_err_t ret;
    
    ESP_LOGI(TAG, "接收任务启动");
    
    // 设置为接收模式
    ret = nrf24l01_set_mode_rx();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置接收模式失败: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);
        return;
    }
    
    while (app_running) {
        // 检查是否有数据
        ret = nrf24l01_available();
        if (ret == ESP_OK) {
            // 接收数据包
            ret = nrf24l01_receive_packet(&rx_packet);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "接收数据包: %.*s", rx_packet.length, rx_packet.data);
                
                // 将数据放入队列
                if (data_queue != NULL) {
                    data_packet_t queue_packet;
                    memcpy(queue_packet.data, rx_packet.data, rx_packet.length);
                    queue_packet.length = rx_packet.length;
                    queue_packet.timestamp = esp_timer_get_time() / 1000;
                    queue_packet.sequence = 0; // 接收端不维护序列号
                    
                    if (xQueueSend(data_queue, &queue_packet, 0) != pdTRUE) {
                        ESP_LOGW(TAG, "数据队列已满，丢弃数据包");
                    }
                }
            } else {
                ESP_LOGE(TAG, "接收数据包失败: %s", esp_err_to_name(ret));
            }
        }
        
        // 短暂延时
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    ESP_LOGI(TAG, "接收任务结束");
    vTaskDelete(NULL);
}

// 数据处理任务
static void data_processor_task(void *pvParameters)
{
    data_packet_t packet;
    
    ESP_LOGI(TAG, "数据处理任务启动");
    
    while (app_running) {
        // 从队列获取数据
        if (xQueueReceive(data_queue, &packet, pdMS_TO_TICKS(100)) == pdTRUE) {
            ESP_LOGI(TAG, "处理数据包: %.*s", packet.length, packet.data);
            
            // 这里可以添加数据处理逻辑
            // 例如：解析命令、控制其他设备等
        }
    }
    
    ESP_LOGI(TAG, "数据处理任务结束");
    vTaskDelete(NULL);
}

// 初始化NRF24L01
static esp_err_t init_nrf24l01(void)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "初始化NRF24L01...");
    
    // 获取默认配置
    ret = nrf24l01_get_default_config(&nrf24l01_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "获取默认配置失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 打印配置
    nrf24l01_print_config(&nrf24l01_config);
    
    // 初始化NRF24L01
    ret = nrf24l01_init(&nrf24l01_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NRF24L01初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "NRF24L01初始化成功");
    return ESP_OK;
}

// 初始化应用配置
static esp_err_t init_app_config(void)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "初始化应用配置...");
    
    // 获取默认应用配置
    ret = nrf24l01_get_default_app_config(&app_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "获取默认应用配置失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 根据编译时配置设置模式
    #ifdef CONFIG_NRF24L01_MODE_SENDER
        app_config.mode = NRF24L01_MODE_SENDER;
        ESP_LOGI(TAG, "设置为发送模式");
    #elif defined(CONFIG_NRF24L01_MODE_RECEIVER)
        app_config.mode = NRF24L01_MODE_RECEIVER;
        ESP_LOGI(TAG, "设置为接收模式");
    #else
        app_config.mode = NRF24L01_MODE_BOTH;
        ESP_LOGI(TAG, "设置为双向模式");
    #endif
    
    // 打印应用配置
    nrf24l01_print_app_config(&app_config);
    
    return ESP_OK;
}

// 启动应用任务
static esp_err_t start_app_tasks(void)
{
    esp_err_t ret = ESP_OK;
    
    ESP_LOGI(TAG, "启动应用任务...");
    
    // 创建数据队列
    data_queue = xQueueCreate(10, sizeof(data_packet_t));
    if (data_queue == NULL) {
        ESP_LOGE(TAG, "创建数据队列失败");
        return ESP_ERR_NO_MEM;
    }
    
    app_running = true;
    
    // 根据模式启动任务
    switch (app_config.mode) {
        case NRF24L01_MODE_SENDER:
            // 启动发送任务
            ret = xTaskCreate(sender_task, "sender_task", 4096, NULL, 5, &sender_task_handle);
            if (ret != pdPASS) {
                ESP_LOGE(TAG, "创建发送任务失败");
                return ESP_ERR_NO_MEM;
            }
            break;
            
        case NRF24L01_MODE_RECEIVER:
            // 启动接收任务和数据处理任务
            ret = xTaskCreate(receiver_task, "receiver_task", 4096, NULL, 5, &receiver_task_handle);
            if (ret != pdPASS) {
                ESP_LOGE(TAG, "创建接收任务失败");
                return ESP_ERR_NO_MEM;
            }
            
            ret = xTaskCreate(data_processor_task, "data_processor_task", 4096, NULL, 4, NULL);
            if (ret != pdPASS) {
                ESP_LOGE(TAG, "创建数据处理任务失败");
                return ESP_ERR_NO_MEM;
            }
            break;
            
        case NRF24L01_MODE_BOTH:
            // 启动所有任务
            ret = xTaskCreate(sender_task, "sender_task", 4096, NULL, 5, &sender_task_handle);
            if (ret != pdPASS) {
                ESP_LOGE(TAG, "创建发送任务失败");
                return ESP_ERR_NO_MEM;
            }
            
            ret = xTaskCreate(receiver_task, "receiver_task", 4096, NULL, 5, &receiver_task_handle);
            if (ret != pdPASS) {
                ESP_LOGE(TAG, "创建接收任务失败");
                return ESP_ERR_NO_MEM;
            }
            
            ret = xTaskCreate(data_processor_task, "data_processor_task", 4096, NULL, 4, NULL);
            if (ret != pdPASS) {
                ESP_LOGE(TAG, "创建数据处理任务失败");
                return ESP_ERR_NO_MEM;
            }
            break;
    }
    
    ESP_LOGI(TAG, "应用任务启动成功");
    return ESP_OK;
}

// 停止应用任务 (用于调试和清理)
// static void stop_app_tasks(void)
// {
//     ESP_LOGI(TAG, "停止应用任务...");
//     
//     app_running = false;
//     
//     // 等待任务结束
//     if (sender_task_handle != NULL) {
//         vTaskDelay(pdMS_TO_TICKS(100));
//         sender_task_handle = NULL;
//     }
//     
//     if (receiver_task_handle != NULL) {
//         vTaskDelay(pdMS_TO_TICKS(100));
//         receiver_task_handle = NULL;
//     }
//     
//     // 删除队列
//     if (data_queue != NULL) {
//         vQueueDelete(data_queue);
//         data_queue = NULL;
//     }
//     
//     ESP_LOGI(TAG, "应用任务已停止");
// }

// 清理函数 (可在需要时调用)
// static void cleanup_on_exit(void)
// {
//     stop_app_tasks();
//     nrf24l01_deinit();
// }

// 主函数
void app_main(void)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "=== NRF24L01 控制器启动 ===");
    ESP_LOGI(TAG, "编译时间: %s %s", __DATE__, __TIME__);
    
    // 初始化NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 初始化应用配置
    ret = init_app_config();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "初始化应用配置失败");
        return;
    }
    
    // 初始化NRF24L01
    ret = init_nrf24l01();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "初始化NRF24L01失败");
        return;
    }
    
    // 启动应用任务
    ret = start_app_tasks();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "启动应用任务失败");
        nrf24l01_deinit();
        return;
    }
    
    ESP_LOGI(TAG, "=== NRF24L01 控制器启动完成 ===");
    ESP_LOGI(TAG, "硬件连接说明:");
    ESP_LOGI(TAG, "  CE  -> GPIO%d", NRF24L01_CE_PIN);
    ESP_LOGI(TAG, "  CSN -> GPIO%d", NRF24L01_CSN_PIN);
    ESP_LOGI(TAG, "  MISO-> GPIO%d", NRF24L01_MISO_PIN);
    ESP_LOGI(TAG, "  MOSI-> GPIO%d", NRF24L01_MOSI_PIN);
    ESP_LOGI(TAG, "  SCK -> GPIO%d", NRF24L01_SCK_PIN);
    ESP_LOGI(TAG, "  VCC -> 3.3V");
    ESP_LOGI(TAG, "  GND -> GND");
    
    // 主循环
    while (1) {
        // 检查系统状态
        uint8_t status = nrf24l01_read_status();
        ESP_LOGD(TAG, "NRF24L01状态: 0x%02X", status);
        
        // 每10秒打印一次状态
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
