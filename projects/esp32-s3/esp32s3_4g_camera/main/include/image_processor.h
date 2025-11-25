#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include "esp_err.h"
#include "esp_camera.h"
#include <stdint.h>

// 图像处理配置
typedef struct {
    uint8_t jpeg_quality;      // JPEG质量 (0-100)
    uint32_t max_size;         // 最大图像尺寸
    bool resize_enable;        // 是否启用缩放
} image_processor_config_t;

/**
 * @brief 初始化图像处理器
 * 
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t image_processor_init(void);

/**
 * @brief 处理图像帧
 * 
 * @param fb 原始图像帧
 * @param out_buf 输出缓冲区
 * @param out_len 输出长度
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t image_processor_process(camera_fb_t *fb, uint8_t **out_buf, size_t *out_len);

/**
 * @brief 设置JPEG质量
 * 
 * @param quality 质量值 (0-100)
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t image_processor_set_quality(uint8_t quality);

/**
 * @brief 获取当前配置
 * 
 * @param config 保存配置的指针
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t image_processor_get_config(image_processor_config_t *config);

#endif // IMAGE_PROCESSOR_H

