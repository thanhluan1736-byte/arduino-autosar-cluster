/* Adc.c (S32K144) - ADC0, 12 bit, 12 MHz ADC clock, calibrated at init, one conversion per interrupt */
#include "Adc.h"
#include "Det.h"
#include "SchM.h"
#include "S32K144_Regs.h"

#define ADC_NO_GROUP  0xFFU

static const Adc_ConfigType *Adc_ConfigPtr = NULL_PTR;
static Adc_ValueGroupType *Adc_ResultBuffer[ADC_MAX_GROUPS];
static volatile Adc_StatusType Adc_GroupStatus[ADC_MAX_GROUPS];
static volatile uint8 Adc_ActiveGroup = ADC_NO_GROUP;
static volatile uint8 Adc_ActiveIndex = 0U;

static void Adc_StartChannel(Adc_ChannelType ch)
{
    ADC0_SC1A = ADC_SC1_AIEN | ADC_SC1_ADCH(ch);   /* writing SC1A starts the conversion */
}

static void Adc_Calibrate(void)
{
    uint32 timeout = 1000000UL;

    ADC0_SC3 = ADC_SC3_CAL | ADC_SC3_AVGE | ADC_SC3_AVGS_32;
    while (((ADC0_SC1A & ADC_SC1_COCO) == 0U) && (timeout > 0U))
    {
        timeout--;
    }
    (void)ADC0_RA;
    ADC0_SC3 = ADC_SC3_AVGE | ADC_SC3_AVGS_4;   /* normal use: 4-sample hardware average (smoother joystick) */
}

void Adc_Init(const Adc_ConfigType *ConfigPtr)
{
    uint8 g;

    if ((ConfigPtr == NULL_PTR) || (ConfigPtr->NumGroups > ADC_MAX_GROUPS))
    {
        (void)Det_ReportError(ADC_MODULE_ID, 0U, ADC_SID_INIT, ADC_E_PARAM_POINTER);
        return;
    }
    for (g = 0U; g < ConfigPtr->NumGroups; g++)
    {
        Adc_ResultBuffer[g] = NULL_PTR;
        Adc_GroupStatus[g] = ADC_IDLE;
    }
    PCC_REG(PCC_ADC0_INDEX) = 0U;
    PCC_REG(PCC_ADC0_INDEX) = PCC_PCS(PCC_PCS_FIRCDIV2) | PCC_CGC;   /* 48 MHz */
    ADC0_SC1A = ADC_SC1_ADCH_DISABLED;
    ADC0_CFG1 = ADC_CFG1_ADIV_4 | ADC_CFG1_MODE_12BIT;              /* 12 MHz, 12 bit */
    ADC0_CFG2 = 12U;                                                /* sample time */
    ADC0_SC2  = 0U;                                                 /* software trigger, VREFH/VREFL */
    Adc_Calibrate();
    Adc_ActiveGroup = ADC_NO_GROUP;
    Nvic_EnableIrq(IRQ_ADC0, 3U);
    Adc_ConfigPtr = ConfigPtr;
}

Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr)
{
    if (Adc_ConfigPtr == NULL_PTR)
    {
        (void)Det_ReportError(ADC_MODULE_ID, 0U, ADC_SID_SETUP_BUFFER, ADC_E_UNINIT);
        return E_NOT_OK;
    }
    if ((Group >= Adc_ConfigPtr->NumGroups) || (DataBufferPtr == NULL_PTR))
    {
        (void)Det_ReportError(ADC_MODULE_ID, 0U, ADC_SID_SETUP_BUFFER, ADC_E_PARAM_GROUP);
        return E_NOT_OK;
    }
    Adc_ResultBuffer[Group] = DataBufferPtr;
    return E_OK;
}

void Adc_StartGroupConversion(Adc_GroupType Group)
{
    if (Adc_ConfigPtr == NULL_PTR)
    {
        (void)Det_ReportError(ADC_MODULE_ID, 0U, ADC_SID_START_GROUP, ADC_E_UNINIT);
        return;
    }
    if (Group >= Adc_ConfigPtr->NumGroups)
    {
        (void)Det_ReportError(ADC_MODULE_ID, 0U, ADC_SID_START_GROUP, ADC_E_PARAM_GROUP);
        return;
    }
    if (Adc_ResultBuffer[Group] == NULL_PTR)
    {
        (void)Det_ReportError(ADC_MODULE_ID, 0U, ADC_SID_START_GROUP, ADC_E_BUFFER_UNINIT);
        return;
    }
    SchM_Enter_Exclusive();
    if (Adc_ActiveGroup != ADC_NO_GROUP)
    {
        SchM_Exit_Exclusive();
        (void)Det_ReportError(ADC_MODULE_ID, 0U, ADC_SID_START_GROUP, ADC_E_BUSY);
        return;
    }
    Adc_ActiveGroup = Group;
    Adc_ActiveIndex = 0U;
    Adc_GroupStatus[Group] = ADC_BUSY;
    Adc_StartChannel(Adc_ConfigPtr->Groups[Group].Channels[0]);
    SchM_Exit_Exclusive();
}

Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group)
{
    if ((Adc_ConfigPtr == NULL_PTR) || (Group >= Adc_ConfigPtr->NumGroups))
    {
        return ADC_IDLE;
    }
    return Adc_GroupStatus[Group];
}

Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr)
{
    uint8 c;

    if ((Adc_ConfigPtr == NULL_PTR) || (Group >= Adc_ConfigPtr->NumGroups) || (DataBufferPtr == NULL_PTR))
    {
        (void)Det_ReportError(ADC_MODULE_ID, 0U, ADC_SID_READ_GROUP, ADC_E_PARAM_GROUP);
        return E_NOT_OK;
    }
    if ((Adc_GroupStatus[Group] != ADC_COMPLETED) && (Adc_GroupStatus[Group] != ADC_STREAM_COMPLETED))
    {
        return E_NOT_OK;
    }
    for (c = 0U; c < Adc_ConfigPtr->Groups[Group].NumChannels; c++)
    {
        DataBufferPtr[c] = Adc_ResultBuffer[Group][c];
    }
    Adc_GroupStatus[Group] = ADC_IDLE;
    return E_OK;
}

void ADC0_IRQHandler(void)
{
    uint8 g = Adc_ActiveGroup;
    uint16 value = (uint16)(ADC0_RA & 0xFFFU);   /* reading R[0] clears COCO */
    const Adc_GroupConfigType *grp;

    if ((g == ADC_NO_GROUP) || (Adc_ConfigPtr == NULL_PTR))
    {
        return;
    }
    grp = &Adc_ConfigPtr->Groups[g];
    Adc_ResultBuffer[g][Adc_ActiveIndex] = value;
    Adc_ActiveIndex++;
    if (Adc_ActiveIndex < grp->NumChannels)
    {
        Adc_StartChannel(grp->Channels[Adc_ActiveIndex]);
    }
    else
    {
        Adc_GroupStatus[g] = ADC_STREAM_COMPLETED;
        Adc_ActiveGroup = ADC_NO_GROUP;
    }
}
