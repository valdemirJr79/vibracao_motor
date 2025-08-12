/*
  Este é um código completo para um sistema de monitoramento de vibração
  com o Wemos D1 (ESP8266). Ele utiliza a comunicação MQTT para enviar dados
  de um sensor, receber comandos de controle (para LED, alarme e limite de vibração),
  e armazena as configurações de rede na memória EEPROM da placa para que
  elas não sejam perdidas mesmo se a energia for cortada.
*/

// --- INCLUSÃO DE BIBLIOTECAS ---
#include <ESP8266WiFi.h>      // Biblioteca para gerenciar a conexão WiFi no ESP8266.
#include <PubSubClient.h>     // Biblioteca para comunicação com o broker MQTT.
#include <EEPROM.h>           // Biblioteca para ler e escrever dados na memória flash (EEPROM) do ESP8266.
#include <ArduinoJson.h>      // Biblioteca para serializar e desserializar dados em formato JSON, ideal para MQTT.

// --- CONFIGURAÇÕES PADRÃO E ESTRUTURA DE DADOS ---
#define DEFAULT_SSID "REDEWORK"         // Define o nome de Wi-Fi padrão.
#define DEFAULT_PASS "Acessonet05"      // Define a senha de Wi-Fi padrão.
#define DEFAULT_MQTT_SERVER "192.168.1.4" // Define o endereço IP do broker MQTT padrão.
#define CONFIG_VERSION "V2"             // Versão da estrutura de configuração, usada para checar se a EEPROM está atualizada.

// A estrutura 'Config' define como os dados serão armazenados na EEPROM.
// Ela contém as credenciais de rede, servidor MQTT e um ID de cliente.
struct Config {
  char version[4];        // Versão da configuração, para compatibilidade futura.
  char ssid[32];          // Nome da rede Wi-Fi.
  char password[32];      // Senha da rede Wi-Fi.
  char mqtt_server[16];   // Endereço IP do servidor MQTT.
  int mqtt_port;          // Porta do servidor MQTT (padrão é 1883).
  char client_id[32];     // Identificador único para este cliente MQTT.
};

Config config; // Cria uma variável global do tipo 'Config'.

// --- TÓPICOS MQTT ---
const char* topic_vibration = "vibracao_motor/sensor";  // Tópico para publicar dados do sensor de vibração.
const char* topic_commands = "vibracao_motor/comandos"; // Tópico para receber comandos de controle.
const char* topic_status = "vibracao_motor/status";     // Tópico para publicar o estado atual do dispositivo.

// --- PINOS DO WEMOS D1 ---
#define SENSOR_PIN A0     // Define o pino A0 como entrada para o sensor de vibração.
#define LED_PIN D4        // Define o pino D4 (LED interno) para feedback visual.
#define BUZZER_PIN D3     // Define o pino D3 para o buzzer de alarme.

// --- VARIÁVEIS DE CONTROLE E ESTADO ---
unsigned long lastVibrationRead = 0;  // Armazena o tempo do último envio de dados de vibração.
unsigned long lastReconnectAttempt = 0; // Armazena o tempo da última tentativa de reconexão MQTT.
float vibrationThreshold = 2.5;       // Limite de vibração em m/s² para ativar o alarme (pode ser alterado por MQTT).
bool alarmActive = false;             // Flag para controlar se o alarme está ativado ou não.

// --- OBJETOS DE CLIENTE ---
WiFiClient espClient;          // Objeto para gerenciar a conexão Wi-Fi de baixo nível.
PubSubClient client(espClient); // Objeto para gerenciar a conexão MQTT, usando o cliente Wi-Fi.

// --- FUNÇÕES DE CONFIGURAÇÃO E ARMAZENAMENTO ---

// Esta função carrega a configuração da memória EEPROM.
void loadConfig() {
  EEPROM.begin(sizeof(Config)); // Inicia a EEPROM com o tamanho da estrutura.
  EEPROM.get(0, config);        // Lê os dados da EEPROM e os armazena na variável 'config'.
  
  // Verifica se a versão da configuração lida é a mesma que a padrão.
  if (strcmp(config.version, CONFIG_VERSION) != 0) {
    Serial.println("Configuração não encontrada ou inválida. Usando valores padrão.");
    // Se a versão não for compatível, usa as configurações padrão.
    strcpy(config.version, CONFIG_VERSION);
    strcpy(config.ssid, DEFAULT_SSID);
    strcpy(config.password, DEFAULT_PASS);
    strcpy(config.mqtt_server, DEFAULT_MQTT_SERVER);
    config.mqtt_port = 1883;
    strcpy(config.client_id, "WemosD1_Vibration");
    saveConfig(); // Salva as configurações padrão na EEPROM.
  }
}

// Esta função salva a configuração atual na EEPROM.
void saveConfig() {
  EEPROM.put(0, config);  // Escreve a variável 'config' na memória EEPROM.
  EEPROM.commit();        // Confirma a gravação dos dados na memória flash.
}

// --- FUNÇÕES DE CONEXÃO ---

// Esta função tenta conectar o Wemos D1 à rede Wi-Fi.
void setup_wifi() {
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(config.ssid);

  WiFi.begin(config.ssid, config.password); // Inicia a conexão com o SSID e senha salvos.

  int attempts = 0;
  // Loop para aguardar a conexão. Tenta até 20 vezes (10 segundos).
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Pisca o LED interno para feedback visual.
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_PIN, LOW); // LED aceso para indicar que a conexão foi bem-sucedida.
  } else {
    Serial.println("\nFalha na conexão WiFi!");
    ESP.restart(); // Se falhar, reinicia a placa para tentar novamente.
  }
}

