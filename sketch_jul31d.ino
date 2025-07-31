#define LED_BUILTIN 2 // Define o pino onde o LED está conectado (geralmente o pino 2 para ESP32 ou 13 para Arduino UNO)

void setup() { 
  pinMode(LED_BUILTIN, OUTPUT); 
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000); 
  digitalWrite(LED_BUILTIN, LOW); 
  delay(1000); 
}