#include <WiFi.h>            // Biblioteca para conexão WiFi (ESP32/ESP8266)
#include <ArduinoOTA.h>      // Biblioteca para atualizações OTA (Over-the-Air)
#include <Wire.h>            // Biblioteca para comunicação I2C
#include <ADXL345.h>         // Biblioteca para sensor acelerômetro ADXL345 (não usada diretamente no código, mas incluída)

// Credenciais da rede WiFi
const char *ssid = "REDEWORK";         // Nome da rede WiFi
const char *password = "Acessonet05";  // Senha da rede WiFi

// Definições para o ADXL345 - endereço e registradores usados no sensor
#define ADXL345_ADDRESS 0x53             // Endereço I2C padrão do ADXL345

// Registradores do ADXL345 usados para leitura e configuração
#define REG_DEVID   0x00                 // Registrador ID do dispositivo (não usado diretamente)
#define REG_DATAX0  0x32                 // Registrador baixa ordem do eixo X
#define REG_DATAX1  0x33                 // Registrador alta ordem do eixo X
#define REG_DATAY0  0x34                 // Registrador baixa ordem do eixo Y
#define REG_DATAY1  0x35                 // Registrador alta ordem do eixo Y
#define REG_DATAZ0  0x36                 // Registrador baixa ordem do eixo Z
#define REG_DATAZ1  0x37                 // Registrador alta ordem do eixo Z
#define REG_POWER_CTL 0x2D               // Registrador de controle de energia
#define POWER_CTL_MEASURE_MODE 0x08     // Valor para ativar modo de medição (ligar sensor)


// Função setup, executada uma vez na inicialização do dispositivo
void setup() {
  Serial.begin(115200);                // Inicializa comunicação serial a 115200 baud
  Serial.println("Iniciando...");     // Imprime mensagem inicial no monitor serial

  // Configura o modo WiFi para estação (client), ou seja, conecta a uma rede WiFi
  WiFi.mode(WIFI_STA);

  // Inicia a conexão WiFi com SSID e senha definidos
  WiFi.begin(ssid, password);

  // Aguarda até conectar ao WiFi (loop até conectar)
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Falha na conexão! Reiniciando...");  // Caso não conecte, avisa
    delay(5000);                                          // Espera 5 segundos
    ESP.restart();                                        // Reinicia o microcontrolador
  }

  // Configuração do OTA (atualização por rede)
  ArduinoOTA
    .onStart([]() {             // Função chamada quando começa atualização OTA
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {  // Verifica se está atualizando código (sketch)
        type = "sketch";
      } else {                                    // Ou sistema de arquivos
        type = "filesystem";
      }
      Serial.println("Iniciando atualização: " + type);  // Imprime mensagem de início
    })
    .onEnd([]() {               // Função chamada no fim da atualização OTA
      Serial.println("\nAtualização concluída");
    })
    .onProgress([](unsigned int progress, unsigned int total) {  // Função que mostra progresso
      Serial.printf("Progresso: %u%%\r", (progress * 100) / total);  // Imprime percentual
    })
    .onError([](ota_error_t error) {  // Função chamada em caso de erro durante OTA
      Serial.printf("Erro[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Falha na autenticação");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Erro no início");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Erro de conexão");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Erro de recepção");
      else if (error == OTA_END_ERROR) Serial.println("Erro no final");
    });

  ArduinoOTA.begin();          // Inicializa o serviço OTA
  Serial.println("OTA pronto");   // Mensagem que OTA está pronto para uso
  Serial.print("Endereço IP: ");  
  Serial.println(WiFi.localIP());  // Mostra endereço IP obtido na rede WiFi

  // Inicializa a comunicação I2C (Wire) para o sensor ADXL345
  Wire.begin();

  // Inicializa o sensor ADXL345 configurando-o para modo de medição
  initADXL345();
}

// Função loop executada repetidamente após setup
void loop() {
  ArduinoOTA.handle();     // Escuta e trata requisições OTA para atualizações do firmware

  // Declara variáveis para armazenar os valores lidos dos eixos X, Y e Z
  int16_t x, y, z;

  // Lê os valores dos eixos do acelerômetro ADXL345 e armazena em x, y e z
  readADXL345(&x, &y, &z);

  // Imprime no monitor serial os valores lidos do sensor
  Serial.print("X: ");
  Serial.print(x);
  Serial.print("  Y: ");
  Serial.print(y);
  Serial.print("  Z: ");
  Serial.println(z);

  delay(100);   // Aguarda 100 milissegundos antes de próxima leitura
}


// Função que inicializa o ADXL345 para modo de medição
void initADXL345() {
  writeRegister(REG_POWER_CTL, POWER_CTL_MEASURE_MODE);  // Escreve no registrador de controle de energia para ligar sensor
}

// Função para ler os dados do ADXL345
// Recebe ponteiros para onde armazenar os valores lidos
void readADXL345(int16_t *x, int16_t *y, int16_t *z) {
  *x = readRegister(REG_DATAX0, REG_DATAX1);  // Lê dados do eixo X (dois registradores)
  *y = readRegister(REG_DATAY0, REG_DATAY1);  // Lê dados do eixo Y (dois registradores)
  *z = readRegister(REG_DATAZ0, REG_DATAZ1);  // Lê dados do eixo Z (dois registradores)
}

// Função auxiliar para ler um valor de 16 bits (2 bytes) a partir de dois registradores do sensor
int16_t readRegister(byte reg_low, byte reg_high) {
  Wire.beginTransmission(ADXL345_ADDRESS);  // Inicia comunicação I2C com o sensor no endereço especificado
  Wire.write(reg_low);                       // Envia o registrador de menor byte para leitura
  Wire.endTransmission(false);               // Finaliza transmissão, mas mantém conexão aberta
  Wire.requestFrom(ADXL345_ADDRESS, 2);     // Solicita 2 bytes de dados do sensor
  byte lowByte = Wire.read();                // Lê byte menos significativo
  byte highByte = Wire.read();               // Lê byte mais significativo
  return (int16_t)((highByte << 8) | lowByte);  // Junta os dois bytes em um inteiro de 16 bits e retorna
}

// Função auxiliar para escrever um valor em um registrador do sensor ADXL345
void writeRegister(byte reg, byte value) {
  Wire.beginTransmission(ADXL345_ADDRESS);  // Inicia comunicação I2C com o sensor
  Wire.write(reg);                           // Escreve o endereço do registrador
  Wire.write(value);                         // Escreve o valor a ser gravado no registrador
  Wire.endTransmission();                    // Finaliza a transmissão I2C
}
