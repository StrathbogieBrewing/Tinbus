#ifndef TINBUS_H
#define TINBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define TINBUS_BUFFER_SIZE (15)

typedef struct tinbus_t{
    uint8_t length;
    uint8_t buffer[TINBUS_BUFFER_SIZE];
} tinbus_frame_t;

typedef  enum {
    TINBUS_OK = 0,
    TINBUS_TX_BUSY,
    TINBUS_TX_COMPLETE,
    TINBUS_TX_COLLISION,

    TINBUS_RX_COMPLETE,
} tinbus_status_t;

typedef char tinbus_dump_t[TINBUS_BUFFER_SIZE * (8 + 1)];

void tinbus_dump(tinbus_dump_t *dest, const tinbus_frame_t *frame);

bool tinbus_transmit_pulse(void);

typedef void (*tinbus_callback_t)(tinbus_status_t status);

void tinbus_init(tinbus_callback_t callback);

uint8_t tinbus_read(tinbus_frame_t *frame);
uint8_t tinbus_write(tinbus_frame_t *frame);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // TINBUS_H