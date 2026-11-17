/* Port_Cfg.c (S32K144) - pin configuration of the MaaZEDU board */
#include "Port.h"
#include "Dio_Cfg.h"

static const Port_PinConfigType Port_PinConfig[] =
{
    /* Pin                               Mode                   Direction     Init      PullUp  DirChangeable */
    { DioConf_DioChannel_JOY_SW,         PORT_PIN_MODE_GPIO,    PORT_PIN_IN,  STD_LOW,  TRUE,   FALSE },  /* J8-5  */
    { PORT_PIN(PORT_C, 14U),             PORT_PIN_MODE_ANALOG,  PORT_PIN_IN,  STD_LOW,  FALSE,  FALSE },  /* J8-18 VRy, ADC0_SE12 */
    { PORT_PIN(PORT_C, 15U),             PORT_PIN_MODE_ANALOG,  PORT_PIN_IN,  STD_LOW,  FALSE,  FALSE },  /* J8-20 VRx, ADC0_SE13 */
    { DioConf_DioChannel_STEP_IN1,       PORT_PIN_MODE_GPIO,    PORT_PIN_OUT, STD_LOW,  FALSE,  FALSE },  /* J8-19 */
    { DioConf_DioChannel_STEP_IN2,       PORT_PIN_MODE_GPIO,    PORT_PIN_OUT, STD_LOW,  FALSE,  FALSE },  /* J8-17 */
    { DioConf_DioChannel_STEP_IN3,       PORT_PIN_MODE_GPIO,    PORT_PIN_OUT, STD_LOW,  FALSE,  FALSE },  /* J8-15 */
    { DioConf_DioChannel_STEP_IN4,       PORT_PIN_MODE_GPIO,    PORT_PIN_OUT, STD_LOW,  FALSE,  FALSE },  /* J8-13 */
    { DioConf_DioChannel_BUZZER,         PORT_PIN_MODE_GPIO,    PORT_PIN_OUT, STD_LOW,  FALSE,  FALSE },  /* J8-11 */
    { DioConf_DioChannel_LED_OVERSPEED,  PORT_PIN_MODE_GPIO,    PORT_PIN_OUT, STD_HIGH, FALSE,  FALSE },  /* red, off */
    { DioConf_DioChannel_LED_GREEN,      PORT_PIN_MODE_GPIO,    PORT_PIN_OUT, STD_HIGH, FALSE,  FALSE },  /* off */
    { DioConf_DioChannel_LED_BLUE,       PORT_PIN_MODE_GPIO,    PORT_PIN_OUT, STD_HIGH, FALSE,  FALSE },  /* off */
    { PORT_PIN(PORT_A, 2U),              PORT_PIN_MODE_ALT3,    PORT_PIN_IN,  STD_LOW,  TRUE,   FALSE },  /* J8-12 LPI2C0 SDA */
    { PORT_PIN(PORT_A, 3U),              PORT_PIN_MODE_ALT3,    PORT_PIN_IN,  STD_LOW,  TRUE,   FALSE },  /* J8-10 LPI2C0 SCL */
    { PORT_PIN(PORT_C, 6U),              PORT_PIN_MODE_ALT2,    PORT_PIN_IN,  STD_LOW,  FALSE,  FALSE },  /* LPUART1 RX (USB COM) */
    { PORT_PIN(PORT_C, 7U),              PORT_PIN_MODE_ALT2,    PORT_PIN_IN,  STD_LOW,  FALSE,  FALSE },  /* LPUART1 TX (USB COM) */
};

const Port_ConfigType Port_Config =
{
    Port_PinConfig,
    (uint8)(sizeof(Port_PinConfig) / sizeof(Port_PinConfig[0]))
};
