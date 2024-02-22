#pragma once

#include <stdint.h>

namespace i2c_peripheral {
    void begin();
    bool wait_for_byte(uint8_t *data);
}
