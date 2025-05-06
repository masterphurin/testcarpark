#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Libraries for WiFi and HTTP requests
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define LED_BUILTIN 2 // LED บนบอร์ด ESP32

// WiFi credentials
const char *ssid = "THTPBN_2.4G";
const char *password = "41084108";

// URL ของเซิร์ฟเวอร์
const char *url = "http://192.168.1.144/getData.php";

int fetchDataOnce(); // ประกาศฟังก์ชัน

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
	pinMode(LED_BUILTIN, OUTPUT); // กำหนดให้ LED เป็น output

	pwm.begin();

	pwm.setOscillatorFrequency(27000000);
	pwm.setPWMFreq(SERVO_FREQ); // Analog servos run at ~50 Hz updates

	// เชื่อมต่อ WiFi
	WiFi.begin(ssid, password);
	Serial.print("Connecting to WiFi");
	while (WiFi.status() != WL_CONNECTED)
	{
		digitalWrite(LED_BUILTIN, HIGH); // เปิด LED
		delay(500);
		digitalWrite(LED_BUILTIN, LOW); // ปิด LED
		delay(500);
		Serial.print(".");
	}
	Serial.println("\nConnected to WiFi");
	digitalWrite(LED_BUILTIN, HIGH); // เปิด LED

	// If SIG_PIN is digital, use pinMode too
	// pinMode(SIG_PIN, INPUT); // สำหรับ digital sensor
}

void loop()
{
	// for (uint8_t i = 0; i < 4; i++)
	// {
	// 	selectChannel(i);
	// 	delay(5); // Wait for switching to settle

	// 	int sensorValue = analogRead(SIG_PIN); // เปลี่ยนเป็น digitalRead() หากเป็น digital sensor
	// 	Serial.print("Channel ");
	// 	Serial.print(i);
	// 	Serial.print(": ");
	// 	Serial.println(sensorValue);

	// 	// PCA9685 PWM control
	// 	if (sensorValue < 1000)
	// 	{
	// 		int pulseWidth = SERVOMIN + (SERVOMAX - SERVOMIN) / 2; // Calculate pulse width for 90 degrees
	// 		pwm.setPWM(i, 0, pulseWidth);						   // Set the servo to 90 degrees
	// 	}
	// 	else if (sensorValue > 3000)
	// 	{
	// 		pwm.setPWM(i, 0, SERVOMIN); // Set the servo to 0 degrees
	// 	}
	// }

	HTTPClient http;

	int value = fetchDataOnce(); // เรียกใช้ฟังก์ชันดึงค่า

	selectChannel(value); // เลือกช่องที่ได้จากฟังก์ชัน
	delay(5);			  // Wait for switching to settle

	int sensorValue = analogRead(SIG_PIN); // เปลี่ยนเป็น digitalRead() หากเป็น digital sensor
	Serial.print("Channel ");
	Serial.print(value);
	Serial.print(": ");
	Serial.println(sensorValue);

	// PCA9685 PWM control
	int pulseWidth = SERVOMIN + (SERVOMAX - SERVOMIN) / 2; // Calculate pulse width for 90 degrees
	pwm.setPWM(value, 0, pulseWidth);		
	
	String urls = String("http://192.168.1.144/setData.php?slot_number=") + String(value);
	Serial.println("URL: " + urls);
	http.begin(urls);
	int httpCode = http.GET(); // หรือสามารถใช้ POST ถ้าต้องการ
	if (httpCode > 0)
	{
		String payload = http.getString();
		Serial.println("HTTP Response code: " + String(httpCode));
		Serial.println("Response: " + payload);
	}
	else
	{
		Serial.println("Error on HTTP request");
	}
	http.end();
	// Set the servo to 90 degrees

	// delay(1000); // Delay for 1 second before the next loop iteration

	delay(500);

	pwm.setPWM(value, 0, SERVOMIN); // Set the servo to 0 degrees
	delay(1000); // Delay for 1 second before the next loop iteration
}

int fetchDataOnce()
{
	if (WiFi.status() == WL_CONNECTED)
	{
		HTTPClient http;
		http.begin(url);

		int httpResponseCode = http.GET();
		if (httpResponseCode > 0)
		{
			String payload = http.getString();
			// Serial.println("HTTP Response:");
			// Serial.println(payload);

			// แปลง JSON
			StaticJsonDocument<1024> jsonDoc;
			DeserializationError error = deserializeJson(jsonDoc, payload);
			if (!error)
			{
				// Serial.println("Parsed JSON:");
				// serializeJsonPretty(jsonDoc, Serial);

				// สมมติว่าเราต้องการดึงค่าจาก key "value"
				if (jsonDoc.containsKey("slot_number"))
				{
					int value = jsonDoc["slot_number"];
					http.end();
					return value; // คืนค่าที่อ่านได้
				}
				else
				{
					Serial.println("Key 'slot_number' not found in JSON");
				}
			}
			else
			{
				Serial.print("Failed to parse JSON: ");
				Serial.println(error.c_str());
			}
		}
		else
		{
			Serial.print("HTTP GET request failed: ");
			Serial.println(httpResponseCode);
		}

		http.end();
	}
	else
	{
		Serial.println("WiFi not connected");
	}

	return -1; // คืนค่า -1 หากเกิดข้อผิดพลาด
}