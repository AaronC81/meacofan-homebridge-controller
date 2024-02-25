#include <Arduino.h>

#include "nec_ir.hpp"
#include "i2c_peripheral.hpp"
#include "pins.hpp"
#include "fan.hpp"

#include <digitalWriteFast.h>

Fan fan;

void setup() {
    nec_ir::begin();
    i2c_peripheral::begin();

    // Show that we've got power!
    for (int i = 0; i < 3; i++) {
        digitalWrite(I2C_ACTIVITY_PIN, HIGH);
        delay(200);
        digitalWrite(I2C_ACTIVITY_PIN, LOW);
        delay(200);
    }
}

void loop() {
    uint8_t data;
    if (i2c_peripheral::wait_for_byte(&data)) {
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
} 
