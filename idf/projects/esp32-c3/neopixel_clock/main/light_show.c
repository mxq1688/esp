/**
 * @file light_show.c
 * @brief Light show modes implementation
 */

#include "light_show.h"
#include "neopixel_driver.h"
#include "esp_log.h"
#include "esp_random.h"
#include <string.h>
#include <math.h>

static const char *TAG = "light_show";

// 当前状态
static light_show_mode_t s_current_mode = LIGHT_SHOW_OFF;
static custom_params_t s_custom_params = {
    .color1 = {0, 255, 255},
    .color2 = {255, 0, 128},
    .color3 = {255, 255, 0},
    .speed = 5,
    .brightness = 80,
    .effect = 1,
    .tail_length = 10,
    .direction = 0
};

// 动画状态
static uint32_t s_frame = 0;

// HSV to RGB
static rgb_color_t hsv_to_rgb(int hue, int sat, int val)
{
    hue = hue % 360;
    int h = hue / 60;
    int f = (hue % 60) * 255 / 60;
    int p = val * (255 - sat) / 255;
    int q = val * (255 - (sat * f / 255)) / 255;
    int t = val * (255 - (sat * (255 - f) / 255)) / 255;
    
    switch (h) {
        case 0: return (rgb_color_t){val, t, p};
        case 1: return (rgb_color_t){q, val, p};
        case 2: return (rgb_color_t){p, val, t};
        case 3: return (rgb_color_t){p, q, val};
        case 4: return (rgb_color_t){t, p, val};
        default: return (rgb_color_t){val, p, q};
    }
}

// 彩虹旋转效果
static void effect_rainbow(void)
{
    for (int i = 0; i < 60; i++) {
        int hue = ((i * 6) + s_frame * 3) % 360;
        rgb_color_t color = hsv_to_rgb(hue, 255, 150);
        neopixel_set_pixel(i, color);
    }
}

// 呼吸灯效果 - 彩色呼吸，每次呼吸换颜色
static void effect_breathing(void)
{
    // 定义呼吸灯颜色
    static const rgb_color_t breath_colors[] = {
        {255, 0, 100},    // 玫红
        {0, 255, 200},    // 青色
        {255, 100, 0},    // 橙色
        {150, 0, 255},    // 紫色
        {0, 255, 100},    // 绿色
        {100, 150, 255},  // 淡蓝
    };
    static int color_index = 0;
    static int last_direction = 1;
    
    // 使用正弦波计算亮度
    float phase = (float)(s_frame % 120) / 120.0f * 3.14159f * 2;
    float sin_val = sinf(phase);
    int brightness = (int)((sin_val + 1.0f) * 0.5f * 200);
    
    // 检测呼吸周期切换（从暗到亮的瞬间换颜色）
    int current_direction = sin_val > 0 ? 1 : -1;
    if (current_direction == 1 && last_direction == -1) {
        color_index = (color_index + 1) % 6;
    }
    last_direction = current_direction;
    
    rgb_color_t base = breath_colors[color_index];
    rgb_color_t color = {
        base.r * brightness / 255,
        base.g * brightness / 255,
        base.b * brightness / 255
    };
    
    for (int i = 0; i < 60; i++) {
        neopixel_set_pixel(i, color);
    }
}

// 流水追逐效果
static void effect_chase(void)
{
    neopixel_clear();
    
    int head = s_frame % 60;
    for (int i = 0; i < 10; i++) {
        int pos = (head - i + 60) % 60;
        int brightness = 200 - i * 20;
        if (brightness < 0) brightness = 0;
        
        rgb_color_t color = {
            s_custom_params.color1.r * brightness / 255,
            s_custom_params.color1.g * brightness / 255,
            s_custom_params.color1.b * brightness / 255
        };
        neopixel_set_pixel(pos, color);
    }
}

// 星光闪烁效果
static void effect_sparkle(void)
{
    // 渐暗所有像素
    for (int i = 0; i < 60; i++) {
        rgb_color_t color;
        neopixel_get_pixel(i, &color);
        color.r = color.r > 10 ? color.r - 10 : 0;
        color.g = color.g > 10 ? color.g - 10 : 0;
        color.b = color.b > 10 ? color.b - 10 : 0;
        neopixel_set_pixel(i, color);
    }
    
    // 随机点亮几个像素
    if (s_frame % 3 == 0) {
        int pos = esp_random() % 60;
        rgb_color_t white = {200, 200, 200};
        neopixel_set_pixel(pos, white);
    }
}

