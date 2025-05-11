#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define LED_BUILTIN 2 // LED บนบอร์ด ESP32

// WiFi credentials
const char* ssid = "THTPBN_2.4G";
const char* password = "41084108";

// URL ของเซิร์ฟเวอร์
const char* url = "http://192.168.1.140/getData.php";

int fetchDataOnce(); // ประกาศฟังก์ชัน

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT); // กำหนดให้ LED เป็น output
    // เชื่อมต่อ WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(LED_BUILTIN, HIGH); // เปิด LED
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);  // ปิด LED
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    digitalWrite(LED_BUILTIN, HIGH); // เปิด LED
}

void loop() {
    static unsigned long lastMillis = 0;
    if (millis() - lastMillis >= 3000) {
        lastMillis = millis();

        int value = fetchDataOnce(); // เรียกใช้ฟังก์ชันสุ่มค่า
        if (value != -1) {
            Serial.print("Fetched value: ");
            Serial.println(value);
        } else {
            Serial.println("Failed to fetch data");
        }

    //     if (WiFi.status() == WL_CONNECTED) {
    //         HTTPClient http;
    //         http.begin(url);

    //         int httpResponseCode = http.GET();
    //         if (httpResponseCode > 0) {
    //             String payload = http.getString();
    //             Serial.println("HTTP Response:");
    //             Serial.println(payload);

    //             // แปลง JSON
    //             StaticJsonDocument<1024> jsonDoc;
    //             DeserializationError error = deserializeJson(jsonDoc, payload);
    //             if (!error) {
    //                 Serial.println("Parsed JSON:");
    //                 serializeJsonPretty(jsonDoc, Serial);
    //             } else {
    //                 Serial.print("Failed to parse JSON: ");
    //                 Serial.println(error.c_str());
    //             }
    //         } else {
    //             Serial.print("HTTP GET request failed: ");
    //             Serial.println(httpResponseCode);
    //         }

    //         http.end();
    //     } else {
    //         Serial.println("WiFi not connected");
    //     }
    }
    // ไม่ต้องทำอะไรใน loop
}

int fetchDataOnce() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(url);

        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            String payload = http.getString();
            // Serial.println("HTTP Response:");
            // Serial.println(payload);

            // แปลง JSON
            StaticJsonDocument<1024> jsonDoc;
            DeserializationError error = deserializeJson(jsonDoc, payload);
            if (!error) {
                // Serial.println("Parsed JSON:");
                // serializeJsonPretty(jsonDoc, Serial);

                // สมมติว่าเราต้องการดึงค่าจาก key "value"
                if (jsonDoc.containsKey("slot_number")) {
                    int value = jsonDoc["slot_number"];
                    http.end();
                    return value; // คืนค่าที่อ่านได้
                } else {
                    Serial.println("Key 'slot_number' not found in JSON");
                }
            } else {
                Serial.print("Failed to parse JSON: ");
                Serial.println(error.c_str());
            }
        } else {
            Serial.print("HTTP GET request failed: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("WiFi not connected");
    }

    return -1; // คืนค่า -1 หากเกิดข้อผิดพลาด
}