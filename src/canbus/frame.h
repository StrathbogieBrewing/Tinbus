#ifndef FRAME_H
#define FRAME_H

#include "canbus.h"
#include "tinbus.h"

typedef enum frame_error_t {
    FRAME_OK = 0,
    FRAME_ERROR,
} frame_error_t;

frame_error_t frame_enframe(const canbus_message_t *can, tinbus_frame_t * tin);
frame_error_t frame_deframe(const tinbus_frame_t * tin, canbus_message_t *can);


#endif // FRAME_H

