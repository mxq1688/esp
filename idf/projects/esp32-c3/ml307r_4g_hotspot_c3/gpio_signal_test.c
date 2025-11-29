/*
 * ESP32-C3 GPIO信号测试程序
 * 在GPIO4和GPIO5上产生可测量的信号
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define GPIO_OUTPUT_PIN_4    4  // 对应ML307R的RX
#define GPIO_OUTPUT_PIN_5    5  // 对应ML307R的TX

static const char *TAG = "GPIO_TEST";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C3 GPIO信号测试开始");
    
    // 配置GPIO4为输出
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << GPIO_OUTPUT_PIN_4) | (1ULL << GPIO_OUTPUT_PIN_5),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "GPIO4和GPIO5配置为输出模式");
    ESP_LOGI(TAG, "请用万用表测量GPIO4和GPIO5的电压变化");
    ESP_LOGI(TAG, "高电平应为3.3V，低电平应为0V");
    
    int count = 0;
    while (1) {
        // GPIO4输出高电平，GPIO5输出低电平
        gpio_set_level(GPIO_OUTPUT_PIN_4, 1);
        gpio_set_level(GPIO_OUTPUT_PIN_5, 0);
        ESP_LOGI(TAG, "步骤%d: GPIO4=高(3.3V), GPIO5=低(0V)", ++count);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
        // GPIO4输出低电平，GPIO5输出高电平  
        gpio_set_level(GPIO_OUTPUT_PIN_4, 0);
        gpio_set_level(GPIO_OUTPUT_PIN_5, 1);
        ESP_LOGI(TAG, "步骤%d: GPIO4=低(0V), GPIO5=高(3.3V)", ++count);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
        if (count >= 10) {
            ESP_LOGI(TAG, "测试完成，重新开始循环");
            count = 0;
        }
    }
}
