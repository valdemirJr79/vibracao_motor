#define LED_BUILTIN 2 // Define o pino do LED embutido (pode variar dependendo do modelo)
void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // Define o pino do LED como saída
}
void loop() {
  digitalWrite(LED_BUILTIN, HIGH); // Liga o LED
  delay(1000);                    // Aguarda 1 segundo
  digitalWrite(LED_BUILTIN, LOW);  // Desliga o LED
  delay(1000);                    // Aguarda 1 segundo
}
