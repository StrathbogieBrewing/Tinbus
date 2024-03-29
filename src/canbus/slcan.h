#ifndef SLCAN_H
#define SLCAN_H

#include <stdbool.h>
#include <stdint.h>

#include "canbus.h"

#define SLCAN_BUFFER_SIZE (32)

typedef struct slcan_t {
    uint8_t size;
    uint8_t buffer[SLCAN_BUFFER_SIZE];
} slcan_frame_t;

typedef enum slcan__error_t {
    SLCAN_OK = 0,
    SLCAN_NOT_DATA,
    SLCAN_INVALID_ID,
    SLCAN_INVALID_DLC,
    SLCAN_ERROR,
} slcan_error_t;

slcan_error_t slcan_enframe(slcan_frame_t *slcan, const canbus_message_t *can);
slcan_error_t slcan_deframe(canbus_message_t *can, const slcan_frame_t *slcan);

#endif // SLCAN_H