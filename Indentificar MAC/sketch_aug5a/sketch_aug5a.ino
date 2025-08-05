#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Obtendi endereço MAC...");
  Serial.print("MAC anddress: ");

  //Puxa o MAC do meu Dispositivo
  Serial.println(WiFi.macAddress());

}

void loop() {

}
