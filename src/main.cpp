#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVO_MIN 150  // ค่าพัลส์ที่ 0 องศา
#define SERVO_MAX 600  // ค่าพัลส์ที่ 180 องศา

void setup() {
  Serial.begin(115200);

  pwm.begin();  // เริ่มการทำงานกับ PCA9685
  pwm.setPWMFreq(50);  // กำหนดความถี่ PWM สำหรับ Servo (50Hz คือความถี่ทั่วไปของ Servo)
  delay(10);
}

void loop() {
  // หมุน Servo 5 ตัวไปที่ 0 องศา
  for (int i = 0; i < 5; i++) {
    pwm.setPWM(i, 0, SERVO_MIN);  // หมุนที่ 0 องศา
    Serial.print("Servo ");
    Serial.print(i);
    Serial.println(" at 0 degrees");
  }
  delay(1000);  // หน่วงเวลา 1 วินาที

  // หมุน Servo 5 ตัวไปที่ 180 องศา
  // for (int i = 0; i < 5; i++) {
  //   pwm.setPWM(i, 0, SERVO_MAX);  // หมุนที่ 180 องศา
  //   Serial.print("Servo ");
  //   Serial.print(i);
  //   Serial.println(" at 180 degrees");
  // }
  delay(1000);  // หน่วงเวลา 1 วินาที
}

