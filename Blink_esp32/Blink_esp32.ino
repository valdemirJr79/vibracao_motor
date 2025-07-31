//Escrever um código para fazer um LED piscar em um ESP32 usando a IDE do Arduino. 
//O código deve definir o pino do LED como saída e, em seguida, alterna entre os estados HIGH (ligado) e LOW (desligado) 
//com um atraso de tempo. 

#define LED_BUILTIN 2

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}