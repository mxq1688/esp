#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include "esp_err.h"
#include "esp_http_server.h"

// API处理器函数声明
esp_err_t api_status_handler(httpd_req_t *req);
esp_err_t api_led_color_handler(httpd_req_t *req);
esp_err_t api_led_power_handler(httpd_req_t *req);
esp_err_t api_led_effect_handler(httpd_req_t *req);
esp_err_t api_wifi_connect_handler(httpd_req_t *req);
esp_err_t api_options_handler(httpd_req_t *req);

#endif // API_HANDLERS_H
