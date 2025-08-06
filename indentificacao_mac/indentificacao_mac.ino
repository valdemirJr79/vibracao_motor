#include <WiFi.h>

void setup() {
  Serial.begin(115200); // Define o tempo de transmissão de dados.
  Serial.println("Obtendo endereço MAC...");
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress()); // Coleta o endereço único da placa de rede (MAC).
}

void loop() {
  //
}