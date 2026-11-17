/* Cdd_Stepper.h - complex driver for the 28BYJ-48 needle motor (ULN2003, full step, 2 coils on) */
#ifndef CDD_STEPPER_H
#define CDD_STEPPER_H

#include "Std_Types.h"

/* 28BYJ-48: ~2048 full steps per output revolution. Gauge dial = 240 degrees -> 1365 steps */
#define CDD_STEPPER_MAX_STEPS     1365
#define CDD_STEPPER_REVERSE       STD_OFF   /* STD_ON if the needle turns the wrong way */
#define CDD_STEPPER_IDLE_OFF      250U      /* x 2 ms: release the coils after 0.5 s at target (no heat) */

void   Cdd_Stepper_Init(void);
void   Cdd_Stepper_SetTarget(sint16 Steps);   /* clamped to 0..CDD_STEPPER_MAX_STEPS */
sint16 Cdd_Stepper_GetPosition(void);
sint16 Cdd_Stepper_GetTarget(void);
void   Cdd_Stepper_MainFunction(void);        /* every 2 ms: at most one step */

#endif /* CDD_STEPPER_H */
