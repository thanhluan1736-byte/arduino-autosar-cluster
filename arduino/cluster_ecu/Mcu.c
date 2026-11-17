/* Mcu.c (AVR) - 16 MHz crystal is set by the fuses; reset flags in MCUSR, watchdog off */
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "Mcu.h"

static Mcu_ResetType Mcu_ResetReason = MCU_RESET_UNDEFINED;

void Mcu_Init(void)
{
    uint8 mcusr = MCUSR;

    MCUSR = 0U;          /* clear the flags, else a watchdog reset loops forever */
    wdt_disable();

    if ((mcusr & (uint8)(1U << WDRF)) != 0U)       { Mcu_ResetReason = MCU_WATCHDOG_RESET; }
    else if ((mcusr & (uint8)(1U << BORF)) != 0U)  { Mcu_ResetReason = MCU_BROWNOUT_RESET; }
    else if ((mcusr & (uint8)(1U << EXTRF)) != 0U) { Mcu_ResetReason = MCU_EXTERNAL_RESET; }
    else if ((mcusr & (uint8)(1U << PORF)) != 0U)  { Mcu_ResetReason = MCU_POWER_ON_RESET; }
    else                                           { Mcu_ResetReason = MCU_RESET_UNDEFINED; }   /* bootloader cleared it */
}

Mcu_ResetType Mcu_GetResetReason(void)
{
    return Mcu_ResetReason;
}

void Mcu_DelayUs(uint32 Us)
{
    while (Us > 0U)
    {
        _delay_us(1);
        Us--;
    }
}
