#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 baseline ready");
}

void loop() {
    while (Serial.available() > 0) {
        Serial.write(static_cast<uint8_t>(Serial.read()));
    }
    delay(1);
}
