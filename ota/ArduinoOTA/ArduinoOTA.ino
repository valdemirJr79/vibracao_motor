#include <WiFi.h>
#include <ArduinoOTA.h>

// Altere com as credenciais da sua rede Wi-Fi
const char *ssid = "REDEWORK";
const char *password = "Acessonet05";

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Loop para aguardar a conexão (corrigido)
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // --- Bloco de configuração do ArduinoOTA ---
  ArduinoOTA
    .onStart([]() {
      String type;
      // Corrigido o erro de digitação de "ArduinoOT" para "ArduinoOTA"
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }
      // NOTA: se estiver atualizando o SPIFFS, este seria o lugar para desmontá-lo usando SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    }); // <-- Adicionado o ponto e vírgula (;) obrigatório aqui.

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // A função handle() precisa ser chamada no loop para que o OTA funcione
  ArduinoOTA.handle();
}