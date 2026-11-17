/* Dio.h - AUTOSAR Dio driver (S32K144 GPIO) */
#ifndef DIO_H
#define DIO_H

#include "Std_Types.h"
#include "Dio_Cfg.h"

#define DIO_MODULE_ID                    120U

#define DIO_SID_READ_CHANNEL             0x00U
#define DIO_SID_WRITE_CHANNEL            0x01U
#define DIO_SID_READ_CHANNEL_GROUP       0x04U
#define DIO_SID_WRITE_CHANNEL_GROUP      0x05U
#define DIO_SID_FLIP_CHANNEL             0x11U

#define DIO_E_PARAM_INVALID_CHANNEL_ID   0x0AU
#define DIO_E_PARAM_INVALID_GROUP        0x1FU

typedef uint8  Dio_ChannelType;
typedef uint8  Dio_PortType;
typedef uint8  Dio_LevelType;
typedef uint32 Dio_PortLevelType;

typedef struct
{
    Dio_PortLevelType mask;     /* bits of the group inside the 32-bit port */
    uint8             offset;   /* position of the lowest bit */
    Dio_PortType      port;     /* PORT_A .. PORT_E */
} Dio_ChannelGroupType;

/* stepper coils IN1..IN4 = PTD9..PTD12 */
extern const Dio_ChannelGroupType DioConf_DioChannelGroup_Stepper;

Dio_LevelType     Dio_ReadChannel(Dio_ChannelType ChannelId);
void              Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);
Dio_LevelType     Dio_FlipChannel(Dio_ChannelType ChannelId);
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr);
void              Dio_WriteChannelGroup(const Dio_ChannelGroupType *ChannelGroupIdPtr, Dio_PortLevelType Level);

#endif /* DIO_H */
