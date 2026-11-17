/* Gpt.h - AUTOSAR Gpt driver: channel 0 = Timer0, continuous mode, 4 us per tick */
#ifndef GPT_H
#define GPT_H

#include "Std_Types.h"

#define GPT_MODULE_ID                 100U

#define GPT_SID_INIT                  0x01U
#define GPT_SID_START_TIMER           0x05U
#define GPT_SID_ENABLE_NOTIFICATION   0x07U

#define GPT_E_UNINIT                  0x0AU
#define GPT_E_PARAM_CHANNEL           0x14U
#define GPT_E_PARAM_VALUE             0x15U
#define GPT_E_PARAM_POINTER           0x16U

typedef uint8  Gpt_ChannelType;
typedef uint16 Gpt_ValueType;          /* ticks of 4 us (prescaler 64 at 16 MHz) */
typedef void (*Gpt_NotificationType)(void);

typedef struct
{
    Gpt_NotificationType Notification;
} Gpt_ChannelConfigType;

typedef struct
{
    const Gpt_ChannelConfigType *Channels;
    uint8                        NumChannels;
} Gpt_ConfigType;

#define GPT_TICK_US                   4U
#define GpConf_GptChannel_OsTick      0U

extern const Gpt_ConfigType Gpt_Config;

void Gpt_Init(const Gpt_ConfigType *ConfigPtr);
void Gpt_StartTimer(Gpt_ChannelType Channel, Gpt_ValueType Value);
void Gpt_StopTimer(Gpt_ChannelType Channel);
void Gpt_EnableNotification(Gpt_ChannelType Channel);
void Gpt_DisableNotification(Gpt_ChannelType Channel);

#endif /* GPT_H */
