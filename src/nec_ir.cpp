#include "nec_ir.hpp"
#include  "pins.hpp"

#include <Arduino.h>
#include <IRremote.h> // TODO: needed for digitalWriteFast; remove dependency

namespace nec_ir {
    void begin() {
        pinMode(IR_PIN, OUTPUT);
        digitalWrite(IR_PIN, LOW);
    }

    void transmit(uint8_t address, uint8_t command) {
        start();
        send_byte(address);
        send_byte(~address);
        send_byte(command);
        send_byte(~command);
        end();
    }

    void start() {
        // Leading pulse burst
        for (uint16_t i = 0; i < 333; i++) {
            digitalWriteFast(IR_PIN, HIGH);
            delayMicroseconds(10);
            digitalWriteFast(IR_PIN, LOW);
            delayMicroseconds(21);
        }

        delayMicroseconds(4500);
    }

    void mark() {
        for (uint8_t i = 0; i < 21; i++) {
            digitalWriteFast(IR_PIN, HIGH);
            delayMicroseconds(10);
            digitalWriteFast(IR_PIN, LOW);
            delayMicroseconds(21);
        }
    }

    void end() {
        mark();
    }

    void send_zero() {
        mark();
        digitalWrite(IR_PIN, LOW);
        delayMicroseconds(562);
    }

    void send_one() {
        mark();
        digitalWrite(IR_PIN, LOW);
        delayMicroseconds(1686);
    }

    void send_byte(uint8_t b) {
        for (uint8_t i = 0; i < 8; i++) {
            if (b & 1) {
                send_one();
            } else {
                send_zero();
            }
            b >>= 1;
        }
    }
}
