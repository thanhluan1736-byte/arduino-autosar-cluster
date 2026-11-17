/* Swc_Cfg.h - application calibration shared by the cluster SWCs */
#ifndef SWC_CFG_H
#define SWC_CFG_H

#include "Std_Types.h"   /* the generated Rte_Type.h has no uint32 / sint types */

/* ---- IF_ButtonEvent.Element_Event values ---- */
#define CLUSTER_BTN_NONE              0U
#define CLUSTER_BTN_SHORT             1U
#define CLUSTER_BTN_LONG              2U
#define CLUSTER_BTN_ODO_RESET         3U       /* joystick pulled back 3 times quickly */
#define CLUSTER_BTN_LONG_PRESS_10MS   150U     /* 1.5 s */

/* ---- odometer reset gesture: pull the stick fully back 3 times within 3 s, car standing still ---- */
#define ODO_GESTURE_PULL_ON_PCT       80U      /* brake >= 80 % counts as a pull ... */
#define ODO_GESTURE_PULL_OFF_PCT      20U      /* ... and the stick must come back below 20 % in between */
#define ODO_GESTURE_PULLS             3U
#define ODO_GESTURE_WINDOW_10MS       300U     /* 3 s from the first pull */
#define DISP_ODO_RESET_MSG_100MS      20U      /* "ODO reset" shown for 2 s */

/* ---- vehicle model (all per 10 ms step, speed unit = 0.001 km/h) ---- */
#define VEH_SPEED_MAX_MKMH            220000UL /* 220 km/h hard limit */
#define VEH_ACCEL_MAX_PER_STEP        108UL    /* 3.0 m/s^2 = 10.8 km/h per s at 100 % throttle */
#define VEH_BRAKE_MAX_PER_STEP        288UL    /* 8.0 m/s^2 at 100 % brake */
#define VEH_ROLLING_PER_STEP          5UL      /* rolling resistance */
#define VEH_AERO_DIVISOR              400UL    /* drag = v_kmh^2 / 400 -> top speed ~200 km/h */

/* ---- gauge ---- */
#define GAUGE_FULL_SCALE_KMH10        2400UL   /* dial 0..240 km/h over CDD_STEPPER_MAX_STEPS */
/* speed warning: one beep per heartbeat, and the heart beats faster with the vehicle speed (a racing heart).
 * Pattern = BEEPS beeps spread over a window, then a pause; one beep -> window = beep length (ms). */
#define GAUGE_WARN_L1_KMH10           650U     /* >= 65 km/h : beep every 2.3 s (~26 bpm, calm)    */
#define GAUGE_WARN_L2_KMH10           900U     /* >= 90 km/h : beep every 1.5 s (40 bpm)           */
#define GAUGE_WARN_L3_KMH10           1200U    /* >= 120 km/h: beep every 1.0 s (60 bpm)           */
#define GAUGE_WARN_L4_KMH10           1501U    /* > 150 km/h : beep every 0.6 s (100 bpm, anxious) */
#define GAUGE_WARN_HYST_KMH10         20U      /* 2 km/h below a threshold before the level drops */
#define GAUGE_WARN_BEEPS              1U       /* one beep per beat (2 = "lub-dub") */
#define GAUGE_WARN_L1_WINDOW_MS       100U
#define GAUGE_WARN_L2_WINDOW_MS       100U
#define GAUGE_WARN_L3_WINDOW_MS       100U
#define GAUGE_WARN_L4_WINDOW_MS       100U
#define GAUGE_WARN_L1_PAUSE_MS        2200U    /* 0.1 + 2.2 = 2.3 s */
#define GAUGE_WARN_L2_PAUSE_MS        1400U    /* 0.1 + 1.4 = 1.5 s */
#define GAUGE_WARN_L3_PAUSE_MS        900U     /* 0.1 + 0.9 = 1.0 s */
#define GAUGE_WARN_L4_PAUSE_MS        500U     /* 0.1 + 0.5 = 0.6 s */
#define GAUGE_BEEP_ON_MS              100U     /* length of one beep */
#define GAUGE_RUNNABLE_PERIOD_MS      20U
#define GAUGE_OVERSPEED_ON_KMH10      GAUGE_WARN_L1_KMH10   /* telltale LED and "!" on the LCD */
#define GAUGE_STARTUP_BEEP_20MS       15U      /* 300 ms beep at power-up (buzzer check) */

/* ---- odometer ---- */
#define ODO_SAVE_DISTANCE_M           1000UL   /* save at least every km ... */
#define ODO_SAVE_STANDSTILL_100MS     10U      /* ... and 1 s after the car stops */
#define ODO_LOG_PERIOD_100MS          10U

/* ---- display ---- */
#define DISP_LAMPTEST_100MS           15U      /* "8888" for 1.5 s at power-up */
#define DISP_NVM_ERROR_100MS          20U      /* "Err" for 2 s if the odometer was lost */

#endif /* SWC_CFG_H */
