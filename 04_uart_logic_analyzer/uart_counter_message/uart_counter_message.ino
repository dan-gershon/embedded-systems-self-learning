int counter = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.print("Hello UART | counter = ");
  Serial.println(counter);

  counter++;

  delay(1000);
}
