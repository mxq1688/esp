/**
 * @file inmp441_config.h
 * @brief INMP441麦克风配置头文件
 * @author AI Assistant Team
 * @version 1.0.0
 */

#ifndef INMP441_CONFIG_H
#define INMP441_CONFIG_H

#include "driver/i2s.h"

// INMP441麦克风引脚定义
#define INMP441_SCK_PIN     4     // 串行时钟
#define INMP441_WS_PIN      5     // 字选择信号
#define INMP441_SD_PIN      6     // 串行数据输出
#define INMP441_L_R_PIN     -1    // 左右声道选择(硬件接GND)

// INMP441音频参数
#define INMP441_SAMPLE_RATE     16000
#define INMP441_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_32BIT  // 接收32位数据
#define INMP441_CHANNEL_FORMAT  I2S_CHANNEL_FMT_ONLY_LEFT  // 左声道
#define INMP441_I2S_PORT        I2S_NUM_0

// INMP441 I2S配置
#define INMP441_I2S_CONFIG_DEFAULT() { \
    .mode = I2S_MODE_MASTER | I2S_MODE_RX, \
    .sample_rate = INMP441_SAMPLE_RATE, \
    .bits_per_sample = INMP441_BITS_PER_SAMPLE, \
    .channel_format = INMP441_CHANNEL_FORMAT, \
    .communication_format = I2S_COMM_FORMAT_STAND_I2S, \
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2, \
    .dma_buf_count = 8, \
    .dma_buf_len = 1024, \
    .use_apll = false, \
    .tx_desc_auto_clear = false, \
    .fixed_mclk = 0 \
}

// INMP441引脚配置
#define INMP441_PIN_CONFIG_DEFAULT() { \
    .bck_io_num = INMP441_SCK_PIN, \
    .ws_io_num = INMP441_WS_PIN, \
    .data_out_num = I2S_PIN_NO_CHANGE, \
    .data_in_num = INMP441_SD_PIN \
}

#endif // INMP441_CONFIG_H
