/*
 * ESP32-S3 呼吸灯示例
 * 使用 LEDC 模块实现 LED 亮度渐变
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

// LED 配置
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (38)     // 定义 LED GPIO
#define LEDC_CHANNEL           LEDC_CHANNEL_0
#define LEDC_DUTY_RES          LEDC_TIMER_10_BIT  // 降低分辨率到10位
#define LEDC_FREQUENCY         (1000)    // 降低频率到1KHz
#define LEDC_FADE_TIME         (1000)    // 渐变时间改为1秒

static void breathing_led_init(void)
{
    // LED 定时器配置
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz         = LEDC_FREQUENCY,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // LED 通道配置
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // 配置渐变功能
    ESP_ERROR_CHECK(ledc_fade_func_install(0));
}

void app_main(void)
{
    // 初始化 LED 控制
    breathing_led_init();
    
    printf("呼吸灯示例启动\n");
    printf("LED 连接到 GPIO %d\n", LEDC_OUTPUT_IO);

    while (1) {
        printf("LED 渐亮...\n");
        // 从暗到亮
        ESP_ERROR_CHECK(ledc_set_fade_with_time(LEDC_MODE,
                                              LEDC_CHANNEL,
                                              1023,  // 最大亮度 (2^10-1)
                                              LEDC_FADE_TIME));
        ESP_ERROR_CHECK(ledc_fade_start(LEDC_MODE,
                                     LEDC_CHANNEL,
                                     LEDC_FADE_NO_WAIT));
        vTaskDelay(pdMS_TO_TICKS(LEDC_FADE_TIME + 200));  // 增加一点延时

        printf("LED 渐暗...\n");
        // 从亮到暗
        ESP_ERROR_CHECK(ledc_set_fade_with_time(LEDC_MODE,
                                              LEDC_CHANNEL,
                                              0,
                                              LEDC_FADE_TIME));
        ESP_ERROR_CHECK(ledc_fade_start(LEDC_MODE,
                                     LEDC_CHANNEL,
                                     LEDC_FADE_NO_WAIT));
        vTaskDelay(pdMS_TO_TICKS(LEDC_FADE_TIME + 200));  // 增加一点延时
    }
}
