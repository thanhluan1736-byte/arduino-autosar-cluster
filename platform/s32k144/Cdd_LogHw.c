/* Cdd_LogHw.c (S32K144) - LPUART1 on PTC6 / PTC7 = the USB virtual COM port of the MaaZEDU board */
#include "Cdd_LogHw.h"
#include "Cdd_Log.h"
#include "S32K144_Regs.h"

void Cdd_LogHw_Init(uint32 Baud)
{
    PCC_REG(PCC_LPUART1_INDEX) = 0U;
    PCC_REG(PCC_LPUART1_INDEX) = PCC_PCS(PCC_PCS_FIRCDIV2) | PCC_CGC;          /* 48 MHz */
    LPUART1_CTRL = 0U;
    LPUART1_BAUD = LPUART_BAUD_OSR(15U) | LPUART_BAUD_SBR(PERIPH_CLOCK_HZ / (16UL * Baud));   /* 26 -> 115385 Bd */
    LPUART1_CTRL = LPUART_CTRL_TE;                                             /* 8N1, transmitter on */
    Nvic_EnableIrq(IRQ_LPUART1_RXTX, 6U);
}

void Cdd_LogHw_StartTx(void)
{
    LPUART1_CTRL |= LPUART_CTRL_TIE;
}

boolean Cdd_LogHw_IrqEnabled(void)
{
    uint32 primask;

    __asm volatile ("mrs %0, primask" : "=r" (primask));
    return (primask == 0U) ? TRUE : FALSE;
}

void Cdd_LogHw_PollWrite(uint8 Byte)
{
    while ((LPUART1_STAT & LPUART_STAT_TDRE) == 0U) { }
    LPUART1_DATA = Byte;
}

void LPUART1_RxTx_IRQHandler(void)
{
    uint8 b;

    if ((LPUART1_STAT & LPUART_STAT_TDRE) != 0U)
    {
        if (Cdd_Log_GetTxByte(&b) == TRUE)
        {
            LPUART1_DATA = b;
        }
        else
        {
            LPUART1_CTRL &= ~LPUART_CTRL_TIE;   /* nothing left */
        }
    }
}
