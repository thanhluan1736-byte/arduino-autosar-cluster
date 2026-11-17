/* Port.c (S32K144) - PORTx_PCRn (mux, pull) and GPIO direction */
#include "Port.h"
#include "Det.h"
#include "S32K144_Regs.h"

static const Port_ConfigType *Port_ConfigPtr = NULL_PTR;

static void Port_ApplyDirection(const Port_PinConfigType *cfg, Port_PinDirectionType dir)
{
    uint8 port = PORT_PIN_PORT(cfg->Pin);
    uint32 mask = 1UL << PORT_PIN_NUM(cfg->Pin);

    if (dir == PORT_PIN_OUT)
    {
        /* level first, so the pin does not glitch when it becomes an output (PSOR/PCOR are atomic) */
        if (cfg->InitialLevel == STD_HIGH) { GPIO_PSOR(port) = mask; }
        else                               { GPIO_PCOR(port) = mask; }
        GPIO_PDDR(port) |= mask;
    }
    else
    {
        GPIO_PDDR(port) &= ~mask;
    }
}

void Port_Init(const Port_ConfigType *ConfigPtr)
{
    uint8 i;

    if (ConfigPtr == NULL_PTR)
    {
        (void)Det_ReportError(PORT_MODULE_ID, 0U, PORT_SID_INIT, PORT_E_PARAM_CONFIG);
        return;
    }
    /* clocks of all five ports */
    PCC_REG(PCC_PORTA_INDEX) = PCC_CGC;
    PCC_REG(PCC_PORTB_INDEX) = PCC_CGC;
    PCC_REG(PCC_PORTC_INDEX) = PCC_CGC;
    PCC_REG(PCC_PORTD_INDEX) = PCC_CGC;
    PCC_REG(PCC_PORTE_INDEX) = PCC_CGC;

    for (i = 0U; i < ConfigPtr->NumPins; i++)
    {
        const Port_PinConfigType *cfg = &ConfigPtr->Pins[i];
        uint32 pcr;

        if (!PORT_PIN_VALID(cfg->Pin))
        {
            (void)Det_ReportError(PORT_MODULE_ID, 0U, PORT_SID_INIT, PORT_E_PARAM_PIN);
            continue;
        }
        pcr = PORT_PCR_MUX(cfg->Mode);
        if (cfg->PullUp == TRUE)
        {
            pcr |= PORT_PCR_PE | PORT_PCR_PS;
        }
        if (cfg->Mode == PORT_PIN_MODE_GPIO)
        {
            Port_ApplyDirection(cfg, cfg->Direction);
        }
        PORT_PCR(PORT_PIN_PORT(cfg->Pin), PORT_PIN_NUM(cfg->Pin)) = pcr;
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
