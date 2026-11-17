/* Pwm.c - Timer2 fast PWM, TOP = OCR2A (mode 7), output on OC2B = PD3 = Arduino D3 */
#include <avr/io.h>
#include "Pwm.h"
#include "Det.h"

#define PWM_TIMER_HZ  (F_CPU / 32UL)                                  /* prescaler 32 -> 500 kHz */
#define PWM_TOP       ((uint8)((PWM_TIMER_HZ / PWM_BUZZER_FREQ_HZ) - 1UL))   /* 124 -> 4.00 kHz */

static boolean Pwm_Initialized = FALSE;

void Pwm_Init(void)
{
    TCCR2A = (uint8)((1U << WGM21) | (1U << WGM20));     /* fast PWM, output disconnected (pin = PORT level, low) */
    TCCR2B = (uint8)((1U << WGM22) | (1U << CS21) | (1U << CS20));   /* TOP = OCR2A, clk/32 */
    OCR2A = PWM_TOP;
    OCR2B = 0U;
    Pwm_Initialized = TRUE;
}

void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, uint16 DutyCycle)
{
    if (Pwm_Initialized == FALSE)
    {
        (void)Det_ReportError(PWM_MODULE_ID, 0U, PWM_SID_SET_DUTY_CYCLE, PWM_E_UNINIT);
        return;
    }
    if (ChannelNumber != PwmConf_PwmChannel_Buzzer)
    {
        (void)Det_ReportError(PWM_MODULE_ID, 0U, PWM_SID_SET_DUTY_CYCLE, PWM_E_PARAM_CHANNEL);
        return;
    }
    if (DutyCycle > PWM_DUTY_100)
    {
        DutyCycle = PWM_DUTY_100;
    }
    if (DutyCycle == PWM_DUTY_0)
    {
        /* 0 %: disconnect OC2B, the pin goes back to its Port level (LOW) - no 1-cycle spikes */
        TCCR2A &= (uint8)~((1U << COM2B1) | (1U << COM2B0));
    }
    else
    {
        OCR2B = (uint8)(((uint32)DutyCycle * ((uint32)PWM_TOP + 1UL)) / 0x8000UL);
        TCCR2A |= (uint8)(1U << COM2B1);                 /* non-inverting output on OC2B */
    }
}