// 火焰效果
static void effect_fire(void)
{
    for (int i = 0; i < 60; i++) {
        int flicker = esp_random() % 80;
        int r = 200 + (esp_random() % 55) - flicker;
        int g = 50 + (esp_random() % 50) - flicker / 2;
        int b = 0;
        
        if (r < 0) r = 0;
        if (g < 0) g = 0;
        if (r > 255) r = 255;
        if (g > 255) g = 255;
        
        rgb_color_t color = {r, g, b};
        neopixel_set_pixel(i, color);
    }
}

// 海洋波浪效果
static void effect_ocean(void)
{
    for (int i = 0; i < 60; i++) {
        float phase = (float)(i + s_frame) / 10.0f;
        int brightness = (int)((sinf(phase) + 1.0f) * 0.5f * 150) + 50;
        
        rgb_color_t color = {
            0,
            brightness / 3,
            brightness
        };
        neopixel_set_pixel(i, color);
    }
}

// 流星雨效果
static void effect_meteor(void)
{
    // 渐暗背景
    for (int i = 0; i < 60; i++) {
        rgb_color_t color;
        neopixel_get_pixel(i, &color);
        color.r = color.r > 20 ? color.r - 20 : 0;
        color.g = color.g > 20 ? color.g - 20 : 0;
        color.b = color.b > 20 ? color.b - 20 : 0;
        neopixel_set_pixel(i, color);
    }
    
    // 多条流星
    for (int m = 0; m < 3; m++) {
        int head = (s_frame + m * 20) % 60;
        for (int i = 0; i < 8; i++) {
            int pos = (head - i + 60) % 60;
            int brightness = 255 - i * 30;
            if (brightness > 0) {
                rgb_color_t color = {brightness, brightness, brightness};
                neopixel_set_pixel(pos, color);
            }
        }
    }
}

// 警灯效果
static void effect_police(void)
{
    int phase = (s_frame / 10) % 4;
    neopixel_clear();
    
    for (int i = 0; i < 60; i++) {
        rgb_color_t color = {0, 0, 0};
        if (phase == 0 || phase == 1) {
            // 左半边红色
            if (i < 30) {
                color = (rgb_color_t){phase == 0 ? 255 : 100, 0, 0};
            }
        }
        if (phase == 2 || phase == 3) {
            // 右半边蓝色
            if (i >= 30) {
                color = (rgb_color_t){0, 0, phase == 2 ? 255 : 100};
            }
        }
        neopixel_set_pixel(i, color);
    }
}

// 糖果色效果
static void effect_candy(void)
{
    // 粉色、薄荷绿、淡紫、柠檬黄循环
    static const rgb_color_t candy_colors[] = {
        {255, 150, 200},  // 粉色
        {150, 255, 200},  // 薄荷绿
        {200, 150, 255},  // 淡紫
        {255, 255, 150},  // 柠檬黄
        {150, 200, 255},  // 天蓝
    };
    
    for (int i = 0; i < 60; i++) {
        int color_idx = ((i + s_frame / 5) / 12) % 5;
        rgb_color_t color = candy_colors[color_idx];
        // 稍微降低亮度
        color.r = color.r * 3 / 4;
        color.g = color.g * 3 / 4;
        color.b = color.b * 3 / 4;
        neopixel_set_pixel(i, color);
    }
}

// 极光效果
static void effect_aurora(void)
{
    for (int i = 0; i < 60; i++) {
        // 多层波形叠加
        float wave1 = sinf((float)(i + s_frame) / 8.0f);
        float wave2 = sinf((float)(i * 2 + s_frame) / 12.0f);
        float wave3 = sinf((float)(i + s_frame * 2) / 15.0f);
        
        float combined = (wave1 + wave2 + wave3) / 3.0f;
        int brightness = (int)((combined + 1.0f) * 0.5f * 180);
        
        // 绿色为主，带蓝紫色调
        int g = brightness;
        int b = brightness * 2 / 3;
        int r = (int)(brightness * fabsf(wave2) / 3);
        
        rgb_color_t color = {r, g, b};
        neopixel_set_pixel(i, color);
    }
}

