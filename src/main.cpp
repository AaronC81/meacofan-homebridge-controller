#include <Arduino.h>
#include <IRremote.h>

void setup() {
    IrSender.begin(2);
}

void loop() {
    while (1) {
        IrSender.sendNEC(0x80, 0x92, 3);
        delay(5000);
    }
} 
