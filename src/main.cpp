#include <Arduino.h>

#include "nec_ir.hpp"
#include "pins.hpp"
#include "fan.hpp"

#include <digitalWriteFast.h>

Fan fan;

void setup() {
    nec_ir::begin();
    
    pinMode(IR_PIN, OUTPUT);

    pinMode(I2C_SCL_PIN, INPUT_PULLUP);
    pinMode(I2C_SDA_PIN, INPUT_PULLUP);
    pinMode(I2C_ACTIVITY_PIN, OUTPUT);
}

void loop() {
    // If SCL goes low, we're going to receive a transmission
    // TODO: set up a timer to get us back to a nice state if things go awry
    if (digitalReadFast(I2C_SCL_PIN) == 0) {
        // Read address
        uint8_t addressAndMode = 0;
        uint8_t i = 7;
        while (1) {
            // Wait for clock high
            while (digitalReadFast(I2C_SCL_PIN) == 0);

            // Read bit
            if (digitalReadFast(I2C_SDA_PIN)) {
                addressAndMode |= 1 << i;
            }

            if (i == 0) break;
            i--;

            // Wait for clock low
            while (digitalReadFast(I2C_SCL_PIN) == 1);
        }

        // The byte transmission we just received actually encodes two data items; the first 7 bits
        // is the address, and the last bit is write (0) or read (1)
        uint8_t address = addressAndMode >> 1;

        // Once we've received the address, pull down SDA to ACK, wait a cycle, and release
        if (address != 0x40)
            return;

        digitalWriteFast(I2C_ACTIVITY_PIN, HIGH);

        digitalWriteFast(I2C_SDA_PIN, LOW);
        pinModeFast(I2C_SDA_PIN, OUTPUT);
        while (digitalReadFast(I2C_SCL_PIN) == 1);
        while (digitalReadFast(I2C_SCL_PIN) == 0);
        while (digitalReadFast(I2C_SCL_PIN) == 1);
        pinModeFast(I2C_SDA_PIN, INPUT_PULLUP);

        // Read data
        uint8_t data = 0;
        i = 7;
        while (1) {
            // Wait for clock high
            while (digitalReadFast(I2C_SCL_PIN) == 0);

            // Read bit
            data |= digitalReadFast(I2C_SDA_PIN) << i;

            if (i == 0) break;
            i--;

            // Wait for clock low
            while (digitalReadFast(I2C_SCL_PIN) == 1);
        }

        // Once we've received the address, pull down SDA to ACK, wait a cycle, and release
        digitalWriteFast(I2C_SDA_PIN, LOW);
        pinModeFast(I2C_SDA_PIN, OUTPUT);
        while (digitalReadFast(I2C_SCL_PIN) == 1);
        while (digitalReadFast(I2C_SCL_PIN) == 0);
        // We don't get a return to high this time
        pinModeFast(I2C_SDA_PIN, INPUT_PULLUP);

        // Packet format:
        //   1 x x x s s s s
        //           '--.--'
        //              '------ new desired speed (0-12), 0 means off
        if (data & 0b10000000) {
            uint8_t desired_speed = data & 0x0F;
            if (desired_speed > 12) return;

            bool power; uint8_t up; uint8_t down;
            fan.steps_to_change_speed(desired_speed, &power, &up, &down);

            if (power) {
                fan.toggle_power();
                delay(200);
            }
            for (uint8_t i = 0; i < up; i++) {
                fan.speed_up();
                delay(200);
            }
            for (uint8_t i = 0; i < down; i++) {
                fan.speed_down();
                delay(200);
            }
        }
    }

    digitalWriteFast(I2C_ACTIVITY_PIN, LOW);
} 
