#define LED_PIN 2

bool blinkMode = false;
bool ledState = false;

unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 500;

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println("ESP32 ready");
  Serial.println("Commands: on, off, blink");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "on") {
      blinkMode = false;
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED is ON");
    }

    else if (command == "off") {
      blinkMode = false;
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED is OFF");
    }

    else if (command == "blink") {
      blinkMode = true;
      Serial.println("LED is BLINKING");
    }

    else {
      Serial.println("Unknown command");
    }
  }

  if (blinkMode == true) {
    unsigned long currentTime = millis();

    if (currentTime - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = currentTime;

      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  }
}
