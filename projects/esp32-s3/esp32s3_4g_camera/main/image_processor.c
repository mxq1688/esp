#include "include/image_processor.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "IMAGE_PROC";

// 全局配置
static image_processor_config_t config = {
    .jpeg_quality = 80,
    .max_size = 1024 * 100,  // 100KB
    .resize_enable = false,
};

// 初始化图像处理器
esp_err_t image_processor_init(void)
{
    ESP_LOGI(TAG, "Image processor initialized");
    ESP_LOGI(TAG, "JPEG Quality: %d", config.jpeg_quality);
    ESP_LOGI(TAG, "Max Size: %lu bytes", config.max_size);
    
    return ESP_OK;
}

// 处理图像帧
esp_err_t image_processor_process(camera_fb_t *fb, uint8_t **out_buf, size_t *out_len)
{
    if (fb == NULL || out_buf == NULL || out_len == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // 对于JPEG格式，直接使用原始数据
    if (fb->format == PIXFORMAT_JPEG) {
        *out_buf = fb->buf;
        *out_len = fb->len;
        
        ESP_LOGD(TAG, "JPEG image processed: %zu bytes", *out_len);
        return ESP_OK;
    }

    // 对于其他格式，需要进行编码处理（这里简化处理）
    ESP_LOGW(TAG, "Non-JPEG format not supported yet");
    return ESP_FAIL;
}

// 设置JPEG质量
esp_err_t image_processor_set_quality(uint8_t quality)
{
    if (quality > 100) {
        quality = 100;
    }
    
    config.jpeg_quality = quality;
    ESP_LOGI(TAG, "JPEG quality set to %d", quality);
    
    return ESP_OK;
}

// 获取当前配置
esp_err_t image_processor_get_config(image_processor_config_t *out_config)
{
    if (out_config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(out_config, &config, sizeof(image_processor_config_t));
    return ESP_OK;
}