// 心跳效果
static void effect_heartbeat(void)
{
    int cycle = s_frame % 100;
    int brightness = 0;
    
    // 双峰心跳波形
    if (cycle < 10) {
        brightness = cycle * 25;  // 快速上升
    } else if (cycle < 20) {
        brightness = 250 - (cycle - 10) * 20;  // 快速下降
    } else if (cycle < 30) {
        brightness = 50 + (cycle - 20) * 15;  // 第二次上升
    } else if (cycle < 45) {
        brightness = 200 - (cycle - 30) * 13;  // 缓慢下降
    } else {
        brightness = 0;  // 休息
    }
    
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;
    
    rgb_color_t color = {brightness, 0, brightness / 4};
    for (int i = 0; i < 60; i++) {
        neopixel_set_pixel(i, color);
    }
}

// 辅助函数：根据方向计算位置
static int get_position(int base_pos, int direction) {
    switch (direction) {
        case 1: return (60 - base_pos) % 60;  // 逆时针
        default: return base_pos % 60;         // 顺时针
    }
}

// 辅助函数：三色插值
static rgb_color_t interpolate_3colors(float ratio, custom_color_t c1, custom_color_t c2, custom_color_t c3) {
    rgb_color_t result;
    if (ratio < 0.5f) {
        float r = ratio * 2.0f;
        result.r = (uint8_t)(c1.r * (1 - r) + c2.r * r);
        result.g = (uint8_t)(c1.g * (1 - r) + c2.g * r);
        result.b = (uint8_t)(c1.b * (1 - r) + c2.b * r);
    } else {
        float r = (ratio - 0.5f) * 2.0f;
        result.r = (uint8_t)(c2.r * (1 - r) + c3.r * r);
        result.g = (uint8_t)(c2.g * (1 - r) + c3.g * r);
        result.b = (uint8_t)(c2.b * (1 - r) + c3.b * r);
    }
    return result;
}

