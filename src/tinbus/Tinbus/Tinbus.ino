#include "tinbus_driver.h"

void callback(tinbus_status_t status, const tinbus_frame_t *frame) {}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    tinbus_init(callback);
}


void loop() {
    // tinbus_frame_t frame;

    // frame.data[0] = 0x01;
    // frame.data[1] = 0x23;
    // frame.data[2] = 0x45;
    // frame.data[3] = 0x67;
    // frame.size = 4;

    // digitalWrite(LED_BUILTIN, HIGH);
    // delay(50);

    // tinbus_write(&frame);

    // digitalWrite(LED_BUILTIN, LOW);
    // delay(50);

    // tinbus_write(&frame);
    PORTB ^= (1 << PORTB2);
}