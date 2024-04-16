#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define BAUD 19200L
#include <util/setbaud.h>

// #include "hex_asc.h"
#include "cobsm.h"
#include "mb_crc.h"
#include "uart1.h"

#define BIT_PERIOD_TICKS ((50UL * F_CPU) / 1000000UL)
#define BUFFER_SIZE_MAX 32
#define MASK 0x80
#define CRC_ERROR_COUNT 0xFF

enum { RX_ZERO = 0, RX_ONE, RX_END };

volatile static uint16_t timer1_capture = 0;
volatile static bool timer1_timeout = false;

ISR(TIMER1_CAPT_vect) {
    if (timer1_capture) {
        PORTC ^= (1 << PORTC3);
    }
    if (ICR1) {
        timer1_capture = ICR1;
    } else {
        timer1_capture = 1; // zero is used as a sentinel
    }
    OCR1A = ICR1 + (8 * BIT_PERIOD_TICKS); // setup end of frame timeout
    TIFR1 |= (1 << OCF1A);
    TIMSK1 |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {
    timer1_timeout = true;
    TIMSK1 &= ~(1 << OCIE1A);
}

void rx_data(uint8_t data) {
    static uint8_t mask = MASK;
    static uint8_t buffer[BUFFER_SIZE_MAX];
    static uint8_t index = 0;
    static uint16_t crc = 0xFFFF;
    static uint8_t crc_error_count = 0;
    if (data == RX_END) {
        crc = mb_crc(crc, buffer[index]);
        uint8_t size = index + 1;
        if (crc) {
            crc_error_count++;
            crc = 0xFFFF;
            buffer[0] = CRC_ERROR_COUNT;  // send message with crc error count to indicate crc error
            crc = mb_crc(crc, buffer[0]);
            buffer[1] = crc_error_count;
            crc = mb_crc(crc, buffer[1]);
            buffer[2] = crc & 0xFF;
            buffer[3] = crc >> 8;
            size = 4;

        } else {
            size = cobsm_encode(buffer, size);  // encoding removes all zeros from data
            // size = cobsm_decode(buffer, size);
        }

        uint8_t i = 0;
        while (i < size) {
            putc(buffer[i++], stdout);
        }
        putc(0x00, stdout);  // mark end of frame with added zero

        index = 0; // reset receiver buffer
        buffer[index] = 0;
        mask = MASK;
        crc = 0xFFFF;
        return;
    }
    if (mask == 0) {
        crc = mb_crc(crc, buffer[index]);
        if (index < BUFFER_SIZE_MAX - 1) {
            index++;
        } else {
            index = 0; // just wrap on overflow for now
        }
        mask = MASK;
        buffer[index] = 0;
    }
    if (data == RX_ONE) {
        buffer[index] |= mask;
    }
    mask >>= 1;
}

void init_timer1(void) {
    TCCR1B = (1 << CS10) | (1 << ICNC1); // -ve edge, noise cancel enabled, no prescale
    TIMSK1 = (1 << ICIE1);               // | (1 << TOIE1);
    sei();
}

int main(void) {
    wdt_enable(WDTO_250MS);
    init_timer1();

    stdout = uart1_device;
    stdin = uart1_device;

    uint16_t last_capture = 0;

    DDRC |= (1 << PORTC3);

    while (1) {
        wdt_reset();
        cli();
        uint16_t capture = timer1_capture;
        timer1_capture = 0;
        bool timeout = timer1_timeout;
        timer1_timeout = false;
        sei();
        if (capture) {
            if (last_capture) {
                uint16_t delta = capture - last_capture;
                if (delta < (4 * BIT_PERIOD_TICKS)) {
                    rx_data(RX_ZERO);

                } else if (delta < (8 * BIT_PERIOD_TICKS)) {
                    rx_data(RX_ONE);
                } else {
                    last_capture = 0;
                }
            }
            last_capture = capture;
        }
        if (timeout) {
            last_capture = 0;
            rx_data(RX_END);
        }
    }
    return 0;
}