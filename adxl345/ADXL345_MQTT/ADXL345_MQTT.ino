#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ==== CONFIGURAÇÕES Wi-Fi e MQTT ====
const char* ssid = "redeiot";
const char* password = "652162561";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "iot/vibracao/adxl345";

// ==== OBJETOS ====
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// ==== FUNÇÕES ====
void setup_wifi() {
  delay(10);
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop até conectar
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect("ESP32_ADXL345")) {
      Serial.println("Conectado!");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Inicializa o sensor
  if(!accel.begin()) {
    Serial.println("Erro ao detectar ADXL345. Verifique a conexão!");
    while(1);
  }
  Serial.println("ADXL345 detectado!");

  // Inicializa Wi-Fi e MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sensors_event_t event; 
  accel.getEvent(&event);
  
  // Monta a mensagem JSON
  String payload = "{";
  payload += "\"x\":" + String(event.acceleration.x, 2) + ",";
  payload += "\"y\":" + String(event.acceleration.y, 2) + ",";
  payload += "\"z\":" + String(event.acceleration.z, 2);
  payload += "}";

  Serial.print("Publicando: ");
  Serial.println(payload);

  // Publica no tópico MQTT
  client.publish(mqtt_topic, payload.c_str());

  delay(2000); // envia a cada 2 segundos
}

