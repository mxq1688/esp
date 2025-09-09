/**
 * @file max98357a_config.h
 * @brief MAX98357A音频放大器配置头文件
 * @author AI Assistant Team
 * @version 1.0.0
 */

#ifndef MAX98357A_CONFIG_H
#define MAX98357A_CONFIG_H

#include "driver/i2s.h"

// MAX98357A放大器引脚定义
#define MAX98357A_BCLK_PIN  7     // 位时钟
#define MAX98357A_LRC_PIN   15    // 左右声道时钟
#define MAX98357A_DIN_PIN   16    // 数据输入
#define MAX98357A_SD_PIN    -1    // 关断控制(硬件接3.3V)
#define MAX98357A_GAIN_PIN  -1    // 增益控制(悬空=9dB)

// MAX98357A音频参数
#define MAX98357A_SAMPLE_RATE     16000
#define MAX98357A_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define MAX98357A_CHANNEL_FORMAT  I2S_CHANNEL_FMT_ONLY_LEFT
#define MAX98357A_I2S_PORT        I2S_NUM_1

// MAX98357A增益设置
typedef enum {
    MAX98357A_GAIN_3DB = 0,   // GAIN接GND
    MAX98357A_GAIN_6DB,       // GAIN接VIN
    MAX98357A_GAIN_9DB,       // GAIN悬空(默认)
    MAX98357A_GAIN_12DB,      // GAIN接100K电阻到GND
    MAX98357A_GAIN_15DB       // GAIN接100K电阻到VIN
} max98357a_gain_t;

// MAX98357A I2S配置
#define MAX98357A_I2S_CONFIG_DEFAULT() { \
    .mode = I2S_MODE_MASTER | I2S_MODE_TX, \
    .sample_rate = MAX98357A_SAMPLE_RATE, \
    .bits_per_sample = MAX98357A_BITS_PER_SAMPLE, \
    .channel_format = MAX98357A_CHANNEL_FORMAT, \
    .communication_format = I2S_COMM_FORMAT_STAND_I2S, \
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2, \
    .dma_buf_count = 8, \
    .dma_buf_len = 1024, \
    .use_apll = false, \
    .tx_desc_auto_clear = true, \
    .fixed_mclk = 0 \
}

// MAX98357A引脚配置
#define MAX98357A_PIN_CONFIG_DEFAULT() { \
    .bck_io_num = MAX98357A_BCLK_PIN, \
    .ws_io_num = MAX98357A_LRC_PIN, \
    .data_out_num = MAX98357A_DIN_PIN, \
    .data_in_num = I2S_PIN_NO_CHANGE \
}

#endif // MAX98357A_CONFIG_H
