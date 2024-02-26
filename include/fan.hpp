#pragma once

#include <stdint.h>

#include <nec_ir.hpp>

/**
 * @brief Models the fan and provides high-level control.
 */
class Fan {
public:
    const uint8_t ADDRESS = 0x80;

    const uint8_t COMMAND_POWER = 0x92;
    const uint8_t COMMAND_SPEED_UP = 0x89;
    const uint8_t COMMAND_SPEED_DOWN = 0x9F;
    const uint8_t COMMAND_LIGHT = 0x9E;

    /**
     * @brief The amount of time before the fan goes into sleep mode while turned on, requiring
     *        an additional button press to wake up first.
     */
    const unsigned long SLEEP_TIMEOUT_MS = 60000;

    /**
     * @brief Send a command to the fan, accounting for any special requirements for sleep mode. 
     * 
     * @param command The IR data byte to transmit.
     */
    void transmit(uint8_t command) {
        if (on && millis() - last_interaction_millis > SLEEP_TIMEOUT_MS) {
            // The fan is asleep! Wake it up by sending LIGHT.
            // This isn't tracked as part of the state, so if we're slightly off with our timing,
            // there's no noticeable side effects on our part.
            nec_ir::transmit(ADDRESS, COMMAND_LIGHT);

            delay(500); // Wait a little bit, just in case
        }

        nec_ir::transmit(ADDRESS, command);

        // Update last interaction time, so we know if we're asleep by the next interaction
        last_interaction_millis = millis();
    }

    /**
     * @brief Toggle the fan's power, and update the model accordingly.
     */
    void toggle_power() {
        transmit(COMMAND_POWER);
        on = !on;
    }

    /**
     * @brief Increase the fan's speed, and update the model accordingly.
     */
    void speed_up() {
        transmit(COMMAND_SPEED_UP);
        if (current_speed != 12) current_speed++;
    }

    /**
     * @brief Decrease the fan's speed, and update the model accordingly.
     */
    void speed_down() {
        transmit(COMMAND_SPEED_DOWN);
        if (current_speed != 1) current_speed--;
    }

    /**
     * @brief Determines the steps required to change the speed to a new speed value.
     * 
     * @param desired_speed The new desired speed. 0 means off.
     * @param[out] power Whether to press the "power" button. If so, this should happen first.
     * @param[out] up The number of times to press the "speed up" button.
     * @param[out] down The number of times to press the "speed down" button.
     */
    void steps_to_change_speed(uint8_t desired_speed, bool *power, uint8_t *up, uint8_t *down) {
        *power = false;
        *up = 0;
        *down = 0;

        // Figure out whether we simply need to turn the fan off
        if (desired_speed == 0) {
            if (on) {
                *power = true;
            }
            return;
        }

        // Figure out whether we need to turn the fan on first
        if (!on && desired_speed != 0) {
            *power = true;
        }

        // Determine steps required to reach desired speed
        if (desired_speed > current_speed) {
            *up = (desired_speed - current_speed);
        } else if (desired_speed < current_speed) {
            *down = (current_speed - desired_speed);
        }
    }

private:

    /**
     * @brief The current speed. This will hold a value even if the fan is powered off.
     */
    uint8_t current_speed = 1;

    /**
     * @brief Whether the fan is currently powered on.
     */
    bool on = false;

    /**
     * @brief The `millis` value the last time a keypress was sent to the fan. Used to keep track of
     *        whether it is asleep, based on `SLEEP_TIMEOUT_MS`.
     */
    unsigned long last_interaction_millis = 0;
};