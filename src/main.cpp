#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN 150  // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 600  // This is the 'maximum' pulse length count (out of 4096)
#define USMIN 600	  // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX 2400	  // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

// our servo # counter
uint8_t servonum = 0;

// Pin Mapping
const int S0 = 16;
const int S1 = 17;
const int S2 = 18;
const int S3 = 19;
const int SIG_PIN = 34; // Analog input pin

void selectChannel(uint8_t channel)
{
	digitalWrite(S0, channel & 0x01);
	digitalWrite(S1, (channel >> 1) & 0x01);
	digitalWrite(S2, (channel >> 2) & 0x01);
	digitalWrite(S3, (channel >> 3) & 0x01);
}

void setup()
{
	Serial.begin(115200);

	// Set up selector pins
	pinMode(S0, OUTPUT);
	pinMode(S1, OUTPUT);
	pinMode(S2, OUTPUT);
	pinMode(S3, OUTPUT);

	pwm.begin();

	pwm.setOscillatorFrequency(27000000);
	pwm.setPWMFreq(SERVO_FREQ); // Analog servos run at ~50 Hz updates

	// If SIG_PIN is digital, use pinMode too
	// pinMode(SIG_PIN, INPUT); // สำหรับ digital sensor
}

void loop()
{
	for (uint8_t i = 0; i < 4; i++)
	{
		selectChannel(i);
		delay(5); // Wait for switching to settle

		int sensorValue = analogRead(SIG_PIN); // เปลี่ยนเป็น digitalRead() หากเป็น digital sensor
		Serial.print("Channel ");
		Serial.print(i);
		Serial.print(": ");
		Serial.println(sensorValue);

		// PCA9685 PWM control
		if (sensorValue < 1000)
		{
			int pulseWidth = SERVOMIN + (SERVOMAX - SERVOMIN) / 2; // Calculate pulse width for 90 degrees
			pwm.setPWM(i, 0, pulseWidth); // Set the servo to 90 degrees
		}
		else if (sensorValue > 3000)
		{
			pwm.setPWM(i, 0, SERVOMIN); // Set the servo to 0 degrees
		}
	}

	delay(1000);
}
