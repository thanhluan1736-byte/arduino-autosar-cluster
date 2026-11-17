/* Os.h - minimal time-triggered cooperative OS (OSEK-like basic tasks, 1 ms tick from Gpt) */
#ifndef OS_H
#define OS_H

#include "Std_Types.h"

typedef void (*Os_TaskFuncType)(void);

typedef struct
{
    Os_TaskFuncType Func;
    uint16          PeriodMs;
    uint16          OffsetMs;    /* first activation, spreads tasks over the ticks */
} Os_TaskConfigType;

#define OS_TICK_MS   1U

extern const Os_TaskConfigType Os_TaskTable[];
extern const uint8 Os_NumTasks;

void   Os_Init(void);
void   Os_Start(void) __attribute__((noreturn));
void   Os_TickNotification(void);          /* Gpt callback, ISR context */
uint32 Os_GetTimeMs(void);
uint16 Os_GetOverrunCount(void);           /* ticks that arrived while a tick was still being processed */

#endif /* OS_H */
