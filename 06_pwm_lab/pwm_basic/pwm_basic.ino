#define PWM_PIN 18

const int pwmFreq = 1000;
const int pwmResolution = 8;

void setup() {
  ledcAttach(PWM_PIN, pwmFreq, pwmResolution);

  // 8-bit resolution: duty range is 0 to 255
  // 128 is approximately 50% duty cycle
  ledcWrite(PWM_PIN, 128);
}

void loop() {
}
