/* Gpt_Cfg.c (S32K144) - Gpt channel configuration */
#include "Gpt.h"
#include "Os.h"

static const Gpt_ChannelConfigType Gpt_Channels[] =
{
    { Os_TickNotification },   /* GpConf_GptChannel_OsTick: 1 ms system tick */
};

const Gpt_ConfigType Gpt_Config =
{
    Gpt_Channels,
    (uint8)(sizeof(Gpt_Channels) / sizeof(Gpt_Channels[0]))
};
