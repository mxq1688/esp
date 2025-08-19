/*
 * ESP32-C3 摇杆和LED控制器实现
 * 
 * 该文件实现了摇杆ADC采集和LED控制功能
 */

#include "include/joystick_led_controller.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <math.h>

static const char *TAG = "JOYSTICK_LED";

/* ADC校准句柄 */
static adc_cali_handle_t adc_cali_handle = NULL;

/* HSV转RGB函数 */
static void hsv_to_rgb(int hue, int saturation, int value, uint8_t *red, uint8_t *green, uint8_t *blue)
{
    int region, remainder, p, q, t;

    if (saturation == 0) {
        *red = *green = *blue = value;
        return;
    }

    region = hue / 43;
    remainder = (hue - (region * 43)) * 6;

    p = (value * (255 - saturation)) >> 8;
    q = (value * (255 - ((saturation * remainder) >> 8))) >> 8;
    t = (value * (255 - ((saturation * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:
            *red = value; *green = t; *blue = p;
            break;
        case 1:
            *red = q; *green = value; *blue = p;
            break;
        case 2:
            *red = p; *green = value; *blue = t;
            break;
        case 3:
            *red = p; *green = q; *blue = value;
            break;
        case 4:
            *red = t; *green = p; *blue = value;
            break;
        default:
            *red = value; *green = p; *blue = q;
            break;
    }
}

/* 初始化ADC校准 */
static esp_err_t init_adc_calibration(void)
{
    esp_err_t ret = ESP_OK;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "Calibration scheme version is Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = ADC_UNIT_1,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "Calibration scheme version is Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = ADC_UNIT_1,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    if (!calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
        adc_cali_handle = NULL;
    }

    return ret;
}

esp_err_t joystick_led_init(joystick_led_handle_t *handle, const joystick_led_config_t *config)
{
    if (!handle || !config) {
        ESP_LOGE(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(&handle->config, config, sizeof(joystick_led_config_t));
    handle->initialized = false;
    handle->led_state = false;
    handle->led_brightness = 50;
    handle->led_hue = 0;

    ESP_LOGI(TAG, "Initializing joystick and LED controller");

    // 初始化ADC
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    
    esp_err_t ret = adc_oneshot_new_unit(&init_config, &handle->adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ADC unit: %s", esp_err_to_name(ret));
        return ret;
    }

    // 配置ADC通道
    adc_oneshot_chan_cfg_t adc_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ret = adc_oneshot_config_channel(handle->adc_handle, config->x_axis_channel, &adc_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure X axis ADC channel: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = adc_oneshot_config_channel(handle->adc_handle, config->y_axis_channel, &adc_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure Y axis ADC channel: %s", esp_err_to_name(ret));
        return ret;
    }

    // 初始化ADC校准
    init_adc_calibration();

    // 配置按钮GPIO
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << config->button_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    ret = gpio_config(&button_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure button GPIO: %s", esp_err_to_name(ret));
        return ret;
    }

    // 初始化LED灯带
    ESP_LOGI(TAG, "Configuring WS2812 RGB LED on GPIO%d", config->led_pin);
    
    led_strip_config_t strip_config = {
        .strip_gpio_num = config->led_pin,
        .max_leds = config->led_count,
    };
    
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    
    ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &handle->led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED strip: %s", esp_err_to_name(ret));
        return ret;
    }

    // 清除LED
    led_strip_clear(handle->led_strip);

    handle->initialized = true;
    ESP_LOGI(TAG, "Joystick and LED controller initialized successfully");

    // 校准摇杆中心位置
    return joystick_calibrate_center(handle);
}

esp_err_t joystick_read(joystick_led_handle_t *handle, joystick_data_t *data)
{
    if (!handle || !handle->initialized || !data) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret;

    // 读取X轴ADC值
    ret = adc_oneshot_read(handle->adc_handle, handle->config.x_axis_channel, (int*)&data->x_raw);
    if (ret != ESP_OK) {
        return ret;
    }

    // 读取Y轴ADC值
    ret = adc_oneshot_read(handle->adc_handle, handle->config.y_axis_channel, (int*)&data->y_raw);
    if (ret != ESP_OK) {
        return ret;
    }

    // 转换为相对值 (-100 到 +100)
    int16_t x_offset = (int16_t)data->x_raw - (int16_t)handle->config.x_center;
    int16_t y_offset = (int16_t)data->y_raw - (int16_t)handle->config.y_center;

    // 应用死区
    if (abs(x_offset) < handle->config.deadzone) {
        x_offset = 0;
    }
    if (abs(y_offset) < handle->config.deadzone) {
        y_offset = 0;
    }

    // 将偏移量映射到-100到+100的范围
    // 假设ADC满量程为4096，中心值约为2048
    data->x_value = (x_offset * 100) / 2048;
    data->y_value = (y_offset * 100) / 2048;

    // 限制范围
    if (data->x_value > 100) data->x_value = 100;
    if (data->x_value < -100) data->x_value = -100;
    if (data->y_value > 100) data->y_value = 100;
    if (data->y_value < -100) data->y_value = -100;

    // 读取按钮状态 (按下时为低电平)
    data->button_pressed = !gpio_get_level(handle->config.button_pin);

    return ESP_OK;
}

esp_err_t led_set_state(joystick_led_handle_t *handle, bool on)
{
    if (!handle || !handle->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    handle->led_state = on;
    
    if (on) {
        // 使用当前色调和亮度
        uint8_t red, green, blue;
        hsv_to_rgb(handle->led_hue, 255, handle->led_brightness, &red, &green, &blue);
        return led_set_color(handle, red, green, blue);
    } else {
        return led_set_color(handle, 0, 0, 0);
    }
}

esp_err_t led_set_color(joystick_led_handle_t *handle, uint8_t red, uint8_t green, uint8_t blue)
{
    if (!handle || !handle->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = led_strip_set_pixel(handle->led_strip, 0, red, green, blue);
    if (ret != ESP_OK) {
        return ret;
    }

    return led_strip_refresh(handle->led_strip);
}

esp_err_t led_set_hsv(joystick_led_handle_t *handle, uint16_t hue, uint8_t saturation, uint8_t value)
{
    if (!handle || !handle->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    handle->led_hue = hue % 360;
    handle->led_brightness = value;

    uint8_t red, green, blue;
    hsv_to_rgb(hue, saturation, value, &red, &green, &blue);
    
    return led_set_color(handle, red, green, blue);
}

esp_err_t led_toggle(joystick_led_handle_t *handle)
{
    if (!handle || !handle->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    return led_set_state(handle, !handle->led_state);
}

esp_err_t joystick_calibrate_center(joystick_led_handle_t *handle)
{
    if (!handle || !handle->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Calibrating joystick center position...");

    uint32_t x_sum = 0, y_sum = 0;
    const int samples = 100;

    for (int i = 0; i < samples; i++) {
        int x_raw, y_raw;
        esp_err_t ret = adc_oneshot_read(handle->adc_handle, handle->config.x_axis_channel, &x_raw);
        if (ret != ESP_OK) {
            return ret;
        }
        
        ret = adc_oneshot_read(handle->adc_handle, handle->config.y_axis_channel, &y_raw);
        if (ret != ESP_OK) {
            return ret;
        }

        x_sum += x_raw;
        y_sum += y_raw;
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    handle->config.x_center = x_sum / samples;
    handle->config.y_center = y_sum / samples;

    ESP_LOGI(TAG, "Joystick calibrated - Center: X=%d, Y=%d", 
             handle->config.x_center, handle->config.y_center);

    // 检查校准值是否合理
    if (handle->config.x_center > 4000 || handle->config.y_center > 4000) {
        ESP_LOGW(TAG, "Warning: Calibration values seem too high. Check joystick connection!");
        ESP_LOGW(TAG, "Expected center values should be around 2000-3000");
        ESP_LOGW(TAG, "If joystick is not connected, using default center values");
        
        // 使用默认中心值
        handle->config.x_center = 2048;
        handle->config.y_center = 2048;
    }

    return ESP_OK;
}

esp_err_t joystick_led_deinit(joystick_led_handle_t *handle)
{
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }

    if (handle->initialized) {
        // 关闭LED
        led_strip_clear(handle->led_strip);
        
        // 删除ADC单元
        if (handle->adc_handle) {
            adc_oneshot_del_unit(handle->adc_handle);
        }

        // 删除校准句柄
        if (adc_cali_handle) {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
            adc_cali_delete_scheme_curve_fitting(adc_cali_handle);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
            adc_cali_delete_scheme_line_fitting(adc_cali_handle);
#endif
            adc_cali_handle = NULL;
        }

        handle->initialized = false;
    }

    return ESP_OK;
}
