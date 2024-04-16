#ifndef TERM_H
#define TERM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "uart.h"

#define TERM_STR_MAX_SIZE UART_BUFFER_SIZE

typedef enum {
    TERM_BUSY = 0,
    TERM_LINE_GOOD,
    TERM_LINE_BAD,
} term_status;

typedef enum {
    TERM_RX_IDLE = 0,
    TERM_RX_READY,
    TERM_RX_DATA,
    TERM_RX_CR,
    TERM_RX_ESC,
    TERM_RX_OPEN_BRACKET,
    TERM_RX_GREATER_THAN,
    TERM_RX_LRC_CHECK,
    TERM_RX_LRC_OK,
    TERM_RX_LF,
    TERM_RX_END,
} term_state;

typedef char term_str_t[TERM_STR_MAX_SIZE];

typedef struct term_ctx_t {
    term_str_t chars;
    uint8_t length;
    uint8_t lrc;
    term_state state;
} term_ctx_t;

// given string in format "{DATA}"
// sends to stream as "{DATA}{CR}{ESC}[>{CSUM}{CSUM}{LF}"
bool term_tx(FILE *fd, term_str_t str);

// reads bytewise from stream as "{DATA}{CR}{ESC}[>{CSUM}{CSUM}{LF}" or "{DATA}{CR}{LF}"
// on success populates term_ctx_t.chars with "{DATA}" and returns TERM_LINE_GOOD
// on failure or error returns TERM_LINE_BAD
// or TERM_BUSY whilst waiting or parsing incoming stream
term_status term_rx(FILE *fd, term_ctx_t *ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // TERM_H
