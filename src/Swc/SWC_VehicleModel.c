/* SWC_VehicleModel - simple longitudinal model: throttle / brake / rolling + aero drag -> speed */
#include "Rte_SWC_VehicleModel.h"
#include "Swc_Cfg.h"

static uint32 VehicleModel_SpeedMkmh = 0U;   /* 0.001 km/h */

void Runnable_VehicleModel_10ms(void)
{
    dt_uint8 throttle = 0U;
    dt_uint8 brake = 0U;
    uint32 v = VehicleModel_SpeedMkmh;
    uint32 vKmh;
    uint32 decel;
    uint32 speed10;

    /* never received -> stay safe (no throttle) */
    if (Rte_Read_Rp_Pedal_Element_Throttle(&throttle) != RTE_E_OK) { throttle = 0U; }
    if (Rte_Read_Rp_Pedal_Element_Brake(&brake) != RTE_E_OK)       { brake = 0U; }

    v += ((uint32)throttle * VEH_ACCEL_MAX_PER_STEP) / 100U;

    vKmh  = v / 1000U;
    decel = ((uint32)brake * VEH_BRAKE_MAX_PER_STEP) / 100U;
    if (v > 0U)
    {
        decel += VEH_ROLLING_PER_STEP + ((vKmh * vKmh) / VEH_AERO_DIVISOR);
    }
    v = (decel >= v) ? 0U : (v - decel);
    if (v > VEH_SPEED_MAX_MKMH)
    {
        v = VEH_SPEED_MAX_MKMH;
    }
    VehicleModel_SpeedMkmh = v;

    speed10 = v / 100U;   /* 0.1 km/h */
    (void)Rte_Write_Pp_SpeedGauge_Element_Speed((dt_uint16)speed10);
    (void)Rte_Write_Pp_SpeedOdo_Element_Speed((dt_uint16)speed10);
    (void)Rte_Write_Pp_SpeedDisplay_Element_Speed((dt_uint16)speed10);
}
