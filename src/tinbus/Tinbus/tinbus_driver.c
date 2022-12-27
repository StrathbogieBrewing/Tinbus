#include "Arduino.h"

#include "tinbus_driver.h"

char *tinbus_dump_binary(char *dest, uint8_t value) {
    uint8_t bit_mask = 0b10000000;
    while (bit_mask) {
        if (bit_mask & value) {
            *dest = '1';
        } else {
            *dest = '0';
        }
        dest++;
        bit_mask >>= 1;
    }
    return dest;
}

void tinbus_dump(tinbus_dump_t *dest, const tinbus_frame_t *frame) {
    char *dest_ptr = (char *)dest;
    if (frame->size < TINBUS_BUFFER_SIZE) {
        uint8_t index = 0;
        while (index < frame->size) {
            dest_ptr = tinbus_dump_binary(dest_ptr, frame->data[index]);
            index++;
            if (index < frame->size) {
                *dest_ptr = ' '; // add a space between bytes
                dest_ptr++;
            }
        }
    }
    *dest_ptr = '\0'; // terminate string with a null
}


static tinbus_frame_t tinbus_rx_frame = {0};
static volatile uint8_t tinbus_rx_bit_mask = 0;
static volatile uint8_t tinbus_rx_recessive_count = 0;

static tinbus_callback_t tinbus_callback = 0;

ISR(SPI_STC_vect) {
    // PORTB |= (1 << PORTB2);

    uint8_t tinbus_rx_data = SPDR;
    SPDR = 0x1f; // dummy write

    for(uint8_t bit_count = 0; bit_count < 8 ; bit_count++){
        if (tinbus_rx_data & 0x01) {
            // recessive state
            tinbus_rx_recessive_count += 1;
            if (tinbus_rx_recessive_count == 16) {
                // end of frame
                if (tinbus_rx_bit_mask != 0x01) {
                    tinbus_callback(TINBUS_RX_FRAME_ERROR, &tinbus_rx_frame);
                } else {
                    tinbus_callback(TINBUS_RX_COMPLETE, &tinbus_rx_frame);
                }
                tinbus_rx_frame.size = 0;
                tinbus_rx_bit_mask = 0x01;
            }
            if (tinbus_rx_recessive_count > 24) {
                tinbus_rx_recessive_count = 24;
            }
        } else {
            // dominant state
            if (tinbus_rx_recessive_count) {
                if (tinbus_rx_recessive_count < 4) {
                    // data bit = '0'
                } else if (tinbus_rx_recessive_count < 16) {
                    // data bit = '1'
                    tinbus_rx_frame.data[tinbus_rx_frame.size] |= tinbus_rx_bit_mask;
                }
                tinbus_rx_recessive_count = 0;

                tinbus_rx_bit_mask <<= 1;
                if (tinbus_rx_bit_mask == 0) {
                    tinbus_rx_bit_mask = 0x01;
                    if (tinbus_rx_frame.size < (TINBUS_BUFFER_SIZE - 2)) {
                        tinbus_rx_frame.size += 1;
                    } else {
                        // rx buffer overrun
                        tinbus_callback(TINBUS_RX_FRAME_OVERRUN, &tinbus_rx_frame);
                        tinbus_rx_frame.size = 0;
                        tinbus_rx_bit_mask = 0x01;
                    }
                }
            }
        }
        tinbus_rx_data <<= 1;
    }

    // PORTB &= ~(1 << PORTB2);
}

void tinbus_init(tinbus_callback_t callback) {
    DDRB |= (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB5); // debug output enable
    // DDRB &= ~(1 << PORTB5);

    SPCR = (1 << MSTR) | (1 << SPE) | (1 << SPIE) | (1 << SPR0) | (1 << SPR1);

    SPDR = 0xFF;
}

uint8_t tinbus_read(tinbus_frame_t *frame) { return 0; }

uint8_t tinbus_write(tinbus_frame_t *frame) {
    // PORTB |= (1 << PORTB2);
    // // SPDR = 0xF7;
    // PORTB &= ~(1 << PORTB2);
    return 0;
}

// // if time_us is zero then a timeout occurred,
// // otherwise time_us is time between last event and this edge event
// void tinbus_receive_edge_cb(uint16_t time_us) {}

// // #define PULSE_LENGTH_US (20)

// #define TINBUS_PERIOD_TICKS (20 * 16)

// #define INPUT_CAPTURE_PIN 8     // receive
// #define OUTPUT_COMPARE_A_PIN 9  // transmit
// #define OUTPUT_COMPARE_B_PIN 10 // unusable PWM

// #define CONFIG_TIMER_NOPRESCALE() (TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1 << ICNC1) | (1 << CS10))
// #define CONFIG_TIMER_PRESCALE_8() (TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1 << ICNC1) | (1 << CS11))
// #define CONFIG_TIMER_PRESCALE_256() (TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1 << ICNC1) | (1 << CS12))
// #define CONFIG_MATCH_NORMAL() (TCCR1A = TCCR1A & ~((1 << COM1A1) | (1 << COM1A0)))
// #define CONFIG_MATCH_TOGGLE() (TCCR1A = (TCCR1A & ~(1 << COM1A1)) | (1 << COM1A0))
// #define CONFIG_MATCH_CLEAR() (TCCR1A = (TCCR1A | (1 << COM1A1)) & ~(1 << COM1A0))

