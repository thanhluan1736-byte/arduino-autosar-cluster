/* Port.h - AUTOSAR Port driver (pin direction, initial level, pull-up) */
#ifndef PORT_H
#define PORT_H

#include "Std_Types.h"
#include "Mcal_Reg.h"

#define PORT_MODULE_ID                  124U

#define PORT_SID_INIT                   0x00U
#define PORT_SID_SET_PIN_DIRECTION      0x01U

#define PORT_E_PARAM_PIN                0x0AU
#define PORT_E_DIRECTION_UNCHANGEABLE   0x0BU
#define PORT_E_PARAM_CONFIG             0x0CU
#define PORT_E_UNINIT                   0x0FU

typedef uint8 Port_PinType;

typedef enum
{
    PORT_PIN_IN  = 0U,
    PORT_PIN_OUT = 1U
} Port_PinDirectionType;

typedef struct
{
    Port_PinType          Pin;
    Port_PinDirectionType Direction;
    uint8                 InitialLevel;          /* STD_LOW / STD_HIGH, outputs only */
    boolean               PullUp;                /* inputs only */
    boolean               DirectionChangeable;
} Port_PinConfigType;

typedef struct
{
    const Port_PinConfigType *Pins;
    uint8                     NumPins;
} Port_ConfigType;

extern const Port_ConfigType Port_Config;

void Port_Init(const Port_ConfigType *ConfigPtr);
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction);

#endif /* PORT_H */
