#pragma once

#include <stdint.h>

namespace nec_ir {
    void begin();
    void transmit(uint8_t address, uint8_t command);

    void start();
    void mark();
    void end();
    void send_zero();
    void send_one();
    void send_byte(uint8_t b);
}
