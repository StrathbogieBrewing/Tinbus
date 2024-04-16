#include <string.h>

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "asm.h"
#include "mb_crc.h"

#define RX_IS_RELEASED (PIND & (1 << PORTD0))

#define TX_ACTIVE (PORTD |= (1 << PORTD2))
#define TX_RELEASE (PORTD &= ~(1 << PORTD2))
#define TX_IS_ACTIVE (PORTD & (1 << PORTD2))

#define TICK 6
#define MESSAGE_SIZE 5

uint8_t getSwitch(void) {
    return (0x0F & (((uint8_t)(~PIND)) >> 3)); // read adress from rotory switch
}

bool rx_wait(uint8_t ticks) {
    TCNT0 = 0;
    TIFR |= (1 << TOV0);
    while ((TIFR & (1 << TOV0)) == 0) {
        if (!RX_IS_RELEASED) {
            return false;
        }
    }
    TIFR |= (1 << TOV0);
    while ((TIFR & (1 << TOV0)) == 0) {
        if (!RX_IS_RELEASED) {
            return false;
        }
    }
    return true;
}

// returns true if successful
bool tx_pulse_zero(void) {
    if (RX_IS_RELEASED) {
        TX_ACTIVE;
        TCNT0 = 0;
        while (TCNT0 < 40) {
        }
        if (!TX_IS_ACTIVE) {
            return false;
        }
        TX_RELEASE;
        while (TCNT0 < 80) {
        }
        while (TCNT0 < 98) {
            if (!RX_IS_RELEASED) {
                return false;
            }
        }
        return true;
    }
    return false;
}

// returns true if successful
bool tx_pulse_one(void) {
    if (RX_IS_RELEASED) {
        TX_ACTIVE;
        TCNT0 = 0;
        while (TCNT0 < 40) {
        }
        if (!TX_IS_ACTIVE) {
            return false;
        }
        TX_RELEASE;
        while (TCNT0 < 80) {
        }
        while (TCNT0 < 236) {
            if (!RX_IS_RELEASED) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool send(uint8_t data[], uint8_t bytes_to_send) {
    if (rx_wait(TICK * 12)) {
            while (bytes_to_send--) {
                uint8_t mask = 0x80;
                while (mask) {
                    if (mask & *data) {
                        if(!tx_pulse_one()){
                           return false; 
                        }
                    } else {
                        if(!tx_pulse_zero()){
                            return false; 
                        }
                    }
                    mask >>= 1;
                }
                data++;
            }
            return tx_pulse_one();
    }
    return false;
}

int main(void) {
    wdt_enable(WDTO_250MS);
    asm_init();

    DDRD |= (1 << PORTD1); // init bus outputs
    DDRD |= (1 << PORTD2);

    DDRB |= (1 << PORTB2); // debug

    while (1) {
        wdt_reset();
        if (asm_ADC_doConversion()) // we now have about 150 ms before next interrupt
        {
            int16_t value = asm_ADC_getRawResult();

            uint8_t id = getSwitch();
            uint8_t buffer[MESSAGE_SIZE];
            buffer[0] = id;
            buffer[1] = value >> 8;
            buffer[2] = value;

            uint16_t crc = mb_crc(0xFFFF, buffer[0]);
            crc = mb_crc(crc, buffer[1]);
            crc = mb_crc(crc, buffer[2]);
            buffer[3] = crc;
            buffer[4] = crc >> 8;

            uint8_t tries = 0;
            while(tries < 5){
                if(send(buffer, MESSAGE_SIZE)){
                    break;
                }
                tries++;
                uint8_t i = tries + id;
                while(i--){
                    _delay_ms(1);
                }
            }
        }
    }

    return 0;
}
