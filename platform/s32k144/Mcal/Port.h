/* Port.h - AUTOSAR Port driver (S32K144): pin mux, direction, initial level, pull-up */
#ifndef PORT_H
#define PORT_H

#include "Std_Types.h"

#define PORT_MODULE_ID                  124U

#define PORT_SID_INIT                   0x00U
#define PORT_SID_SET_PIN_DIRECTION      0x01U

#define PORT_E_PARAM_PIN                0x0AU
#define PORT_E_DIRECTION_UNCHANGEABLE   0x0BU
#define PORT_E_PARAM_CONFIG             0x0CU
#define PORT_E_UNINIT                   0x0FU

/* a pin / Dio channel is encoded as port * 32 + pin: PTD13 = 3 * 32 + 13 */
#define PORT_A      0U
#define PORT_B      1U
#define PORT_C      2U
#define PORT_D      3U
#define PORT_E      4U
#define PORT_PIN(port, pin)      ((uint8)(((uint8)(port) << 5U) | (uint8)(pin)))
#define PORT_PIN_PORT(id)        ((uint8)((uint8)(id) >> 5U))
#define PORT_PIN_NUM(id)         ((uint8)((uint8)(id) & 31U))
#define PORT_PIN_VALID(id)       (PORT_PIN_PORT(id) <= PORT_E)

typedef uint8 Port_PinType;

typedef enum
{
    PORT_PIN_IN  = 0U,
    PORT_PIN_OUT = 1U
} Port_PinDirectionType;

typedef enum
{
    PORT_PIN_MODE_ANALOG = 0U,   /* ALT0: ADC input */
    PORT_PIN_MODE_GPIO   = 1U,   /* ALT1 */
    PORT_PIN_MODE_ALT2   = 2U,   /* e.g. LPUART1 on PTC6 / PTC7 */
    PORT_PIN_MODE_ALT3   = 3U    /* e.g. LPI2C0 on PTA2 / PTA3 */
} Port_PinModeType;

typedef struct
{
    Port_PinType          Pin;
    Port_PinModeType      Mode;
    Port_PinDirectionType Direction;            /* GPIO only */
    uint8                 InitialLevel;         /* GPIO outputs: STD_LOW / STD_HIGH */
    boolean               PullUp;
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
