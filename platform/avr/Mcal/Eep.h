/* Eep.h - AUTOSAR Eep driver for the 1 KB internal EEPROM */
#ifndef EEP_H
#define EEP_H

#include "Std_Types.h"
#include "MemIf_Types.h"

#define EEP_MODULE_ID         90U

#define EEP_SID_READ          0x02U
#define EEP_SID_WRITE         0x03U

#define EEP_E_UNINIT          0x11U
#define EEP_E_BUSY            0x12U
#define EEP_E_PARAM_ADDRESS   0x13U
#define EEP_E_PARAM_DATA      0x14U
#define EEP_E_PARAM_LENGTH    0x15U

#define EEP_SIZE              1024U

/* NvM redundant block copies (any byte address works on this EEPROM) */
#define EEP_NVM_COPY0_ADDR    0x0010U
#define EEP_NVM_COPY1_ADDR    0x0040U

typedef uint16 Eep_AddressType;
typedef uint16 Eep_LengthType;

void                Eep_Init(void);
/* synchronous: reading the AVR EEPROM takes 4 CPU cycles per byte */
Std_ReturnType      Eep_Read(Eep_AddressType EepromAddress, uint8 *DataBufferPtr, Eep_LengthType Length);
/* asynchronous: one byte (~3.4 ms) is programmed per Eep_MainFunction call; unchanged bytes are skipped */
Std_ReturnType      Eep_Write(Eep_AddressType EepromAddress, const uint8 *DataBufferPtr, Eep_LengthType Length);
MemIf_StatusType    Eep_GetStatus(void);
MemIf_JobResultType Eep_GetJobResult(void);
void                Eep_MainFunction(void);

#endif /* EEP_H */
