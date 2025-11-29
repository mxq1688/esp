# ESP32-C3 AI助手主组件
# 注意: ESP32-C3不支持音频处理，暂时排除voice_processor.c
idf_component_register(SRCS "main.c" "ai_engine.c" "web_interface.c" "wifi_manager.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_http_client json nvs_flash esp_wifi esp_event)
