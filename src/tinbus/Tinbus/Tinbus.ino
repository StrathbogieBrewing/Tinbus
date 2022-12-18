#include "tinbus_driver.h"

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    tinbus_init();
}

void loop() {
    // tinbus_frame_t frame;
    // strcpy(frame.buffer, "Hello");
    // frame.length = 5;
    
    //  = {.buffer = {'H', 'e', 'l', 'l', 'o'}, .length = 5};
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    tinbus_transmit_pulse();
    // tinbus_write(&frame);

    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    tinbus_transmit_pulse();
    // tinbus_write(&frame);
}