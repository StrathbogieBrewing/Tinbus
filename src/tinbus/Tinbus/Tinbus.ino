#include "tinbus_driver.h"

void callback(tinbus_status_t status) {}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    tinbus_init(callback);
}

void loop() {
    tinbus_frame_t frame;

    frame.buffer[0] = 0x01;
    frame.buffer[1] = 0x23;
    frame.buffer[2] = 0x45;
    frame.buffer[3] = 0x67;
    frame.length = 4;

    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);

    tinbus_write(&frame);

    digitalWrite(LED_BUILTIN, LOW);
    delay(50);

    tinbus_write(&frame);
}