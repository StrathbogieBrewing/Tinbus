#ifndef MODBUS_ASC_H
#define MODBUS_ASC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "mb.h"

typedef enum {
    MB_ASC_IDLE = 0,
    MB_ASC_COLON,
    MB_ASC_DATA_HI,
    MB_ASC_DATA_LO,
    MB_ASC_CR,
    MB_ASC_LF,
} mb_asc_state;

typedef struct mb_asc_ctx_t {
    mb_frame_t frame;
    uint8_t lrc;   
    mb_asc_state state;   
} mb_asc_ctx_t;

mb_status mb_asc_tx(FILE *fd, const mb_asc_ctx_t *ctx);
mb_status mb_asc_rx(FILE *fd, mb_asc_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_ASC_H */