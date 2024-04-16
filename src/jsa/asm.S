// ADC interrupt routine

#include <avr/io.h>

#include "asm.h"

#define kTXBufferSize               12
#define kRXBufferSize               12
#define kADCBufferSize              24
#define kTimeOutCharacters          2
#define kNumberOfADCBits            18
#define kFirstPeriodMin             (13 - 2)
#define kFirstPeriodMax             (13 + 2)

#define kADCMaxValue                0x7FFF    //+32767
#define kADCMinValue                0x8001    //-32767 (avoid problems with -32768)

#define DEBUGPIN                    PORTB2

#define FIVEQUARTERBITS             0x41
#define SEVENQUARTERBITS            0x59

// declare memory for the adc received data and uart buffers
// in .bss section (initialised to zero by c startup code)
// changing .lcomm (local) to .comm makes these appear in the
// map file in the COMMON section.
// BUT .comm values are not ordered as they appear here!
// .global directive makes the symbols visible in map file.
// on check, linker appears to assign lcomm symbolx at ascending addresses

    .lcomm ADCByteCount,    1
    .lcomm ADCBuffer,       (kADCBufferSize)
    .lcomm ADCResultHigh    1
    .lcomm ADCResultLow     1

    .section .text
    .global ANA_COMP_vect
    .global asm_init
    .global asm_ADC_doConversion
    .global asm_ADC_getRawResult

// analogue comparator interrupt
// used to read the ADC serial bit stream
ANA_COMP_vect:
    // push used registers onto stack
    push    r24
    in      r24, _SFR_IO_ADDR(SREG)
    push    r24
    push    r25
    push    ZL
    push    YL

ADCService:
    sbi     _SFR_IO_ADDR(ACSR), ACI     // clear comparator flag
    rcall   setMaxPWM                   // run max PWM for DCDC during data reception
    clr     r25                         // clear counter
    out     _SFR_IO_ADDR(TCNT0), r25
    in      r24, _SFR_IO_ADDR(TIFR)     // clear counter overflow flag
    ori     r24, (1 << TOV0)
    out     _SFR_IO_ADDR(TIFR), r24
    ldi     YL, ADCBuffer               // set up buffer index

    cbi     _SFR_IO_ADDR(PORTD), PORTD2 // set bus to recessive

ADCIdleLoop:
    sbic    _SFR_IO_ADDR(ACSR), ACI     // is comparator interrupt triggered
    rjmp    ADCNewEdge
    in      r24, _SFR_IO_ADDR(TIFR)   // test for time out
    sbrs    r24, TOV0
    rjmp    ADCIdleLoop
    rjmp    ADCTimeOut

ADCNewEdge:
    in      ZL, _SFR_IO_ADDR(TCNT0)     // save counter value
    clr     r25
    out     _SFR_IO_ADDR(TCNT0), r25    // reset counter
    sbi     _SFR_IO_ADDR(ACSR), ACI     // clear  comparator flag
    st      Y+, ZL                      // store data timer value
    //rcall    uart_service             // poll serial comms
    rjmp    ADCIdleLoop

ADCTimeOut:
    // save number of bytes in buffer
    clr     r25
    st      Y, r25
    subi    YL, ADCBuffer
    sts     ADCByteCount, YL

setMinPWM:
    // reduce power to ADC once data received
    ldi     r24, OCR1BLMINPWM
    out     _SFR_IO_ADDR(OCR1BL), r24
    ldi     r24, ICR1LMINPWM
    out     _SFR_IO_ADDR(ICR1L), r24
    clr     r25
    ldi     r24, (ICR1LMINPWM - 1)
    out     _SFR_IO_ADDR(TCNT1H), r25
    out     _SFR_IO_ADDR(TCNT1L), r24
    sbi     _SFR_IO_ADDR(ACSR), ACI     // clear  comparator flag

exitISR:
//        sbi        _SFR_IO_ADDR(PORTD), PORTD6

    // pop registers back from stack
    pop     YL
    pop     ZL
    pop     r25
    pop     r24
    out     _SFR_IO_ADDR(SREG), r24
    pop     r24
    reti

