int pinoDigital = 14; // D5 no NodeMCU
int pinoAnalogico = A0;

void setup() {
  Serial.begin(115200);
  pinMode(pinoDigital, INPUT);
}

void loop() {
  int valorDigital = digitalRead(pinoDigital);
  int valorAnalogico = analogRead(pinoAnalogico);

  Serial.print("Digital: ");
  Serial.print(valorDigital);
  Serial.print(" | Analogico: ");
  Serial.println(valorAnalogico);

  delay(1000);
}
