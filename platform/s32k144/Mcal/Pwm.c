/* Pwm.c (S32K144) - LPIT0 channel 1 interrupts at 2 x f and toggles the buzzer pin (PTD13, J8-11) */
#include "Pwm.h"
#include "Gpt.h"
#include "Det.h"
#include "Dio.h"
#include "S32K144_Regs.h"

#define PWM_TOGGLE_TICKS   (PERIPH_CLOCK_HZ / (2UL * PWM_BUZZER_FREQ_HZ))   /* 6000 -> 4 kHz */

static boolean Pwm_Initialized = FALSE;
static boolean Pwm_Running = FALSE;

void Pwm_Init(void)
{
    Gpt_LpitModuleInit();
    LPIT_TCTRL(1U) = 0U;
    LPIT_TVAL(1U) = PWM_TOGGLE_TICKS - 1U;
    LPIT_MIER |= (1UL << 1);
    Nvic_EnableIrq(IRQ_LPIT0_CH1, 1U);
    Pwm_Running = FALSE;
    Pwm_Initialized = TRUE;
}

void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, uint16 DutyCycle)
{
    if (Pwm_Initialized == FALSE)
    {
        (void)Det_ReportError(PWM_MODULE_ID, 0U, PWM_SID_SET_DUTY_CYCLE, PWM_E_UNINIT);
        return;
    }
    if (ChannelNumber != PwmConf_PwmChannel_Buzzer)
    {
        (void)Det_ReportError(PWM_MODULE_ID, 0U, PWM_SID_SET_DUTY_CYCLE, PWM_E_PARAM_CHANNEL);
        return;
    }
    if (DutyCycle == PWM_DUTY_0)
    {
        if (Pwm_Running == TRUE)
        {
            LPIT_TCTRL(1U) = 0U;
            Dio_WriteChannel(DioConf_DioChannel_BUZZER, STD_LOW);   /* rest level */
            Pwm_Running = FALSE;
        }
    }
    else if (Pwm_Running == FALSE)
    {
        LPIT_MSR = (1UL << 1);
        LPIT_TCTRL(1U) = LPIT_TCTRL_T_EN;
        Pwm_Running = TRUE;
    }
    else
    {
        /* already sounding */
    }
}

void LPIT0_Ch1_IRQHandler(void)
{
    LPIT_MSR = (1UL << 1);
    (void)Dio_FlipChannel(DioConf_DioChannel_BUZZER);
}
