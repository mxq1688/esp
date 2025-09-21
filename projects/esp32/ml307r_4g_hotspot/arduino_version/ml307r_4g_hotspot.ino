#include <WiFi.h>
#include <WiFiAP.h>

// WiFi热点配置
const char* ssid = "ESP32_4G_Hotspot";
const char* password = "12345678";

// ML307R UART配置
#define ML307R_UART_NUM 2
#define ML307R_RXD_PIN 16
#define ML307R_TXD_PIN 17
#define ML307R_BAUD_RATE 115200

// 串口2用于ML307R通信
HardwareSerial ML307R_Serial(ML307R_UART_NUM);

void setup() {
  // 初始化串口监视器
  Serial.begin(115200);
  Serial.println("ESP32 ML307R 4G Hotspot Starting...");
  
  // 初始化ML307R串口
  ML307R_Serial.begin(ML307R_BAUD_RATE, SERIAL_8N1, ML307R_RXD_PIN, ML307R_TXD_PIN);
  Serial.println("ML307R UART initialized");
  
  // 初始化WiFi热点
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.printf("WiFi Hotspot: %s, Password: %s\n", ssid, password);
  
  // 初始化ML307R模块
  initML307R();
  
  Serial.println("System ready! Connect your devices to the hotspot.");
}

void loop() {
  // 检查连接的设备数量
  int connectedDevices = WiFi.softAPgetStationNum();
  static int lastDeviceCount = -1;
  
  if (connectedDevices != lastDeviceCount) {
    Serial.printf("Connected devices: %d\n", connectedDevices);
    lastDeviceCount = connectedDevices;
  }
  
  // 处理ML307R通信
  handleML307R();
  
  delay(1000);
}

void initML307R() {
  Serial.println("Initializing ML307R 4G module...");
  
  delay(3000); // 等待模块启动
  
  // 发送AT命令测试
  sendATCommand("AT");
  delay(1000);
  
  // 关闭回显
  sendATCommand("ATE0");
  delay(1000);
  
  // 检查SIM卡
  sendATCommand("AT+CPIN?");
  delay(1000);
  
  // 检查网络注册
  sendATCommand("AT+CREG?");
  delay(1000);
  
  sendATCommand("AT+CGREG?");
  delay(1000);
  
  // 设置PDP上下文
  sendATCommand("AT+CGDCONT=1,\"IP\",\"cmnet\"");
  delay(1000);
  
  // 激活PDP上下文
  sendATCommand("AT+CGACT=1,1");
  delay(5000);
  
  // 获取IP地址
  sendATCommand("AT+CGPADDR=1");
  delay(1000);
  
  Serial.println("ML307R initialization completed");
}

void sendATCommand(String command) {
  Serial.print("Sending: ");
  Serial.println(command);
  
  ML307R_Serial.println(command);
  
  // 等待响应
  delay(500);
  String response = "";
  while (ML307R_Serial.available()) {
    response += (char)ML307R_Serial.read();
  }
  
  if (response.length() > 0) {
    Serial.print("Response: ");
    Serial.println(response);
  }
}

void handleML307R() {
  // 处理ML307R的数据
  if (ML307R_Serial.available()) {
    String data = ML307R_Serial.readString();
    Serial.print("ML307R Data: ");
    Serial.println(data);
  }
  
  // 可以在这里添加更多的ML307R处理逻辑
}

void onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_AP_STACONNECTED:
      Serial.println("Station connected to AP");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.println("Station disconnected from AP");
      break;
    default:
      break;
  }
}
