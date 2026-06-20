#define RX2_PIN 16
#define TX2_PIN 17

int counter = 0;

bool parsePacket(String packet, int &rxCounter, int &rxValue, int &rxChecksum) {
  packet.trim();

  if (!packet.startsWith("<")) {
    return false;
  }

  if (!packet.endsWith(">")) {
    return false;
  }

  packet.remove(0, 1);                 // remove <
  packet.remove(packet.length() - 1);  // remove >

  int firstComma = packet.indexOf(',');
  int secondComma = packet.indexOf(',', firstComma + 1);

  if (firstComma == -1 || secondComma == -1) {
    return false;
  }

  String counterStr = packet.substring(0, firstComma);
  String valueStr = packet.substring(firstComma + 1, secondComma);
  String checksumStr = packet.substring(secondComma + 1);

  rxCounter = counterStr.toInt();
  rxValue = valueStr.toInt();
  rxChecksum = checksumStr.toInt();

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

  Serial.println("UART2 packet parser lab started");
  Serial.println("Format: <counter,value,checksum>");
}

void loop() {
  int value = counter * 3;
  int checksum = counter + value;

  Serial2.print("<");
  Serial2.print(counter);
  Serial2.print(",");
  Serial2.print(value);
  Serial2.print(",");
  Serial2.print(checksum);
  Serial2.println(">");

  Serial.print("Sent packet: <");
  Serial.print(counter);
  Serial.print(",");
  Serial.print(value);
  Serial.print(",");
  Serial.print(checksum);
  Serial.println(">");

  delay(50);

  while (Serial2.available() > 0) {
    String packet = Serial2.readStringUntil('\n');

    int rxCounter = 0;
    int rxValue = 0;
    int rxChecksum = 0;

    bool parsed = parsePacket(packet, rxCounter, rxValue, rxChecksum);

    if (!parsed) {
      Serial.print("Bad packet format: ");
      Serial.println(packet);
    } else {
      int calculatedChecksum = rxCounter + rxValue;

      Serial.print("Received: counter=");
      Serial.print(rxCounter);
      Serial.print(", value=");
      Serial.print(rxValue);
      Serial.print(", checksum=");
      Serial.print(rxChecksum);

      if (calculatedChecksum == rxChecksum) {
        Serial.println(" -> Packet OK");
      } else {
        Serial.print(" -> Checksum ERROR, calculated=");
        Serial.println(calculatedChecksum);
      }
    }
  }

  counter++;
  delay(1000);
}
