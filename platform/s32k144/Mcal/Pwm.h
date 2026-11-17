/* Pwm.h - AUTOSAR Pwm driver (S32K144): one 50 % square-wave channel for the buzzer.
 * The wave is made by LPIT0 channel 1 toggling the buzzer pin, so any GPIO of the J8 header can be used. */
#ifndef PWM_H
#define PWM_H

#include "Std_Types.h"

#define PWM_MODULE_ID                121U
#define PWM_SID_SET_DUTY_CYCLE       0x02U
#define PWM_E_UNINIT                 0x11U
#define PWM_E_PARAM_CHANNEL          0x12U

typedef uint8 Pwm_ChannelType;

/* AUTOSAR duty cycle scale: 0x0000 = 0 %, 0x8000 = 100 %. Any value > 0 gives the 50 % tone. */
#define PWM_DUTY_0                   0x0000U
#define PWM_DUTY_50                  0x4000U
#define PWM_DUTY_100                 0x8000U

#define PwmConf_PwmChannel_Buzzer    0U
#define PWM_BUZZER_FREQ_HZ           4000UL   /* same high beep as the Arduino version */

void Pwm_Init(void);
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, uint16 DutyCycle);

#endif /* PWM_H */
