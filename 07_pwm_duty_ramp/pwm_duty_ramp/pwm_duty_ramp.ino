#define PWM_PIN 18

const int pwmFreq = 1000;
const int pwmResolution = 8;

int dutyPercent = 0;
int direction = 1;

int dutyPercentToValue(int percent) {
  if (percent < 0) {
    percent = 0;
  }

  if (percent > 100) {
    percent = 100;
  }

  return map(percent, 0, 100, 0, 255);
}

void setup() {
  Serial.begin(115200);

  ledcAttach(PWM_PIN, pwmFreq, pwmResolution);

  Serial.println("PWM Duty Ramp Lab Started");
  Serial.println("Duty will ramp from 0% to 100% and back down.");
}

void loop() {
  ledcWrite(PWM_PIN, dutyPercentToValue(dutyPercent));

  Serial.print("Duty = ");
  Serial.print(dutyPercent);
  Serial.println("%");

  dutyPercent += direction * 5;

  if (dutyPercent >= 100) {
    dutyPercent = 100;
    direction = -1;
  }

  if (dutyPercent <= 0) {
    dutyPercent = 0;
    direction = 1;
  }

  delay(300);
}
