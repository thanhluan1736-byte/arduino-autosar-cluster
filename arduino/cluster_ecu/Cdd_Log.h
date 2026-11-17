/* Cdd_Log.h - debug log on USART0 (115200 8N1), interrupt driven ring buffer */
#ifndef CDD_LOG_H
#define CDD_LOG_H

#include <avr/pgmspace.h>
#include "Std_Types.h"

#define CDD_LOG_BAUD   115200UL

/* constant strings stay in flash (2 KB of RAM is precious) */
#define LOG_P(s)       Cdd_Log_StrP(PSTR(s))

void Cdd_Log_Init(void);
void Cdd_Log_StrP(const char *FlashStr);
void Cdd_Log_U32(uint32 Value);
void Cdd_Log_Fixed(uint32 Value, uint8 Decimals);   /* 12345, 3 -> "12.345" */
void Cdd_Log_Hex8(uint8 Value);

#endif /* CDD_LOG_H */
