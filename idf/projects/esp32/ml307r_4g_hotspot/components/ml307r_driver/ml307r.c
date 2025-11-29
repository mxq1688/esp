#include "ml307r.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

static const char *TAG = "ML307R";

esp_err_t ml307r_init(ml307r_t *ml307r)
{
    if (ml307r == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // 配置UART参数
    uart_config_t uart_config = {
        .baud_rate = ml307r->baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // 安装UART驱动
    esp_err_t ret = uart_driver_install(ml307r->uart_num, ML307R_BUFFER_SIZE * 2, 0, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver: %s", esp_err_to_name(ret));
        return ret;
    }

    // 配置UART参数
    ret = uart_param_config(ml307r->uart_num, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART: %s", esp_err_to_name(ret));
        return ret;
    }

    // 设置UART引脚
    ret = uart_set_pin(ml307r->uart_num, ml307r->txd_pin, ml307r->rxd_pin, 
                       UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins: %s", esp_err_to_name(ret));
        return ret;
    }

    ml307r->initialized = true;
    ml307r->net_status = ML307R_NET_NOT_REGISTERED;
    memset(ml307r->ip_address, 0, sizeof(ml307r->ip_address));

    ESP_LOGI(TAG, "ML307R initialized successfully");
    return ESP_OK;
}

esp_err_t ml307r_send_command(ml307r_t *ml307r, const char *command)
{
    if (ml307r == NULL || command == NULL || !ml307r->initialized) {
        return ESP_ERR_INVALID_ARG;
    }

    int len = uart_write_bytes(ml307r->uart_num, command, strlen(command));
    if (len < 0) {
        ESP_LOGE(TAG, "Failed to send command: %s", command);
        return ESP_FAIL;
    }

    ESP_LOGD(TAG, "Sent: %s", command);
    return ESP_OK;
}

int ml307r_read_response(ml307r_t *ml307r, char *response, size_t max_len, uint32_t timeout_ms)
{
    if (ml307r == NULL || response == NULL || !ml307r->initialized) {
        return -1;
    }

    int len = uart_read_bytes(ml307r->uart_num, (uint8_t*)response, max_len - 1, 
                              timeout_ms / portTICK_PERIOD_MS);
    if (len > 0) {
        response[len] = '\0';
        ESP_LOGD(TAG, "Received: %s", response);
    }

    return len;
}

ml307r_status_t ml307r_wait_response(ml307r_t *ml307r, const char *expected, uint32_t timeout_ms)
{
    if (ml307r == NULL || expected == NULL || !ml307r->initialized) {
        return ML307R_ERROR;
    }

    char response[ML307R_BUFFER_SIZE];
    uint32_t start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    while ((xTaskGetTickCount() * portTICK_PERIOD_MS - start_time) < timeout_ms) {
        int len = ml307r_read_response(ml307r, response, sizeof(response), 100);
        if (len > 0) {
            if (strstr(response, expected) != NULL) {
                return ML307R_OK;
            }
            if (strstr(response, "ERROR") != NULL) {
                return ML307R_ERROR;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    return ML307R_TIMEOUT;
}

ml307r_status_t ml307r_check_sim(ml307r_t *ml307r)
{
    if (ml307r == NULL || !ml307r->initialized) {
        return ML307R_ERROR;
    }

    ESP_LOGI(TAG, "Checking SIM card status...");
    
    esp_err_t ret = ml307r_send_command(ml307r, ML307R_AT_CPIN);
    if (ret != ESP_OK) {
        return ML307R_ERROR;
    }

    ml307r_status_t status = ml307r_wait_response(ml307r, "+CPIN: READY", 3000);
    if (status == ML307R_OK) {
        ESP_LOGI(TAG, "SIM card is ready");
    } else {
        ESP_LOGE(TAG, "SIM card not ready or error");
    }

    return status;
}

ml307r_status_t ml307r_check_network(ml307r_t *ml307r)
{
    if (ml307r == NULL || !ml307r->initialized) {
        return ML307R_ERROR;
    }

    ESP_LOGI(TAG, "Checking network registration...");
    
    // 检查CS网络注册
    esp_err_t ret = ml307r_send_command(ml307r, ML307R_AT_CREG);
    if (ret != ESP_OK) {
        return ML307R_ERROR;
    }

    char response[ML307R_BUFFER_SIZE];
    int len = ml307r_read_response(ml307r, response, sizeof(response), 3000);
    if (len > 0) {
        if (strstr(response, "+CREG: 0,1") != NULL || strstr(response, "+CREG: 0,5") != NULL) {
            ml307r->net_status = ML307R_NET_REGISTERED_HOME;
            ESP_LOGI(TAG, "CS network registered (home)");
        } else if (strstr(response, "+CREG: 0,2") != NULL || strstr(response, "+CREG: 0,6") != NULL) {
            ml307r->net_status = ML307R_NET_REGISTERED_ROAMING;
            ESP_LOGI(TAG, "CS network registered (roaming)");
        } else {
            ml307r->net_status = ML307R_NET_NOT_REGISTERED;
            ESP_LOGW(TAG, "CS network not registered");
        }
    }

    // 检查PS网络注册
    ret = ml307r_send_command(ml307r, ML307R_AT_CGREG);
    if (ret != ESP_OK) {
        return ML307R_ERROR;
    }

    len = ml307r_read_response(ml307r, response, sizeof(response), 3000);
    if (len > 0) {
        if (strstr(response, "+CGREG: 0,1") != NULL || strstr(response, "+CGREG: 0,5") != NULL) {
            ESP_LOGI(TAG, "PS network registered (home)");
            return ML307R_OK;
        } else if (strstr(response, "+CGREG: 0,2") != NULL || strstr(response, "+CGREG: 0,6") != NULL) {
            ESP_LOGI(TAG, "PS network registered (roaming)");
            return ML307R_OK;
        } else {
            ESP_LOGW(TAG, "PS network not registered");
            return ML307R_NOT_READY;
        }
    }

    return ML307R_ERROR;
}

ml307r_status_t ml307r_activate_pdp(ml307r_t *ml307r)
{
    if (ml307r == NULL || !ml307r->initialized) {
        return ML307R_ERROR;
    }

    ESP_LOGI(TAG, "Activating PDP context...");
    
    // 设置PDP上下文
    esp_err_t ret = ml307r_send_command(ml307r, ML307R_AT_CGDCONT);
    if (ret != ESP_OK) {
        return ML307R_ERROR;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // 附着到GPRS
    ret = ml307r_send_command(ml307r, ML307R_AT_CGATT);
    if (ret != ESP_OK) {
        return ML307R_ERROR;
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // 激活PDP上下文
    ret = ml307r_send_command(ml307r, ML307R_AT_CGACT);
    if (ret != ESP_OK) {
        return ML307R_ERROR;
    }

    ml307r_status_t status = ml307r_wait_response(ml307r, "OK", 10000);
    if (status == ML307R_OK) {
        ESP_LOGI(TAG, "PDP context activated successfully");
    } else {
        ESP_LOGE(TAG, "Failed to activate PDP context");
    }

    return status;
}

ml307r_status_t ml307r_get_ip_address(ml307r_t *ml307r)
{
    if (ml307r == NULL || !ml307r->initialized) {
        return ML307R_ERROR;
    }

    ESP_LOGI(TAG, "Getting IP address...");
    
    esp_err_t ret = ml307r_send_command(ml307r, ML307R_AT_CGPADDR);
    if (ret != ESP_OK) {
        return ML307R_ERROR;
    }

    char response[ML307R_BUFFER_SIZE];
    int len = ml307r_read_response(ml307r, response, sizeof(response), 5000);
    if (len > 0) {
        // 解析IP地址
        char *ip_start = strstr(response, "+CGPADDR: 1,\"");
        if (ip_start != NULL) {
            ip_start += 13; // 跳过 "+CGPADDR: 1,\""
            char *ip_end = strchr(ip_start, '"');
            if (ip_end != NULL) {
                int ip_len = ip_end - ip_start;
                if (ip_len < sizeof(ml307r->ip_address)) {
                    strncpy(ml307r->ip_address, ip_start, ip_len);
                    ml307r->ip_address[ip_len] = '\0';
                    ESP_LOGI(TAG, "Got IP address: %s", ml307r->ip_address);
                    return ML307R_OK;
                }
            }
        }
    }

    ESP_LOGE(TAG, "Failed to get IP address");
    return ML307R_ERROR;
}

ml307r_status_t ml307r_connect_4g(ml307r_t *ml307r)
{
    if (ml307r == NULL || !ml307r->initialized) {
        return ML307R_ERROR;
    }

    ESP_LOGI(TAG, "Starting 4G connection process...");

    // 关闭回显
    ml307r_send_command(ml307r, ML307R_AT_ECHO_OFF);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // 测试AT命令
    ml307r_send_command(ml307r, ML307R_AT_TEST);
    if (ml307r_wait_response(ml307r, "OK", 3000) != ML307R_OK) {
        ESP_LOGE(TAG, "AT test failed");
        return ML307R_ERROR;
    }
    ESP_LOGI(TAG, "AT test passed");

    // 检查SIM卡
    if (ml307r_check_sim(ml307r) != ML307R_OK) {
        ESP_LOGE(TAG, "SIM card check failed");
        return ML307R_ERROR;
    }

    // 检查网络注册
    if (ml307r_check_network(ml307r) != ML307R_OK) {
        ESP_LOGE(TAG, "Network registration failed");
        return ML307R_ERROR;
    }

    // 激活PDP上下文
    if (ml307r_activate_pdp(ml307r) != ML307R_OK) {
        ESP_LOGE(TAG, "PDP activation failed");
        return ML307R_ERROR;
    }

    // 获取IP地址
    if (ml307r_get_ip_address(ml307r) != ML307R_OK) {
        ESP_LOGE(TAG, "Failed to get IP address");
        return ML307R_ERROR;
    }

    ESP_LOGI(TAG, "4G connection established successfully!");
    return ML307R_OK;
}
