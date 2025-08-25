idf_component_register(SRCS "servo_control_main.c" "servo_driver.c" "joystick_led_controller.c" "bluetooth_controller.c"
                       PRIV_REQUIRES driver esp_adc led_strip bt nvs_flash
                       INCLUDE_DIRS "include")
