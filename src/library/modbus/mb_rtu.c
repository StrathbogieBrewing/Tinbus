#include <avr/io.h>

#include "mb.h"
#include "mb_asc.h"
#include "mb_crc.h"
#include "mb_rtu.h"
#include "timer.h"
#include "uart.h"

#define MB_RTU_TIMEOUT_TICKS (1823UL / TIMER_US_PER_TICK)

static uint8_t echo_count = 0;

mb_status mb_rtu_tx(FILE *fd, const mb_rtu_ctx_t *ctx) {
    mb_status status = MB_BUSY;
    // update intercharacter timer
    uint32_t time_ticks = timer_get_ticks();
    uint32_t delta_ticks = time_ticks - ctx->rx_time_ticks;
    if (delta_ticks > MB_RTU_TIMEOUT_TICKS) {
        // end of frame after 3.5 characters, clear to send
        uint16_t crc = 0xFFFF;
        crc = mb_crc(crc, ctx->frame.address);
        fputc(ctx->frame.address, fd);
        crc = mb_crc(crc, ctx->frame.function);
        fputc(ctx->frame.function, fd);
        uint8_t bytes_sent = 0;
        while (bytes_sent < ctx->frame.data_size) {
            crc = mb_crc(crc, ctx->frame.data[bytes_sent]);
            fputc(ctx->frame.data[bytes_sent], fd);
            bytes_sent++;
        }
        fputc((crc & 0xFF), fd);
        fputc((crc >> 8) & 0xFF, fd);
        echo_count = 2 + bytes_sent + 2;
        status = MB_FRAME_GOOD;
    }
    return status;
}

mb_status mb_rtu_rx(FILE *fd, mb_rtu_ctx_t *ctx) {
    mb_status status = MB_BUSY;
    // update intercharacter timer
    uint32_t time_ticks = timer_get_ticks();
    uint32_t delta_ticks = time_ticks - ctx->rx_time_ticks;
    if ((delta_ticks > MB_RTU_TIMEOUT_TICKS) && (!ctx->timed_out)) {
        // end of frame after 3.5 characters
        if (ctx->crc == 0) {
            if (ctx->frame.data_size >= 2) {
                ctx->frame.data_size -= 2; // remove the crc from the frame
                status = MB_FRAME_GOOD;
            } else {
                status = MB_FRAME_BAD;
            }
        } else {
            status = MB_FRAME_BAD;
        }
        ctx->index = 0;
        ctx->crc = 0xFFFF;
        ctx->timed_out = true;
    } else {
        int rx_byte = fgetc(fd);
        if (rx_byte != EOF) {
            if (echo_count) {
                echo_count--;
            } else {
                ctx->timed_out = false;
                ctx->rx_time_ticks = time_ticks;
                ctx->crc = mb_crc(ctx->crc, rx_byte);
                if (ctx->frame.data_size < MB_MAX_DATA_SIZE) {
                    switch (ctx->index) {
                    case 0:
                        ctx->frame.data_size = 0;
                        ctx->frame.address = rx_byte;
                        break;
                    case 1:
                        ctx->frame.function = rx_byte;
                        break;
                    default:
                        ctx->frame.data[ctx->frame.data_size] = rx_byte;
                        ctx->frame.data_size++;
                        break;
                    }
                    ctx->index++;
                }
            }
        }
    }
    return status;
}