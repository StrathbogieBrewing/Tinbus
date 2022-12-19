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
    if (frame->length < TINBUS_BUFFER_SIZE) {
        uint8_t index = 0;
        while (index < frame->length) {
            dest_ptr = tinbus_dump_binary(dest_ptr, frame->buffer[index]);
            index++;
            if (index < frame->length) {
                *dest_ptr = ' '; // add a space between bytes
                dest_ptr++;
            }
        }
    }
    *dest_ptr = '\0'; // terminate string with a null
}

// if time_us is zero then a timeout occurred,
// otherwise time_us is time between last event and this edge event
void tinbus_receive_edge_cb(uint16_t time_us) {}

// #define PULSE_LENGTH_US (20)

#define TINBUS_PERIOD_TICKS (10 * 16)

#define INPUT_CAPTURE_PIN 8     // receive
#define OUTPUT_COMPARE_A_PIN 9  // transmit
#define OUTPUT_COMPARE_B_PIN 10 // unusable PWM

#define CONFIG_TIMER_NOPRESCALE() (TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1 << ICNC1) | (1 << CS10))
#define CONFIG_TIMER_PRESCALE_8() (TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1 << ICNC1) | (1 << CS11))
#define CONFIG_TIMER_PRESCALE_256() (TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1 << ICNC1) | (1 << CS12))
#define CONFIG_MATCH_NORMAL() (TCCR1A = TCCR1A & ~((1 << COM1A1) | (1 << COM1A0)))
#define CONFIG_MATCH_TOGGLE() (TCCR1A = (TCCR1A & ~(1 << COM1A1)) | (1 << COM1A0))
#define CONFIG_MATCH_CLEAR() (TCCR1A = (TCCR1A | (1 << COM1A1)) & ~(1 << COM1A0))

#define IS_MATCH_SET_ENABLED() (TCCR1A & (1 << COM1A0))
#define CONFIG_MATCH_SET() (TCCR1A = TCCR1A | ((1 << COM1A1) | (1 << COM1A0)))
#define CONFIG_CAPTURE_FALLING_EDGE() (TCCR1B &= ~(1 << ICES1))
#define CONFIG_CAPTURE_RISING_EDGE() (TCCR1B |= (1 << ICES1))
#define ENABLE_INT_INPUT_CAPTURE() (TCCR1B |= (1 << ICNC1), TIFR1 |= (1 << ICF1), TIMSK1 |= (1 << ICIE1))
#define ENABLE_COMPARE_A_INTERRUPT() (TIFR1 = (1 << OCF1A), TIMSK1 |= (1 << OCIE1A))
#define ENABLE_COMPARE_B_INTERRUPT() (TIFR1 = (1 << OCF1B), TIMSK1 |= (1 << OCIE1B))
#define IS_COMPARE_B_INTERRUPT_ENABLED() (TIMSK1 & (1 << OCIE1B)
#define DISABLE_CAPTURE_INTERRUPT() (TIMSK1 &= ~(1 << ICIE1))
#define DISABLE_COMPARE_A_INTERRUPT() (TIMSK1 &= ~(1 << OCIE1A))
#define DISABLE_COMPARE_B_INTERRUPT() (TIMSK1 &= ~(1 << OCIE1B))
#define GET_TIMER_COUNT() (TCNT1)
#define GET_CAPTURE_COUNT() (ICR1)
#define IS_CAPTURE_FLAG_SET() (TIFR1 & (1 << ICF1))
#define CLEAR_CAPTURE_FLAG() (TIFR1 |= (1 << ICF1))
#define GET_COMPARE_A() (OCR1A)
#define GET_COMPARE_B() (OCR1B)
#define SET_COMPARE_A(val) (OCR1A = (val))
#define SET_COMPARE_B(val) (OCR1B = (val))
#define CAPTURE_INTERRUPT TIMER1_CAPT_vect
#define COMPARE_A_INTERRUPT TIMER1_COMPA_vect
#define COMPARE_B_INTERRUPT TIMER1_COMPB_vect

