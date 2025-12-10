/**
 * @file light_show.h
 * @brief Light show modes for NeoPixel LED strip
 */

#ifndef LIGHT_SHOW_H
#define LIGHT_SHOW_H

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

// 预设灯光秀模式
typedef enum {
    LIGHT_SHOW_OFF = 0,         // 关闭（显示时钟）
    LIGHT_SHOW_RAINBOW,         // 彩虹旋转
    LIGHT_SHOW_BREATHING,       // 呼吸灯
    LIGHT_SHOW_CHASE,           // 流水追逐
    LIGHT_SHOW_SPARKLE,         // 星光闪烁
    LIGHT_SHOW_FIRE,            // 火焰效果
    LIGHT_SHOW_OCEAN,           // 海洋波浪
    LIGHT_SHOW_METEOR,          // 流星雨
    LIGHT_SHOW_POLICE,          // 警灯
    LIGHT_SHOW_CANDY,           // 糖果色
    LIGHT_SHOW_AURORA,          // 极光
    LIGHT_SHOW_HEARTBEAT,       // 心跳
    LIGHT_SHOW_CUSTOM,          // 用户自定义
    LIGHT_SHOW_MAX
} light_show_mode_t;

// 自定义颜色结构
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} custom_color_t;

// 自定义模式参数
typedef struct {
    custom_color_t color1;      // 主色
    custom_color_t color2;      // 辅色
    custom_color_t color3;      // 第三色
    uint8_t speed;              // 速度 (1-10)
    uint8_t brightness;         // 亮度 (5-100)
    uint8_t effect;             // 效果类型 (0-9)
    uint8_t tail_length;        // 尾迹长度 (3-30)
    uint8_t direction;          // 方向 (0=顺时针, 1=逆时针, 2=双向扩散, 3=双向收缩)
} custom_params_t;

/**
 * @brief 初始化灯光秀模块
 */
esp_err_t light_show_init(void);

/**
 * @brief 设置灯光秀模式
 * @param mode 模式
 */
void light_show_set_mode(light_show_mode_t mode);

/**
 * @brief 获取当前模式
 */
light_show_mode_t light_show_get_mode(void);

/**
 * @brief 设置自定义参数
 */
void light_show_set_custom_params(const custom_params_t *params);

/**
 * @brief 获取自定义参数
 */
void light_show_get_custom_params(custom_params_t *params);

/**
 * @brief 灯光秀更新（在主循环中调用）
 * @return true 如果灯光秀正在运行，false 如果应该显示时钟
 */
bool light_show_update(void);

/**
 * @brief 检查灯光秀是否激活
 */
bool light_show_is_active(void);

/**
 * @brief 获取模式名称
 */
const char* light_show_get_mode_name(light_show_mode_t mode);

#endif // LIGHT_SHOW_H

