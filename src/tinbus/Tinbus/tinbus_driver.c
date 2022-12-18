#include "Arduino.h"

#include "tinbus_driver.h"

#define TINBUS_PERIOD_TICKS (40)

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

#define PULSE_LENGTH_US (20)

#define INPUT_CAPTURE_PIN 8     // receive
#define OUTPUT_COMPARE_A_PIN 9  // transmit
#define OUTPUT_COMPARE_B_PIN 10 // unusable PWM

#define CONFIG_TIMER_NOPRESCALE() (TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1 << ICNC1) | (1 << CS10))
#define CONFIG_TIMER_PRESCALE_8() (TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1 << ICNC1) | (1 << CS11))
#define CONFIG_TIMER_PRESCALE_256() (TIMSK1 = 0, TCCR1A = 0, TCCR1B = (1 << ICNC1) | (1 << CS12))
#define CONFIG_MATCH_NORMAL() (TCCR1A = TCCR1A & ~((1 << COM1A1) | (1 << COM1A0)))
#define CONFIG_MATCH_TOGGLE() (TCCR1A = (TCCR1A & ~(1 << COM1A1)) | (1 << COM1A0))
#define CONFIG_MATCH_CLEAR() (TCCR1A = (TCCR1A | (1 << COM1A1)) & ~(1 << COM1A0))
#define CONFIG_MATCH_SET() (TCCR1A = TCCR1A | ((1 << COM1A1) | (1 << COM1A0)))
#define CONFIG_CAPTURE_FALLING_EDGE() (TCCR1B &= ~(1 << ICES1))
#define CONFIG_CAPTURE_RISING_EDGE() (TCCR1B |= (1 << ICES1))
#define ENABLE_INT_INPUT_CAPTURE() (TIFR1 = (1 << ICF1), TIMSK1 = (1 << ICIE1))
#define ENABLE_INT_COMPARE_A() (TIFR1 = (1 << OCF1A), TIMSK1 |= (1 << OCIE1A))
#define ENABLE_INT_COMPARE_B() (TIFR1 = (1 << OCF1B), TIMSK1 |= (1 << OCIE1B))
#define DISABLE_INT_INPUT_CAPTURE() (TIMSK1 &= ~(1 << ICIE1))
#define DISABLE_INT_COMPARE_A() (TIMSK1 &= ~(1 << OCIE1A))
#define DISABLE_INT_COMPARE_B() (TIMSK1 &= ~(1 << OCIE1B))
#define GET_TIMER_COUNT() (TCNT1)
#define GET_INPUT_CAPTURE() (ICR1)
#define GET_COMPARE_A() (OCR1A)
#define GET_COMPARE_B() (OCR1B)
#define SET_COMPARE_A(val) (OCR1A = (val))
#define SET_COMPARE_B(val) (OCR1B = (val))
#define CAPTURE_INTERRUPT TIMER1_CAPT_vect
#define COMPARE_A_INTERRUPT TIMER1_COMPA_vect
#define COMPARE_B_INTERRUPT TIMER1_COMPB_vect

// ISR(TIMER1_OVF_vect) {
//     // PORTB ^= (1 << PORTB2);
//     // OCR1A = 0x03FF;
//     // TIMSK0 &= ~(1 << TOIE1); // disable timer overflow interrupt
//     // TCCR1A &= ~(1 << COM1A1);
//     // TCCR1A &= ~(1 << COM1A0);
//     TCCR1A &=  ~(1 << COM1A0);
//     TIMSK1 &= ~(1 << TOIE1); // disable timer overflow interrupt
//     TCCR1A |=  (1 << FOC1A);
// }

// { return TIFR & (1 << ICF1); }

// ISR(TIMER1_COMPA_vect) {
//     // PORTB &= ~(1 << PORTB2);

//     // CONFIG_MATCH_CLEAR();
//     // SET_COMPARE_A(GET_COMPARE_A() + TINBUS_PERIOD_TICKS);
// }

// ISR(TIMER1_CAPT_vect) { // for receiving pulses
//     // uint16_t x = ICR1 - OCR1B;
//     // OCR1A = ICR1;
//     // OCR1B = ICR1; // dodgy! used to store capture value
// }

// schedule a transmit pulse at time_us from the last event
void tinbus_transmit_pulse(void) {

    noInterrupts();
    PORTB |= (1 << PORTB2);

    uint16_t time_now = GET_TIMER_COUNT();
    CONFIG_MATCH_SET();
    SET_COMPARE_A(time_now + 2);
    while (GET_TIMER_COUNT() <= GET_COMPARE_A())
        ;

    CONFIG_MATCH_CLEAR();
    SET_COMPARE_A(time_now + 2 + TINBUS_PERIOD_TICKS);

    PORTB &= ~(1 << PORTB2);
    interrupts();

    return true;
}

void tinbus_init(void) {
    DDRB |= (1 << PORTB1);
    DDRB |= (1 << PORTB2); // debug output

    TCCR1A = 0;
    TCCR1B = (1 << CS11);

    // ENABLE_INT_COMPARE_A();

    // TIMSK1 |= (1 << OCIE1A); // enable timer interrupt
    TCCR1A = (1 << COM1A0);  //(1 << WGM10) | (1 << WGM11);
}

uint8_t tinbus_read(tinbus_frame_t *frame) {}

uint8_t tinbus_write(tinbus_frame_t *frame) {
    if (frame->length < TINBUS_BUFFER_SIZE) {
        cli();
        uint8_t index = 0;
        tinbus_transmit_pulse();
        while (index < frame->length) {
            uint8_t byte = frame->buffer[index];
            index++;
            uint8_t bit_mask = 0b10000000;
            while (bit_mask) {
                if (bit_mask & byte) {
                    while (TCNT1 < TINBUS_PERIOD_TICKS * 6) // wait 6T
                    {
                        ;
                    }
                    tinbus_transmit_pulse();
                } else {

                    while (TCNT1 < TINBUS_PERIOD_TICKS * 3) // wait 3T
                    {
                        ;
                    }
                    tinbus_transmit_pulse();
                }
                bit_mask >>= 1;
            }
        }
        while (TCNT1 < TINBUS_PERIOD_TICKS * 12) // wait 12T
        {
            ;
        }
        sei();
    }
}
