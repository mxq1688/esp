#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "BREATHING_LED";

// WS2812 LED配置
#define BUILTIN_RGB_LED_GPIO 48
#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      BUILTIN_RGB_LED_GPIO

// WS2812协议时序参数 (单位: RMT ticks)
#define WS2812_T0H_TICKS    (uint32_t)(0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000)   // 0.3us
#define WS2812_T0L_TICKS    (uint32_t)(0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000)   // 0.9us
#define WS2812_T1H_TICKS    (uint32_t)(0.6 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000)   // 0.6us
#define WS2812_T1L_TICKS    (uint32_t)(0.6 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000)   // 0.6us
#define WS2812_RESET_TICKS  (uint32_t)(50 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000)    // 50us

typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    int state;
    rmt_symbol_word_t reset_code;
} rmt_ws2812_encoder_t;

static rmt_channel_handle_t led_chan = NULL;
static rmt_encoder_handle_t led_encoder = NULL;

// WS2812编码器实现
static size_t rmt_encode_ws2812(rmt_encoder_t *encoder, rmt_channel_handle_t channel,
                               const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
    rmt_ws2812_encoder_t *ws2812_encoder = __containerof(encoder, rmt_ws2812_encoder_t, base);
    rmt_encoder_handle_t bytes_encoder = ws2812_encoder->bytes_encoder;
    rmt_encoder_handle_t copy_encoder = ws2812_encoder->copy_encoder;
    rmt_encode_state_t session_state = 0;
    rmt_encode_state_t state = 0;
    size_t encoded_symbols = 0;
    
    switch (ws2812_encoder->state) {
    case 0: // send RGB data
        encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            ws2812_encoder->state = 1; // switch to next state when current encoding session finished
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out; // yield if there's no free space for encoding artifacts
        }
    // fall-through
    case 1: // send reset code
        encoded_symbols += copy_encoder->encode(copy_encoder, channel, &ws2812_encoder->reset_code,
                                              sizeof(ws2812_encoder->reset_code), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            ws2812_encoder->state = 0; // back to the initial encoding session
            state |= RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out; // yield if there's no free space for encoding artifacts
        }
    }
out:
    *ret_state = state;
    return encoded_symbols;
}

static esp_err_t rmt_del_ws2812_encoder(rmt_encoder_t *encoder)
{
    rmt_ws2812_encoder_t *ws2812_encoder = __containerof(encoder, rmt_ws2812_encoder_t, base);
    rmt_del_encoder(ws2812_encoder->bytes_encoder);
    rmt_del_encoder(ws2812_encoder->copy_encoder);
    free(ws2812_encoder);
    return ESP_OK;
}

static esp_err_t rmt_ws2812_encoder_reset(rmt_encoder_t *encoder)
{
    rmt_ws2812_encoder_t *ws2812_encoder = __containerof(encoder, rmt_ws2812_encoder_t, base);
    rmt_encoder_reset(ws2812_encoder->bytes_encoder);
    rmt_encoder_reset(ws2812_encoder->copy_encoder);
    ws2812_encoder->state = 0;
    return ESP_OK;
}

esp_err_t rmt_new_ws2812_encoder(const void *config, rmt_encoder_handle_t *ret_encoder)
{
    esp_err_t ret = ESP_OK;
    rmt_ws2812_encoder_t *ws2812_encoder = NULL;
    
    ws2812_encoder = calloc(1, sizeof(rmt_ws2812_encoder_t));
    if (!ws2812_encoder) {
        return ESP_ERR_NO_MEM;
    }
    
    ws2812_encoder->base.encode = rmt_encode_ws2812;
    ws2812_encoder->base.del = rmt_del_ws2812_encoder;
    ws2812_encoder->base.reset = rmt_ws2812_encoder_reset;
    
    // different led strip might have its own timing requirements, following parameter is for WS2812
    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = WS2812_T0H_TICKS,
            .level1 = 0,
            .duration1 = WS2812_T0L_TICKS,
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = WS2812_T1H_TICKS,
            .level1 = 0,
            .duration1 = WS2812_T1L_TICKS,
        },
        .flags.msb_first = 1 // WS2812 transfer bit order: G7...G0R7...R0B7...B0
    };
    
    ret = rmt_new_bytes_encoder(&bytes_encoder_config, &ws2812_encoder->bytes_encoder);
    if (ret != ESP_OK) {
        goto err;
    }
    
    rmt_copy_encoder_config_t copy_encoder_config = {};
    ret = rmt_new_copy_encoder(&copy_encoder_config, &ws2812_encoder->copy_encoder);
    if (ret != ESP_OK) {
        goto err;
    }
    
    ws2812_encoder->reset_code = (rmt_symbol_word_t) {
        .level0 = 0, .duration0 = WS2812_RESET_TICKS,
        .level1 = 0, .duration1 = WS2812_RESET_TICKS,
    };
    
    *ret_encoder = &ws2812_encoder->base;
    return ESP_OK;
    
err:
    if (ws2812_encoder) {
        if (ws2812_encoder->bytes_encoder) {
            rmt_del_encoder(ws2812_encoder->bytes_encoder);
        }
        if (ws2812_encoder->copy_encoder) {
            rmt_del_encoder(ws2812_encoder->copy_encoder);
        }
        free(ws2812_encoder);
    }
    return ret;
}

// 初始化WS2812
static esp_err_t ws2812_init(void)
{
    ESP_LOGI(TAG, "Create RMT TX channel");
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = RMT_LED_STRIP_GPIO_NUM,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(TAG, "Install led strip encoder");
    ESP_ERROR_CHECK(rmt_new_ws2812_encoder(NULL, &led_encoder));

    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    return ESP_OK;
}

// 设置RGB颜色
static void set_rgb_color(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t led_strip_pixels[3];
    // WS2812的颜色顺序是GRB
    led_strip_pixels[0] = g;
    led_strip_pixels[1] = r;
    led_strip_pixels[2] = b;

    rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no transfer loop
    };
    
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
}

// 呼吸灯任务
static void breathing_task(void *parameter)
{
    float brightness = 0.2;  // 起始亮度
    bool breath_up = true;
    const float breath_speed = 0.01;  // 呼吸速度
    
    ESP_LOGI(TAG, "🔴 红色呼吸灯效果开始");
    
    while (1) {
        // 计算呼吸效果
        if (breath_up) {
            brightness += breath_speed;
            if (brightness >= 1.0) {
                brightness = 1.0;
                breath_up = false;
            }
        } else {
            brightness -= breath_speed;
            if (brightness <= 0.2) {
                brightness = 0.2;
                breath_up = true;
            }
        }
        
        // 计算红色LED的亮度
        uint8_t red_value = (uint8_t)(255 * brightness);
        
        // 设置红色LED
        set_rgb_color(red_value, 0, 0);
        
        vTaskDelay(50 / portTICK_PERIOD_MS);  // 50ms更新一次，呼吸效果更平滑
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "💡 ESP32-S3 呼吸灯启动");
    
    // 初始化WS2812
    ESP_LOGI(TAG, "Initializing WS2812 RGB LED...");
    ESP_ERROR_CHECK(ws2812_init());
    ESP_LOGI(TAG, "WS2812 initialized successfully");
    
    // 先点亮LED 1秒确认工作
    ESP_LOGI(TAG, "Testing LED - Red color");
    set_rgb_color(255, 0, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    // 关闭LED
    set_rgb_color(0, 0, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    
    // 创建呼吸灯任务
    xTaskCreate(breathing_task, "breathing_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "✨ 呼吸灯就绪!");
} 