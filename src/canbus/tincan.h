#ifndef TINCAN_H
#define TINCAN_H

#include "canbus.h"
#include "tinbus.h"

typedef enum tincan_error_t {
    TINCAN_OK = 0,
    TINCAN_ERROR,
} tincan_error_t;

tincan_error_t tincan_enframe(const canbus_message_t *can, tinbus_frame_t * tin);
tincan_error_t tincan_deframe(const tinbus_frame_t * tin, canbus_message_t *can);

#endif // TINCAN_H

