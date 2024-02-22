#include "i2c_peripheral.hpp"
#include "pins.hpp"

#include <Arduino.h>
#include <digitalWriteFast.h>

namespace i2c_peripheral {
    void begin() {
        pinMode(I2C_SCL_PIN, INPUT_PULLUP);
        pinMode(I2C_SDA_PIN, INPUT_PULLUP);
        pinMode(I2C_ACTIVITY_PIN, OUTPUT);
    }

    // Convenience macros. Don't want to risk the overhead of a non-inlined function for these!
    #define WAIT_FOR_CLOCK_HIGH while (digitalReadFast(I2C_SCL_PIN) == 0);
    #define WAIT_FOR_CLOCK_LOW  while (digitalReadFast(I2C_SCL_PIN) == 1);

    bool wait_for_byte(uint8_t *data_ptr) {
        while (1) {
            // If SCL goes low, we're going to receive a transmission
            // TODO: set up a timer to get us back to a nice state if things go awry
            if (digitalReadFast(I2C_SCL_PIN) == 0) {
                // Read address
                uint8_t addressAndMode = 0;
                uint8_t i = 7;
                while (1) {
                    WAIT_FOR_CLOCK_HIGH;

                    // Read bit
                    addressAndMode |= digitalReadFast(I2C_SDA_PIN) << i;

                    if (i == 0) break;
                    i--;

                    WAIT_FOR_CLOCK_LOW;
                }

                // The byte transmission we just received actually encodes two data items; the first
                // 7 bits is the address, and the last bit is write (0) or read (1)
                uint8_t address = addressAndMode >> 1;

                // Once we've received the address, pull down SDA to ACK, wait a cycle, and release
                if (address != I2C_ADDRESS)
                    return false;

                digitalWriteFast(I2C_ACTIVITY_PIN, HIGH);

                digitalWriteFast(I2C_SDA_PIN, LOW);
                pinModeFast(I2C_SDA_PIN, OUTPUT);
                WAIT_FOR_CLOCK_LOW;
                WAIT_FOR_CLOCK_HIGH;
                WAIT_FOR_CLOCK_LOW;
                pinModeFast(I2C_SDA_PIN, INPUT_PULLUP);

                // Read data
                uint8_t data = 0;
                i = 7;
                while (1) {
                    WAIT_FOR_CLOCK_HIGH;

                    // Read bit
                    data |= digitalReadFast(I2C_SDA_PIN) << i;

                    if (i == 0) break;
                    i--;

                    // Wait for clock low
                    WAIT_FOR_CLOCK_LOW;
                }

                // Once we've received the address, pull down SDA to ACK, wait a cycle, and release
                digitalWriteFast(I2C_SDA_PIN, LOW);
                pinModeFast(I2C_SDA_PIN, OUTPUT);
                WAIT_FOR_CLOCK_LOW;
                WAIT_FOR_CLOCK_HIGH;
                // We don't get a return to high this time
                pinModeFast(I2C_SDA_PIN, INPUT_PULLUP);

                *data_ptr = data;
                return true;
            }

            digitalWriteFast(I2C_ACTIVITY_PIN, LOW);
        }

        return false;
    }

    #undef WAIT_FOR_CLOCK_HIGH
    #undef WAIT_FOR_CLOCK_LOW
}
