/*
  Blink para ESP32

  Pisca o LED interno do ESP32 (conectado ao pino GPIO2 na maioria das placas)
  por um segundo ligado e um segundo desligado, repetidamente.

  Nota: Ao contrário do Arduino, no ESP32 o LED_BUILTIN normalmente não está
  definido por padrão. Vamos usar o pino GPIO2 que é onde o LED está conectado
  na maioria das placas ESP32 (como ESP32 DevKit).

  Criado para o ESP32
*/

#define LED_BUILTIN 2  // Define o pino do LED para GPIO2 (comum em ESP32 DevKit)

// a função setup roda uma vez quando você liga ou reseta a placa
void setup() {
  // inicializa o pino digital LED_BUILTIN como saída
  pinMode(LED_BUILTIN, OUTPUT);
}

// a função loop roda repetidamente para sempre
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // liga o LED (HIGH é o nível de tensão)
  delay(1000);                      // espera um segundo
  digitalWrite(LED_BUILTIN, LOW);   // desliga o LED (LOW é o nível de tensão)
  delay(1000);                      // espera um segundo
}