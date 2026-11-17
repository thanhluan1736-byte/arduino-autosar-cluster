/* IoHwAb.c - joystick -> throttle / brake, joystick push button, buzzer and telltale outputs */
#include "IoHwAb.h"
#include "Adc.h"
#include "Dio.h"
#include "Pwm.h"

static Adc_ValueGroupType IoHwAb_AdcBuffer[2];
static uint16  IoHwAb_JoyRawX = IOHWAB_JOY_CENTER;
static uint16  IoHwAb_JoyRawY = IOHWAB_JOY_CENTER;
static uint8   IoHwAb_Throttle = 0U;
static uint8   IoHwAb_Brake = 0U;
static boolean IoHwAb_Button = FALSE;
static uint8   IoHwAb_DebounceCnt = 0U;

static uint8 IoHwAb_Scale(uint16 delta, uint16 span)
{
    uint32 pct = ((uint32)delta * 100U) / span;
    return (pct > 100U) ? 100U : (uint8)pct;
}

static uint16 IoHwAb_Deflection(uint16 raw)
{
    return (raw >= IOHWAB_JOY_CENTER) ? (uint16)(raw - IOHWAB_JOY_CENTER) : (uint16)(IOHWAB_JOY_CENTER - raw);
}

static void IoHwAb_UpdatePedals(uint16 raw)
{
    const uint16 hi = IOHWAB_JOY_CENTER + IOHWAB_JOY_DEADZONE;
    const uint16 lo = IOHWAB_JOY_CENTER - IOHWAB_JOY_DEADZONE;

#if (IOHWAB_JOY_Y_INVERT == STD_ON)
    raw = (uint16)(ADC_RESULT_MAX - raw);
#endif
    IoHwAb_Throttle = 0U;
    IoHwAb_Brake = 0U;
    if (raw > hi)
    {
        IoHwAb_Throttle = IoHwAb_Scale((uint16)(raw - hi), (uint16)(ADC_RESULT_MAX - hi));
    }
    else if (raw < lo)
    {
        IoHwAb_Brake = IoHwAb_Scale((uint16)(lo - raw), lo);
    }
    else
    {
        /* dead zone: coasting */
    }
}

void IoHwAb_Init(void)
{
    (void)Adc_SetupResultBuffer(AdcConf_AdcGroup_Joystick, IoHwAb_AdcBuffer);
    Adc_StartGroupConversion(AdcConf_AdcGroup_Joystick);
    IoHwAb_SetBuzzer(FALSE);
    IoHwAb_SetTelltaleOverspeed(FALSE);
}

void IoHwAb_MainFunction(void)
{
    Adc_ValueGroupType value[2];
    boolean raw;

    /* analog: read the finished conversion, start the next one */
    if (Adc_GetGroupStatus(AdcConf_AdcGroup_Joystick) == ADC_STREAM_COMPLETED)
    {
        if (Adc_ReadGroup(AdcConf_AdcGroup_Joystick, value) == E_OK)
        {
            IoHwAb_JoyRawX = value[0];
            IoHwAb_JoyRawY = value[1];
            /* the module can be held either way round: the axis that is pushed further is the pedal */
            IoHwAb_UpdatePedals((IoHwAb_Deflection(IoHwAb_JoyRawX) > IoHwAb_Deflection(IoHwAb_JoyRawY)) ?
                                IoHwAb_JoyRawX : IoHwAb_JoyRawY);
        }
        Adc_StartGroupConversion(AdcConf_AdcGroup_Joystick);
    }

    /* push button: active low, accept a new state after N equal samples */
    raw = (Dio_ReadChannel(DioConf_DioChannel_JOY_SW) == STD_LOW) ? TRUE : FALSE;
    if (raw != IoHwAb_Button)
    {
        IoHwAb_DebounceCnt++;
        if (IoHwAb_DebounceCnt >= IOHWAB_DEBOUNCE_SAMPLES)
        {
            IoHwAb_Button = raw;
            IoHwAb_DebounceCnt = 0U;
        }
    }
    else
    {
        IoHwAb_DebounceCnt = 0U;
    }
}

void IoHwAb_GetDebug(uint16 *RawX, uint16 *RawY, boolean *Button)
{
    *RawX = IoHwAb_JoyRawX;
    *RawY = IoHwAb_JoyRawY;
    *Button = IoHwAb_Button;
}

uint8 IoHwAb_GetThrottle(void)
{
    return IoHwAb_Throttle;
}

uint8 IoHwAb_GetBrake(void)
{
    return IoHwAb_Brake;
}

boolean IoHwAb_GetButton(void)
{
    return IoHwAb_Button;
}

void IoHwAb_SetBuzzer(boolean On)
{
    /* 4 kHz square wave: works for the kit's passive buzzer and for an active one */
    Pwm_SetDutyCycle(PwmConf_PwmChannel_Buzzer, (On == TRUE) ? PWM_DUTY_50 : PWM_DUTY_0);
}

void IoHwAb_SetTelltaleOverspeed(boolean On)
{
    Dio_WriteChannel(DioConf_DioChannel_LED_OVERSPEED,
                     (On == TRUE) ? DIO_LED_OVERSPEED_ON_LEVEL : (Dio_LevelType)(DIO_LED_OVERSPEED_ON_LEVEL ^ 1U));
}
