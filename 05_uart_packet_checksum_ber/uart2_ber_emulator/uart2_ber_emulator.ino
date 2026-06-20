#define RX2_PIN 16
#define TX2_PIN 17

const int BER_PPM = 10000;
// 10000 ppm = 1% bit flip probability
// 1000 ppm  = 0.1%
// 100 ppm   = 0.01%

uint8_t seqNum = 0;

unsigned long totalBits = 0;
unsigned long bitErrors = 0;
unsigned long totalPackets = 0;
unsigned long badPackets = 0;
unsigned long checksumErrors = 0;

uint8_t checksum8(uint8_t *data, int len) {
  uint8_t sum = 0;

  for (int i = 0; i < len; i++) {
    sum += data[i];
  }

  return sum;
}

uint8_t corruptByte(uint8_t b) {
  for (int bit = 0; bit < 8; bit++) {
    long r = random(1000000);

    if (r < BER_PPM) {
      b ^= (1 << bit);
    }
  }

  return b;
}

int countBitErrors(uint8_t a, uint8_t b) {
  uint8_t x = a ^ b;
  int count = 0;

  for (int i = 0; i < 8; i++) {
    if (x & (1 << i)) {
      count++;
    }
  }

  return count;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

  randomSeed(esp_random());

  Serial.println("UART noisy channel BER lab started");
  Serial.println("Frame: < seq d0 d1 d2 d3 checksum >");
  Serial.print("BER target ppm = ");
  Serial.println(BER_PPM);
  Serial.println();
}

void loop() {
  uint8_t original[8];
  uint8_t noisy[8];

  original[0] = '<';
  original[1] = seqNum;
  original[2] = seqNum + 1;
  original[3] = seqNum + 2;
  original[4] = seqNum + 3;
  original[5] = seqNum + 4;

  uint8_t payloadForChecksum[5] = {
    original[1],
    original[2],
    original[3],
    original[4],
    original[5]
  };

  original[6] = checksum8(payloadForChecksum, 5);
  original[7] = '>';

  for (int i = 0; i < 8; i++) {
    noisy[i] = original[i];
  }

  // Do not corrupt start and end markers, only internal frame bytes
  for (int i = 1; i <= 6; i++) {
    noisy[i] = corruptByte(noisy[i]);
  }

  Serial2.write(noisy, 8);

  delay(20);

  if (Serial2.available() >= 8) {
    uint8_t received[8];

    for (int i = 0; i < 8; i++) {
      received[i] = Serial2.read();
    }

    totalPackets++;

    bool frameFormatOK = (received[0] == '<') && (received[7] == '>');

    int packetBitErrors = 0;

    for (int i = 1; i <= 6; i++) {
      int e = countBitErrors(original[i], received[i]);
      packetBitErrors += e;
      bitErrors += e;
      totalBits += 8;
    }

    uint8_t rxPayloadForChecksum[5] = {
      received[1],
      received[2],
      received[3],
      received[4],
      received[5]
    };

    uint8_t calculatedChecksum = checksum8(rxPayloadForChecksum, 5);
    bool checksumOK = (calculatedChecksum == received[6]);

    if (!frameFormatOK || packetBitErrors > 0) {
      badPackets++;
    }

    if (!checksumOK) {
      checksumErrors++;
    }

    double measuredBER = 0.0;

    if (totalBits > 0) {
      measuredBER = (double)bitErrors / (double)totalBits;
    }

    Serial.print("SEQ=");
    Serial.print(seqNum);

    Serial.print(" | bit errors in packet=");
    Serial.print(packetBitErrors);

    Serial.print(" | checksum=");
    if (checksumOK) {
      Serial.print("OK");
    } else {
      Serial.print("ERROR");
    }

    Serial.print(" | total BER=");
    Serial.print(measuredBER, 6);

    Serial.print(" | packets=");
    Serial.print(totalPackets);

    Serial.print(" | bad packets=");
    Serial.print(badPackets);

    Serial.print(" | checksum errors=");
    Serial.println(checksumErrors);
  }

  seqNum++;
  delay(300);
}
