/* Cdd_LogHw.h - UART hardware hooks of Cdd_Log, implemented once per platform (platform/<mcu>/Cdd_LogHw.c) */
#ifndef CDD_LOGHW_H
#define CDD_LOGHW_H

#include "Std_Types.h"

void    Cdd_LogHw_Init(uint32 Baud);
void    Cdd_LogHw_StartTx(void);           /* enable the "TX empty" interrupt; the ISR pulls bytes with Cdd_Log_GetTxByte() */
boolean Cdd_LogHw_IrqEnabled(void);        /* global interrupts on? */
void    Cdd_LogHw_PollWrite(uint8 Byte);   /* blocking write, used while interrupts are still off (EcuM_Init) */

#endif /* CDD_LOGHW_H */
