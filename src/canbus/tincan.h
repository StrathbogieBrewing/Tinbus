#ifndef TINCAN_H
#define TINCAN_H

#include "canbus.h"
#include "tinbus.h"

typedef enum tincan_error_t {
    TINCAN_OK = 0,
    TINCAN_ERROR,
} tincan_error_t;

tincan_error_t tincan_enframe(tinbus_frame_t *tin, const canbus_message_t *can);
tincan_error_t tincan_deframe(canbus_message_t *can, const tinbus_frame_t *tin);

#endif // TINCAN_H

