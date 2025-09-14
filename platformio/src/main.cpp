#include <Arduino.h>

// 定义 LED 引脚 (ESP32 开发板通常内置 LED 在 GPIO2)
#define LED_PIN 2

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  
  // 设置 LED 引脚为输出模式
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("ESP32 LED 闪烁示例程序启动!");
}

void loop() {
  // 点亮 LED
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED 开启");
  delay(1000);  // 延时 1 秒
  
  // 熄灭 LED
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED 关闭");
  delay(1000);  // 延时 1 秒
}
