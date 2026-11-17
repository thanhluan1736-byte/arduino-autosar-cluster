/* IoHwAb.h - I/O hardware abstraction: physical values for the SWCs, no register access above this layer */
#ifndef IOHWAB_H
#define IOHWAB_H

#include "Std_Types.h"
#include "Adc.h"

/* joystick calibration, scaled to the ADC resolution of the platform (10 bit AVR, 12 bit S32K) */
#define IOHWAB_JOY_CENTER       ((ADC_RESULT_MAX + 1U) / 2U)
#define IOHWAB_JOY_DEADZONE     ((ADC_RESULT_MAX * 6U) / 100U)   /* 6 % around the centre */
#define IOHWAB_JOY_Y_INVERT     STD_OFF   /* STD_ON if pushing the stick "up" brakes instead of accelerating */
#define IOHWAB_DEBOUNCE_SAMPLES 3U        /* x 10 ms */

void    IoHwAb_Init(void);
void    IoHwAb_MainFunction(void);        /* 10 ms: ADC group, button debounce */

uint8   IoHwAb_GetThrottle(void);         /* 0..100 % */
uint8   IoHwAb_GetBrake(void);            /* 0..100 % */
boolean IoHwAb_GetButton(void);           /* debounced, TRUE = pressed */
void    IoHwAb_GetDebug(uint16 *RawX, uint16 *RawY, boolean *Button);   /* raw values for the UART log */

void    IoHwAb_SetBuzzer(boolean On);
void    IoHwAb_SetTelltaleOverspeed(boolean On);

#endif /* IOHWAB_H */
