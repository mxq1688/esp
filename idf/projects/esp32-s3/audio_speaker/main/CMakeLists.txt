# Main component CMakeLists.txt for ESP32-S3 Audio Speaker

idf_component_register(
    SRCS 
        "main.c"
        "audio_player.c"
        "wifi_audio.c"
    INCLUDE_DIRS 
        "."
        "include"
    REQUIRES 
        driver
        esp_driver_i2s
        esp_timer
        esp_system
        freertos
        log
        esp_common
        nvs_flash
        esp_wifi
        esp_event
        esp_netif
        esp_http_client
)

# 设置组件版本信息
set_property(TARGET ${COMPONENT_LIB} PROPERTY VERSION "1.0.0")

