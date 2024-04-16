#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "mb.h"

typedef struct mb_rtu_ctx_t {
    mb_frame_t frame;
    uint32_t rx_time_ticks;
    uint32_t tx_time_ticks;
    uint8_t index;
    uint16_t crc;
    bool timed_out;

} mb_rtu_ctx_t;

mb_status mb_rtu_rx(FILE *fd, mb_rtu_ctx_t *ctx);
mb_status mb_rtu_tx(FILE *fd, const mb_rtu_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_RTU_H */