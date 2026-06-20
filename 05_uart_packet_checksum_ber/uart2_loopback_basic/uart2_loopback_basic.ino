#define RX2_PIN 16
#define TX2_PIN 17

int counter = 0;

void setup() {
  Serial.begin(115200);

  Serial2.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

  Serial.println("UART2 loopback lab started");
  Serial.println("TX2 = GPIO17, RX2 = GPIO16");
}

void loop() {
  Serial2.print("MSG ");
  Serial2.println(counter);

  Serial.print("Sent: MSG ");
  Serial.println(counter);

  delay(50);

  while (Serial2.available() > 0) {
    String received = Serial2.readStringUntil('\n');
    received.trim();

    Serial.print("Received: ");
    Serial.println(received);
  }

  counter++;
  delay(1000);
}