static volatile uint16_t capture = 0;
static tinbus_frame_t tx_frame = {0};
static volatile uint8_t tx_index = 0;
static volatile uint8_t tx_bit_mask = 0;

static tinbus_callback_t tinbus_callback = 0;

// ISR(CAPTURE_INTERRUPT) { // for receiving leading edge of pulse
//     ENABLE_COMPARE_B_INTERRUPT();
//     DISABLE_CAPTURE_INTERRUPT();

//     capture = GET_CAPTURE_COUNT();
//     SET_COMPARE_B(capture + TINBUS_PERIOD_TICKS);
// }

ISR(COMPARE_A_INTERRUPT) {
    if (IS_MATCH_SET_ENABLED()) {
        // trailing edge, set up next bit
        if (tx_index != tx_frame.length) {
            if ((tx_bit_mask & tx_frame.buffer[tx_index]) != 0) {
                SET_COMPARE_A(GET_COMPARE_A() + TINBUS_PERIOD_TICKS * 5);
            } else {
                SET_COMPARE_A(GET_COMPARE_A() + TINBUS_PERIOD_TICKS * 2);
            }
            CONFIG_MATCH_CLEAR();
            tx_bit_mask >>= 1;
            if (tx_bit_mask == 0) {
                tx_bit_mask = 0b10000000;
                tx_index += 1;
            }
        } else {
            SET_COMPARE_A(GET_COMPARE_A() + TINBUS_PERIOD_TICKS);
            CONFIG_MATCH_SET();
            DISABLE_COMPARE_A_INTERRUPT();
            // tx done call back
            tinbus_callback(TINBUS_TX_COMPLETE);
        }
    } else {
        // leading edge, restore bus to recessive mode
        SET_COMPARE_A(GET_COMPARE_A() + TINBUS_PERIOD_TICKS);
        CONFIG_MATCH_SET();
    }
}

// ISR(COMPARE_B_INTERRUPT) {
//     ENABLE_INT_INPUT_CAPTURE();
//     if (GET_COMPARE_B() == capture + TINBUS_PERIOD_TICKS) {
//         // capture blanking timeout
//         SET_COMPARE_B(capture + TINBUS_PERIOD_TICKS * 8);
//     } else {
//         // receive frame break timeout
//         DISABLE_COMPARE_B_INTERRUPT();
//     }
// }

// schedule a transmit pulse at time_us from the last event
// bool tinbus_transmit_pulse(void) {

//     noInterrupts();
//     PORTB |= (1 << PORTB2);

//     uint16_t time_now = GET_TIMER_COUNT();
//     CONFIG_MATCH_CLEAR();
//     SET_COMPARE_A(time_now + 2);
//     // while (GET_TIMER_COUNT() <= GET_COMPARE_A())
//     //     ;
//     // CONFIG_MATCH_SET();
//     // SET_COMPARE_A(time_now + 2 + TINBUS_PERIOD_TICKS);

//     // CLEAR_CAPTURE_FLAG();

//     PORTB &= ~(1 << PORTB2);
//     interrupts();

//     return true;
// }

void tinbus_init(tinbus_callback_t callback) {
    tinbus_callback = callback;

    DDRB |= (1 << PORTB1);
    DDRB |= (1 << PORTB2); // debug output

    CONFIG_TIMER_NOPRESCALE();
    ENABLE_COMPARE_A_INTERRUPT();

    // ENABLE_INT_INPUT_CAPTURE();
    // CONFIG_CAPTURE_FALLING_EDGE();
}

// uint8_t tinbus_read(tinbus_frame_t *frame) {}

uint8_t tinbus_write(tinbus_frame_t *frame) {
    PINB |= (1 << PORTB2);

    memcpy(&tx_frame, frame, sizeof(tinbus_frame_t));
    tx_index = 0;
    tx_bit_mask = 0b10000000;

    cli();
    CONFIG_MATCH_CLEAR();
    SET_COMPARE_A(GET_TIMER_COUNT() + 16);
    ENABLE_COMPARE_A_INTERRUPT();
    sei();
    
    PORTB &= ~(1 << PORTB2);

    return 0;
}
