/*
 * 摇杆测试程序 - 用于调试摇杆连接问题
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"

static const char *TAG = "JOYSTICK_TEST";

#define JOYSTICK_X_AXIS_CHANNEL ADC_CHANNEL_0  // GPIO0
#define JOYSTICK_Y_AXIS_CHANNEL ADC_CHANNEL_1  // GPIO1
#define JOYSTICK_BUTTON_PIN     GPIO_NUM_3     // GPIO3

void app_main(void)
{
    ESP_LOGI(TAG, "=== 摇杆测试程序 ===");
    
    // 初始化ADC
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    
    esp_err_t ret = adc_oneshot_new_unit(&init_config, &adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC初始化失败: %s", esp_err_to_name(ret));
        return;
    }
    
    // 配置ADC通道
    adc_oneshot_chan_cfg_t adc_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    
    ret = adc_oneshot_config_channel(adc_handle, JOYSTICK_X_AXIS_CHANNEL, &adc_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "X轴ADC配置失败: %s", esp_err_to_name(ret));
        return;
    }
    
    ret = adc_oneshot_config_channel(adc_handle, JOYSTICK_Y_AXIS_CHANNEL, &adc_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Y轴ADC配置失败: %s", esp_err_to_name(ret));
        return;
    }
    
    // 配置按钮GPIO
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << JOYSTICK_BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    ret = gpio_config(&button_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "按钮GPIO配置失败: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "摇杆测试程序初始化完成");
    ESP_LOGI(TAG, "请移动摇杆并按下按钮，观察数值变化");
    ESP_LOGI(TAG, "正常摇杆中心值应该在2000-3000之间");
    ESP_LOGI(TAG, "格式: X轴值 | Y轴值 | 按钮状态");
    
    while (1) {
        int x_raw, y_raw;
        bool button_pressed;
        
        // 读取X轴
        ret = adc_oneshot_read(adc_handle, JOYSTICK_X_AXIS_CHANNEL, &x_raw);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "X轴读取失败: %s", esp_err_to_name(ret));
            continue;
        }
        
        // 读取Y轴
        ret = adc_oneshot_read(adc_handle, JOYSTICK_Y_AXIS_CHANNEL, &y_raw);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Y轴读取失败: %s", esp_err_to_name(ret));
            continue;
        }
        
        // 读取按钮状态
        button_pressed = !gpio_get_level(JOYSTICK_BUTTON_PIN);
        
        // 输出结果
        ESP_LOGI(TAG, "X:%4d | Y:%4d | 按钮:%s", 
                x_raw, y_raw, button_pressed ? "按下" : "释放");
        
        vTaskDelay(pdMS_TO_TICKS(500)); // 每500ms读取一次
    }
}
