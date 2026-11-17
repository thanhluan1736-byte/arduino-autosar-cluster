/* Mcu.h - AUTOSAR Mcu driver (subset): init, reset reason, busy-wait delays */
#ifndef MCU_H
#define MCU_H

#include "Std_Types.h"

#define MCU_PLATFORM_NAME   "S32K144 / MaaZEDU"

typedef enum
{
    MCU_POWER_ON_RESET = 0U,
    MCU_EXTERNAL_RESET,
    MCU_WATCHDOG_RESET,
    MCU_SW_RESET,
    MCU_BROWNOUT_RESET,
    MCU_RESET_UNDEFINED
} Mcu_ResetType;

void          Mcu_Init(void);                 /* clocks, reset reason, cycle counter (watchdog is off since startup) */
Mcu_ResetType Mcu_GetResetReason(void);
void          Mcu_DelayUs(uint32 Us);         /* busy wait, for init sequences only */

#endif /* MCU_H */
