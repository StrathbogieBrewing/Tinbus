#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "timer.h"
#include "uart0.h"

#ifndef UART0_BAUD
#define UART0_BAUD 19200
#endif

#undef BAUD // avoid compiler warning
#define BAUD UART0_BAUD
#include <util/setbaud.h>

#define UART0_RS485_ENABLE_DDR DDRD
#define UART0_RS485_ENABLE_PORT PORTD
#define UART0_RS485_ENABLE_PIN 4

static volatile uint8_t uart0_rx_head;
static volatile uint8_t uart0_rx_tail;
static uint8_t uart0_rx_data[UART_BUFFER_SIZE];

static volatile uint8_t uart0_tx_head;
static volatile uint8_t uart0_tx_tail;
static uint8_t uart0_tx_data[UART_BUFFER_SIZE];

static uart_t uart0_udata = {0};

static bool uart0_initialised = false;

void uart0_init(void);
int uart0_getchar(void);
int uart0_putchar(int c);

int uart0_fputc(char c, FILE *fd) {
    if (!uart0_initialised) {
        uart0_init();
        uart0_initialised = true;
    }

    return uart0_putchar(c);
}

int uart0_fgetc(FILE *fd) {
    if (!uart0_initialised) {
        uart0_init();
        uart0_initialised = true;
    }
    return uart0_getchar();
}

static FILE uart0 = FDEV_SETUP_STREAM(uart0_fputc, uart0_fgetc, _FDEV_SETUP_RW);

FILE *uart0_device = &uart0;

void uart0_init(void) {
    timer_init();
    fdev_set_udata(uart0_device, &uart0_udata);

    UART0_RS485_ENABLE_PORT &= ~(1 << UART0_RS485_ENABLE_PIN); // disable 485 driver
    UART0_RS485_ENABLE_DDR |= (1 << UART0_RS485_ENABLE_PIN);

    cli();
    uart0_tx_head = 0;
    uart0_tx_tail = 0;
    uart0_rx_head = 0;
    uart0_rx_tail = 0;
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // 8n1
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A |= (1 << U2X0);
#else
    UCSR0A &= ~(1 << U2X0);
#endif
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    sei();
}

int uart0_getchar(void) {
    cli();
    if (uart0_rx_head == uart0_rx_tail) {
        sei();
        return EOF;
    }
    int data = (unsigned int)uart0_rx_data[uart0_rx_tail];
    uart0_rx_tail = (uart0_rx_tail + 1) & UART_BUFFER_MASK;
    sei();
    return data;
}

int uart0_putchar(int c) {
    cli();
    if ((uart0_tx_head == uart0_tx_tail) && (UCSR0A & (1 << UDRE1))) {
        UDR0 = (uint8_t)c; // send byte immediately
        UCSR0A |= (1 << TXC0);
        UCSR0B |= (1 << TXCIE0);                                  // enable tx complete interrupt
        UART0_RS485_ENABLE_PORT |= (1 << UART0_RS485_ENABLE_PIN); // enable 485 driver
        sei();
        return 0;
    }
    uint8_t tx_next = (uart0_tx_head + 1) & UART_BUFFER_MASK;
    if (tx_next == uart0_tx_tail) // buffer is full
    {
        sei();
        return EOF;
    }
    uart0_tx_data[uart0_tx_head] = (uint8_t)c;
    uart0_tx_head = tx_next;
    UCSR0B |= (1 << UDRIE0);  // enable UDRE interrupt
    UCSR0B &= ~(1 << TXCIE0); // disable tx complete interrupt
    sei();
    return 0;
}

ISR(USART0_UDRE_vect) // tx data register empty interrupt
{
    UDR0 = uart0_tx_data[uart0_tx_tail];
    uart0_tx_tail = (uart0_tx_tail + 1) & UART_BUFFER_MASK;
    uart0_udata.tx_time_ticks = timer_get_ticks_from_isr();
    if (uart0_tx_head == uart0_tx_tail) {
        UCSR0B &= ~(1 << UDRIE0); // disable data ready interrupt
        UCSR0A |= (1 << TXC0);    // clear tx complete interrupt flag
        UCSR0B |= (1 << TXCIE0);  // enable tx complete interrupt
    }
}

ISR(USART0_TX_vect) // tx complete interrupt
{
    uart0_udata.tx_time_ticks = timer_get_ticks_from_isr();
    UCSR0B &= ~(1 << TXCIE0);                                  // disable tx complete interrupt
    UART0_RS485_ENABLE_PORT &= ~(1 << UART0_RS485_ENABLE_PIN); // disable 485 driver
}

ISR(USART0_RX_vect) // rx data interrupt
{
    uart0_udata.rx_time_ticks = timer_get_ticks_from_isr();
    uart0_rx_data[uart0_rx_head] = UDR0; // always place new data into the buffer
    uart0_rx_head = (uart0_rx_head + 1) & UART_BUFFER_MASK;
    if (uart0_rx_head == uart0_rx_tail) // overwrite old data if the buffer is full
    {
        uart0_rx_tail = (uart0_rx_tail + 1) & UART_BUFFER_MASK;
    }
}