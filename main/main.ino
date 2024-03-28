#include "DHT20.h"
#include <M5AtomS3.h>
DHT20 DHT;

#include <WiFi.h>
#include <PubSubClient.h>


#define ATOM_S3_SDA_PIN 38
#define ATOM_S3_SCL_PIN 39

#define MQTT_UPDATE_INTERVAL 10000

const char* ssid = "Server"; ///Tên Wifi
const char* password = "12345678"; //Mật Khẩu Wifi
//Thông Tin Kết Nối MQTT
const char* mqttServer = "mqttserver.tk";
const int mqttPort = 1883;
const char* mqttUser = "innovation";
const char* mqttPassword = "Innovation_RgPQAZoA5N";
const char* MQTT_TOPIC_PUB = "/innovation/algriculture/AtomS3_Project";



WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient", mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC_PUB); // Subscribe to the topic to listen for messages (if needed)
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("DHT20 LIBRARY VERSION: ");
  Serial.println(DHT20_LIB_VERSION);
  Serial.println();

  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  Wire.begin(ATOM_S3_SDA_PIN, ATOM_S3_SCL_PIN);
  DHT.begin();

  delay(1000);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - DHT.lastRead() >= MQTT_UPDATE_INTERVAL) {
    //  READ DATA
    uint32_t start = micros();
    int status = DHT.read();
    uint32_t stop = micros();

    Serial.print("DHT20 \t");
    Serial.print(DHT.getHumidity(), 1);
    Serial.print("\t\t");
    Serial.print(DHT.getTemperature(), 1);
    Serial.print("\t\t");
    Serial.print(stop - start);
    Serial.print("\t\t");
    switch (status) {
      case DHT20_OK:
        Serial.print("OK");
        break;
      case DHT20_ERROR_CHECKSUM:
        Serial.print("Checksum error");
        break;
      case DHT20_ERROR_CONNECT:
        Serial.print("Connect error");
        break;
      case DHT20_MISSING_BYTES:
        Serial.print("Missing bytes");
        break;
      case DHT20_ERROR_BYTES_ALL_ZERO:
        Serial.print("All bytes read zero");
        break;
      case DHT20_ERROR_READ_TIMEOUT:
        Serial.print("Read time out");
        break;
      case DHT20_ERROR_LASTREAD:
        Serial.print("Error read too fast");
        break;
      default:
        Serial.print("Unknown error");
        break;
    }
    Serial.print("\n");

    // Publish sensor data to MQTT topic
    char sensorData[100];
    snprintf(sensorData, sizeof(sensorData), "{\"humidity\": %.1f, \"temperature\": %.1f}", DHT.getHumidity(), DHT.getTemperature());
    client.publish(MQTT_TOPIC_PUB, sensorData); // Publish sensor data to the MQTT topic
  }
}
