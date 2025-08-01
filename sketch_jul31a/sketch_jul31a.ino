#define LED_BUILTIN 2

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // configura o pino 2 como saída
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH); // acende o LED
  delay(1000);                     // espera 1 segundo
  digitalWrite(LED_BUILTIN, LOW);  // apaga o LED
  delay(1000);                     // espera 1 segundo
}