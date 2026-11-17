/* I2c.c - TWI master write state machine, shared by the polled and the interrupt driven path */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "I2c.h"
#include "Det.h"

/* TWI status codes (TWSR & 0xF8) */
#define TW_START        0x08U
#define TW_REP_START    0x10U
#define TW_MT_SLA_ACK   0x18U
#define TW_MT_DATA_ACK  0x28U

#define I2C_SYNC_TIMEOUT  20000U   /* polling loops per bus event (~ several ms) */

static uint8 I2c_Buf[I2C_MAX_LENGTH];
static uint8 I2c_Len = 0U;
static uint8 I2c_Index = 0U;
static uint8 I2c_Sla = 0U;
static volatile I2c_StatusType I2c_Status = I2C_IDLE;
static boolean I2c_UseIrq = FALSE;

static uint8 I2c_Control(void)
{
    return (uint8)((1U << TWINT) | (1U << TWEN) | ((I2c_UseIrq == TRUE) ? (1U << TWIE) : 0U));
}

static void I2c_Stop(I2c_StatusType result)
{
    TWCR = (uint8)((1U << TWINT) | (1U << TWEN) | (1U << TWSTO));   /* STOP, interrupt off */
    I2c_Status = result;
}

/* one step of the transfer, called when TWINT is set */
static void I2c_Step(void)
{
    uint8 st = (uint8)(TWSR & 0xF8U);

    switch (st)
    {
    case TW_START:
    case TW_REP_START:
        TWDR = I2c_Sla;
        TWCR = I2c_Control();
        break;

    case TW_MT_SLA_ACK:
    case TW_MT_DATA_ACK:
        if (I2c_Index < I2c_Len)
        {
            TWDR = I2c_Buf[I2c_Index];
            I2c_Index++;
            TWCR = I2c_Control();
        }
        else
        {
            I2c_Stop(I2C_IDLE);       /* all bytes acknowledged */
        }
        break;

    default:
        I2c_Stop(I2C_ERROR);          /* SLA or data NACK, arbitration lost, bus error */
        break;
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
    while ((TWCR & (uint8)(1U << TWSTO)) != 0U) { }   /* previous STOP still on the bus (a few us) */
    for (i = 0U; i < Length; i++)
    {
        I2c_Buf[i] = Data[i];
    }
    I2c_Len = Length;
    I2c_Index = 0U;
    I2c_Sla = (uint8)(Address7 << 1U);      /* R/W bit = 0: write */
    I2c_UseIrq = useIrq;
    I2c_Status = I2C_BUSY;
    TWCR = (uint8)(I2c_Control() | (1U << TWSTA));   /* START */
    return E_OK;
}

void I2c_Init(void)
{
    TWSR = 0U;                                                  /* prescaler 1 */
    TWBR = (uint8)(((F_CPU / I2C_BUS_HZ) - 16UL) / 2UL);        /* 72 -> 100 kHz at 16 MHz */
    TWCR = (uint8)(1U << TWEN);
    I2c_Status = I2C_IDLE;
}

Std_ReturnType I2c_WriteSync(uint8 Address7, const uint8 *Data, uint8 Length)
{
    if (I2c_Begin(Address7, Data, Length, FALSE) != E_OK)
    {
        return E_NOT_OK;
    }
    while (I2c_Status == I2C_BUSY)
    {
        uint16 t = 0U;
        while ((TWCR & (uint8)(1U << TWINT)) == 0U)
        {
            if (++t >= I2C_SYNC_TIMEOUT)
            {
                I2c_Stop(I2C_ERROR);       /* bus stuck (no pull-ups / nothing connected) */
                return E_NOT_OK;
            }
        }
        I2c_Step();
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

ISR(TWI_vect)
{
    I2c_Step();
}
