/* Eep.h - "EEPROM" driver on the S32K144 D-flash (FlexNVM, 2 KB sectors), same API as the AVR Eep.
 * A write erases the 2 KB sector that holds the address, then programs 8-byte phrases, so every
 * block written with Eep_Write must own its sector. Writes are asynchronous (FTFC runs in the
 * background, the code keeps running from P-flash). */
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

#define EEP_SIZE              0x10000UL   /* 64 KB D-flash (FlexNVM not partitioned for EEE) */
#define EEP_SECTOR_SIZE       0x800UL
#define EEP_PHRASE_SIZE       8U
#define EEP_MAX_WRITE         64U

/* NvM redundant block copies: one 2 KB sector each, so writing copy 1 never erases copy 0 */
#define EEP_NVM_COPY0_ADDR    0x0000U
#define EEP_NVM_COPY1_ADDR    0x0800U

typedef uint32 Eep_AddressType;
typedef uint16 Eep_LengthType;

void                Eep_Init(void);
Std_ReturnType      Eep_Read(Eep_AddressType EepromAddress, uint8 *DataBufferPtr, Eep_LengthType Length);
Std_ReturnType      Eep_Write(Eep_AddressType EepromAddress, const uint8 *DataBufferPtr, Eep_LengthType Length);
MemIf_StatusType    Eep_GetStatus(void);
MemIf_JobResultType Eep_GetJobResult(void);
void                Eep_MainFunction(void);

#endif /* EEP_H */
