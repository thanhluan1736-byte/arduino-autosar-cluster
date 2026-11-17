/* Pwm.h - AUTOSAR Pwm driver: one channel on Timer2 / OC2B (Arduino D3), fixed period */
#ifndef PWM_H
#define PWM_H

#include "Std_Types.h"

#define PWM_MODULE_ID                121U
#define PWM_SID_SET_DUTY_CYCLE       0x02U
#define PWM_E_UNINIT                 0x11U
#define PWM_E_PARAM_CHANNEL          0x12U

typedef uint8 Pwm_ChannelType;

/* AUTOSAR duty cycle scale: 0x0000 = 0 %, 0x8000 = 100 % */
#define PWM_DUTY_0                   0x0000U
#define PWM_DUTY_50                  0x4000U
#define PWM_DUTY_100                 0x8000U

/* buzzer channel: a passive buzzer needs a square wave, a steady HIGH only clicks */
#define PwmConf_PwmChannel_Buzzer    0U
#define PWM_BUZZER_FREQ_HZ           4000UL   /* high, sharp beep (countdown-timer style) */

void Pwm_Init(void);
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, uint16 DutyCycle);

#endif /* PWM_H */
