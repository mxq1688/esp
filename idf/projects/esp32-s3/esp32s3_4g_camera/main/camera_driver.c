#include "include/camera_driver.h"
#include "esp_log.h"
#include "esp_camera.h"
#include <string.h>

static const char *TAG = "CAMERA";

// 全局变量
static camera_state_t camera_state = CAMERA_STATE_UNINITIALIZED;
static camera_config_ex_t current_config;

// 初始化摄像头
esp_err_t camera_driver_init(void)
{
    if (camera_state != CAMERA_STATE_UNINITIALIZED) {
        ESP_LOGW(TAG, "Camera already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing camera...");

    // 摄像头配置
    camera_config_t config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = CAM_XCLK_FREQ,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG,  // 使用JPEG格式
        .frame_size = FRAMESIZE_SVGA,    // 800x600
        .jpeg_quality = 12,              // JPEG质量 (0-63, 越小质量越高)
        .fb_count = CAM_FB_COUNT,        // 帧缓冲数量
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    };

    // 初始化摄像头
    esp_err_t ret = esp_camera_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: %s", esp_err_to_name(ret));
        camera_state = CAMERA_STATE_ERROR;
        return ret;
    }

    // 保存当前配置
    current_config.frame_size = FRAMESIZE_SVGA;
    current_config.pixel_format = PIXFORMAT_JPEG;
    current_config.jpeg_quality = 12;
    current_config.fb_count = CAM_FB_COUNT;

    // 获取sensor配置
    sensor_t *s = esp_camera_sensor_get();
    if (s != NULL) {
        // 设置图像质量
        s->set_quality(s, 12);
        // 垂直翻转
        s->set_vflip(s, 1);
        // 水平镜像
        s->set_hmirror(s, 0);
        // 设置亮度 (默认0, 范围-2到2)
        s->set_brightness(s, 0);
        // 设置对比度 (默认0, 范围-2到2)
        s->set_contrast(s, 0);
        // 设置饱和度 (默认0, 范围-2到2)
        s->set_saturation(s, 0);
    }

    camera_state = CAMERA_STATE_READY;
    ESP_LOGI(TAG, "✅ Camera initialized successfully");
    ESP_LOGI(TAG, "Frame size: %dx%d", 
             config.frame_size == FRAMESIZE_SVGA ? 800 : 0,
             config.frame_size == FRAMESIZE_SVGA ? 600 : 0);

    return ESP_OK;
}

// 反初始化摄像头
esp_err_t camera_driver_deinit(void)
{
    if (camera_state == CAMERA_STATE_UNINITIALIZED) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing camera...");
    
    esp_err_t ret = esp_camera_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera deinit failed: %s", esp_err_to_name(ret));
        return ret;
    }

    camera_state = CAMERA_STATE_UNINITIALIZED;
    ESP_LOGI(TAG, "Camera deinitialized");
    
    return ESP_OK;
}

// 采集一帧图像
camera_fb_t* camera_driver_capture(void)
{
    if (camera_state != CAMERA_STATE_READY && camera_state != CAMERA_STATE_STREAMING) {
        ESP_LOGE(TAG, "Camera not ready (state: %d)", camera_state);
        return NULL;
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (fb == NULL) {
        ESP_LOGE(TAG, "Failed to capture image");
        return NULL;
    }

    ESP_LOGD(TAG, "Image captured: %zu bytes", fb->len);
    return fb;
}

// 释放图像帧缓冲
void camera_driver_release_frame(camera_fb_t *fb)
{
    if (fb != NULL) {
        esp_camera_fb_return(fb);
    }
}

// 获取摄像头状态
camera_state_t camera_driver_get_state(void)
{
    return camera_state;
}

// 检查摄像头是否就绪
bool camera_driver_is_ready(void)
{
    return (camera_state == CAMERA_STATE_READY || camera_state == CAMERA_STATE_STREAMING);
}

// 设置摄像头配置
esp_err_t camera_driver_set_config(const camera_config_ex_t *config)
{
    if (config == NULL || !camera_driver_is_ready()) {
        return ESP_ERR_INVALID_ARG;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        return ESP_FAIL;
    }

    // 设置帧尺寸
    if (config->frame_size != current_config.frame_size) {
        if (s->set_framesize(s, config->frame_size) != 0) {
            ESP_LOGE(TAG, "Failed to set frame size");
            return ESP_FAIL;
        }
        current_config.frame_size = config->frame_size;
    }

    // 设置JPEG质量
    if (config->jpeg_quality != current_config.jpeg_quality) {
        if (s->set_quality(s, config->jpeg_quality) != 0) {
            ESP_LOGE(TAG, "Failed to set JPEG quality");
            return ESP_FAIL;
        }
        current_config.jpeg_quality = config->jpeg_quality;
    }

    ESP_LOGI(TAG, "Camera config updated");
    return ESP_OK;
}

// 获取当前摄像头配置
esp_err_t camera_driver_get_config(camera_config_ex_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(config, &current_config, sizeof(camera_config_ex_t));
    return ESP_OK;
}

// 设置图像质量
esp_err_t camera_driver_set_quality(uint8_t quality)
{
    if (!camera_driver_is_ready()) {
        return ESP_ERR_INVALID_STATE;
    }

    // JPEG质量范围 0-63, 越小质量越高
    if (quality > 63) {
        quality = 63;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        return ESP_FAIL;
    }

    if (s->set_quality(s, quality) != 0) {
        ESP_LOGE(TAG, "Failed to set quality");
        return ESP_FAIL;
    }

    current_config.jpeg_quality = quality;
    ESP_LOGI(TAG, "JPEG quality set to %d", quality);
    
    return ESP_OK;
}

// 设置图像尺寸
esp_err_t camera_driver_set_framesize(framesize_t size)
{
    if (!camera_driver_is_ready()) {
        return ESP_ERR_INVALID_STATE;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        return ESP_FAIL;
    }

    if (s->set_framesize(s, size) != 0) {
        ESP_LOGE(TAG, "Failed to set frame size");
        return ESP_FAIL;
    }

    current_config.frame_size = size;
    ESP_LOGI(TAG, "Frame size set to %d", size);
    
    return ESP_OK;
}

