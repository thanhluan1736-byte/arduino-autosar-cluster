/* Mcu.c (S32K144) - core on FIRC 48 MHz (reset default), FIRCDIV2 = 48 MHz for the peripherals */
#include "Mcu.h"
#include "S32K144_Regs.h"

static Mcu_ResetType Mcu_ResetReason = MCU_RESET_UNDEFINED;

void Mcu_Init(void)
{
    uint32 srs = RCM_SRS;

    /* peripheral clock sources: FIRCDIV1 / FIRCDIV2 = 48 MHz (they are off after reset) */
    SCG_FIRCDIV = SCG_FIRCDIV1(1U) | SCG_FIRCDIV2(1U);

    /* DWT cycle counter for Mcu_DelayUs */
    DEMCR |= DEMCR_TRCENA;
    DWT_CYCCNT = 0U;
    DWT_CTRL |= DWT_CTRL_CYCCNTENA;

    if ((srs & RCM_SRS_WDOG) != 0U)                            { Mcu_ResetReason = MCU_WATCHDOG_RESET; }
    else if ((srs & (RCM_SRS_SW | RCM_SRS_LOCKUP)) != 0U)      { Mcu_ResetReason = MCU_SW_RESET; }   /* also J-Link reset */
    else if ((srs & RCM_SRS_LVD) != 0U)                        { Mcu_ResetReason = MCU_BROWNOUT_RESET; }
    else if ((srs & RCM_SRS_POR) != 0U)                        { Mcu_ResetReason = MCU_POWER_ON_RESET; }
    else if ((srs & RCM_SRS_PIN) != 0U)                        { Mcu_ResetReason = MCU_EXTERNAL_RESET; }
    else                                                       { Mcu_ResetReason = MCU_RESET_UNDEFINED; }
}

Mcu_ResetType Mcu_GetResetReason(void)
{
    return Mcu_ResetReason;
}

void Mcu_DelayUs(uint32 Us)
{
    uint32 start = DWT_CYCCNT;
    uint32 cycles = Us * (CORE_CLOCK_HZ / 1000000UL);

    while ((uint32)(DWT_CYCCNT - start) < cycles) { }
}
