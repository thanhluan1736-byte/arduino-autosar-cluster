/* Os.c - time-triggered scheduler: one tick per ms, tasks run to completion, idle = CPU sleep */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "Os.h"
#include "Gpt.h"

#define OS_MAX_TASKS        8U
#define OS_TIMER_TICKS_1MS  (1000U / GPT_TICK_US)   /* 250 x 4 us */

static volatile uint8 Os_PendingTicks = 0U;
static uint16 Os_Countdown[OS_MAX_TASKS];
static uint32 Os_TimeMs = 0U;
static uint16 Os_Overruns = 0U;

void Os_TickNotification(void)
{
    if (Os_PendingTicks < 0xFFU)
    {
        Os_PendingTicks++;
    }
}

void Os_Init(void)
{
    uint8 i;

    for (i = 0U; (i < Os_NumTasks) && (i < OS_MAX_TASKS); i++)
    {
        Os_Countdown[i] = Os_TaskTable[i].OffsetMs;
    }
    Os_PendingTicks = 0U;
    Os_TimeMs = 0U;
    Os_Overruns = 0U;
}

static void Os_RunTick(void)
{
    uint8 i;

    for (i = 0U; (i < Os_NumTasks) && (i < OS_MAX_TASKS); i++)
    {
        if (Os_Countdown[i] == 0U)
        {
            Os_Countdown[i] = (uint16)(Os_TaskTable[i].PeriodMs - 1U);
            Os_TaskTable[i].Func();
        }
        else
        {
            Os_Countdown[i]--;
        }
    }
    Os_TimeMs++;
}

void Os_Start(void)
{
    Gpt_EnableNotification(GpConf_GptChannel_OsTick);
    Gpt_StartTimer(GpConf_GptChannel_OsTick, OS_TIMER_TICKS_1MS);
    set_sleep_mode(SLEEP_MODE_IDLE);
    sei();

    for (;;)
    {
        uint8 pending;

        cli();
        pending = Os_PendingTicks;
        if (pending == 0U)
        {
            /* idle: sleep until the next interrupt; sei+sleep is atomic on AVR */
            sleep_enable();
            sei();
            sleep_cpu();
            sleep_disable();
            continue;
        }
        Os_PendingTicks = (uint8)(pending - 1U);
        sei();
        if (pending > 1U)
        {
            Os_Overruns++;       /* the previous tick took longer than 1 ms */
        }
        Os_RunTick();
    }
}

uint32 Os_GetTimeMs(void)
{
    return Os_TimeMs;
}

uint16 Os_GetOverrunCount(void)
{
    return Os_Overruns;
}
