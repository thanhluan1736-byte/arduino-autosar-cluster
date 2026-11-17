/* Cdd_Log.c - non-blocking UART log: ring buffer + number formatting (portable part) */
#include "Cdd_Log.h"
#include "Cdd_LogHw.h"

#define CDD_LOG_BUF_SIZE  128U   /* power of two */

static volatile uint8 Cdd_Log_Buf[CDD_LOG_BUF_SIZE];
static volatile uint8 Cdd_Log_Head = 0U;   /* written by the task */
static volatile uint8 Cdd_Log_Tail = 0U;   /* written by the ISR */

void Cdd_Log_Init(void)
{
    Cdd_Log_Head = 0U;
    Cdd_Log_Tail = 0U;
    Cdd_LogHw_Init(CDD_LOG_BAUD);
}

boolean Cdd_Log_GetTxByte(uint8 *Byte)
{
    if (Cdd_Log_Tail == Cdd_Log_Head)
    {
        return FALSE;
    }
    *Byte = Cdd_Log_Buf[Cdd_Log_Tail];
    Cdd_Log_Tail = (uint8)((Cdd_Log_Tail + 1U) & (CDD_LOG_BUF_SIZE - 1U));
    return TRUE;
}

static void Cdd_Log_PutChar(uint8 c)
{
    uint8 next = (uint8)((Cdd_Log_Head + 1U) & (CDD_LOG_BUF_SIZE - 1U));

    while (next == Cdd_Log_Tail)
    {
        uint8 b;

        if (Cdd_LogHw_IrqEnabled() == TRUE)
        {
            return;   /* buffer full while running: drop, never block the scheduler */
        }
        /* interrupts off (EcuM_Init): drain one byte by polling */
        if (Cdd_Log_GetTxByte(&b) == TRUE)
        {
            Cdd_LogHw_PollWrite(b);
        }
    }
    Cdd_Log_Buf[Cdd_Log_Head] = c;
    Cdd_Log_Head = next;
    Cdd_LogHw_StartTx();
}

void Cdd_Log_StrP(const char *FlashStr)
{
    char c;

    while ((c = CDD_LOG_READ_CHAR(FlashStr)) != '\0')
    {
        Cdd_Log_PutChar((uint8)c);
        FlashStr++;
    }
}

void Cdd_Log_Fixed(uint32 Value, uint8 Decimals)
{
    char digits[11];
    uint8 n = 0U;

    do
    {
        digits[n] = (char)('0' + (Value % 10U));
        Value /= 10U;
        n++;
    } while ((Value != 0U) || (n <= Decimals));   /* at least one digit before the point */

    while (n > 0U)
    {
        n--;
        Cdd_Log_PutChar((uint8)digits[n]);
        if ((Decimals != 0U) && (n == Decimals))
        {
            Cdd_Log_PutChar((uint8)'.');
        }
    }
}

void Cdd_Log_U32(uint32 Value)
{
    Cdd_Log_Fixed(Value, 0U);
}

void Cdd_Log_Hex8(uint8 Value)
{
    static const char hex[] = "0123456789ABCDEF";
    Cdd_Log_PutChar((uint8)hex[Value >> 4U]);
    Cdd_Log_PutChar((uint8)hex[Value & 0x0FU]);
}
