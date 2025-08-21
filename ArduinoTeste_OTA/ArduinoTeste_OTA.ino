#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <ADXL345.h>


const char *ssid = "REDEWORK";
const char *password = "Acessonet05";

// Definições para o ADXL345
#define ADXL345_ADDRESS 0x53 // Endereço I2C do ADXL345
// Definições dos registradores do ADXL345

#define REG_DEVID   0x00
#define REG_DATAX0  0x32
#define REG_DATAX1  0x33
#define REG_DATAY0  0x34
#define REG_DATAY1  0x35
#define REG_DATAZ0  0x36
#define REG_DATAZ1  0x37
#define REG_POWER_CTL 0x2D
#define POWER_CTL_MEASURE_MODE 0x08


void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");

  // Conecta ao WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Falha na conexão! Reiniciando...");
    delay(5000);
    ESP.restart();
  }

  // Inicializa OTA
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {
        type = "filesystem";
      }
      Serial.println("Iniciando atualização: " + type);
    })
    .onEnd([]() {
      Serial.println("\nAtualização concluída");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progresso: %u%%\r", (progress * 100) / total);
    })
    .onError([](ota_error_t error) {
      Serial.printf("Erro[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Falha na autenticação");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Erro no início");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Erro de conexão");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Erro de recepção");
      else if (error == OTA_END_ERROR) Serial.println("Erro no final");
    });

  ArduinoOTA.begin();
  Serial.println("OTA pronto");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Inicializa I2C e o ADXL345
  Wire.begin();
  initADXL345();
}


void loop() {
  ArduinoOTA.handle();

  // Lê dados do ADXL345
  int16_t x, y, z;
  readADXL345(&x, &y, &z);

  // Exibe dados no Serial
  Serial.print("X: ");
  Serial.print(x);
  Serial.print("  Y: ");
  Serial.print(y);
  Serial.print("  Z: ");
  Serial.println(z);

  delay(100);
}


// Função para inicializar o ADXL345
void initADXL345() {
  // Define o modo de medida
  writeRegister(REG_POWER_CTL, POWER_CTL_MEASURE_MODE);
}

// Função para ler os dados do ADXL345
void readADXL345(int16_t *x, int16_t *y, int16_t *z) {
  *x = readRegister(REG_DATAX0, REG_DATAX1);
  *y = readRegister(REG_DATAY0, REG_DATAY1);
  *z = readRegister(REG_DATAZ0, REG_DATAZ1);
}

// Função auxiliar para ler um valor de 16 bits (2 bytes) de registradores específicos
int16_t readRegister(byte reg_low, byte reg_high) {
  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(reg_low);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345_ADDRESS, 2);
  byte lowByte = Wire.read();
  byte highByte = Wire.read();
  return (int16_t)((highByte << 8) | lowByte);
}

// Função auxiliar para escrever em um registrador
void writeRegister(byte reg, byte value) {
  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}