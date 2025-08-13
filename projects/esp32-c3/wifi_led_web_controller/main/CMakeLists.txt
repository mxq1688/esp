# Main component CMakeLists.txt for ESP32-C3 WiFi LED Controller

idf_component_register(
    SRCS 
        "main.c"
        "wifi_manager.c"
        "led_controller.c"
        "web_server.c"
        "api_handlers.c"
    INCLUDE_DIRS 
        "."
        "include"
    REQUIRES 
        nvs_flash
        esp_wifi
        esp_event
        esp_http_server
        json
        driver
        esp_timer
        esp_netif
        lwip
        esp_system
        freertos
        log
        esp_common
)

# 设置组件版本信息
set_property(TARGET ${COMPONENT_LIB} PROPERTY VERSION "2.0.0")
