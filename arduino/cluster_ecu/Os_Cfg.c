/* Os_Cfg.c - task table: which BSW main functions and the RTE run when */
#include "Os.h"
#include "Rte.h"
#include "IoHwAb.h"
#include "Eep.h"
#include "NvM.h"
#include "Cdd_Stepper.h"
#include "Cdd_Lcd.h"
#include "EcuM.h"

/* 10 ms: inputs and memory stack (EEPROM programs one byte per call) */
static void OsTask_10ms(void)
{
    IoHwAb_MainFunction();
    Eep_MainFunction();
    NvM_MainFunction();
}

/* 1 ms: RTE dispatches the SWC runnables (TimingEvents 10 / 20 / 100 ms) */
static void OsTask_1ms(void)
{
    Rte_MainFunction();
}

/* 2 ms: one full step of the needle motor (500 steps/s is safe for a 28BYJ-48)
         and at most one changed LCD character (I2C job runs in the background) */
static void OsTask_2ms(void)
{
    Cdd_Stepper_MainFunction();
    Cdd_Lcd_MainFunction();
}

/* 1 s: ECU state / statistics on the UART log */
static void OsTask_1000ms(void)
{
    EcuM_MainFunction();
}

const Os_TaskConfigType Os_TaskTable[] =
{
    /* Func            PeriodMs  OffsetMs */
    { OsTask_10ms,       10U,      0U },
    { OsTask_1ms,         1U,      0U },
    { OsTask_2ms,         2U,      1U },
    { OsTask_1000ms,   1000U,    500U },
};

const uint8 Os_NumTasks = (uint8)(sizeof(Os_TaskTable) / sizeof(Os_TaskTable[0]));
