#ifndef HANDLER_H
#define HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mb.h"
#include "term.h"

#define HANDLER_MODBUS_GATEWAY_ADDRESS (0x00)
#define HANDLER_MODBUS_RELAY_START_ADDRESS (0x01)
#define HANDLER_MODBUS_RELAY_END_ADDRESS (0x09)
#define HANDLER_MODBUS_DSA_START_ADDRESS (0x30)
#define HANDLER_MODBUS_DSA_END_ADDRESS (0x37)
#define HANDLER_MODBUS_SCPI_START_ADDRESS (0x40)
#define HANDLER_MODBUS_SCPI_END_ADDRESS (0x99)

#define HANDLER_ADDRESS_HIGH_OFFSET 0
#define HANDLER_ADDRESS_LOW_OFFSET 1
#define HANDLER_ORIGIN_OFFSET 2
#define HANDLER_COMMAND_OFFSET 3

typedef enum {
    HANDLER_NOT_FOUND = 0,
    HANDLER_MESSAGE,
    HANDLER_NO_MESSAGE,
    HANDLER_UNSUPPORTED_MODBUS,
    HANDLER_MODBUS_REQUEST,
} handler_status_t;

typedef handler_status_t (*handler_request)(term_str_t request, mb_frame_t *mb_frame);
typedef handler_status_t (*handler_response)(term_str_t response, mb_frame_t *mb_frame);

typedef struct handler_t {
    handler_request request;
    handler_response response;
} handler_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // HANDLER_H
