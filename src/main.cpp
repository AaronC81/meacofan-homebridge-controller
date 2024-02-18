#include <Arduino.h>

void setup() {
    pinMode(2, OUTPUT);
}

void loop() {
    while (1) {
        digitalWrite(2, HIGH);
        delay(1000);

        digitalWrite(2, LOW);
        delay(1000);
    }
}
