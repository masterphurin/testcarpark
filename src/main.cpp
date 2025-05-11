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
const char *url = "http://192.168.1.140/getData.php";

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
	HTTPClient http;

	int value = fetchDataOnce(); // ดึงค่าช่องจากเซิร์ฟเวอร์

	if (value < 0 || value > 15) {
		Serial.println("Invalid slot_number received.");
		delay(1000);
		return;
	}

	Serial.print("Selected Channel: ");
	Serial.println(value);

	selectChannel(value); // เลือกช่องของมัลติเพล็กเซอร์
	delay(5);			  // รอให้มั่นใจว่าสวิตช์ทำงานแล้ว

	int sensorValue = analogRead(SIG_PIN); // อ่านค่าจากเซ็นเซอร์
	Serial.print("Sensor value: ");
	Serial.println(sensorValue);

	// คำนวณค่า pulseWidth สำหรับ 90 องศา
	int pulseWidth = SERVOMIN + (SERVOMAX - SERVOMIN) / 2;

	// ถ้าเป็นช่อง 4-9 ให้กลับทิศทางการหมุน
	if (value >= 4 && value <= 9) {
		pulseWidth = SERVOMAX - (pulseWidth - SERVOMIN);
	}

	// แสดงค่าพัลส์ที่ตั้งค่าไป
	Serial.print("Pulse Width: ");
	Serial.println(pulseWidth);

	// สั่งให้เซอร์โวหมุนไปตำแหน่งที่ต้องการ
	pwm.setPWM(value, 0, pulseWidth);		

	// ส่งค่ากลับไปยังเซิร์ฟเวอร์ (HTTP GET)
	String urls = String("http://192.168.1.140/setData.php?slot_number=") + String(value);
	Serial.println("URL: " + urls);
	http.begin(urls);
	int httpCode = http.GET(); // ส่ง HTTP GET
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

	delay(500);

	// ตอนนี้กลับเซอร์โวไปที่ตำแหน่งเริ่มต้น:
	// ถ้าช่อง 4-9 จะหมุนไปที่ 180 องศา (SERVOMAX), ถ้าช่อง 0-3 จะหมุนไปที่ 0 องศา (SERVOMIN)
	int resetPulse = SERVOMIN;
	// ถ้าเป็นช่อง 4-9 ให้หมุนไปทางกวาดขึ้นด้านขวา
	if (value >= 4 && value <= 9) {
		resetPulse = SERVOMAX; // หมุนขึ้นด้านขวา
	} else {
		resetPulse = SERVOMIN; // หมุนลงไปที่เดิม
	}
	pwm.setPWM(value, 0, resetPulse); // สั่งให้เซอร์โวหมุนไปตำแหน่งรีเซ็ต

	// แสดงค่าพัลส์ที่ใช้ในการรีเซ็ต
	Serial.print("Reset Pulse Width: ");
	Serial.println(resetPulse);

	delay(1000); // รอก่อนเริ่มรอบถัดไป
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
				if (jsonDoc["slot_number"].is<int>())
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