/* Gpt.c - AUTOSAR Gpt driver on Timer0 (CTC mode, compare match A interrupt) */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Gpt.h"
#include "Det.h"

static const Gpt_ConfigType *Gpt_ConfigPtr = NULL_PTR;
static volatile boolean Gpt_NotifEnabled = FALSE;

void Gpt_Init(const Gpt_ConfigType *ConfigPtr)
{
    if ((ConfigPtr == NULL_PTR) || (ConfigPtr->NumChannels != 1U))
    {
        (void)Det_ReportError(GPT_MODULE_ID, 0U, GPT_SID_INIT, GPT_E_PARAM_POINTER);
        return;
    }
    TCCR0B = 0U;                         /* stopped */
    TCCR0A = (uint8)(1U << WGM01);       /* CTC, TOP = OCR0A */
    TIMSK0 = 0U;
    Gpt_NotifEnabled = FALSE;
    Gpt_ConfigPtr = ConfigPtr;
}

void Gpt_StartTimer(Gpt_ChannelType Channel, Gpt_ValueType Value)
{
    if (Gpt_ConfigPtr == NULL_PTR)
    {
        (void)Det_ReportError(GPT_MODULE_ID, 0U, GPT_SID_START_TIMER, GPT_E_UNINIT);
        return;
    }
    if (Channel >= Gpt_ConfigPtr->NumChannels)
    {
        (void)Det_ReportError(GPT_MODULE_ID, 0U, GPT_SID_START_TIMER, GPT_E_PARAM_CHANNEL);
        return;
    }
    if ((Value == 0U) || (Value > 256U))  /* 8-bit timer */
    {
        (void)Det_ReportError(GPT_MODULE_ID, 0U, GPT_SID_START_TIMER, GPT_E_PARAM_VALUE);
        return;
    }
    OCR0A = (uint8)(Value - 1U);
    TCNT0 = 0U;
    TIFR0 = (uint8)(1U << OCF0A);
    TCCR0B = (uint8)((1U << CS01) | (1U << CS00));   /* prescaler 64 -> 4 us */
}

void Gpt_StopTimer(Gpt_ChannelType Channel)
{
    (void)Channel;
    TCCR0B = 0U;
}

void Gpt_EnableNotification(Gpt_ChannelType Channel)
{
    if ((Gpt_ConfigPtr == NULL_PTR) || (Channel >= Gpt_ConfigPtr->NumChannels))
    {
        (void)Det_ReportError(GPT_MODULE_ID, 0U, GPT_SID_ENABLE_NOTIFICATION, GPT_E_PARAM_CHANNEL);
        return;
    }
    Gpt_NotifEnabled = TRUE;
    TIMSK0 |= (uint8)(1U << OCIE0A);
}

void Gpt_DisableNotification(Gpt_ChannelType Channel)
{
    (void)Channel;
    TIMSK0 &= (uint8)~(1U << OCIE0A);
    Gpt_NotifEnabled = FALSE;
}

ISR(TIMER0_COMPA_vect)
{
    if ((Gpt_NotifEnabled == TRUE) && (Gpt_ConfigPtr->Channels[0].Notification != NULL_PTR))
    {
        Gpt_ConfigPtr->Channels[0].Notification();
    }
}