asm_ADC_doConversion:
    clr     r25                         // calling convention requires clearing r25, but we know that is not necessary here
    clr     r24

    cli                                 // don't let interrupt set ADCByteCount between test and clear
    lds      r22, ADCByteCount          // check that new data available
    sts      ADCByteCount, r25          // always clear byte count. If it's set to anything interupt has finished and updated the buffer.
    sei

    cp      r22, r24
    brne    asm_ADC_decode              // decode returns with r24 set (unless there was a problem)
    ret                                 // return with r24 cleared

// on entry r20 undefined, r21 undefined, r22 is ADCByteCount, r24 is cleared. r25 is undefined,

// in calculation r20 is bit counter, r21 is timer data value, r22 is end-of-data pointer
// after calculate r24 is ADCResultLow, r25 is ADCResultHigh, r23 is high bits 16,17,18
// on exit r25:r24 is set to true.

asm_ADC_decode:

    ldi     ZL, (ADCBuffer + 1)         // set up indirect addressing
    clr     r20                         // initialise bit counter
    add     r22, ZL                     // initialise end-of-data pointer to byte after last data point

asm_ADC_testLastBitZero:
    cp      ZL, r22                     // has the last byte been processed
    brsh    asm_ADC_decodeDone
    ld      r21, Z+                     // get measurement

asm_ADC_testTwoHalfBitsLastBitZero:
    cpi     r21, FIVEQUARTERBITS
    brsh    asm_ADC_testThreeHalfBitsLastBitZero
    rcall   asm_ADC_putZero
    rjmp    asm_ADC_testLastBitZero

asm_ADC_testThreeHalfBitsLastBitZero:
    cpi     r21, SEVENQUARTERBITS
    brsh    asm_ADC_testFourHalfBitsLastBitZero
    rcall   asm_ADC_putOne
    rcall   asm_ADC_putOne
    rjmp    asm_ADC_testLastBitOne

asm_ADC_testFourHalfBitsLastBitZero:
    rcall   asm_ADC_putOne
    rcall   asm_ADC_putZero
    rjmp    asm_ADC_testLastBitZero

asm_ADC_testLastBitOne:
    cp      ZL, r22                     // has the last byte been processed
    brsh    asm_ADC_decodeDone
    ld      r21, Z+                     // get measurement

asm_ADC_testTwoHalfBitsLastBitOne:
    cpi     r21, FIVEQUARTERBITS
    brsh    asm_ADC_testThreeHalfBitsLastBitOne
    rcall   asm_ADC_putOne              //rotate left, add 1
    rjmp    asm_ADC_testLastBitOne

asm_ADC_testThreeHalfBitsLastBitOne:
    rcall   asm_ADC_putZero             //rotate left, stuff zero bit in
    rjmp    asm_ADC_testLastBitZero

asm_ADC_decodeDone:
    lsr     r23                         //bits 16,17,18    //load sign bit (16) into carry bit
    ror     r25                         //high byte        //load sign bit (16) into sign bit (15)
    ror     r24                         //low byte
    cpi     r20, kNumberOfADCBits
    breq    asm_ADC_rangeCheck
    rcall   asm_ADC_putOne              // rotate left, stuff an extra bit

asm_ADC_rangeCheck:
    sbrc    r23, 0                      //sign bit (b16)
    rjmp    asm_ADC_SIGSet          
    sbrc    r25, 7                      //high bit (b15)
    rjmp    asm_ADC_check_corrupt
    ldi     r24, lo8(kADCMinValue)      // limit 16+sign value to 16 bit
    ldi     r25, hi8(kADCMinValue)      // limit 16+sign value to 16 bit
    rjmp    asm_ADC_check_corrupt

