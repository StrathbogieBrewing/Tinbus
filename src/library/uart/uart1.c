#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "timer.h"
#include "uart1.h"

#ifndef UART1_BAUD
#define UART1_BAUD 19200
#endif

#undef BAUD // avoid compiler warning
#define BAUD UART1_BAUD
#include <util/setbaud.h>

static volatile uint8_t uart1_rx_head;
static volatile uint8_t uart1_rx_tail;
static uint8_t uart1_rx_data[UART_BUFFER_SIZE];

static volatile uint8_t uart1_tx_head;
static volatile uint8_t uart1_tx_tail;
static uint8_t uart1_tx_data[UART_BUFFER_SIZE];

static uart_t uart1_udata = {0};

static bool uart1_initialised = false;

void uart1_init(void);
int uart1_getchar(void);
int uart1_putchar(int c);

int uart1_fputc(char c, FILE *fd) {
    if (!uart1_initialised) {
        uart1_init();
        uart1_initialised = true;
    }
    return uart1_putchar(c);
}

int uart1_fgetc(FILE *fd) {
    if (!uart1_initialised) {
        uart1_init();
        uart1_initialised = true;
    }
    return uart1_getchar();
}

static FILE uart1 = FDEV_SETUP_STREAM(uart1_fputc, uart1_fgetc, _FDEV_SETUP_RW);

FILE *uart1_device = &uart1;

void uart1_init(void) {
    timer_init();
    fdev_set_udata(uart1_device, &uart1_udata);

    cli();
    uart1_tx_head = 0;
    uart1_tx_tail = 0;
    uart1_rx_head = 0;
    uart1_rx_tail = 0;

#if UART1_7_BITS_ODD_PARITY
    UCSR1C = (1 << UPM10) | (1 << UPM11) | (1 << UCSZ11); // 7 bit odd parity 1 stop bit
#else
    UCSR1C = (1 << UCSZ10) | (1 << UCSZ11); // 8 bit no parity 1 stop bit - default
#endif

    UBRR1H = UBRRH_VALUE;
    UBRR1L = UBRRL_VALUE;

#if USE_2X
    UCSR1A |= (1 << U2X1);
#else
    UCSR1A &= ~(1 << U2X1);
#endif

    UCSR1B = (1 << RXCIE1) | (1 << RXEN1); // | (1 << TXEN1);

    sei();
}

int uart1_getchar(void) {
    cli();
    if (uart1_rx_head == uart1_rx_tail) {
        sei();
        return EOF;
    }
    int data = (unsigned int)uart1_rx_data[uart1_rx_tail];
    uart1_rx_tail = (uart1_rx_tail + 1) & UART_BUFFER_MASK;
    sei();
    return data;
}

int uart1_putchar(int c) {
    cli();
    UCSR1B |= (1 << TXEN1);
    if ((uart1_tx_head == uart1_tx_tail) && (UCSR1A & (1 << UDRE1))) {
        UDR1 = (uint8_t)c; // send byte immediately
        UCSR1A |= (1 << TXC1);
        UCSR1B |= (1 << TXCIE1); // enable tx complete interrupt
        sei();
        return 0;
    }
    uint8_t tx_next = (uart1_tx_head + 1) & UART_BUFFER_MASK;
    if (tx_next == uart1_tx_tail) // buffer is full
    {
        sei();
        return EOF;
    }
    uart1_tx_data[uart1_tx_head] = (uint8_t)c;
    uart1_tx_head = tx_next;
    UCSR1B |= (1 << UDRIE1);  // enable UDRE interrupt
    UCSR1B &= ~(1 << TXCIE1); // disable tx complete interrupt
    sei();
    return 0;
}

ISR(USART1_UDRE_vect) // tx data register empty interrupt
{
    UDR1 = uart1_tx_data[uart1_tx_tail];
    uart1_tx_tail = (uart1_tx_tail + 1) & UART_BUFFER_MASK;
    uart1_udata.tx_time_ticks = timer_get_ticks_from_isr();
    if (uart1_tx_head == uart1_tx_tail) {
        UCSR1B &= ~(1 << UDRIE1); // disable data ready interrupt
        UCSR1A |= (1 << TXC1);    // clear tx complete interrupt flag
        UCSR1B |= (1 << TXCIE1);  // enable tx complete interrupt
    }
}

ISR(USART1_TX_vect) // tx complete interrupt
{
    uart1_udata.tx_time_ticks = timer_get_ticks_from_isr();
    UCSR1B &= ~(1 << TXCIE1); // disable tx complete interrupt
}

ISR(USART1_RX_vect) // rx data interrupt
{
    uart1_udata.rx_time_ticks = timer_get_ticks_from_isr();
    uart1_rx_data[uart1_rx_head] = UDR1; // always place new data into the buffer
    uart1_rx_head = (uart1_rx_head + 1) & UART_BUFFER_MASK;
    if (uart1_rx_head == uart1_rx_tail) // overwrite old data if the buffer is full
    {
        uart1_rx_tail = (uart1_rx_tail + 1) & UART_BUFFER_MASK;
    }
}