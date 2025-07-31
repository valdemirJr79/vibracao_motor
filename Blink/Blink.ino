#include <WiFi.h>

// Configurações da rede
const char* ssid = "redework";
const char* passwords[] = {"Acessonetos", "Acessoneto5"}; // Lista de senhas
const int passwordCount = 2;

// Configuração do LED - Definindo manualmente para GPIO2
const int ledPin = 2; // GPIO2 para LED interno na maioria das ESP32
bool ledState = LOW;

// Variáveis de controle
unsigned long previousMillis = 0;
const long interval = 10000; // Intervalo para verificar conexão (10s)

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  // Padrão inicial do LED (piscar rápido)
  blinkLED(200, 5); // Pisca rápido 5 vezes
  
  connectToWiFi();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Verifica periodicamente a conexão
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Conexão perdida. Tentando reconectar...");
      blinkLED(500, 2); // Indica tentativa de reconexão
      connectToWiFi();
    } else {
      // Pisca suave para indicar conexão estável
      digitalWrite(ledPin, !digitalRead(ledPin));
      Serial.println("Conexão estável. Piscando suave...");
    }
  }
}

void connectToWiFi() {
  Serial.println("\nIniciando conexão WiFi...");
  
  for (int i = 0; i < passwordCount; i++) {
    Serial.printf("Tentativa %d com senha: %s\n", i+1, passwords[i]);
    
    WiFi.begin(ssid, passwords[i]);
    int attempts = 0;
    
    // Padrão de LED durante tentativa (piscar médio)
    while (WiFi.status() != WL_CONNECTED && attempts < 15) {
      digitalWrite(ledPin, !digitalRead(ledPin));
      delay(300);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      connectionSuccess();
      return;
    }
    
    Serial.println("\nFalha na tentativa com esta senha.");
    blinkLED(100, 3); // Indica falha na tentativa
  }
  
  connectionFailed();
}

void connectionSuccess() {
  Serial.println("\nConexão estabelecida com sucesso!");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  
  // Sinalização visual de sucesso (3 piscadas rápidas e depois permanece aceso)
  blinkLED(150, 3);
  digitalWrite(ledPin, HIGH);
}

void connectionFailed() {
  Serial.println("\nFalha ao conectar com todas as senhas disponíveis.");
  
  // Sinalização visual de falha (piscar lento em loop)
  while (true) {
    blinkLED(1000, 1); // Pisca lento continuamente
    Serial.println("Tentando novamente em 10 segundos...");
    delay(10000);
    connectToWiFi(); // Tenta reconectar após pausa
  }
}

void blinkLED(int delayTime, int blinks) {
  for (int i = 0; i < blinks; i++) {
    digitalWrite(ledPin, HIGH);
    delay(delayTime);
    digitalWrite(ledPin, LOW);
    delay(delayTime);
  }
}