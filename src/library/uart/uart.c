#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "timer.h"

uint32_t uart_get_rx_ticks(FILE *uart)
{
    cli();
    uint32_t ticks = ((uart_t*)(uart->udata))->rx_time_ticks;
    sei();
    return ticks;
}

uint32_t uart_get_tx_ticks(FILE *uart)
{
    cli();
    uint32_t ticks = ((uart_t*)(uart->udata))->tx_time_ticks;
    sei();
    return ticks;
}