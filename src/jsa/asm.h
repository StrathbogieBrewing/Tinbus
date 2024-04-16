/**
    \file asm.h

    ADC and UART functions.
*/

#ifndef ASM_H
#define ASM_H

#define OCR1ALMAXPWM                1
#define OCR1BLMAXPWM                1
#define ICR1LMAXPWM                 (OCR1BLMAXPWM + 2)

#define OCR1ALMINPWM                1
#define OCR1BLMINPWM                20
#define ICR1LMINPWM                 (OCR1BLMINPWM + 2)

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stdbool.h>

#define ASM_ADC_IDLE (ICR1L == ICR1LMINPWM)

// **** assembly ADC functions **** //

// initialise all assembly based modules ADC / DCDC / UART
void asm_init(void);

// **** assembly ADC functions **** //

// get raw ADC result
int16_t asm_ADC_getRawResult(void);

// do ADC data decode. Return False if no decode was available.
uint8_t asm_ADC_doConversion(void);

// returns true is successful
bool asm_TX_pulse(void);

// returns true is successful
bool asm_RX_wait(uint8_t ticks);

#endif // __ASSEMBLER__

#endif // ASM_H
