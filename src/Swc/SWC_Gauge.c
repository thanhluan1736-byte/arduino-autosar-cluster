/* SWC_Gauge - speedometer needle (with power-up needle sweep) and heartbeat speed warning (telltale + chime) */
#include "Rte_SWC_Gauge.h"
#include "Cdd_Stepper.h"
#include "IoHwAb.h"
#include "Swc_Cfg.h"

typedef enum
{
    GAUGE_INIT = 0U,
    GAUGE_SWEEP_UP,
    GAUGE_SWEEP_DOWN,
    GAUGE_NORMAL
} Gauge_StateType;

static Gauge_StateType Gauge_State = GAUGE_INIT;
static uint8   Gauge_WarnLevel = 0U;     /* 0 = no warning, 1..4 */
static uint16  Gauge_WarnMs = 0U;        /* time inside the current heartbeat */
static uint8   Gauge_ChimeCnt = 0U;      /* power-up beep */

typedef struct
{
    uint16 WindowMs;     /* the beeps of one beat are spread evenly over this window ... */
    uint16 PauseMs;      /* ... followed by this silence, then the next beat */
    uint8  Beeps;        /* beeps per beat (one every WindowMs / Beeps) */
} Gauge_WarnPatternType;

/* index = level - 1: the heart beats faster as the speed rises */
static const Gauge_WarnPatternType Gauge_WarnPattern[4] =
{
    { GAUGE_WARN_L1_WINDOW_MS, GAUGE_WARN_L1_PAUSE_MS, GAUGE_WARN_BEEPS },
    { GAUGE_WARN_L2_WINDOW_MS, GAUGE_WARN_L2_PAUSE_MS, GAUGE_WARN_BEEPS },
    { GAUGE_WARN_L3_WINDOW_MS, GAUGE_WARN_L3_PAUSE_MS, GAUGE_WARN_BEEPS },
    { GAUGE_WARN_L4_WINDOW_MS, GAUGE_WARN_L4_PAUSE_MS, GAUGE_WARN_BEEPS },
};

static const uint16 Gauge_WarnThreshold[4] =
{
    GAUGE_WARN_L1_KMH10, GAUGE_WARN_L2_KMH10, GAUGE_WARN_L3_KMH10, GAUGE_WARN_L4_KMH10
};

static sint16 Gauge_SpeedToSteps(uint16 speed10)
{
    uint32 steps = ((uint32)speed10 * (uint32)CDD_STEPPER_MAX_STEPS) / GAUGE_FULL_SCALE_KMH10;
    return (steps > (uint32)CDD_STEPPER_MAX_STEPS) ? CDD_STEPPER_MAX_STEPS : (sint16)steps;
}

/* number of thresholds reached by speed10 + offset */
static uint8 Gauge_LevelFor(uint16 speed10, uint16 offset)
{
    uint8 i;
    uint8 level = 0U;

    for (i = 0U; i < 4U; i++)
    {
        if ((uint32)speed10 + offset >= Gauge_WarnThreshold[i])
        {
            level = (uint8)(i + 1U);
        }
    }
    return level;
}

static void Gauge_Warning(uint16 speed10)
{
    uint8 up = Gauge_LevelFor(speed10, 0U);
    uint8 down = Gauge_LevelFor(speed10, GAUGE_WARN_HYST_KMH10);
    uint8 level = Gauge_WarnLevel;
    boolean beep = FALSE;

    /* rise at once, fall only 2 km/h below the threshold (no flicker around 65 / 90 / 120 / 150) */
    if (up > level)        { level = up; }
    else if (down < level) { level = down; }
    else                   { /* keep */ }

    if (level != Gauge_WarnLevel)
    {
        Gauge_WarnLevel = level;
        Gauge_WarnMs = 0U;                  /* a new level starts with a beat */
    }

    if (level >= 1U)
    {
        const Gauge_WarnPatternType *p = &Gauge_WarnPattern[level - 1U];

        /* inside the window, beep k starts at k * Window / Beeps: t is in a beep when
         * (t * Beeps) mod Window < On * Beeps. Integer maths over the whole window: no rounding drift. */
        beep = ((Gauge_WarnMs < p->WindowMs) &&
                ((uint16)(((uint32)Gauge_WarnMs * p->Beeps) % p->WindowMs) < (uint16)(GAUGE_BEEP_ON_MS * p->Beeps))) ? TRUE : FALSE;
        Gauge_WarnMs = (uint16)(Gauge_WarnMs + GAUGE_RUNNABLE_PERIOD_MS);
        if (Gauge_WarnMs >= (uint16)(p->WindowMs + p->PauseMs))
        {
            Gauge_WarnMs = 0U;
        }
    }
    else
    {
        Gauge_WarnMs = 0U;
    }
    IoHwAb_SetBuzzer(beep);
    IoHwAb_SetTelltaleOverspeed((level >= 1U) ? TRUE : FALSE);
}

void Runnable_Gauge_20ms(void)
{
    dt_uint16 speed10 = 0U;

    switch (Gauge_State)
    {
    case GAUGE_INIT:
        /* needle sweep = check that the gauge works, as real clusters do at ignition on */
        IoHwAb_SetTelltaleOverspeed(TRUE);    /* bulb check */
        IoHwAb_SetBuzzer(TRUE);               /* chime check: one short beep at power-up */
        Gauge_ChimeCnt = 0U;
        Cdd_Stepper_SetTarget(CDD_STEPPER_MAX_STEPS);
        Gauge_State = GAUGE_SWEEP_UP;
        break;

    case GAUGE_SWEEP_UP:
        if (Gauge_ChimeCnt < GAUGE_STARTUP_BEEP_20MS)
        {
            Gauge_ChimeCnt++;
            if (Gauge_ChimeCnt == GAUGE_STARTUP_BEEP_20MS)
            {
                IoHwAb_SetBuzzer(FALSE);
            }
        }
        if (Cdd_Stepper_GetPosition() >= CDD_STEPPER_MAX_STEPS)
        {
            Cdd_Stepper_SetTarget(0);
            Gauge_State = GAUGE_SWEEP_DOWN;
        }
        break;

    case GAUGE_SWEEP_DOWN:
        if (Cdd_Stepper_GetPosition() <= 0)
        {
            IoHwAb_SetTelltaleOverspeed(FALSE);
            Gauge_State = GAUGE_NORMAL;
        }
        break;

    case GAUGE_NORMAL:
    default:
        if (Rte_Read_Rp_SpeedGauge_Element_Speed(&speed10) != RTE_E_OK)
        {
            speed10 = 0U;   /* no data -> needle at 0 */
        }
        Cdd_Stepper_SetTarget(Gauge_SpeedToSteps(speed10));
        Gauge_Warning(speed10);
        break;
    }
}
