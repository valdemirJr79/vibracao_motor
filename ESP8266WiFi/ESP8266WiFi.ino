#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Configurações ajustáveis
// LED_BUILTIN no ESP8266 corresponde ao pino D4 (GPIO2)
#define LED_PIN LED_BUILTIN
#define SERIAL_BAUD 115200
#define WIFI_TIMEOUT 15000 // 15 segundos de timeout

const char* ssid = "redework";
const char* passwords[] = {"Acessonetos", "Acessoneto5"};
const int passwordCount = 2;

// Cria um objeto servidor na porta 80 (a porta padrão para HTTP)
ESP8266WebServer server(80);

// Variáveis globais para armazenar o IP e o MAC
String localIP;
String macAddress;

void connectToWiFi();
void startupSequence();
void indicateReconnect();
void indicateFailure();
void handleCriticalFailure();
void showConnectionSuccess();
void handleRoot();

void setup() {
  Serial.begin(SERIAL_BAUD);
  // O LED_BUILTIN no ESP8266 é active-low, HIGH desliga e LOW liga
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  
  // Sequência inicial de diagnóstico
  startupSequence();
  
  Serial.println("\n[Wemos D1] Iniciando conexão WiFi...");
  connectToWiFi();
  
  // Define qual funcao será executada quando o servidor receber uma solicitacao na pagina inicial
  server.on("/", handleRoot);
  
  // Inicia o servidor somente após a conexão bem-sucedida
  server.begin();
  Serial.println("Servidor web iniciado.");
}

void loop() {
  static unsigned long lastCheck = 0;
  
  if (millis() - lastCheck >= 10000) { // Verifica a cada 10s
    lastCheck = millis();
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WIFI] Conexão perdida! Reconectando...");
      indicateReconnect();
      connectToWiFi();
      // Reinicia o servidor caso a reconexão seja necessária
      server.begin();
    } else {
      // Mantém o LED aceso se a conexão estiver OK
      digitalWrite(LED_PIN, LOW);
    }
  }
  
  server.handleClient();
}

void handleRoot() {
  // Constrói a página HTML que será exibida
  String html = "<html><body>";
  html += "<h1>Conectador por Moises</h1>";
  html += "<p>Servidor Web do Wemos D1 funcionando!</p>";
  html += "<p><strong>Endereco IP:</strong> " + localIP + "</p>";
  html += "<p><strong>Endereco MAC:</strong> " + macAddress + "</p>";
  html += "</body></html>";
  
  // Envia a resposta HTTP com o código 200 (OK) e o conteúdo HTML
  server.send(200, "text/html", html);
}

void connectToWiFi() {
  WiFi.disconnect(true); // Limpa conexões anteriores
  delay(1000);
  
  for (int i = 0; i < passwordCount; i++) {
    Serial.printf("\n[WIFI] Tentativa %d/%d com a senha: %s\n", i+1, passwordCount, passwords[i]);
    
    WiFi.begin(ssid, passwords[i]);
    unsigned long startAttemptTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startAttemptTime < WIFI_TIMEOUT) {
      // Pisca durante tentativa (active-low)
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
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
  // Armazena o IP e o MAC nas variáveis globais
  localIP = WiFi.localIP().toString();
  macAddress = WiFi.macAddress();
  
  Serial.println("\n[WIFI] Conectado com sucesso!");
  Serial.println("=== INFORMAÇÕES DE REDE ===");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(localIP);
  Serial.print("MAC: ");
  Serial.println(macAddress);
  Serial.print("Sinal: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.println("===========================");
  
  // Sinalização visual: pisca e fica aceso (active-low)
  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);
  delay(300);
  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);
}

void startupSequence() {
  // Pisca 3 vezes (active-low)
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
  }
}

void indicateReconnect() {
  // Pisca 2 vezes (active-low)
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    delay(500);
  }
}

void indicateFailure() {
  // Pisca 4 vezes rápido (active-low)
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
  }
}

void handleCriticalFailure() {
  Serial.println("\n[ERRO] Não foi possível conectar a nenhuma rede!");
  while (true) {
    // Pisca lentamente (active-low)
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(1900);
  }
}