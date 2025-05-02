#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // คุณอาจต้องติดตั้งไลบรารีนี้
#include <Arduino.h>

const char *ssid = "THTPBN_2.4G";  // ชื่อ Wi-Fi
const char *password = "41084108"; // รหัสผ่าน Wi-Fi

String sensor = "Temperature"; // ตัวอย่างชื่อเซ็นเซอร์
String location = "Room1";     // ตัวอย่างตำแหน่ง

#define LED_BUILTIN 2 // LED บนบอร์ด ESP32

// ฟังก์ชันเพื่อสุ่มค่า
String getRandomValue()
{
    return String(random(20, 30)); // ค่าในช่วง 20 ถึง 30 องศา
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT); // กำหนดให้ LED เป็น output
    WiFi.begin(ssid, password);



    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
        digitalWrite(LED_BUILTIN, HIGH); // เปิด LED
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);  // ปิด LED
    }

    Serial.println("Connected to WiFi!");
    digitalWrite(LED_BUILTIN, HIGH); // เปิด LED

}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        // สุ่มค่า value1, value2, value3 ทุกๆ 5 วินาที
        String value1 = getRandomValue();
        String value2 = getRandomValue();
        String value3 = getRandomValue();

        // สร้าง URL สำหรับส่งข้อมูล
        String url = "http://192.168.1.123/post-esp-data.php?sensor=" + sensor + "&location=" + location + "&value1=" + value1 + "&value2=" + value2 + "&value3=" + value3;

        // เริ่มทำการร้องขอ
        http.begin(url);
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
    }
    else
    {
        Serial.println("WiFi not connected");
    }

    delay(5000); // รอ 5 วินาที แล้วส่งข้อมูลใหม่
}