// --- FUNÇÕES MQTT ---

// Esta é a função de callback, chamada quando uma mensagem MQTT é recebida.
void callback(char* topic, byte* payload, unsigned int length) {
  // Converte o payload da mensagem (bytes) em uma string.
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  
  Serial.printf("Mensagem recebida [%s]: %s\n", topic, message);

  // Verifica se o tópico recebido é o de comandos.
  if (String(topic) == topic_commands) {
    DynamicJsonDocument doc(256);        // Cria um objeto JSON para a mensagem.
    deserializeJson(doc, message);       // Desserializa o JSON da mensagem para o objeto.
    
    // Processa os comandos do JSON
    if (doc.containsKey("led")) {
      // Se o JSON contiver "led", liga/desliga o LED interno.
      // A lógica é invertida (LOW = ligado, HIGH = desligado).
      digitalWrite(LED_PIN, doc["led"] ? LOW : HIGH);
    }
    
    if (doc.containsKey("threshold")) {
      // Se o JSON contiver "threshold", atualiza o limite de vibração.
      vibrationThreshold = doc["threshold"];
      Serial.printf("Novo limite de vibração: %.2f m/s²\n", vibrationThreshold);
    }
    
    if (doc.containsKey("alarm")) {
      // Se o JSON contiver "alarm", ativa/desativa o alarme (buzzer).
      alarmActive = doc["alarm"];
      digitalWrite(BUZZER_PIN, alarmActive ? HIGH : LOW);
    }
  }
}

// Esta função tenta reconectar ao broker MQTT de forma não-bloqueante.
void reconnect() {
  long now = millis();
  // Tenta reconectar a cada 5 segundos.
  if (now - lastReconnectAttempt > 5000) {
    lastReconnectAttempt = now;
    
    // Tenta conectar usando o ID de cliente salvo.
    if (client.connect(config.client_id)) {
      Serial.println("Conectado ao MQTT!");
      
      publishSystemStatus(); // Publica o status inicial do sistema.
      
      // Se inscreve no tópico de comandos para receber instruções.
      client.subscribe(topic_commands);
    } else {
      Serial.print("Falha no MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
    }
  }
}

// Publica o status atual do dispositivo em um tópico MQTT.
void publishSystemStatus() {
  DynamicJsonDocument doc(512); // Cria um objeto JSON para o status.
  
  doc["status"] = "online";
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  doc["threshold"] = vibrationThreshold;
  doc["alarm"] = alarmActive;
  
  String output;
  serializeJson(doc, output); // Converte o objeto JSON para uma string.
  client.publish(topic_status, output.c_str()); // Publica a string no tópico.
}

// --- FUNÇÃO DE LEITURA DO SENSOR ---

// Esta função lê o valor do sensor de vibração (simulado).
float readVibration() {
  // Simula a leitura de um sensor analógico no pino A0.
  // Em um projeto real, você usaria a leitura de um sensor como o ADXL335.
  int rawValue = analogRead(SENSOR_PIN);
  
  // Converte o valor bruto do sensor (0-1024) para m/s² (exemplo).
  float vibration = (rawValue / 1024.0) * 5.0; // Assume que 5V = 5 m/s².
  
  // Verifica se a vibração excede o limite e se o alarme está ativo.
  if (vibration > vibrationThreshold && alarmActive) {
    digitalWrite(BUZZER_PIN, HIGH); // Liga o buzzer.
  } else {
    digitalWrite(BUZZER_PIN, LOW);  // Mantém o buzzer desligado.
  }
  
  return vibration; // Retorna o valor da vibração em m/s².
}

// --- FUNÇÕES PRINCIPAIS DO ARDUINO ---

// A função 'setup' é executada uma única vez quando a placa inicia.
void setup() {
  // Inicializa os pinos de saída.
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);      // Inicia com o LED desligado.
  digitalWrite(BUZZER_PIN, LOW);    // Inicia com o buzzer desligado.

  // Inicializa a comunicação serial para debug.
  Serial.begin(115200);
  Serial.println("\nIniciando sistema de monitoramento de vibração...");

  loadConfig();    // Carrega as configurações de rede da EEPROM.
  setup_wifi();    // Conecta-se à rede Wi-Fi.

  // Configura o cliente MQTT com as informações salvas.
  client.setServer(config.mqtt_server, config.mqtt_port);
  client.setCallback(callback); // Define a função que será chamada ao receber mensagens.
}

// A função 'loop' é executada continuamente.
void loop() {
  // Verifica se está conectado ao broker MQTT. Se não, tenta reconectar.
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Processa mensagens MQTT pendentes (envio e recebimento).

  // Leitura e publicação do sensor a cada 1 segundo.
  unsigned long now = millis();
  if (now - lastVibrationRead > 1000) {
    lastVibrationRead = now;
    
    float vibration = readVibration(); // Lê o valor da vibração.
    
    // Constrói um objeto JSON para a mensagem de vibração.
    DynamicJsonDocument doc(256);
    doc["vibration"] = vibration;
    doc["timestamp"] = now;
    
    String output;
    serializeJson(doc, output); // Converte o JSON para string.
    client.publish(topic_vibration, output.c_str()); // Publica a mensagem no tópico.
    
    Serial.printf("Vibração: %.2f m/s²\n", vibration);
  }
}