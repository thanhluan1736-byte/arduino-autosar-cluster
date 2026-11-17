/* Adc_Cfg.c (S32K144) - ADC groups of the cluster */
#include "Adc.h"

/* joystick: VRx = J8-20 (ADC0_SE13, PTC15), VRy = J8-18 (ADC0_SE12, PTC14). Order X, Y as on the Arduino. */
static const Adc_ChannelType Adc_JoystickChannels[] = { 13U /* VRx */, 12U /* VRy */ };

static const Adc_GroupConfigType Adc_Groups[] =
{
    { Adc_JoystickChannels, (uint8)(sizeof(Adc_JoystickChannels) / sizeof(Adc_JoystickChannels[0])) },
};

const Adc_ConfigType Adc_Config =
{
    Adc_Groups,
    (uint8)(sizeof(Adc_Groups) / sizeof(Adc_Groups[0]))
};
