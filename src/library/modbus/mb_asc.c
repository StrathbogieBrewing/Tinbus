#include "mb_asc.h"
#include "mb.h"
#include "hex_asc.h"

#define MB_ASC_MIN_FRAME_SIZE 3

typedef enum {
    MB_ASC_RX_IDLE = 0,
    MB_ASC_RX_DATA_HI,
    MB_ASC_RX_DATA_LO,
    MB_ASC_RX_END,
} mb_asc_rx_state;

int puthex(FILE *fd, uint8_t value) {
    fputc(hex_bin2asc((value >> 4) & 0x0F), fd);
    fputc(hex_bin2asc(value & 0x0F), fd);
    return 0;
}

mb_status mb_asc_tx(FILE *fd, const mb_asc_ctx_t *ctx) {
    fputc(':', fd);
    uint8_t lrc = ctx->frame.address;
    puthex(fd, ctx->frame.address);
    lrc += ctx->frame.function;
    puthex(fd, ctx->frame.function);
    uint8_t bytes_sent = 0;
    while (bytes_sent < ctx->frame.data_size) {
        lrc += ctx->frame.data[bytes_sent];
        puthex(fd, ctx->frame.data[bytes_sent]);
        bytes_sent++;
    }
    puthex(fd, -lrc);
    fputc('\r', fd);
    fputc('\n', fd);
    return MB_FRAME_GOOD;
}

mb_status mb_asc_rx(FILE *fd, mb_asc_ctx_t *ctx) {
    mb_status status = MB_BUSY;
    int rx_byte = fgetc(fd);
    if (rx_byte != EOF) {
        switch (ctx->state) {
        case MB_ASC_RX_IDLE:
            if (rx_byte == ':') {
                ctx->state = MB_ASC_RX_DATA_HI;
                ctx->lrc = 0;
                ctx->frame.data_size = 0;
            }
            break;
        case MB_ASC_RX_DATA_HI: {
            int hex_val = hex_asc2bin(rx_byte);
            if (hex_val < 0) {
                if (rx_byte == '\r') {
                    ctx->state = MB_ASC_RX_END;
                } else {
                    ctx->state = MB_ASC_RX_IDLE;
                    status = MB_FRAME_BAD;
                    puts("Bad Hi");
                }
            } else {
                ctx->frame.data[ctx->frame.data_size] = (uint8_t)hex_val << 4;
                ctx->state = MB_ASC_RX_DATA_LO;
            }
            break;
        }
        case MB_ASC_RX_DATA_LO: {
            int hex_val = hex_asc2bin(rx_byte);
            if (hex_val < 0) {
                ctx->state = MB_ASC_RX_IDLE;
                status = MB_FRAME_BAD;
                puts("Bad Lo");
            } else {
                if (ctx->frame.data_size < MB_MAX_DATA_SIZE) {
                    ctx->frame.data[ctx->frame.data_size] |= (uint8_t)hex_val & 0x0F;
                    ctx->lrc += ctx->frame.data[ctx->frame.data_size];
                    ctx->frame.data_size++;
                    ctx->state = MB_ASC_RX_DATA_HI;
                } else {
                    ctx->state = MB_ASC_RX_IDLE;
                    status = MB_FRAME_BAD;
                    puts("Too big");
                }
            }
            break;
        }
        case MB_ASC_RX_END: {
            if (rx_byte != '\n') {
                ctx->state = MB_ASC_RX_IDLE;
                status = MB_FRAME_BAD;
                puts("Bad End");
            } else {
                if (ctx->lrc) {
                    ctx->state = MB_ASC_RX_IDLE;
                    status = MB_FRAME_BAD;
                    puts("Bad LRC");
                } else {
                    if (ctx->frame.data_size < MB_ASC_MIN_FRAME_SIZE) {
                        ctx->state = MB_ASC_RX_IDLE;
                        status = MB_FRAME_BAD;
                        puts("Too small");
                    }
                    ctx->frame.address = ctx->frame.data[0];
                    ctx->frame.function = ctx->frame.data[1];
                    for (uint8_t i = 0; i < ctx->frame.data_size; i++) {
                        ctx->frame.data[i] = ctx->frame.data[i + 2];
                    }
                    ctx->frame.data_size -= MB_ASC_MIN_FRAME_SIZE;
                    ctx->state = MB_ASC_RX_IDLE;
                    status = MB_FRAME_GOOD;
                }
            }
            break;
        }
        default:
            ctx->state = MB_ASC_RX_IDLE;
            status = MB_FRAME_BAD;
            break;
        }
    }
    return status;
}