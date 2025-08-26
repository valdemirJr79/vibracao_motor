
#include <ArduinoOTA.h>
#include <WiFi.h>


#define LED_BUILTIN 2 // Define o pino do LED embutido

const char* ssid = "REDEWORK";
const char* password = "Acessonet05";

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // Define o pino do LED como saída
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Conectado à rede Wi-Fi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {
      type = "filesystem";
    }
    Serial.println("Iniciando atualização de " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nFim");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progresso: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Erro de autenticação");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Erro ao iniciar");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Erro de conexão");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Erro ao receber");
    else if (error == OTA_END_ERROR) Serial.println("Erro ao finalizar");
  });
  ArduinoOTA.begin();
  Serial.println("Pronto para OTA");



}

void loop() {
  ArduinoOTA.handle();
  // Seu código principal aqui
  digitalWrite(LED_BUILTIN, HIGH); // Liga o LED
  Serial.println("LED Aceso");
  delay(1000); // Aguarda 1 segundo

  digitalWrite(LED_BUILTIN, LOW); // Desliga o LED
  Serial.println("LED Apagado");
  delay(1000); // Aguarda 1 segundo

}