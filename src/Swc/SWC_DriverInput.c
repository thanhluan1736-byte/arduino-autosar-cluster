/* SWC_DriverInput - pedal positions, push-button events (short / long press) and the odometer reset gesture */
#include "Rte_SWC_DriverInput.h"
#include "IoHwAb.h"
#include "Swc_Cfg.h"

static boolean DriverInput_WasPressed = FALSE;
static boolean DriverInput_LongSent = FALSE;
static uint16  DriverInput_PressTime = 0U;
static uint8   DriverInput_Seq = 0U;
static boolean DriverInput_PullArmed = TRUE;   /* stick back near the centre: the next pull counts */
static uint8   DriverInput_Pulls = 0U;
static uint16  DriverInput_PullTimer = 0U;

static void DriverInput_SendEvent(uint8 evt)
{
    /* the sequence counter tells the receiver that a NEW event arrived (S/R data is a last-is-best value) */
    DriverInput_Seq++;
    (void)Rte_Write_Pp_ButtonEvent_Element_Event(evt);
    (void)Rte_Write_Pp_ButtonEvent_Element_Seq(DriverInput_Seq);
}

/* three full pulls of the stick towards "brake" within 3 s -> odometer reset request */
static void DriverInput_PullGesture(uint8 brake)
{
    if (DriverInput_Pulls > 0U)
    {
        DriverInput_PullTimer++;
        if (DriverInput_PullTimer >= ODO_GESTURE_WINDOW_10MS)
        {
            DriverInput_Pulls = 0U;            /* too slow: start again */
        }
    }
    if ((DriverInput_PullArmed == TRUE) && (brake >= ODO_GESTURE_PULL_ON_PCT))
    {
        DriverInput_PullArmed = FALSE;
        if (DriverInput_Pulls == 0U)
        {
            DriverInput_PullTimer = 0U;
        }
        DriverInput_Pulls++;
        if (DriverInput_Pulls >= ODO_GESTURE_PULLS)
        {
            DriverInput_Pulls = 0U;
            DriverInput_SendEvent(CLUSTER_BTN_ODO_RESET);
        }
    }
    else if (brake <= ODO_GESTURE_PULL_OFF_PCT)
    {
        DriverInput_PullArmed = TRUE;
    }
    else
    {
        /* between the two thresholds: no change (hysteresis) */
    }
}

void Runnable_DriverInput_10ms(void)
{
    boolean pressed = IoHwAb_GetButton();

    (void)Rte_Write_Pp_Pedal_Element_Throttle(IoHwAb_GetThrottle());
    (void)Rte_Write_Pp_Pedal_Element_Brake(IoHwAb_GetBrake());
    DriverInput_PullGesture(IoHwAb_GetBrake());

    if (pressed == TRUE)
    {
        if (DriverInput_WasPressed == FALSE)
        {
            DriverInput_PressTime = 0U;
            DriverInput_LongSent = FALSE;
        }
        if (DriverInput_PressTime < 0xFFFFU)
        {
            DriverInput_PressTime++;
        }
        if ((DriverInput_LongSent == FALSE) && (DriverInput_PressTime >= CLUSTER_BTN_LONG_PRESS_10MS))
        {
            DriverInput_SendEvent(CLUSTER_BTN_LONG);   /* fires while still held, like a real trip button */
            DriverInput_LongSent = TRUE;
        }
    }
    else if ((DriverInput_WasPressed == TRUE) && (DriverInput_LongSent == FALSE))
    {
        DriverInput_SendEvent(CLUSTER_BTN_SHORT);
    }
    else
    {
        /* idle */
    }
    DriverInput_WasPressed = pressed;
}
