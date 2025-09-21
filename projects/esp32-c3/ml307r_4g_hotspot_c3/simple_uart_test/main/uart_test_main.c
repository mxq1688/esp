/*
 * ESP32-C3 UART最简单测试程序
 * 专门测试GPIO4/5的UART1通信
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#define UART_TEST_NUM       UART_NUM_1
#define UART_TEST_TXD       4          // ESP32-C3 GPIO4 -> ML307R RX
#define UART_TEST_RXD       5          // ESP32-C3 GPIO5 <- ML307R TX  
#define UART_TEST_BAUD      115200
#define UART_BUF_SIZE       1024

static const char *TAG = "UART_TEST";

void uart_test_task(void *pvParameters)
{
    uint8_t *data = (uint8_t *) malloc(UART_BUF_SIZE);
    
    while (1) {
        // 发送AT命令
        const char* test_str = "AT\r\n";
        ESP_LOGI(TAG, "发送AT命令: %s", test_str);
        uart_write_bytes(UART_TEST_NUM, test_str, strlen(test_str));
        
        // 等待响应
        int len = uart_read_bytes(UART_TEST_NUM, data, UART_BUF_SIZE-1, 3000 / portTICK_PERIOD_MS);
        
        if (len > 0) {
            data[len] = 0;
            ESP_LOGI(TAG, "收到ML307R响应 [%d字节]: %s", len, (char*)data);
        } else {
            ESP_LOGW(TAG, "ML307R无响应 - 检查连接或配置");
        }
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    
    free(data);
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C3 UART测试程序启动");
    ESP_LOGI(TAG, "目标: 测试与ML307R的UART通信");
    
    // UART配置
    uart_config_t uart_config = {
        .baud_rate = UART_TEST_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    ESP_LOGI(TAG, "配置UART参数: 波特率=%d, 数据位=8, 停止位=1, 无校验", UART_TEST_BAUD);
    
    // 安装UART驱动
    ESP_ERROR_CHECK(uart_driver_install(UART_TEST_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_TEST_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_TEST_NUM, UART_TEST_TXD, UART_TEST_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    
    ESP_LOGI(TAG, "UART1引脚配置: TX=GPIO%d, RX=GPIO%d", UART_TEST_TXD, UART_TEST_RXD);
    ESP_LOGI(TAG, "物理连接: ESP32-C3 GPIO%d -> ML307R RX", UART_TEST_TXD);
    ESP_LOGI(TAG, "物理连接: ESP32-C3 GPIO%d <- ML307R TX", UART_TEST_RXD);
    
    // 启动测试任务
    xTaskCreate(uart_test_task, "uart_test", 4096, NULL, 10, NULL);
    
    ESP_LOGI(TAG, "UART测试任务已启动，开始发送AT命令...");
}
