/* Port.c - AUTOSAR Port driver for ATmega328P */
#include "Port.h"
#include "Det.h"
#include "SchM.h"

static const Port_ConfigType *Port_ConfigPtr = NULL_PTR;

static void Port_ApplyDirection(const Port_PinConfigType *cfg, Port_PinDirectionType dir)
{
    uint8 port = MCAL_PIN_PORT(cfg->Pin);
    uint8 mask = MCAL_PIN_MASK(cfg->Pin);

    SchM_Enter_Exclusive();
    if (dir == PORT_PIN_OUT)
    {
        /* set the level first so the pin does not glitch when it becomes an output */
        if (cfg->InitialLevel == STD_HIGH) { MCAL_REG_PORT(port) |= mask; }
        else                               { MCAL_REG_PORT(port) &= (uint8)~mask; }
        MCAL_REG_DDR(port) |= mask;
    }
    else
    {
        MCAL_REG_DDR(port) &= (uint8)~mask;
        if (cfg->PullUp == TRUE) { MCAL_REG_PORT(port) |= mask; }
        else                     { MCAL_REG_PORT(port) &= (uint8)~mask; }
    }
    SchM_Exit_Exclusive();
}

void Port_Init(const Port_ConfigType *ConfigPtr)
{
    uint8 i;

    if (ConfigPtr == NULL_PTR)
    {
        (void)Det_ReportError(PORT_MODULE_ID, 0U, PORT_SID_INIT, PORT_E_PARAM_CONFIG);
        return;
    }
    for (i = 0U; i < ConfigPtr->NumPins; i++)
    {
        const Port_PinConfigType *cfg = &ConfigPtr->Pins[i];
        if (!MCAL_PIN_VALID(cfg->Pin))
        {
            (void)Det_ReportError(PORT_MODULE_ID, 0U, PORT_SID_INIT, PORT_E_PARAM_PIN);
            continue;
        }
        Port_ApplyDirection(cfg, cfg->Direction);
    }
    Port_ConfigPtr = ConfigPtr;
}

void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction)
{
    uint8 i;

    if (Port_ConfigPtr == NULL_PTR)
    {
        (void)Det_ReportError(PORT_MODULE_ID, 0U, PORT_SID_SET_PIN_DIRECTION, PORT_E_UNINIT);
        return;
    }
    for (i = 0U; i < Port_ConfigPtr->NumPins; i++)
    {
        const Port_PinConfigType *cfg = &Port_ConfigPtr->Pins[i];
        if (cfg->Pin == Pin)
        {
            if (cfg->DirectionChangeable == TRUE)
            {
                Port_ApplyDirection(cfg, Direction);
            }
            else
            {
                (void)Det_ReportError(PORT_MODULE_ID, 0U, PORT_SID_SET_PIN_DIRECTION, PORT_E_DIRECTION_UNCHANGEABLE);
            }
            return;
        }
    }
    (void)Det_ReportError(PORT_MODULE_ID, 0U, PORT_SID_SET_PIN_DIRECTION, PORT_E_PARAM_PIN);
}
