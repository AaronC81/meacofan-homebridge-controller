#include <Arduino.h>

#include "nec_ir.hpp"

void setup() {
    nec_ir::begin();
}

void loop() {
    while (1) {
        nec_ir::transmit(0x80, 0x92);
        delay(2000);
    }
} 
