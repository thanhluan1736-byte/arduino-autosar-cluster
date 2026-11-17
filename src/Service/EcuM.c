/* EcuM.c - startup sequence (EcuM_Init -> Os_Start) and statistics (portable) */
#include "EcuM.h"
#include "Mcu.h"
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

static void EcuM_LogResetReason(Mcu_ResetType reason)
{
    LOG_P("[EcuM] reset: ");
    switch (reason)
    {
    case MCU_POWER_ON_RESET:  LOG_P("power-on");  break;
    case MCU_EXTERNAL_RESET:  LOG_P("external");  break;
    case MCU_WATCHDOG_RESET:  LOG_P("watchdog");  break;
    case MCU_SW_RESET:        LOG_P("software");  break;
    case MCU_BROWNOUT_RESET:  LOG_P("brown-out"); break;
    default:                  LOG_P("unknown");   break;
    }
    LOG_P("\r\n");
}

void EcuM_Init(void)
{
    Mcu_Init();              /* clocks, watchdog off, reset reason latched */

    /* --- StartPreOS sequence: MCAL first, then services, ECU abstraction, CDDs, RTE --- */
    Det_Init();
    Port_Init(&Port_Config);
    Cdd_Log_Init();
    LOG_P("\r\n=== Mini AUTOSAR cluster (" MCU_PLATFORM_NAME ") ===\r\n");
    EcuM_LogResetReason(Mcu_GetResetReason());
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
