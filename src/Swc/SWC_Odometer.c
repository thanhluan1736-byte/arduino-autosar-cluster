/* SWC_Odometer - integrates speed into total / trip distance, keeps them in NvM across power cycles */
#include "Rte_SWC_Odometer.h"
#include "NvM.h"
#include "Cdd_Log.h"
#include "Swc_Cfg.h"

static boolean Odo_Initialized = FALSE;
static uint32  Odo_TotalM = 0U;
static uint32  Odo_TripM = 0U;
static uint32  Odo_Remainder = 0U;      /* fractions of a metre, unit = 1/360 m */
static uint32  Odo_SavedTotalM = 0U;
static uint32  Odo_SavedTripM = 0U;
static boolean Odo_SaveRequest = FALSE;
static uint8   Odo_StandstillCnt = 0U;
static uint8   Odo_NvStatus = 0U;       /* 0 = restored from EEPROM, 1 = lost (defaults) */
static uint8   Odo_TripReqSeq = 0U;
static uint8   Odo_OdoReqSeq = 0U;
static uint8   Odo_LogCnt = 0U;

static void Odo_Init(void)
{
    NvM_RequestResultType res = NVM_REQ_NOT_OK;

    (void)NvM_GetErrorStatus(NvMConf_NvMBlock_Odometer, &res);
    Odo_TotalM = NvM_RamBlock_Odometer.OdometerM;   /* filled by NvM_ReadAll (or ROM defaults) */
    Odo_TripM = NvM_RamBlock_Odometer.TripM;
    Odo_SavedTotalM = Odo_TotalM;
    Odo_SavedTripM = Odo_TripM;
    Odo_NvStatus = (res == NVM_REQ_OK) ? 0U : 1U;
    Odo_SaveRequest = (res == NVM_REQ_OK) ? FALSE : TRUE;   /* repair a lost block at once */
    (void)Rte_Read_Rp_TripReset_Element_ReqSeq(&Odo_TripReqSeq);
    (void)Rte_Read_Rp_TripReset_Element_OdoReqSeq(&Odo_OdoReqSeq);
    Odo_Initialized = TRUE;
}

static void Odo_TrySave(void)
{
    NvM_RamBlock_Odometer.OdometerM = Odo_TotalM;
    NvM_RamBlock_Odometer.TripM = Odo_TripM;
    if (NvM_WriteBlock(NvMConf_NvMBlock_Odometer, NULL_PTR) == E_OK)
    {
        Odo_SavedTotalM = Odo_TotalM;
        Odo_SavedTripM = Odo_TripM;
        Odo_SaveRequest = FALSE;
    }
    /* else NvM busy: retry in 100 ms */
}

static void Odo_Log(dt_uint16 speed10)
{
    LOG_P("[Cluster] speed ");
    Cdd_Log_Fixed(speed10, 1U);
    LOG_P(" km/h  odo ");
    Cdd_Log_Fixed(Odo_TotalM, 3U);
    LOG_P(" km  trip ");
    Cdd_Log_Fixed(Odo_TripM, 3U);
    LOG_P(" km\r\n");
}

void Runnable_Odometer_100ms(void)
{
    dt_uint16 speed10 = 0U;
    dt_uint8 reqSeq = 0U;
    uint32 metres;

    if (Odo_Initialized == FALSE)
    {
        Odo_Init();
    }
    if (Rte_Read_Rp_SpeedOdo_Element_Speed(&speed10) != RTE_E_OK)
    {
        speed10 = 0U;
    }

    /* distance in 100 ms = (speed10 / 10) km/h / 3.6 * 0.1 s = speed10 / 360 m */
    Odo_Remainder += speed10;
    metres = Odo_Remainder / 360U;
    Odo_Remainder -= metres * 360U;
    Odo_TotalM += metres;
    Odo_TripM += metres;

    /* trip reset requested by the display (new sequence number) */
    if ((Rte_Read_Rp_TripReset_Element_ReqSeq(&reqSeq) == RTE_E_OK) && (reqSeq != Odo_TripReqSeq))
    {
        Odo_TripReqSeq = reqSeq;
        Odo_TripM = 0U;
        Odo_SaveRequest = TRUE;
        LOG_P("[Cluster] trip reset\r\n");
    }

    /* odometer reset requested by the display (3 pulls of the stick), accepted only at standstill */
    if ((Rte_Read_Rp_TripReset_Element_OdoReqSeq(&reqSeq) == RTE_E_OK) && (reqSeq != Odo_OdoReqSeq))
    {
        Odo_OdoReqSeq = reqSeq;
        if (speed10 == 0U)
        {
            Odo_TotalM = 0U;
            Odo_Remainder = 0U;
            Odo_SavedTotalM = 0U;           /* keeps the "every km" check below from underflowing */
            Odo_SaveRequest = TRUE;
            LOG_P("[Cluster] odometer reset\r\n");
        }
    }

    /* save policy: every km, and once the car has stood still for 1 s */
    if ((Odo_TotalM - Odo_SavedTotalM) >= ODO_SAVE_DISTANCE_M)
    {
        Odo_SaveRequest = TRUE;
    }
    if (speed10 == 0U)
    {
        if (Odo_StandstillCnt < ODO_SAVE_STANDSTILL_100MS)
        {
            Odo_StandstillCnt++;
        }
        else if ((Odo_TotalM != Odo_SavedTotalM) || (Odo_TripM != Odo_SavedTripM))
        {
            Odo_SaveRequest = TRUE;
        }
        else
        {
            /* nothing new */
        }
    }
    else
    {
        Odo_StandstillCnt = 0U;
    }
    if (Odo_SaveRequest == TRUE)
    {
        Odo_TrySave();
    }

    (void)Rte_Write_Pp_Distance_Element_Odo(Odo_TotalM);
    (void)Rte_Write_Pp_Distance_Element_Trip(Odo_TripM);
    (void)Rte_Write_Pp_Distance_Element_NvStatus(Odo_NvStatus);

    Odo_LogCnt++;
    if (Odo_LogCnt >= ODO_LOG_PERIOD_100MS)
    {
        Odo_LogCnt = 0U;
        Odo_Log(speed10);
    }
}
