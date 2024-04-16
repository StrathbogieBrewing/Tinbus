#ifndef SCPI_H
#define SCPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "term.h"
#include "handler.h"

#define SCPI_ERROR "!error"
#define SCPI_BAD_QUERY "!bad-query"
#define SCPI_NO_RESPONSE "!no-response"
#define SCPI_ID_QUERY "*idn?"

typedef enum {
    SCPI_BUSY = 0,
    SCPI_RX_COMPLETE,
} scpi_status;

typedef struct scpi_ctx_t {
    term_str_t data;
    uint8_t data_size;
} scpi_ctx_t;

extern handler_t scpi_handler;

void scpi_tx(FILE *fd, term_str_t str);
scpi_status scpi_rx(FILE *fd, scpi_ctx_t *ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SCPI_H
