#include <WiFi.h>

// Configurações ajustáveis
#define LED_PIN 2          // GPIO2 para maioria das ESP32 (ajuste se necessário)
#define SERIAL_BAUD 115200
#define WIFI_TIMEOUT 15000 // 15 segundos de timeout

const char* ssid = "redework";
const char* passwords[] = {"Acessonetos", "Acessoneto5"};
const int passwordCount = 2;

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(LED_PIN, OUTPUT);
  
  // Sequência inicial de diagnóstico
  startupSequence();
  
  Serial.println("\n[ESP32] Iniciando conexão WiFi...");
  connectToWiFi();
}

void loop() {
  static unsigned long lastCheck = 0;
  
  if (millis() - lastCheck >= 10000) { // Verifica a cada 10s
    lastCheck = millis();
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WIFI] Conexão perdida! Reconectando...");
      indicateReconnect();
      connectToWiFi();
    } else {
      digitalWrite(LED_PIN, WiFi.status() == WL_CONNECTED);
    }
  }
}

void connectToWiFi() {
  WiFi.disconnect(true); // Limpa conexões anteriores
  delay(1000);
  
  for (int i = 0; i < passwordCount; i++) {
    Serial.printf("\n[WIFI] Tentativa %d/%d\n", i+1, passwordCount);
    
    WiFi.begin(ssid, passwords[i]);
    unsigned long startAttemptTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startAttemptTime < WIFI_TIMEOUT) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Pisca durante tentativa
      delay(250);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      showConnectionSuccess();
      return;
    }
    
    Serial.println("\n[WIFI] Falha na tentativa");
    indicateFailure();
  }
  
  handleCriticalFailure();
}

void showConnectionSuccess() {
  Serial.println("\n[WIFI] Conectado com sucesso!");
  Serial.println("=== INFORMAÇÕES DE REDE ===");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Sinal: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.println("===========================");
  
  // Sinalização visual
  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);
  delay(300);
  digitalWrite(LED_PIN, HIGH); // Permanece aceso
}

void startupSequence() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

void indicateReconnect() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
}

void indicateFailure() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

void handleCriticalFailure() {
  Serial.println("\n[ERRO] Não foi possível conectar!");
  while (true) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(1900);
  }
}