#pragma once

#include <stdint.h>

namespace i2c_peripheral {
    void begin();
    bool wait(uint8_t *datum);
}
