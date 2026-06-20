#define ADC_PIN 34

void setup() {
  Serial.begin(115200);
}

void loop() {
  int adcRaw = analogRead(ADC_PIN);

  Serial.println(adcRaw);

  delay(5);
}
