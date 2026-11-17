/* Adc.h - AUTOSAR Adc driver (software-triggered one-shot groups, interrupt driven) */
#ifndef ADC_H
#define ADC_H

#include "Std_Types.h"

#define ADC_MODULE_ID              123U

#define ADC_SID_INIT               0x00U
#define ADC_SID_START_GROUP        0x02U
#define ADC_SID_READ_GROUP         0x04U
#define ADC_SID_SETUP_BUFFER       0x0CU

#define ADC_E_UNINIT               0x0AU
#define ADC_E_BUSY                 0x0BU
#define ADC_E_PARAM_GROUP          0x15U
#define ADC_E_BUFFER_UNINIT        0x19U
#define ADC_E_PARAM_POINTER        0x14U

typedef uint8  Adc_ChannelType;        /* MUX value: 0 = A0 ... 5 = A5 */
typedef uint8  Adc_GroupType;
typedef uint16 Adc_ValueGroupType;     /* 10-bit result */

#define ADC_RESULT_MAX             1023U   /* full scale of a conversion */

typedef enum
{
    ADC_IDLE = 0U,
    ADC_BUSY,
    ADC_COMPLETED,
    ADC_STREAM_COMPLETED
} Adc_StatusType;

typedef struct
{
    const Adc_ChannelType *Channels;
    uint8                  NumChannels;
} Adc_GroupConfigType;

typedef struct
{
    const Adc_GroupConfigType *Groups;
    uint8                      NumGroups;
} Adc_ConfigType;

#define ADC_MAX_GROUPS             2U

/* symbolic groups */
#define AdcConf_AdcGroup_Joystick  0U

extern const Adc_ConfigType Adc_Config;

void           Adc_Init(const Adc_ConfigType *ConfigPtr);
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr);
void           Adc_StartGroupConversion(Adc_GroupType Group);
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr);

#endif /* ADC_H */
