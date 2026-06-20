#define PWM_PIN 18

int pwmFreq = 1000;
const int pwmResolution = 8;
int dutyPercent = 50;

int dutyPercentToValue(int percent) {
  if (percent < 0) {
    percent = 0;
  }

  if (percent > 100) {
    percent = 100;
  }

  return map(percent, 0, 100, 0, 255);
}

void applyPwm() {
  ledcAttach(PWM_PIN, pwmFreq, pwmResolution);
  ledcWrite(PWM_PIN, dutyPercentToValue(dutyPercent));

  Serial.print("PWM updated: freq = ");
  Serial.print(pwmFreq);
  Serial.print(" Hz, duty = ");
  Serial.print(dutyPercent);
  Serial.println("%");
}

void setup() {
  Serial.begin(115200);

  applyPwm();

  Serial.println("PWM Serial Control Lab");
  Serial.println("Commands:");
  Serial.println("duty 10");
  Serial.println("duty 50");
  Serial.println("freq 1000");
  Serial.println("freq 5000");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.startsWith("duty ")) {
      dutyPercent = command.substring(5).toInt();
      applyPwm();
    }

    else if (command.startsWith("freq ")) {
      pwmFreq = command.substring(5).toInt();

      if (pwmFreq < 1) {
        pwmFreq = 1;
      }

      applyPwm();
    }

    else if (command == "status") {
      Serial.print("Current PWM: freq = ");
      Serial.print(pwmFreq);
      Serial.print(" Hz, duty = ");
      Serial.print(dutyPercent);
      Serial.println("%");
    }

    else {
      Serial.println("Unknown command");
      Serial.println("Use: duty 0-100, freq number, status");
    }
  }
}
