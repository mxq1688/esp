#ifndef CAMERA_DRIVER_H
#define CAMERA_DRIVER_H

#include "esp_err.h"
#include "esp_camera.h"
#include <stdint.h>
#include <stdbool.h>

// 摄像头引脚定义 (ESP32-S3-EYE标准引脚)
#define CAM_PIN_PWDN    -1
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5

#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11
#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

// 摄像头配置
#define CAM_XCLK_FREQ   20000000  // 20MHz
#define CAM_FB_COUNT    2         // 帧缓冲数量

// 摄像头状态
typedef enum {
    CAMERA_STATE_UNINITIALIZED = 0,
    CAMERA_STATE_READY,
    CAMERA_STATE_STREAMING,
    CAMERA_STATE_ERROR
} camera_state_t;

// 摄像头配置结构
typedef struct {
    framesize_t frame_size;      // 图像尺寸
    pixformat_t pixel_format;    // 像素格式
    uint8_t jpeg_quality;        // JPEG质量 (0-63, 越小质量越高)
    uint8_t fb_count;            // 帧缓冲数量
} camera_config_ex_t;

/**
 * @brief 初始化摄像头
 * 
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t camera_driver_init(void);

/**
 * @brief 反初始化摄像头
 * 
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t camera_driver_deinit(void);

/**
 * @brief 采集一帧图像
 * 
 * @return camera_fb_t* 图像帧缓冲, NULL表示失败
 */
camera_fb_t* camera_driver_capture(void);

/**
 * @brief 释放图像帧缓冲
 * 
 * @param fb 要释放的帧缓冲
 */
void camera_driver_release_frame(camera_fb_t *fb);

/**
 * @brief 获取摄像头状态
 * 
 * @return camera_state_t 当前状态
 */
camera_state_t camera_driver_get_state(void);

/**
 * @brief 检查摄像头是否就绪
 * 
 * @return true 就绪, false 未就绪
 */
bool camera_driver_is_ready(void);

/**
 * @brief 设置摄像头配置
 * 
 * @param config 配置参数
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t camera_driver_set_config(const camera_config_ex_t *config);

/**
 * @brief 获取当前摄像头配置
 * 
 * @param config 保存配置的指针
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t camera_driver_get_config(camera_config_ex_t *config);

/**
 * @brief 设置图像质量
 * 
 * @param quality JPEG质量 (0-63)
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t camera_driver_set_quality(uint8_t quality);

/**
 * @brief 设置图像尺寸
 * 
 * @param size 图像尺寸
 * @return ESP_OK 成功, 其他值表示失败
 */
esp_err_t camera_driver_set_framesize(framesize_t size);

#endif // CAMERA_DRIVER_H

