#include <Arduino.h>

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
  }

  delay(1000);
}
