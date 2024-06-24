#include <WiFi.h>
#include <ArduinoBLE.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// WiFi credentials
const char *ssid = "xxxxxx";         // 替换为你的WiFi热点名，不要使用中文
const char *password = "12345678";  // 替换为你的Wifi密码

// 指定的蓝牙MAC地址
const char *targetMACAddress = "11:22:33:44:55:66"; // 替换为考勤机的mac地址

// MQTT Broker settings
const char *mqtt_broker = "test.mosquitto.org"; // （可选）替换为你的broker地址
// const char *mqtt_username = "skysilksock"; // 替换为你设置的用户名
// const char *mqtt_password = "2680613764fwy"; // 替换为你设置的密码
const int mqtt_port = 1883; // 端口号设置，如果使用自己的服务器则设置为8883

// WiFi and MQTT client initialization
WiFiClient esp_client;
// WiFiClientSecure esp_client; // 如果使用自己的服务器则使用WiFiClientSecure，注释上行
PubSubClient mqtt_client(esp_client);

char *mqtt_topic = "dingding";
// 定义 MQTT 主题
String topic_commands = String(mqtt_topic) + "/commands";
String topic_status = String(mqtt_topic) + "/status";
String topic_advertisementData = String(mqtt_topic) + "/advData";
String topic_connect = String(mqtt_topic) + "/connect";

// Root CA Certificate
// Load DigiCert Global Root G2, which is used by EMQX Public Broker: broker.emqx.io
// const char *ca_cert = R"EOF(
// -----BEGIN CERTIFICATE-----
// MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
// MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
// d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
// MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
// MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
// b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
// 9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
// 2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
// 1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
// q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
// tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
// vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
// BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
// 5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
// 1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
// NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
// Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
// 8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
// pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
// MrY=
// -----END CERTIFICATE-----
// )EOF";

// Load DigiCert Global Root CA ca_cert, which is used by EMQX Platform Serverless Deployment
const char *ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";

// Function Declarations
void connectToWiFi();

void connectToMQTT();

void mqttCallback(char *topic, byte *payload, unsigned int length);

void setup() {
  Serial.begin(115200);  // 初始化串口
  connectToWiFi();

  // 初始化蓝牙
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1)
      ;
  }

  // 设置根证书
  // esp_client.setCACert(ca_cert); // 如果使用自己的服务器需要设置根证书

  mqtt_client.setServer(mqtt_broker, mqtt_port);  // 设置mqtt服务
  mqtt_client.setKeepAlive(60);
  mqtt_client.setCallback(mqttCallback);  // 设置回调函数
  connectToMQTT();
}

// 用于连接WiFi的函数
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

// 用于连接MQTT服务的函数
void connectToMQTT() {
  while (!mqtt_client.connected()) {
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT Broker as %s...\n", client_id.c_str());
    if (mqtt_client.connect(client_id.c_str())) {
//  if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) // 使用自己的服务器需要传入账户密码参数
      Serial.println("Connected to MQTT broker");
      mqtt_client.subscribe(topic_commands.c_str());
      mqtt_client.publish(topic_status.c_str(), "Hi EMQX I'm ESP32 ^^");  // Publish message upon connection
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" Retrying in 5 seconds.");
      delay(5000);
    }
  }
}

// 该函数指示了如果收到来自mqtt服务器的消息该如何操作
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("\n-----------------------");
  mqtt_client.publish(topic_connect.c_str(), "Hi EMQX I'm ESP32 ^^");
}

void loop() {
  // 如果断开与服务器的连接则尝试重连
  if (!mqtt_client.connected()) {
    connectToMQTT();
  }
  mqtt_client.loop();  // 类似app.run()

  unsigned long lastMsg = millis(); // 当前时间

  BLE.scan();  // 开启蓝牙扫描
  Serial.println("Scanning for BLE devices...");

  // 持续扫描20秒，试图找到对应mac地址的广播数据
  while (millis() - lastMsg < 20 * 1000) {
    // delay(1000); // 每次得出结果后等待1秒
    String s = getAdvertisementData(targetMACAddress);
    if (s.length()) {
      Serial.println("hello");
      mqtt_client.publish(topic_advertisementData.c_str(), s.c_str());
    }
  }

  BLE.stopScan(); // 结束蓝牙扫描

  // 扫描结束后更新消息，用于判断是否有连接
  String msg = "Hello from ESP32" + String(lastMsg);
  Serial.print("Publish message: ");
  Serial.println(msg);
  mqtt_client.publish(topic_status.c_str(), msg.c_str());

  delay(10 * 1000); // 每次扫描结束后等待10秒再进行扫描
}

// 获取蓝牙设备并判断是否是指定设备，如果是则返回其广播数据，未扫描到设备则返回空字符串
String getAdvertisementData(const char *targetMACAddress) {
  // 获取扫描结果
  BLEDevice peripheral = BLE.available();
  if (peripheral) {
    // 打印设备地址
    Serial.print("Found device: ");
    Serial.println(peripheral.address());

    // 检查是否是指定的MAC地址
    if (peripheral.address() == targetMACAddress) {
      Serial.println("Target device found!");

      // 获取广播数据
      int len = peripheral.advertisementDataLength();
      if (len > 0) {
        byte advData[len];
        peripheral.advertisementData(advData, len);

        // 创建 String 用于记录广播数据
        String advDataStr = "";

        // 将 byte 数组数据复制到String
        for (int i = 0; i < len; i++) {
          char hex[3];  // 两位十六进制数加上终止符
          sprintf(hex, "%02X", advData[i]);
          advDataStr += hex;
        }

        return advDataStr;
      }
    }
    return "";
  }
  return "";
}