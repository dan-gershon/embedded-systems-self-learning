#include <ESP32Servo.h>

const int SERVO_PIN = 18;

Servo myServo;

void setup()
{
  Serial.begin(115200);

  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2500);

  Serial.println("Servo sweep test");
}

void loop()
{
  myServo.write(0);
  Serial.println("Angle: 0");
  delay(1000);

  myServo.write(45);
  Serial.println("Angle: 45");
  delay(1000);

  myServo.write(90);
  Serial.println("Angle: 90");
  delay(1000);

  myServo.write(135);
  Serial.println("Angle: 135");
  delay(1000);

  myServo.write(180);
  Serial.println("Angle: 180");
  delay(1000);

  myServo.write(90);
  Serial.println("Angle: 90");
  delay(1000);
}
