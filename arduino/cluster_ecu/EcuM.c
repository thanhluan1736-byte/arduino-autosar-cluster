/* EcuM.c - startup sequence (EcuM_Init -> Os_Start) and statistics */
#include <avr/io.h>
#include <avr/wdt.h>
#include "EcuM.h"
#include "Det.h"
#include "Port.h"
#include "Dio.h"
#include "Adc.h"
#include "Pwm.h"
#include "Gpt.h"
#include "Eep.h"
#include "NvM.h"
#include "Os.h"
#include "IoHwAb.h"
#include "Cdd_Log.h"
#include "Cdd_Stepper.h"
#include "Cdd_Lcd.h"
#include "Rte.h"

static void EcuM_LogResetReason(uint8 mcusr)
{
    LOG_P("[EcuM] reset:");
    if ((mcusr & (uint8)(1U << PORF)) != 0U)  { LOG_P(" power-on"); }
    if ((mcusr & (uint8)(1U << EXTRF)) != 0U) { LOG_P(" external"); }
    if ((mcusr & (uint8)(1U << BORF)) != 0U)  { LOG_P(" brown-out"); }
    if ((mcusr & (uint8)(1U << WDRF)) != 0U)  { LOG_P(" watchdog"); }
    if (mcusr == 0U)                          { LOG_P(" unknown (flags cleared by the bootloader)"); }
    LOG_P("\r\n");
}

void EcuM_Init(void)
{
    uint8 mcusr = MCUSR;

    MCUSR = 0U;          /* clear the flags, else a watchdog reset loops forever */
    wdt_disable();

    /* --- StartPreOS sequence: MCAL first, then services, ECU abstraction, CDDs, RTE --- */
    Det_Init();
    Port_Init(&Port_Config);
    Cdd_Log_Init();
    LOG_P("\r\n=== Mini AUTOSAR cluster (ATmega328P) ===\r\n");
    EcuM_LogResetReason(mcusr);
    Gpt_Init(&Gpt_Config);
    Adc_Init(&Adc_Config);
    Pwm_Init();
    Eep_Init();
    NvM_Init();
    NvM_ReadAll();
    IoHwAb_Init();
    Cdd_Stepper_Init();
    Cdd_Lcd_Init();          /* also initialises the I2c driver */
    Rte_Start();
    Os_Init();
    LOG_P("[EcuM] BSW ready, starting OS\r\n");
}

void EcuM_MainFunction(void)
{
    LOG_P("[EcuM] t=");
    Cdd_Log_U32(Os_GetTimeMs() / 1000U);
    LOG_P(" s  overruns=");
    Cdd_Log_U32(Os_GetOverrunCount());
    LOG_P("  det=");
    Cdd_Log_U32(Det_GetErrorCount());
    {
        uint16 x;
        uint16 y;
        boolean sw;

        IoHwAb_GetDebug(&x, &y, &sw);   /* raw joystick values: helps with wiring / axis checks */
        LOG_P("  joy x=");
        Cdd_Log_U32(x);
        LOG_P(" y=");
        Cdd_Log_U32(y);
        LOG_P(" sw=");
        Cdd_Log_U32(sw);
        LOG_P(" thr=");
        Cdd_Log_U32(IoHwAb_GetThrottle());
        LOG_P(" brk=");
        Cdd_Log_U32(IoHwAb_GetBrake());
    }
    LOG_P("\r\n");
}