// #define IS_MATCH_SET_ENABLED() (TCCR1A & (1 << COM1A0))
// #define CONFIG_MATCH_SET() (TCCR1A = TCCR1A | ((1 << COM1A1) | (1 << COM1A0)))
// #define SET_CAPTURE_FALLING_EDGE() (TCCR1B &= ~(1 << ICES1))
// #define CONFIG_CAPTURE_RISING_EDGE() (TCCR1B |= (1 << ICES1))
// #define ENABLE_CAPTURE_INTERRUPT() (TCCR1B |= (1 << ICNC1), TIFR1 |= (1 << ICF1), TIMSK1 |= (1 << ICIE1))
// #define ENABLE_COMPARE_A_INTERRUPT() (TIFR1 = (1 << OCF1A), TIMSK1 |= (1 << OCIE1A))
// #define ENABLE_COMPARE_B_INTERRUPT() (TIFR1 = (1 << OCF1B), TIMSK1 |= (1 << OCIE1B))

// #define ENABLE_COMPARE_A_OUTPUT() (DDRB |= (1 << PORTB1), PORTB |= (1 << PORTB1))
// // #define DISABLE_COMPARE_A_OUTPUT() (DDRB &= ~(1 << PORTB1), PORTB |= (1 << PORTB1))

// #define IS_COMPARE_A_INTERRUPT_ENABLED() (TIMSK1 & (1 << OCIE1A))
// #define IS_COMPARE_B_INTERRUPT_ENABLED() (TIMSK1 & (1 << OCIE1B))

// #define DISABLE_CAPTURE_INTERRUPT() (TIMSK1 &= ~(1 << ICIE1))
// #define DISABLE_COMPARE_A_INTERRUPT() (TIMSK1 &= ~(1 << OCIE1A))
// #define DISABLE_COMPARE_B_INTERRUPT() (TIMSK1 &= ~(1 << OCIE1B))
// #define GET_TIMER_COUNT() (TCNT1)
// #define GET_CAPTURE_COUNT() (ICR1)
// #define IS_CAPTURE_FLAG_SET() (TIFR1 & (1 << ICF1))
// #define CLEAR_CAPTURE_FLAG() (TIFR1 |= (1 << ICF1))
// #define GET_COMPARE_A() (OCR1A)
// #define GET_COMPARE_B() (OCR1B)
// #define SET_COMPARE_A(val) (OCR1A = (val))
// #define SET_COMPARE_B(val) (OCR1B = (val))
// #define CAPTURE_INTERRUPT TIMER1_CAPT_vect
// #define COMPARE_A_INTERRUPT TIMER1_COMPA_vect
// #define COMPARE_B_INTERRUPT TIMER1_COMPB_vect

// static volatile uint16_t capture = 0;
// static tinbus_frame_t tinbus_tx_frame = {0};
// static volatile uint8_t tinbus_tx_index = 0;
// static volatile uint8_t tinbus_tx_bit_mask = 0;

// typedef enum {
//     TINBUS_TX_IDLE = 0,
//     TINBUS_TX_PULSE_SENT,
//     TINBUS_TX_PULSE_RECEIVED,
// } tinbus_tx_state_t;

// static volatile tinbus_tx_state_t tinbus_tx_state = TINBUS_TX_IDLE;

// static tinbus_callback_t tinbus_callback = 0;

// ISR(COMPARE_A_INTERRUPT) {
//     if (IS_MATCH_SET_ENABLED()) {
//         // trailing edge, set up next bit
//         if (tinbus_tx_index != tinbus_tx_frame.size) {
//             if ((tinbus_tx_bit_mask & tinbus_tx_frame.data[tinbus_tx_index]) != 0) {
//                 SET_COMPARE_A(GET_COMPARE_A() + TINBUS_PERIOD_TICKS * 5);
//             } else {
//                 SET_COMPARE_A(GET_COMPARE_A() + TINBUS_PERIOD_TICKS * 2);
//             }
//             CONFIG_MATCH_CLEAR();
//             // ENABLE_COMPARE_A_OUTPUT();
//             tinbus_tx_state = TINBUS_TX_PULSE_SENT;
//             tinbus_tx_bit_mask >>= 1;
//             if (tinbus_tx_bit_mask == 0) {
//                 tinbus_tx_bit_mask = 0b10000000;
//                 tinbus_tx_index += 1;
//             }
//         } else {
//             SET_COMPARE_A(GET_COMPARE_A() + TINBUS_PERIOD_TICKS);
//             CONFIG_MATCH_SET();
//             DISABLE_COMPARE_A_INTERRUPT();
//             // tx done call back
//             tinbus_callback(TINBUS_TX_COMPLETE, &tinbus_tx_frame);
//         }
//     } else {
//         // leading edge, restore bus to recessive mode
//         SET_COMPARE_A(GET_COMPARE_A() + TINBUS_PERIOD_TICKS);
//         CONFIG_MATCH_SET();
//     }
// }

