#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "millis.h"

volatile static uint32_t timerMillis;

// CPU frequency in Hz
#define F_CPU 3333333   // 20MHz/6 (CPU Clock prescalar=6)
//#define PERIOD_VALUE 0xC  // Overflow after 1 ms = 0.001s, (0.001 * F_CPU / 256) - 1, (TCA0 prescalar=256)
#define PERIOD_VALUE 0xD3
// Initializes the use of the timer functions by setting up the TCA timer.
void initTimer()
{
    /* enable overflow interrupt */
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;

    /* set Normal mode */
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;

    /* disable event counting */
    TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);

    /* set the period */
    TCA0.SINGLE.PER = PERIOD_VALUE;

    /* set clock source (sys_clk/256) */
    //TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm; 
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm; 

}

// TCA overflow handler, called every millisecond.
ISR(TCA0_OVF_vect)
{
    timerMillis++;
    // Clear the interrupt flag (to reset TCA0.CNT)
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

// Gets the milliseconds of the current time.
uint32_t millis()
{
    uint32_t m;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        m = timerMillis;
    }

    return m;
}