// 自定义效果
static void effect_custom(void)
{
    int brightness = s_custom_params.brightness * 255 / 100;
    int tail = s_custom_params.tail_length;
    int dir = s_custom_params.direction;
    
    switch (s_custom_params.effect) {
        case 0: // 纯色
            {
                rgb_color_t color = {
                    s_custom_params.color1.r * brightness / 255,
                    s_custom_params.color1.g * brightness / 255,
                    s_custom_params.color1.b * brightness / 255
                };
                for (int i = 0; i < 60; i++) {
                    neopixel_set_pixel(i, color);
                }
            }
            break;
            
        case 1: // 双色渐变
            {
                for (int i = 0; i < 60; i++) {
                    float ratio = (float)i / 60.0f;
                    rgb_color_t color = {
                        (uint8_t)((s_custom_params.color1.r * (1 - ratio) + s_custom_params.color2.r * ratio) * brightness / 255),
                        (uint8_t)((s_custom_params.color1.g * (1 - ratio) + s_custom_params.color2.g * ratio) * brightness / 255),
                        (uint8_t)((s_custom_params.color1.b * (1 - ratio) + s_custom_params.color2.b * ratio) * brightness / 255)
                    };
                    neopixel_set_pixel(i, color);
                }
            }
            break;
            
        case 2: // 三色渐变
            {
                for (int i = 0; i < 60; i++) {
                    float ratio = (float)i / 60.0f;
                    rgb_color_t base = interpolate_3colors(ratio, s_custom_params.color1, s_custom_params.color2, s_custom_params.color3);
                    rgb_color_t color = {
                        base.r * brightness / 255,
                        base.g * brightness / 255,
                        base.b * brightness / 255
                    };
                    neopixel_set_pixel(i, color);
                }
            }
            break;
            
        case 3: // 彩虹渐变（旋转）
            {
                for (int i = 0; i < 60; i++) {
                    int pos = get_position(i + s_frame, dir);
                    int hue = (pos * 6) % 360;
                    rgb_color_t color = hsv_to_rgb(hue, 255, brightness);
                    neopixel_set_pixel(i, color);
                }
            }
            break;
            
        case 4: // 呼吸闪烁
            {
                float phase = (float)(s_frame % 100) / 100.0f * 3.14159f * 2;
                int breath = (int)((sinf(phase) + 1.0f) * 0.5f * brightness);
                rgb_color_t color = {
                    s_custom_params.color1.r * breath / 255,
                    s_custom_params.color1.g * breath / 255,
                    s_custom_params.color1.b * breath / 255
                };
                for (int i = 0; i < 60; i++) {
                    neopixel_set_pixel(i, color);
                }
            }
            break;
            
        case 5: // 双色流水
            {
                neopixel_clear();
                int head = s_frame % 60;
                if (dir == 1) head = 59 - head;
                
                for (int i = 0; i < tail; i++) {
                    int pos = (head - i + 60) % 60;
                    float ratio = (float)i / (float)tail;
                    int fade = brightness * (tail - i) / tail;
                    
                    rgb_color_t color = {
                        (uint8_t)((s_custom_params.color1.r * (1 - ratio) + s_custom_params.color2.r * ratio) * fade / 255),
                        (uint8_t)((s_custom_params.color1.g * (1 - ratio) + s_custom_params.color2.g * ratio) * fade / 255),
                        (uint8_t)((s_custom_params.color1.b * (1 - ratio) + s_custom_params.color2.b * ratio) * fade / 255)
                    };
                    neopixel_set_pixel(pos, color);
                }
            }
            break;
            
        case 6: // 彗星拖尾
            {
                // 渐暗背景
                for (int i = 0; i < 60; i++) {
                    rgb_color_t color;
                    neopixel_get_pixel(i, &color);
                    color.r = color.r > 15 ? color.r - 15 : 0;
                    color.g = color.g > 15 ? color.g - 15 : 0;
                    color.b = color.b > 15 ? color.b - 15 : 0;
                    neopixel_set_pixel(i, color);
                }
                
                int head = s_frame % 60;
                if (dir == 1) head = 59 - head;
                
                // 彗星头部
                rgb_color_t head_color = {
                    s_custom_params.color1.r * brightness / 255,
                    s_custom_params.color1.g * brightness / 255,
                    s_custom_params.color1.b * brightness / 255
                };
                neopixel_set_pixel(head, head_color);
                
                // 尾迹
                for (int i = 1; i < tail / 2; i++) {
                    int pos = (head - i + 60) % 60;
                    int fade = brightness * (tail / 2 - i) / (tail / 2);
                    rgb_color_t color = {
                        s_custom_params.color2.r * fade / 255,
                        s_custom_params.color2.g * fade / 255,
                        s_custom_params.color2.b * fade / 255
                    };
                    neopixel_set_pixel(pos, color);
                }
            }
            break;
            
        case 7: // 波浪起伏
            {
                for (int i = 0; i < 60; i++) {
                    float phase = (float)(i + s_frame) / 10.0f;
                    if (dir == 1) phase = (float)(60 - i + s_frame) / 10.0f;
                    float wave = (sinf(phase) + 1.0f) * 0.5f;
                    int b = (int)(wave * brightness);
                    
                    rgb_color_t color = {
                        (uint8_t)(s_custom_params.color1.r * b / 255 * (1 - wave) + s_custom_params.color2.r * b / 255 * wave),
                        (uint8_t)(s_custom_params.color1.g * b / 255 * (1 - wave) + s_custom_params.color2.g * b / 255 * wave),
                        (uint8_t)(s_custom_params.color1.b * b / 255 * (1 - wave) + s_custom_params.color2.b * b / 255 * wave)
                    };
                    neopixel_set_pixel(i, color);
                }
            }
            break;
            
        case 8: // 随机闪烁
            {
                // 渐暗
                for (int i = 0; i < 60; i++) {
                    rgb_color_t color;
                    neopixel_get_pixel(i, &color);
                    color.r = color.r > 8 ? color.r - 8 : 0;
                    color.g = color.g > 8 ? color.g - 8 : 0;
                    color.b = color.b > 8 ? color.b - 8 : 0;
                    neopixel_set_pixel(i, color);
                }
                
                // 随机点亮
                if (s_frame % 2 == 0) {
                    int pos = esp_random() % 60;
                    int color_choice = esp_random() % 3;
                    custom_color_t *chosen = color_choice == 0 ? &s_custom_params.color1 : 
                                             color_choice == 1 ? &s_custom_params.color2 : &s_custom_params.color3;
                    rgb_color_t color = {
                        chosen->r * brightness / 255,
                        chosen->g * brightness / 255,
                        chosen->b * brightness / 255
                    };
                    neopixel_set_pixel(pos, color);
                }
            }
            break;
            
        case 9: // 渐变呼吸
            {
                float phase = (float)(s_frame % 200) / 200.0f * 3.14159f * 2;
                int breath = (int)((sinf(phase) + 1.0f) * 0.5f * brightness);
                
                for (int i = 0; i < 60; i++) {
                    float ratio = (float)i / 60.0f;
                    rgb_color_t base = interpolate_3colors(ratio, s_custom_params.color1, s_custom_params.color2, s_custom_params.color3);
                    rgb_color_t color = {
                        base.r * breath / 255,
                        base.g * breath / 255,
                        base.b * breath / 255
                    };
                    neopixel_set_pixel(i, color);
                }
            }
            break;
            
        default:
            break;
    }
}

