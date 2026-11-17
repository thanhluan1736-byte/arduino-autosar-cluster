/* Port_Cfg.c - pin configuration of the cluster board */
#include "Port.h"
#include "Dio_Cfg.h"

static const Port_PinConfigType Port_PinConfig[] =
{
    /* Pin                               Direction     Init      PullUp  DirChangeable */
    { DioConf_DioChannel_JOY_SW,         PORT_PIN_IN,  STD_LOW,  TRUE,   FALSE },
    { MCAL_PIN(MCAL_PORT_C, 0U),         PORT_PIN_IN,  STD_LOW,  FALSE,  FALSE },  /* A0 = joystick VRx (ADC) */
    { MCAL_PIN(MCAL_PORT_C, 1U),         PORT_PIN_IN,  STD_LOW,  FALSE,  FALSE },  /* A1 = joystick VRy (ADC) */
    { DioConf_DioChannel_STEP_IN1,       PORT_PIN_OUT, STD_LOW,  FALSE,  FALSE },
    { DioConf_DioChannel_STEP_IN2,       PORT_PIN_OUT, STD_LOW,  FALSE,  FALSE },
    { DioConf_DioChannel_STEP_IN3,       PORT_PIN_OUT, STD_LOW,  FALSE,  FALSE },
    { DioConf_DioChannel_STEP_IN4,       PORT_PIN_OUT, STD_LOW,  FALSE,  FALSE },
    { DioConf_DioChannel_BUZZER,         PORT_PIN_OUT, STD_LOW,  FALSE,  FALSE },
    { DioConf_DioChannel_LED_OVERSPEED,  PORT_PIN_OUT, STD_LOW,  FALSE,  FALSE },
    /* A4 = SDA, A5 = SCL: inputs without pull-up, the TWI unit takes them over (the LCD backpack has 4.7k pull-ups) */
    { MCAL_PIN(MCAL_PORT_C, 4U),         PORT_PIN_IN,  STD_LOW,  FALSE,  FALSE },
    { MCAL_PIN(MCAL_PORT_C, 5U),         PORT_PIN_IN,  STD_LOW,  FALSE,  FALSE },
};

const Port_ConfigType Port_Config =
{
    Port_PinConfig,
    (uint8)(sizeof(Port_PinConfig) / sizeof(Port_PinConfig[0]))
};
