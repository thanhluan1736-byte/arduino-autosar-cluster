/* Cdd_Stepper.c - position control of the needle, one full step per call */
#include "Cdd_Stepper.h"
#include "Dio.h"

/* IN1..IN4 patterns, two coils energised (more torque than wave drive) */
static const uint8 Cdd_Stepper_Seq[4] = { 0x03U, 0x06U, 0x0CU, 0x09U };

static sint16 Cdd_Stepper_Pos = 0;      /* assumes the needle is at 0 at power-up */
static sint16 Cdd_Stepper_Target = 0;
static uint8  Cdd_Stepper_Phase = 0U;
static uint16 Cdd_Stepper_IdleCnt = CDD_STEPPER_IDLE_OFF;

static void Cdd_Stepper_Output(uint8 pattern)
{
    Dio_WriteChannelGroup(&DioConf_DioChannelGroup_Stepper, pattern);
}

void Cdd_Stepper_Init(void)
{
    Cdd_Stepper_Pos = 0;
    Cdd_Stepper_Target = 0;
    Cdd_Stepper_Phase = 0U;
    Cdd_Stepper_IdleCnt = CDD_STEPPER_IDLE_OFF;
    Cdd_Stepper_Output(0U);
}

void Cdd_Stepper_SetTarget(sint16 Steps)
{
    if (Steps < 0)                          { Steps = 0; }
    else if (Steps > CDD_STEPPER_MAX_STEPS) { Steps = CDD_STEPPER_MAX_STEPS; }
    else                                    { /* in range */ }
    Cdd_Stepper_Target = Steps;
}

sint16 Cdd_Stepper_GetPosition(void)
{
    return Cdd_Stepper_Pos;
}

sint16 Cdd_Stepper_GetTarget(void)
{
    return Cdd_Stepper_Target;
}

void Cdd_Stepper_MainFunction(void)
{
    boolean forward;

    if (Cdd_Stepper_Pos == Cdd_Stepper_Target)
    {
        if (Cdd_Stepper_IdleCnt < CDD_STEPPER_IDLE_OFF)
        {
            Cdd_Stepper_IdleCnt++;
            if (Cdd_Stepper_IdleCnt == CDD_STEPPER_IDLE_OFF)
            {
                Cdd_Stepper_Output(0U);    /* gearbox holds the needle, coils can be released */
            }
        }
        return;
    }
    if (Cdd_Stepper_IdleCnt >= CDD_STEPPER_IDLE_OFF)
    {
        /* coils were off: energise the current phase first so no step is lost */
        Cdd_Stepper_IdleCnt = 0U;
        Cdd_Stepper_Output(Cdd_Stepper_Seq[Cdd_Stepper_Phase]);
        return;
    }
    Cdd_Stepper_IdleCnt = 0U;
    forward = (Cdd_Stepper_Target > Cdd_Stepper_Pos) ? TRUE : FALSE;
    Cdd_Stepper_Pos = (sint16)(Cdd_Stepper_Pos + ((forward == TRUE) ? 1 : -1));
#if (CDD_STEPPER_REVERSE == STD_ON)
    forward = (forward == TRUE) ? FALSE : TRUE;
#endif
    Cdd_Stepper_Phase = (uint8)((Cdd_Stepper_Phase + ((forward == TRUE) ? 1U : 3U)) & 3U);
    Cdd_Stepper_Output(Cdd_Stepper_Seq[Cdd_Stepper_Phase]);
}
