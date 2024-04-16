#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdio.h>

#define UART_BUFFER_BITS (6)
#define UART_BUFFER_SIZE (1 << UART_BUFFER_BITS)
#define UART_BUFFER_MASK (UART_BUFFER_SIZE - 1)

typedef struct uart_t{
    uint32_t rx_time_ticks;
    uint32_t tx_time_ticks;
} uart_t;

uint32_t uart_get_rx_ticks(FILE *stream);
uint32_t uart_get_tx_ticks(FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
