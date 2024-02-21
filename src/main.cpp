#include <Arduino.h>
#include <IRremote.h>

#define IR_PIN 3

void setup() {
    pinMode(IR_PIN, OUTPUT);
    digitalWrite(IR_PIN, LOW);
}

void necTransmit(uint8_t address, uint8_t command);

void loop() {
    while (1) {
        necTransmit(0x80, 0x92);
        delay(2000);
    }
} 

// Reference: https://techdocs.altium.com/display/FPGA/NEC+Infrared+Transmission+Protocol
void necStart() {
    // Leading pulse burst
    for (uint16_t i = 0; i < 333; i++) {
        digitalWriteFast(IR_PIN, HIGH);
        delayMicroseconds(10);
        digitalWriteFast(IR_PIN, LOW);
        delayMicroseconds(21);
    }

    delayMicroseconds(4500);
}

void necMark() {
    for (uint8_t i = 0; i < 21; i++) {
        digitalWriteFast(IR_PIN, HIGH);
        delayMicroseconds(10);
        digitalWriteFast(IR_PIN, LOW);
        delayMicroseconds(21);
    }
}

void necEnd() {
    necMark();
}

inline void necSendZero() {
    necMark();
    digitalWrite(IR_PIN, LOW);
    delayMicroseconds(562);
}

inline void necSendOne() {
    necMark();
    digitalWrite(IR_PIN, LOW);
    delayMicroseconds(1686);
}

void necSendByte(uint8_t b) {
    for (uint8_t i = 0; i < 8; i++) {
        if (b & 1) {
            necSendOne();
        } else {
            necSendZero();
        }
        b >>= 1;
    }
}

void necTransmit(uint8_t address, uint8_t command) {
    necStart();
    necSendByte(address);
    necSendByte(~address);
    necSendByte(command);
    necSendByte(~command);
    necEnd();
}
