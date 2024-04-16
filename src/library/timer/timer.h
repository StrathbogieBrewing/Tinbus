#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <avr/io.h>

#define TIMER_US_PER_TICK ((256UL * 1000000UL) / F_CPU)

#define TIMER_TICK_FREQ_HZ (F_CPU / 256ULL)
#define TIMER_US_TO_TICKS(us) ((us * TIMER_TICK_FREQ_HZ) / 1000000ULL)

extern volatile uint32_t timer_overflows;

void timer_init(void);

uint32_t timer_get_ticks(void);

inline uint32_t timer_get_ticks_from_isr(void) __attribute__((always_inline));
inline uint32_t timer_get_ticks_from_isr(void)
{
    uint32_t overflows = timer_overflows;
    uint8_t tcnt0 = TCNT0;
    if ((TIFR0 & (1 << TOV0)) && (tcnt0 < 255)){
        overflows++;
    }
    return (overflows << 8) + (uint32_t)tcnt0;
}

#ifdef __cplusplus
}
#endif

#endif /* TIMER_H */
