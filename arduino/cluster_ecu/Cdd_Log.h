/* Cdd_Log.h - debug log on a UART (115200 8N1), interrupt driven ring buffer */
#ifndef CDD_LOG_H
#define CDD_LOG_H

#include "Std_Types.h"

#define CDD_LOG_BAUD   115200UL

#if defined(__AVR__)
/* AVR: constant strings stay in flash (2 KB of RAM is precious) */
#include <avr/pgmspace.h>
#define LOG_P(s)                 Cdd_Log_StrP(PSTR(s))
#define CDD_LOG_READ_CHAR(p)     ((char)pgm_read_byte(p))
#else
/* ARM: flash is in the normal address space */
#define LOG_P(s)                 Cdd_Log_StrP(s)
#define CDD_LOG_READ_CHAR(p)     (*(p))
#endif

void Cdd_Log_Init(void);
void Cdd_Log_StrP(const char *FlashStr);
void Cdd_Log_U32(uint32 Value);
void Cdd_Log_Fixed(uint32 Value, uint8 Decimals);   /* 12345, 3 -> "12.345" */
void Cdd_Log_Hex8(uint8 Value);

/* called by the platform UART interrupt: next byte to send, FALSE when the buffer is empty */
boolean Cdd_Log_GetTxByte(uint8 *Byte);

#endif /* CDD_LOG_H */
