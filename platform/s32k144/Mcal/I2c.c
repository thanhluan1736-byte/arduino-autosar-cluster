/* I2c.c (S32K144) - LPI2C0 master: a transfer is a list of FIFO words (START+address, data..., STOP)
 * pushed into the 4-entry TX FIFO by the same step function from the ISR or from a polling loop. */
#include "I2c.h"
#include "Det.h"
#include "S32K144_Regs.h"

#define I2C_SYNC_TIMEOUT   200000UL   /* polling loops (~ tens of ms at 48 MHz) */

static uint16 I2c_Words[I2C_MAX_LENGTH + 2U];
static uint8  I2c_NumWords = 0U;
static volatile uint8 I2c_NextWord = 0U;
static volatile I2c_StatusType I2c_Status = I2C_IDLE;
static boolean I2c_UseIrq = FALSE;

static void I2c_Finish(I2c_StatusType result)
{
    LPI2C0_MIER = 0U;
    if (result != I2C_IDLE)
    {
        LPI2C0_MCR |= LPI2C_MCR_RTF | LPI2C_MCR_RRF;          /* drop what is left */
        if ((LPI2C0_MSR & LPI2C_MSR_MBF) != 0U)
        {
            LPI2C0_MTDR = LPI2C_CMD_STOP;                     /* release the bus after a NACK */
        }
    }
    LPI2C0_MSR = LPI2C_MSR_W1C_ALL;
    I2c_Status = result;
}

/* one step: refill the FIFO, detect NACK / end of transfer */
static void I2c_Step(void)
{
    uint32 msr = LPI2C0_MSR;

    if ((msr & (LPI2C_MSR_NDF | LPI2C_MSR_ALF | LPI2C_MSR_FEF | LPI2C_MSR_PLTF)) != 0U)
    {
        I2c_Finish(I2C_ERROR);
        return;
    }
    while ((I2c_NextWord < I2c_NumWords) && (LPI2C_MFSR_TXCOUNT < LPI2C_FIFO_SIZE))
    {
        LPI2C0_MTDR = I2c_Words[I2c_NextWord];
        I2c_NextWord++;
    }
    if (I2c_NextWord >= I2c_NumWords)
    {
        LPI2C0_MIER &= ~LPI2C_MIER_TDIE;                     /* nothing more to push: wait for STOP */
        if ((msr & LPI2C_MSR_SDF) != 0U)
        {
            I2c_Finish(I2C_IDLE);                             /* STOP sent, all bytes ACKed */
        }
    }
}

static Std_ReturnType I2c_Begin(uint8 Address7, const uint8 *Data, uint8 Length, boolean useIrq)
{
    uint8 i;

    if ((Data == NULL_PTR) || (Length == 0U) || (Length > I2C_MAX_LENGTH) || (Address7 > 0x7FU))
    {
        (void)Det_ReportError(I2C_MODULE_ID, 0U, I2C_SID_WRITE, I2C_E_PARAM);
        return E_NOT_OK;
    }
    if (I2c_Status == I2C_BUSY)
    {
        (void)Det_ReportError(I2C_MODULE_ID, 0U, I2C_SID_WRITE, I2C_E_BUSY);
        return E_NOT_OK;
    }
    I2c_Words[0] = (uint16)(LPI2C_CMD_START | ((uint32)Address7 << 1U));   /* R/W = 0: write */
    for (i = 0U; i < Length; i++)
    {
        I2c_Words[i + 1U] = (uint16)(LPI2C_CMD_TX | Data[i]);
    }
    I2c_Words[Length + 1U] = (uint16)LPI2C_CMD_STOP;
    I2c_NumWords = (uint8)(Length + 2U);
    I2c_NextWord = 0U;
    I2c_UseIrq = useIrq;

    LPI2C0_MCR |= LPI2C_MCR_RTF | LPI2C_MCR_RRF;
    LPI2C0_MSR = LPI2C_MSR_W1C_ALL;
    I2c_Status = I2C_BUSY;
    if (useIrq == TRUE)
    {
        LPI2C0_MIER = LPI2C_MIER_TDIE | LPI2C_MIER_SDIE | LPI2C_MIER_NDIE | LPI2C_MIER_ALIE;   /* ISR starts at once (TDF) */
    }
    else
    {
        I2c_Step();
    }
    return E_OK;
}

void I2c_Init(void)
{
    PCC_REG(PCC_LPI2C0_INDEX) = 0U;
    PCC_REG(PCC_LPI2C0_INDEX) = PCC_PCS(PCC_PCS_FIRCDIV2) | PCC_CGC;   /* 48 MHz */
    LPI2C0_MCR = LPI2C_MCR_RST;
    LPI2C0_MCR = 0U;
    LPI2C0_MCFGR1 = LPI2C_MCFGR1_PRESCALE(3U);                          /* 48 / 8 = 6 MHz */
    /* SCL = 6 MHz / (CLKLO + CLKHI + 2 + latency) ~ 6 MHz / 61 ~ 98 kHz; START/STOP hold 4.7 us, data valid 2.3 us */
    LPI2C0_MCCR0 = LPI2C_MCCR0_VAL(30U, 28U, 28U, 14U);
    LPI2C0_MFCR = LPI2C_MFCR_TXWATER(1U);                               /* TDF when 1 word or less is left */
    LPI2C0_MCR = LPI2C_MCR_MEN | LPI2C_MCR_DBGEN;
    LPI2C0_MIER = 0U;
    Nvic_EnableIrq(IRQ_LPI2C0_MASTER, 4U);
    I2c_Status = I2C_IDLE;
}

Std_ReturnType I2c_WriteSync(uint8 Address7, const uint8 *Data, uint8 Length)
{
    uint32 t = 0U;

    if (I2c_Begin(Address7, Data, Length, FALSE) != E_OK)
    {
        return E_NOT_OK;
    }
    while (I2c_Status == I2C_BUSY)
    {
        I2c_Step();
        if (++t >= I2C_SYNC_TIMEOUT)
        {
            I2c_Finish(I2C_ERROR);           /* bus stuck: no pull-ups / nothing connected */
        }
    }
    return (I2c_Status == I2C_IDLE) ? E_OK : E_NOT_OK;
}

Std_ReturnType I2c_WriteAsync(uint8 Address7, const uint8 *Data, uint8 Length)
{
    return I2c_Begin(Address7, Data, Length, TRUE);
}

I2c_StatusType I2c_GetStatus(void)
{
    return I2c_Status;
}

void LPI2C0_Master_IRQHandler(void)
{
    if ((I2c_UseIrq == TRUE) && (I2c_Status == I2C_BUSY))
    {
        I2c_Step();
    }
    else
    {
        LPI2C0_MIER = 0U;
        LPI2C0_MSR = LPI2C_MSR_W1C_ALL;
    }
}