asm_ADC_SIGSet:
    sbrs    r25, 7                      //high bti (b15)
    rjmp    asm_ADC_check_corrupt
    ldi     r24, lo8(kADCMaxValue)      // limit 16+sign value to 16 bit
    ldi     r25, hi8(kADCMaxValue)      // limit 16+sign value to 16 bit
    rjmp    asm_ADC_check_corrupt

asm_ADC_check_corrupt:
    clr     r20
    lds     r21, ADCByteCount           // check if buffer has been overwritten during decode
    cp      r21, r20
//    brne    asm_ADC_getRawResult      // go back and do it again if buffer was corrupted during decode
    brne    asm_ADC_discardResult

asm_ADC_saveResult:
    sts     ADCResultHigh, r25
    sts     ADCResultLow, r24
    ldi     r24, 1
    rjmp    asm_ADC_decode_exit

asm_ADC_discardResult:
    clr     r24

asm_ADC_decode_exit:
    clr     r25
    ret

//---- asm adc decode subroutine ----

asm_ADC_putOne:
    sec                                 // set carry flag
    rjmp    asm_ADC_shift
asm_ADC_putZero:
    clc                                 // clear carry flag
asm_ADC_shift:                          // rotate bit into data bytes
    rol     r24                         // low byte, also signal byte on exit
    rol     r25                         // high byte, also cleared on exit
    rol     r23                         // bits 16,17,18
    inc     r20                         // bit counter
    ret

//---- end of ADC decode ----

// value is updated by doConversion, polled in main program, not by interupt, so this may be safely
// interupted. (However, doConversion is not protected, but it is expected that conversion is
// always completed correctly before invalid data can happen)
asm_ADC_getRawResult:
    lds     r24, ADCResultLow
    lds     r25, ADCResultHigh
    ret


// called to initialise the ADC / UART / DCDC assembly based modules
// void asm_init(void)
asm_init:
    // *** set system clock prescaler ***
    // Set clock prescaler to divide by 4
    // with 3.6864 MHz clock this give a system clk of 921.6 kHz
    ldi     r24, (1 << CLKPCE)
    out     _SFR_IO_ADDR(CLKPR), r24
    ldi     r24, (1 << CLKPS1)
    out     _SFR_IO_ADDR(CLKPR), r24

    // *** enable analogue comparator ***

    // enable analogue comparator, interrupt on negative edge and
    ldi     r24, (1 << ACI) | (1 << ACIE) | (1 << ACIS1)
    out     _SFR_IO_ADDR(ACSR), r24

    // disable the digital input circuitry
    sbi     _SFR_IO_ADDR(DIDR), AIN0D
    sbi     _SFR_IO_ADDR(DIDR), AIN1D


    // *** enable timer 0 ***

    // set up timer 0, no prescaling
    ldi     r24, (1 << CS00)
    out     _SFR_IO_ADDR(TCCR0B), r24

    // let the rat race begin....
    sei

    // *** enable pwm ***
    sbi     _SFR_IO_ADDR(DDRB), PORTB3  // enable pwm output
    sbi     _SFR_IO_ADDR(DDRB), PORTB4

    ldi     r24, (1 << COM1A0) | (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11)
    out     _SFR_IO_ADDR(TCCR1A), r24   // fast pwm mode and invert one channel

    // no prescaling on counter timer 1
    ldi     r24, (1 << CS10) | (1 << WGM12) | (1 << WGM13)
    out     _SFR_IO_ADDR(TCCR1B), r24

setMaxPWM:
    // set frequency of output to maximum 50 % duty cycle
    ldi     r24, OCR1ALMAXPWM
    out     _SFR_IO_ADDR(OCR1AL), r24
    out     _SFR_IO_ADDR(OCR1BL), r24

    // set counter top value
    ldi     r24, ICR1LMAXPWM
    out     _SFR_IO_ADDR(ICR1L), r24

    // sychronise the counter
    clr     r25
    ldi     r24, (ICR1LMAXPWM - 1)
    out     _SFR_IO_ADDR(TCNT1H), r25
    out     _SFR_IO_ADDR(TCNT1L), r24

    ret
