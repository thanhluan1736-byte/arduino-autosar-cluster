/* Adc_Cfg.c - ADC groups of the cluster */
#include "Adc.h"

/* joystick module of the kit (lesson 11): VRx = A0, VRy = A1. Both are read; IoHwAb uses the axis that moves most. */
static const Adc_ChannelType Adc_JoystickChannels[] = { 0U /* A0 = VRx */, 1U /* A1 = VRy */ };

static const Adc_GroupConfigType Adc_Groups[] =
{
    { Adc_JoystickChannels, (uint8)(sizeof(Adc_JoystickChannels) / sizeof(Adc_JoystickChannels[0])) },
};

const Adc_ConfigType Adc_Config =
{
    Adc_Groups,
    (uint8)(sizeof(Adc_Groups) / sizeof(Adc_Groups[0]))
};
