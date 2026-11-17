/* SWC_Display - LCD1602 pages: start screen, NvM error, speed + ODO / TRIP, trip reset by long press */
#include "Rte_SWC_Display.h"
#include "Cdd_Lcd.h"
#include "Swc_Cfg.h"

typedef enum
{
    DISP_PAGE_ODO = 0U,
    DISP_PAGE_TRIP
} Disp_PageType;

static uint8         Disp_Time = 0U;          /* 100 ms ticks since power-up (saturates) */
static Disp_PageType Disp_Page = DISP_PAGE_ODO;
static boolean       Disp_SeqValid = FALSE;
static uint8         Disp_LastBtnSeq = 0U;
static uint8         Disp_TripReqSeq = 0U;
static uint8         Disp_OdoReqSeq = 0U;
static uint8         Disp_OdoMsgTime = 0U;      /* 100 ms ticks left for the "ODO reset" message */

/* copies Text to Line at column Col, returns the next column */
static uint8 Disp_Put(char Line[17], uint8 Col, const char *Text)
{
    while ((*Text != '\0') && (Col < 16U))
    {
        Line[Col] = *Text;
        Col++;
        Text++;
    }
    return Col;
}

/* right-aligned fixed point: Value 873, Width 5, Decimals 1 -> " 87.3" */
static uint8 Disp_PutFixed(char Line[17], uint8 Col, uint32 Value, uint8 Width, uint8 Decimals)
{
    sint8 i;
    uint8 n = 0U;
    const uint8 minPos = (Decimals == 0U) ? 1U : (uint8)(Decimals + 2U);   /* "0" or "0.0" at least */

    for (i = (sint8)(Width - 1U); i >= 0; i--)
    {
        char c = ' ';
        if ((Decimals != 0U) && (n == Decimals))
        {
            c = '.';
        }
        else
        {
            if ((Value != 0U) || (n < minPos))
            {
                c = (char)('0' + (Value % 10U));
            }
            Value /= 10U;
        }
        if ((uint8)(Col + (uint8)i) < 16U)
        {
            Line[Col + (uint8)i] = c;
        }
        n++;
    }
    return (uint8)(Col + Width);
}

static void Disp_Clear(char Line[17])
{
    uint8 i;
    for (i = 0U; i < 16U; i++)
    {
        Line[i] = ' ';
    }
    Line[16] = '\0';
}

static void Disp_HandleButton(void)
{
    dt_uint8 evt = CLUSTER_BTN_NONE;
    dt_uint8 seq = 0U;

    if ((Rte_Read_Rp_ButtonEvent_Element_Seq(&seq) != RTE_E_OK) ||
        (Rte_Read_Rp_ButtonEvent_Element_Event(&evt) != RTE_E_OK))
    {
        return;   /* button never used yet */
    }
    if ((Disp_SeqValid == TRUE) && (seq == Disp_LastBtnSeq))
    {
        return;   /* no new event */
    }
    Disp_SeqValid = TRUE;
    Disp_LastBtnSeq = seq;

    if (evt == CLUSTER_BTN_SHORT)
    {
        Disp_Page = (Disp_Page == DISP_PAGE_ODO) ? DISP_PAGE_TRIP : DISP_PAGE_ODO;
    }
    else if ((evt == CLUSTER_BTN_LONG) && (Disp_Page == DISP_PAGE_TRIP))
    {
        Disp_TripReqSeq++;
        (void)Rte_Write_Pp_TripReset_Element_ReqSeq(Disp_TripReqSeq);
    }
    else if (evt == CLUSTER_BTN_ODO_RESET)
    {
        dt_uint16 speed10 = 0xFFFFU;

        /* only at standstill (the odometer checks it again) */
        if ((Rte_Read_Rp_SpeedDisplay_Element_Speed(&speed10) == RTE_E_OK) && (speed10 == 0U))
        {
            Disp_OdoReqSeq++;
            (void)Rte_Write_Pp_TripReset_Element_OdoReqSeq(Disp_OdoReqSeq);
            Disp_OdoMsgTime = DISP_ODO_RESET_MSG_100MS;
            Disp_Page = DISP_PAGE_ODO;
        }
    }
    else
    {
        /* long press on the ODO page: ignored */
    }
}

/* "SPD  87.3 km/h !" */
static void Disp_SpeedLine(char Line[17])
{
    dt_uint16 speed10 = 0U;
    uint8 col;

    Disp_Clear(Line);
    col = Disp_Put(Line, 0U, "SPD ");
    if (Rte_Read_Rp_SpeedDisplay_Element_Speed(&speed10) == RTE_E_OK)
    {
        col = Disp_PutFixed(Line, col, speed10, 6U, 1U);
        if (speed10 >= GAUGE_OVERSPEED_ON_KMH10)
        {
            Line[15] = '!';
        }
    }
    else
    {
        col = Disp_Put(Line, col, "  --.-");
    }
    (void)Disp_Put(Line, col, " km/h");
}

/* "ODO    123.4 km" or "TRIP     3.4 km" (unit 0.1 km) */
static void Disp_DistanceLine(char Line[17])
{
    dt_uint32 odo = 0U;
    dt_uint32 trip = 0U;
    uint8 col;

    Disp_Clear(Line);
    col = Disp_Put(Line, 0U, (Disp_Page == DISP_PAGE_ODO) ? "ODO " : "TRIP");
    if ((Rte_Read_Rp_Distance_Element_Odo(&odo) == RTE_E_OK) && (Rte_Read_Rp_Distance_Element_Trip(&trip) == RTE_E_OK))
    {
        uint32 metres = (Disp_Page == DISP_PAGE_ODO) ? odo : trip;
        col = Disp_PutFixed(Line, col, (metres / 100U) % 1000000U, 9U, 1U);
    }
    else
    {
        col = Disp_Put(Line, col, "     --.-");
    }
    (void)Disp_Put(Line, col, " km");
}

void Runnable_Display_100ms(void)
{
    char line0[17];
    char line1[17];
    dt_uint8 nvStatus = 0U;

    if (Disp_Time < 0xFFU)
    {
        Disp_Time++;
    }
    Disp_HandleButton();

    if (Disp_Time <= DISP_LAMPTEST_100MS)
    {
        Disp_Clear(line0);
        Disp_Clear(line1);
        (void)Disp_Put(line0, 0U, " mini AUTOSAR");
        (void)Disp_Put(line1, 0U, " cluster ECU");
    }
    else if ((Rte_Read_Rp_Distance_Element_NvStatus(&nvStatus) == RTE_E_OK) && (nvStatus != 0U) &&
             (Disp_Time <= (DISP_LAMPTEST_100MS + DISP_NVM_ERROR_100MS)))
    {
        Disp_Clear(line0);
        Disp_Clear(line1);
        (void)Disp_Put(line0, 0U, "NvM: data lost");
        (void)Disp_Put(line1, 0U, "ODO reset to 0");
    }
    else if (Disp_OdoMsgTime > 0U)
    {
        Disp_OdoMsgTime--;
        Disp_SpeedLine(line0);
        Disp_Clear(line1);
        (void)Disp_Put(line1, 0U, "ODO reset -> 0");
    }
    else
    {
        Disp_SpeedLine(line0);
        Disp_DistanceLine(line1);
    }
    /* the driver only sends the characters that changed */
    Cdd_Lcd_WriteLine(0U, line0);
    Cdd_Lcd_WriteLine(1U, line1);
}
