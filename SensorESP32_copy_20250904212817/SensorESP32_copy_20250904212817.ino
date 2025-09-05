#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

void setup() {
  Serial.begin(115200);

  // Inicializa I2C nos pinos do ESP32 (SDA = 21, SCL = 22)
  Wire.begin(21, 22);

  Serial.println("Iniciando MPU6050...");
  mpu.initialize();

  // Testa conexão
  if (mpu.testConnection()) {
    Serial.println("MPU6050 conectado com sucesso!");
  } else {
    Serial.println("Erro: MPU6050 não encontrado.");
  }
}

void loop() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  // Lê dados crus de aceleração e giroscópio
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Mostra no Serial Monitor
  Serial.print("Acelerômetro [X,Y,Z]: ");
  Serial.print(ax); Serial.print(" | ");
  Serial.print(ay); Serial.print(" | ");
  Serial.println(az);

  Serial.print("Giroscópio [X,Y,Z]: ");
  Serial.print(gx); Serial.print(" | ");
  Serial.print(gy); Serial.print(" | ");
  Serial.println(gz);

  Serial.println("---------------------------");
  delay(500);
}