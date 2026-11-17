/* Dio.c - AUTOSAR Dio driver for ATmega328P */
#include "Dio.h"
#include "Det.h"
#include "SchM.h"

const Dio_ChannelGroupType DioConf_DioChannelGroup_Stepper = { 0x0FU, 0U, MCAL_PORT_B };

Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId)
{
    if (!MCAL_PIN_VALID(ChannelId))
    {
        (void)Det_ReportError(DIO_MODULE_ID, 0U, DIO_SID_READ_CHANNEL, DIO_E_PARAM_INVALID_CHANNEL_ID);
        return STD_LOW;
    }
    return ((MCAL_REG_PIN(MCAL_PIN_PORT(ChannelId)) & MCAL_PIN_MASK(ChannelId)) != 0U) ? STD_HIGH : STD_LOW;
}

void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level)
{
    uint8 port;
    uint8 mask;

    if (!MCAL_PIN_VALID(ChannelId))
    {
        (void)Det_ReportError(DIO_MODULE_ID, 0U, DIO_SID_WRITE_CHANNEL, DIO_E_PARAM_INVALID_CHANNEL_ID);
        return;
    }
    port = MCAL_PIN_PORT(ChannelId);
    mask = MCAL_PIN_MASK(ChannelId);
    SchM_Enter_Exclusive();   /* read-modify-write of PORTx */
    if (Level == STD_HIGH) { MCAL_REG_PORT(port) |= mask; }
    else                   { MCAL_REG_PORT(port) &= (uint8)~mask; }
    SchM_Exit_Exclusive();
}

Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId)
{
    if (!MCAL_PIN_VALID(ChannelId))
    {
        (void)Det_ReportError(DIO_MODULE_ID, 0U, DIO_SID_FLIP_CHANNEL, DIO_E_PARAM_INVALID_CHANNEL_ID);
        return STD_LOW;
    }
    /* writing 1 to PINx toggles the output on this AVR (atomic, no lock needed) */
    MCAL_REG_PIN(MCAL_PIN_PORT(ChannelId)) = MCAL_PIN_MASK(ChannelId);
    return ((MCAL_REG_PORT(MCAL_PIN_PORT(ChannelId)) & MCAL_PIN_MASK(ChannelId)) != 0U) ? STD_HIGH : STD_LOW;
}

Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr)
{
    if ((ChannelGroupIdPtr == NULL_PTR) || (ChannelGroupIdPtr->port >= MCAL_NUM_PORTS))
    {
        (void)Det_ReportError(DIO_MODULE_ID, 0U, DIO_SID_READ_CHANNEL_GROUP, DIO_E_PARAM_INVALID_GROUP);
        return 0U;
    }
    return (Dio_PortLevelType)((MCAL_REG_PIN(ChannelGroupIdPtr->port) & ChannelGroupIdPtr->mask) >> ChannelGroupIdPtr->offset);
}

void Dio_WriteChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr, Dio_PortLevelType Level)
{
    uint8 port;
    uint8 value;

    if ((ChannelGroupIdPtr == NULL_PTR) || (ChannelGroupIdPtr->port >= MCAL_NUM_PORTS))
    {
        (void)Det_ReportError(DIO_MODULE_ID, 0U, DIO_SID_WRITE_CHANNEL_GROUP, DIO_E_PARAM_INVALID_GROUP);
        return;
    }
    port  = ChannelGroupIdPtr->port;
    value = (uint8)((uint8)(Level << ChannelGroupIdPtr->offset) & ChannelGroupIdPtr->mask);
    SchM_Enter_Exclusive();
    MCAL_REG_PORT(port) = (uint8)((MCAL_REG_PORT(port) & (uint8)~ChannelGroupIdPtr->mask) | value);
    SchM_Exit_Exclusive();
}
