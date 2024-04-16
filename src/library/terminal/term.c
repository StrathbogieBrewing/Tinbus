#include <stdbool.h>
#include <string.h>

#include <avr/io.h>

#include "term.h"

#define TERM_CR ('\r')
#define TERM_LF ('\n')
#define TERM_ESC (0x1B)

#define TERM_PUTC(ch, fd)  do { if (fputc(ch, fd) == EOF) return false; } while (0)

// sends in format "{DATA}{CR}{ESC}[>{CSUM}{CSUM}{LF}"
// returns true if successful
bool term_tx(FILE *fd, term_str_t str) {
    uint8_t lrc = -':';  // initialise the checksum
    for (; *str != '\0'; str++) {
        lrc += *str;
        TERM_PUTC(*str, fd);
    }
    TERM_PUTC(TERM_CR, fd);
    TERM_PUTC(TERM_ESC, fd);
    TERM_PUTC('[', fd);
    TERM_PUTC('>', fd);
    TERM_PUTC('0' + ((lrc >> 4) & 0x07), fd);
    TERM_PUTC('A' + (lrc & 0x0F), fd);
    TERM_PUTC(TERM_LF, fd);
    TERM_PUTC('\0', fd);
    return true;
}

// expects machine formated meassage "{DATA}{CR}{ESC}[>{CSUM}{CSUM}{LF}"
// or keyboard formated message without checksum "{DATA}{CR}{LF}"
term_status term_rx(FILE *fd, term_ctx_t *ctx) {
    term_status status = TERM_BUSY;
    int rx_byte = fgetc(fd);
    if (rx_byte != EOF) {
        fputc(rx_byte, fd); // echo all rx bytes
        if (rx_byte == TERM_LF) {
            if (ctx->state == TERM_RX_CR) {
                ctx->chars[ctx->length] = '\0'; // terminate string
                status = TERM_LINE_GOOD;
            } else if (ctx->state == TERM_RX_LRC_OK) {
                ctx->chars[ctx->length] = '\0'; // terminate string
                status = TERM_LINE_GOOD;
            } else {
                status = TERM_LINE_BAD;
            }
            ctx->state = TERM_RX_LF;
        } else {
            if (ctx->state == TERM_RX_LF) {
                ctx->state = TERM_RX_DATA;
                ctx->length = 0;
                ctx->lrc = -':';  // initialise the checksum;
            }
        }
        switch (ctx->state) {
        case TERM_RX_IDLE:
        case TERM_RX_LF:
            break;
        case TERM_RX_DATA: {
            if (rx_byte == TERM_CR) {
                ctx->state = TERM_RX_CR;
            } else {
                if (ctx->length < TERM_STR_MAX_SIZE - 1) { // allow for terminating null
                    ctx->chars[ctx->length] = (uint8_t)rx_byte;
                    ctx->length++;
                    ctx->lrc += (uint8_t)rx_byte;
                } else {
                    ctx->state = TERM_RX_IDLE;
                    status = TERM_LINE_BAD;
                }
            }
            break;
        }
        case TERM_RX_CR: {
            if (rx_byte == TERM_ESC) {
                ctx->state = TERM_RX_ESC;
            } else {
                ctx->state = TERM_RX_IDLE;
                status = TERM_LINE_BAD;
            }
            break;
        }
        case TERM_RX_ESC: {
            if (rx_byte == '[') {
                ctx->state = TERM_RX_OPEN_BRACKET;
            } else {
                ctx->state = TERM_RX_IDLE;
                status = TERM_LINE_BAD;
            }
            break;
        }
        case TERM_RX_OPEN_BRACKET: {
            if (rx_byte == '>') {
                ctx->state = TERM_RX_GREATER_THAN;
            } else {
                ctx->state = TERM_RX_IDLE;
                status = TERM_LINE_BAD;
            }
            break;
        }
        case TERM_RX_GREATER_THAN: {
            if (rx_byte - ((ctx->lrc >> 4) & 0x07) == '0') {
                ctx->state = TERM_RX_LRC_CHECK;
            } else {
                ctx->state = TERM_RX_IDLE;
                status = TERM_LINE_BAD;
            }
            break;
        }
        case TERM_RX_LRC_CHECK: {
            if (rx_byte - (ctx->lrc & 0x0F) == 'A') {
                ctx->state = TERM_RX_LRC_OK;
            } else {
                ctx->state = TERM_RX_IDLE;
                status = TERM_LINE_BAD;
            }
            break;
        }
        default: {
            ctx->state = TERM_RX_IDLE;
            status = TERM_LINE_BAD;
            break;
        }
        }
    }
    return status;
}
