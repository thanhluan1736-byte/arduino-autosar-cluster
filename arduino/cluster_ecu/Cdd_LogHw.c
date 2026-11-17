/* Cdd_LogHw.c (AVR) - USART0 on D0/D1 (USB serial of the Uno), UDRE interrupt */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Cdd_LogHw.h"
#include "Cdd_Log.h"

void Cdd_LogHw_Init(uint32 Baud)
{
    UBRR0  = (uint16)(((F_CPU / 8UL) / Baud) - 1UL);    /* U2X = 1 -> 16 at 16 MHz / 115200 */
    UCSR0A = (uint8)(1U << U2X0);
    UCSR0C = (uint8)((1U << UCSZ01) | (1U << UCSZ00));   /* 8N1 */
    UCSR0B = (uint8)(1U << TXEN0);
}

void Cdd_LogHw_StartTx(void)
{
    UCSR0B |= (uint8)(1U << UDRIE0);
}

boolean Cdd_LogHw_IrqEnabled(void)
{
    return ((SREG & (uint8)(1U << SREG_I)) != 0U) ? TRUE : FALSE;
}

void Cdd_LogHw_PollWrite(uint8 Byte)
{
    while ((UCSR0A & (uint8)(1U << UDRE0)) == 0U) { }
    UDR0 = Byte;
}

ISR(USART_UDRE_vect)
{
    uint8 b;

    if (Cdd_Log_GetTxByte(&b) == TRUE)
    {
        UDR0 = b;
    }
    else
    {
        UCSR0B &= (uint8)~(1U << UDRIE0);   /* nothing left */
    }
}
