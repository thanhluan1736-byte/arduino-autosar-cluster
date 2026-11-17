/* Adc.c - AUTOSAR Adc driver for ATmega328P: AVcc reference, 125 kHz ADC clock, ADC_vect per channel */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Adc.h"
#include "Det.h"
#include "SchM.h"

#define ADC_NO_GROUP  0xFFU

static const Adc_ConfigType *Adc_ConfigPtr = NULL_PTR;
static Adc_ValueGroupType *Adc_ResultBuffer[ADC_MAX_GROUPS];
static volatile Adc_StatusType Adc_GroupStatus[ADC_MAX_GROUPS];
static volatile uint8 Adc_ActiveGroup = ADC_NO_GROUP;
static volatile uint8 Adc_ActiveIndex = 0U;

static void Adc_StartChannel(Adc_ChannelType ch)
{
    ADMUX  = (uint8)((1U << REFS0) | (ch & 0x0FU));   /* AVcc reference, right adjusted */
    ADCSRA |= (uint8)(1U << ADSC);
}

void Adc_Init(const Adc_ConfigType *ConfigPtr)
{
    uint8 g;
    uint8 c;

    if ((ConfigPtr == NULL_PTR) || (ConfigPtr->NumGroups > ADC_MAX_GROUPS))
    {
        (void)Det_ReportError(ADC_MODULE_ID, 0U, ADC_SID_INIT, ADC_E_PARAM_POINTER);
        return;
    }
    for (g = 0U; g < ConfigPtr->NumGroups; g++)
    {
        Adc_ResultBuffer[g] = NULL_PTR;
        Adc_GroupStatus[g] = ADC_IDLE;
        for (c = 0U; c < ConfigPtr->Groups[g].NumChannels; c++)
        {
            DIDR0 |= (uint8)(1U << ConfigPtr->Groups[g].Channels[c]);   /* disable digital input buffer */
        }
    }
    /* enable ADC, interrupt, prescaler 128 -> 125 kHz at 16 MHz */
    ADCSRA = (uint8)((1U << ADEN) | (1U << ADIE) | (1U << ADPS2) | (1U << ADPS1) | (1U << ADPS0));
    Adc_ActiveGroup = ADC_NO_GROUP;
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
        (void)Det_ReportError(ADC_MODULE_ID, 0U, ADC_SID_START_GROUP, ADC_E_BUSY);   /* one hardware unit */
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
    Adc_GroupStatus[Group] = ADC_IDLE;   /* one-shot group: back to idle after the read */
    return E_OK;
}

ISR(ADC_vect)
{
    uint8 g = Adc_ActiveGroup;
    const Adc_GroupConfigType *grp;

    if ((g == ADC_NO_GROUP) || (Adc_ConfigPtr == NULL_PTR))
    {
        return;
    }
    grp = &Adc_ConfigPtr->Groups[g];
    Adc_ResultBuffer[g][Adc_ActiveIndex] = ADC;
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
