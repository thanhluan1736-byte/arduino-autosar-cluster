/* Gpt.c (S32K144) - LPIT0 channel 0 in 32-bit periodic mode, interrupt on every period */
#include "Gpt.h"
#include "Det.h"
#include "S32K144_Regs.h"

static const Gpt_ConfigType *Gpt_ConfigPtr = NULL_PTR;
static volatile boolean Gpt_NotifEnabled = FALSE;
static boolean Gpt_LpitReady = FALSE;

void Gpt_LpitModuleInit(void)
{
    volatile uint32 i;

    if (Gpt_LpitReady == TRUE)
    {
        return;
    }
    PCC_REG(PCC_LPIT_INDEX) = PCC_PCS(PCC_PCS_FIRCDIV2) | PCC_CGC;
    LPIT_MCR = LPIT_MCR_M_CEN | LPIT_MCR_DBG_EN;       /* run, also while the debugger halts the core */
    for (i = 0U; i < 8U; i++) { }                       /* a few clock cycles before the timer registers are writable */
    Gpt_LpitReady = TRUE;
}

void Gpt_Init(const Gpt_ConfigType *ConfigPtr)
{
    if ((ConfigPtr == NULL_PTR) || (ConfigPtr->NumChannels != 1U))
    {
        (void)Det_ReportError(GPT_MODULE_ID, 0U, GPT_SID_INIT, GPT_E_PARAM_POINTER);
        return;
    }
    Gpt_LpitModuleInit();
    LPIT_TCTRL(0U) = 0U;                                /* stopped, 32-bit periodic */
    LPIT_MIER &= ~(1UL << 0);
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
    if (Value < 2U)
    {
        (void)Det_ReportError(GPT_MODULE_ID, 0U, GPT_SID_START_TIMER, GPT_E_PARAM_VALUE);
        return;
    }
    LPIT_TVAL(0U) = Value - 1U;
    LPIT_MSR = (1UL << 0);                              /* clear a stale flag (write 1) */
    LPIT_TCTRL(0U) = LPIT_TCTRL_T_EN;
}

void Gpt_StopTimer(Gpt_ChannelType Channel)
{
    (void)Channel;
    LPIT_TCTRL(0U) = 0U;
}

void Gpt_EnableNotification(Gpt_ChannelType Channel)
{
    if ((Gpt_ConfigPtr == NULL_PTR) || (Channel >= Gpt_ConfigPtr->NumChannels))
    {
        (void)Det_ReportError(GPT_MODULE_ID, 0U, GPT_SID_ENABLE_NOTIFICATION, GPT_E_PARAM_CHANNEL);
        return;
    }
    Gpt_NotifEnabled = TRUE;
    LPIT_MIER |= (1UL << 0);
    Nvic_EnableIrq(IRQ_LPIT0_CH0, 2U);
}

void Gpt_DisableNotification(Gpt_ChannelType Channel)
{
    (void)Channel;
    LPIT_MIER &= ~(1UL << 0);
    Gpt_NotifEnabled = FALSE;
}

void LPIT0_Ch0_IRQHandler(void)
{
    LPIT_MSR = (1UL << 0);                              /* acknowledge */
    if ((Gpt_NotifEnabled == TRUE) && (Gpt_ConfigPtr->Channels[0].Notification != NULL_PTR))
    {
        Gpt_ConfigPtr->Channels[0].Notification();
    }
}
