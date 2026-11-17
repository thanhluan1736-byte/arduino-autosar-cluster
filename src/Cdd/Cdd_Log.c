/* Cdd_Log.c - non-blocking UART log; falls back to polling while interrupts are off (startup) */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Cdd_Log.h"

#define CDD_LOG_BUF_SIZE  128U   /* power of two */
#define CDD_LOG_UBRR      ((uint16)(((F_CPU / 8UL) / CDD_LOG_BAUD) - 1UL))   /* U2X = 1 -> 16 at 16 MHz */

static volatile uint8 Cdd_Log_Buf[CDD_LOG_BUF_SIZE];
static volatile uint8 Cdd_Log_Head = 0U;   /* written by the task */
static volatile uint8 Cdd_Log_Tail = 0U;   /* written by the ISR */

void Cdd_Log_Init(void)
{
    UBRR0  = CDD_LOG_UBRR;
    UCSR0A = (uint8)(1U << U2X0);
    UCSR0C = (uint8)((1U << UCSZ01) | (1U << UCSZ00));   /* 8N1 */
    UCSR0B = (uint8)(1U << TXEN0);
}

static void Cdd_Log_PutChar(uint8 c)
{
    uint8 next = (uint8)((Cdd_Log_Head + 1U) & (CDD_LOG_BUF_SIZE - 1U));

    while (next == Cdd_Log_Tail)
    {
        if ((SREG & (uint8)(1U << SREG_I)) != 0U)
        {
            return;   /* buffer full while running: drop, never block the scheduler */
        }
        /* interrupts off (EcuM_Init): drain one byte by polling */
        while ((UCSR0A & (uint8)(1U << UDRE0)) == 0U) { }
        UDR0 = Cdd_Log_Buf[Cdd_Log_Tail];
        Cdd_Log_Tail = (uint8)((Cdd_Log_Tail + 1U) & (CDD_LOG_BUF_SIZE - 1U));
    }
    Cdd_Log_Buf[Cdd_Log_Head] = c;
    Cdd_Log_Head = next;
    UCSR0B |= (uint8)(1U << UDRIE0);   /* the ISR sends it */
}

void Cdd_Log_StrP(const char *FlashStr)
{
    char c;

    while ((c = (char)pgm_read_byte(FlashStr)) != '\0')
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

ISR(USART_UDRE_vect)
{
    if (Cdd_Log_Tail == Cdd_Log_Head)
    {
        UCSR0B &= (uint8)~(1U << UDRIE0);   /* nothing left */
        return;
    }
    UDR0 = Cdd_Log_Buf[Cdd_Log_Tail];
    Cdd_Log_Tail = (uint8)((Cdd_Log_Tail + 1U) & (CDD_LOG_BUF_SIZE - 1U));
}
