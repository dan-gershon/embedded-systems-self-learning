#define OUT_PIN 18
#define IN_PIN 19

void setup() {
  pinMode(OUT_PIN, OUTPUT);
  pinMode(IN_PIN, INPUT);

  Serial.begin(115200);
  Serial.println("GPIO loopback test started");
}

void loop() {
  digitalWrite(OUT_PIN, HIGH);
  delay(500);
  Serial.print("OUT = HIGH, IN = ");
  Serial.println(digitalRead(IN_PIN));

  digitalWrite(OUT_PIN, LOW);
  delay(500);
  Serial.print("OUT = LOW, IN = ");
  Serial.println(digitalRead(IN_PIN));
}
