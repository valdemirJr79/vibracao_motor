// Inclui a biblioteca para comunicação I2C, que é usada para o sensor ADXL345
#include <Wire.h> 

// Inclui a biblioteca do sensor ADXL345.
#include <ADXL345.h>

// Inclui a biblioteca para conectar o ESP32 a uma rede Wi-Fi
#include <WiFi.h>

// Inclui a biblioteca para permitir atualizações de firmware over-the-air (OTA)
#include <ArduinoOTA.h>

// --- Configurações do ADXL345 ---
// Define o endereço I2C padrão do sensor ADXL345
#define ADXL345_ADDRESS 0x53 

// Define os endereços dos registradores internos do ADXL345.
#define REG_DEVID 0x00
#define REG_DATAX0 0x32
#define REG_DATAX1 0x33
#define REG_DATAY0 0x34
#define REG_DATAY1 0x35
#define REG_DATAZ0 0x36
#define REG_DATAZ1 0x37
#define REG_POWER_CTL 0x2D
#define POWER_CTL_MEASURE_MODE 0x08

// --- Configurações do OTA ---
// Define o pino do LED embutido da sua placa ESP32 (geralmente é o pino 2)
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// Altere com as credenciais da sua rede Wi-Fi
const char *ssid = "REDEWORK";
const char *password = "Acessonet05";

// --- Funções personalizadas para o ADXL345 ---

// Função auxiliar para escrever em um registrador
void writeRegister(byte reg, byte value) {
  // Inicia a comunicação I2C com o endereço do ADXL345
  Wire.beginTransmission(ADXL345_ADDRESS);
  // Envia o endereço do registrador onde a escrita será feita
  Wire.write(reg);
  // Envia o valor a ser escrito no registrador
  Wire.write(value);
  // Finaliza a transmissão I2C
  Wire.endTransmission();
}

// Função auxiliar para ler um valor de 16 bits (2 bytes) de registradores específicos
int16_t readRegister(byte reg_low, byte reg_high) {
  // Inicia a transmissão para o endereço do ADXL345
  Wire.beginTransmission(ADXL345_ADDRESS);
  // Envia o endereço do registrador "baixo" para a leitura
  Wire.write(reg_low);
  // Finaliza a transmissão, mantendo a conexão aberta para a leitura (parâmetro 'false')
  Wire.endTransmission(false);
  // Solicita 2 bytes (16 bits) do endereço do ADXL345
  Wire.requestFrom(ADXL345_ADDRESS, 2);
  // Lê o byte de menor valor (low byte)
  byte lowByte = Wire.read();
  // Lê o byte de maior valor (high byte)
  byte highByte = Wire.read();
  // Combina os dois bytes para formar um valor de 16 bits e o retorna
  return (int16_t)((highByte << 8) | lowByte);
}

// Função para inicializar o sensor ADXL345
void initADXL345() {
  // Escreve um valor no registrador POWER_CTL para ligar o modo de medição do sensor
  writeRegister(REG_POWER_CTL, POWER_CTL_MEASURE_MODE);
}

// Função para ler os dados de aceleração nos eixos X, Y e Z
void readADXL345(int16_t *x, int16_t *y, int16_t *z) {
  // Chama a função readRegister para ler os dados do eixo X
  *x = readRegister(REG_DATAX0, REG_DATAX1);
  // Chama a função readRegister para ler os dados do eixo Y
  *y = readRegister(REG_DATAY0, REG_DATAY1);
  // Chama a função readRegister para ler os dados do eixo Z
  *z = readRegister(REG_DATAZ0, REG_DATAZ1);
}

// --- Funções de Setup e Loop ---

// A função 'setup()' é executada apenas uma vez ao iniciar o ESP32
void setup() {
  // Inicia a comunicação serial para depuração e saída de dados
  Serial.begin(115200);
  // Inicia a comunicação I2C para interagir com o sensor
  Wire.begin();
  
  // Define o pino do LED como uma saída
  pinMode(LED_BUILTIN, OUTPUT);

  // Imprime uma mensagem no monitor serial para indicar o início do processo
  Serial.println("Booting");
  // Define o modo de operação do Wi-Fi como "estação" (client), para conectar a um roteador
  WiFi.mode(WIFI_STA);
  // Inicia a tentativa de conexão com a rede Wi-Fi usando as credenciais fornecidas
  WiFi.begin(ssid, password);

  // Fica em um loop enquanto a conexão Wi-Fi não for estabelecida
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    // Se a conexão falhar, imprime uma mensagem
    Serial.println("Connection Failed! Rebooting...");
    // Espera 5 segundos antes de reiniciar o ESP32
    delay(5000);
    // Reinicia o microcontrolador
    ESP.restart();
  }

  // Inicializa o sensor ADXL345
  initADXL345();

  // --- Bloco de configuração do ArduinoOTA ---
  // Inicia a configuração do ArduinoOTA
  ArduinoOTA
    // Configura a ação a ser executada quando a atualização começar
    .onStart([]() {
      String type;
      // Verifica se o tipo de atualização é do sketch (o próprio código)
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // Caso contrário, é um "filesystem" (SPIFFS)
        type = "filesystem";
      }
      // Imprime uma mensagem informando que a atualização começou
      Serial.println("Start updating " + type);
    })
    // Configura a ação para quando a atualização terminar
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    // Configura a ação para monitorar o progresso da atualização
    .onProgress([](unsigned int progress, unsigned int total) {
      // Imprime o progresso da atualização em porcentagem
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    // Configura a ação para lidar com erros durante a atualização
    .onError([](ota_error_t error) {
      // Imprime o código do erro
      Serial.printf("Error[%u]: ", error);
      // Imprime uma mensagem específica para cada tipo de erro
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
    });

  // Inicia o serviço OTA
  ArduinoOTA.begin();

  // Imprime uma mensagem informando que o sistema está pronto
  Serial.println("Ready");
  // Imprime o endereço IP local do ESP32 na rede Wi-Fi
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// A função 'loop()' é executada repetidamente após o 'setup()'
void loop() {
  // A função ArduinoOTA.handle() deve ser chamada repetidamente para verificar
  // e gerenciar qualquer tentativa de atualização de firmware OTA
  ArduinoOTA.handle();

  // Lê os dados de aceleração dos três eixos (X, Y, Z)
  int16_t x, y, z;
  readADXL345(&x, &y, &z);

  // Exibe os dados no monitor serial
  Serial.print("X: ");
  Serial.print(x);
  Serial.print("  Y: ");
  Serial.print(y);
  Serial.print("  Z: ");
  Serial.println(z);
  
  // Pisca o LED embutido para indicar que o loop está ativo
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(100);
  digitalWrite(LED_BUILTIN, LOW); 
  delay(900); // O tempo de espera total agora é de 100ms + 900ms = 1s
}