esp_err_t light_show_init(void)
{
    ESP_LOGI(TAG, "Light show initialized");
    return ESP_OK;
}

void light_show_set_mode(light_show_mode_t mode)
{
    if (mode >= LIGHT_SHOW_MAX) {
        mode = LIGHT_SHOW_OFF;
    }
    
    if (s_current_mode != mode) {
        ESP_LOGI(TAG, "Light show mode changed to: %s", light_show_get_mode_name(mode));
        s_current_mode = mode;
        s_frame = 0;
        
        if (mode == LIGHT_SHOW_OFF) {
            neopixel_clear();
            neopixel_refresh();
        }
    }
}

light_show_mode_t light_show_get_mode(void)
{
    return s_current_mode;
}

void light_show_set_custom_params(const custom_params_t *params)
{
    if (params) {
        memcpy(&s_custom_params, params, sizeof(custom_params_t));
        ESP_LOGI(TAG, "Custom params updated: color1=(%d,%d,%d) color2=(%d,%d,%d) speed=%d brightness=%d effect=%d",
                 params->color1.r, params->color1.g, params->color1.b,
                 params->color2.r, params->color2.g, params->color2.b,
                 params->speed, params->brightness, params->effect);
    }
}

void light_show_get_custom_params(custom_params_t *params)
{
    if (params) {
        memcpy(params, &s_custom_params, sizeof(custom_params_t));
    }
}

bool light_show_update(void)
{
    if (s_current_mode == LIGHT_SHOW_OFF) {
        return false;
    }
    
    // 根据速度控制更新频率
    int delay_factor = 11 - s_custom_params.speed; // speed 1-10 -> delay 10-1
    if (delay_factor < 1) delay_factor = 1;
    
    s_frame++;
    
    switch (s_current_mode) {
        case LIGHT_SHOW_RAINBOW:
            effect_rainbow();
            break;
        case LIGHT_SHOW_BREATHING:
            effect_breathing();
            break;
        case LIGHT_SHOW_CHASE:
            effect_chase();
            break;
        case LIGHT_SHOW_SPARKLE:
            effect_sparkle();
            break;
        case LIGHT_SHOW_FIRE:
            effect_fire();
            break;
        case LIGHT_SHOW_OCEAN:
            effect_ocean();
            break;
        case LIGHT_SHOW_METEOR:
            effect_meteor();
            break;
        case LIGHT_SHOW_POLICE:
            effect_police();
            break;
        case LIGHT_SHOW_CANDY:
            effect_candy();
            break;
        case LIGHT_SHOW_AURORA:
            effect_aurora();
            break;
        case LIGHT_SHOW_HEARTBEAT:
            effect_heartbeat();
            break;
        case LIGHT_SHOW_CUSTOM:
            effect_custom();
            break;
        default:
            return false;
    }
    
    neopixel_refresh();
    return true;
}

bool light_show_is_active(void)
{
    return s_current_mode != LIGHT_SHOW_OFF;
}

const char* light_show_get_mode_name(light_show_mode_t mode)
{
    static const char* names[] = {
        "时钟模式",
        "彩虹旋转",
        "呼吸灯",
        "流水追逐",
        "星光闪烁",
        "火焰效果",
        "海洋波浪",
        "流星雨",
        "警灯",
        "糖果色",
        "极光",
        "心跳",
        "自定义"
    };
    
    if (mode >= LIGHT_SHOW_MAX) {
        return "未知";
    }
    return names[mode];
}