// static tinbus_frame_t tinbus_rx_frame = {0};
// // static volatile uint8_t tinbus_rx_index = 0;
// static volatile uint8_t tinbus_rx_bit_mask = 0;

// ISR(CAPTURE_INTERRUPT) { // for receiving leading edge of pulse
//     PINB |= (1 << PORTB2);

//     DISABLE_CAPTURE_INTERRUPT();
//     SET_COMPARE_B(capture + TINBUS_PERIOD_TICKS); // blanking period
//     ENABLE_COMPARE_B_INTERRUPT();

//     uint16_t count = GET_CAPTURE_COUNT();
//     uint16_t delta = count - capture;
//     capture = count;

//     if (delta > (TINBUS_PERIOD_TICKS * 4)) {
//         tinbus_rx_frame.data[tinbus_rx_frame.size] |= tinbus_rx_bit_mask;
//     }

//     tinbus_rx_bit_mask <<= 1;
//     if (tinbus_rx_bit_mask == 0) {
//         tinbus_rx_bit_mask = 0x01;
//         if (tinbus_rx_frame.size < (TINBUS_BUFFER_SIZE - 2)) {
//             tinbus_rx_frame.size += 1;
//         } else {
//             // rx buffer overrun
//             tinbus_callback(TINBUS_RX_FRAME_OVERRUN, &tinbus_rx_frame);
//             tinbus_rx_frame.size = 0;
//             tinbus_rx_bit_mask = 0x01;
//         }
//     }

//     // if (tinbus_tx_state == TINBUS_TX_PULSE_RECEIVED) {
//     //     // abort - detected another transmitter
//     //     tinbus_tx_state = TINBUS_TX_IDLE;
//     //     SET_COMPARE_A(GET_COMPARE_A() + TINBUS_PERIOD_TICKS);
//     //     CONFIG_MATCH_SET();
//     //     DISABLE_COMPARE_A_INTERRUPT();
//     //     // DISABLE_COMPARE_A_OUTPUT();
//     //     // tx collision call back
//     //     tinbus_callback(TINBUS_TX_COLLISION);
//     // }
//     // if (tinbus_tx_state == TINBUS_TX_PULSE_SENT) {
//     //     tinbus_tx_state = TINBUS_TX_PULSE_RECEIVED;
//     // }

//     PORTB &= ~(1 << PORTB2);
// }

// ISR(COMPARE_B_INTERRUPT) {
//     if (GET_COMPARE_B() == capture + TINBUS_PERIOD_TICKS) {
//         // this is a capture blanking timeout
//         SET_COMPARE_B(capture + (TINBUS_PERIOD_TICKS * 8));
//     } else {
//         // receive frame break timeout
//         PINB |= (1 << PORTB2);
//         DISABLE_COMPARE_B_INTERRUPT();

//         if (tinbus_rx_bit_mask != 0x01) {
//             tinbus_rx_bit_mask = 0x01;
//             tinbus_callback(TINBUS_RX_FRAME_ERROR, &tinbus_rx_frame);
//         } else {
//             tinbus_callback(TINBUS_RX_COMPLETE, &tinbus_rx_frame);
//         }
//         tinbus_rx_frame.size = 0;
//         PORTB &= ~(1 << PORTB2);
//     }
//     ENABLE_CAPTURE_INTERRUPT();
// }

// void tinbus_init(tinbus_callback_t callback) {
//     DDRB |= (1 << PORTB2); // debug output enable

//     tinbus_callback = callback;

//     CONFIG_TIMER_NOPRESCALE();

//     ENABLE_COMPARE_A_OUTPUT();
//     ENABLE_COMPARE_A_INTERRUPT();

//     // ENABLE_COMPARE_B_INTERRUPT();
//     ENABLE_CAPTURE_INTERRUPT();
//     SET_CAPTURE_FALLING_EDGE();
// }

// // uint8_t tinbus_read(tinbus_frame_t *frame) {}

// uint8_t tinbus_write(tinbus_frame_t *frame) {
//     // PINB |= (1 << PORTB2);

//     if (IS_COMPARE_A_INTERRUPT_ENABLED()) {
//         return TINBUS_TX_BUSY;
//     }

//     memcpy(&tinbus_tx_frame, frame, sizeof(tinbus_frame_t));
//     tinbus_tx_index = 0;
//     tinbus_tx_bit_mask = 0b10000000;
//     tinbus_tx_state = TINBUS_TX_IDLE;

//     // ENABLE_COMPARE_A_OUTPUT();
//     CONFIG_MATCH_CLEAR();
//     SET_COMPARE_A(GET_TIMER_COUNT() + 16);
//     ENABLE_COMPARE_A_INTERRUPT();

//     // PORTB &= ~(1 << PORTB2);

//     return TINBUS_OK;
// }
