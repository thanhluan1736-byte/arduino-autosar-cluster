/* Dio.c (S32K144) - GPIO with the atomic set / clear / toggle registers */
#include "Dio.h"
#include "Det.h"
#include "SchM.h"
#include "S32K144_Regs.h"

const Dio_ChannelGroupType DioConf_DioChannelGroup_Stepper = { 0x0FUL << 9U, 9U, PORT_D };

Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId)
{
    if (!PORT_PIN_VALID(ChannelId))
    {
        (void)Det_ReportError(DIO_MODULE_ID, 0U, DIO_SID_READ_CHANNEL, DIO_E_PARAM_INVALID_CHANNEL_ID);
        return STD_LOW;
    }
    return ((GPIO_PDIR(PORT_PIN_PORT(ChannelId)) & (1UL << PORT_PIN_NUM(ChannelId))) != 0U) ? STD_HIGH : STD_LOW;
}

void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level)
{
    if (!PORT_PIN_VALID(ChannelId))
    {
        (void)Det_ReportError(DIO_MODULE_ID, 0U, DIO_SID_WRITE_CHANNEL, DIO_E_PARAM_INVALID_CHANNEL_ID);
        return;
    }
    if (Level == STD_HIGH) { GPIO_PSOR(PORT_PIN_PORT(ChannelId)) = 1UL << PORT_PIN_NUM(ChannelId); }
    else                   { GPIO_PCOR(PORT_PIN_PORT(ChannelId)) = 1UL << PORT_PIN_NUM(ChannelId); }
}

Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId)
{
    if (!PORT_PIN_VALID(ChannelId))
    {
        (void)Det_ReportError(DIO_MODULE_ID, 0U, DIO_SID_FLIP_CHANNEL, DIO_E_PARAM_INVALID_CHANNEL_ID);
        return STD_LOW;
    }
    GPIO_PTOR(PORT_PIN_PORT(ChannelId)) = 1UL << PORT_PIN_NUM(ChannelId);
    return ((GPIO_PDOR(PORT_PIN_PORT(ChannelId)) & (1UL << PORT_PIN_NUM(ChannelId))) != 0U) ? STD_HIGH : STD_LOW;
}

Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr)
{
    if ((ChannelGroupIdPtr == NULL_PTR) || (ChannelGroupIdPtr->port > PORT_E))
    {
        (void)Det_ReportError(DIO_MODULE_ID, 0U, DIO_SID_READ_CHANNEL_GROUP, DIO_E_PARAM_INVALID_GROUP);
        return 0U;
    }
    return (GPIO_PDIR(ChannelGroupIdPtr->port) & ChannelGroupIdPtr->mask) >> ChannelGroupIdPtr->offset;
}

void Dio_WriteChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr, Dio_PortLevelType Level)
{
    uint32 set;

    if ((ChannelGroupIdPtr == NULL_PTR) || (ChannelGroupIdPtr->port > PORT_E))
    {
        (void)Det_ReportError(DIO_MODULE_ID, 0U, DIO_SID_WRITE_CHANNEL_GROUP, DIO_E_PARAM_INVALID_GROUP);
        return;
    }
    set = (Level << ChannelGroupIdPtr->offset) & ChannelGroupIdPtr->mask;
    SchM_Enter_Exclusive();   /* clear + set must not be split by an ISR touching the same port */
    GPIO_PCOR(ChannelGroupIdPtr->port) = ChannelGroupIdPtr->mask & ~set;
    GPIO_PSOR(ChannelGroupIdPtr->port) = set;
    SchM_Exit_Exclusive();
}
