/* NvM.h - AUTOSAR NvM (subset): redundant blocks with CRC16, ReadAll at startup, async WriteBlock */
#ifndef NVM_H
#define NVM_H

#include "Std_Types.h"
#include "NvM_Cfg.h"

#define NVM_MODULE_ID               20U

#define NVM_SID_WRITE_BLOCK         0x07U
#define NVM_SID_GET_ERROR_STATUS    0x04U

#define NVM_E_PARAM_BLOCK_ID        0x0AU
#define NVM_E_NOT_INITIALIZED       0x14U
#define NVM_E_BLOCK_PENDING         0x15U

typedef uint16 NvM_BlockIdType;
typedef uint8  NvM_RequestResultType;

#define NVM_REQ_OK                  0U
#define NVM_REQ_NOT_OK              1U
#define NVM_REQ_PENDING             2U
#define NVM_REQ_INTEGRITY_FAILED    3U
#define NVM_REQ_BLOCK_SKIPPED       4U
#define NVM_REQ_NV_INVALIDATED      5U
#define NVM_REQ_CANCELED            6U
#define NVM_REQ_REDUNDANCY_FAILED   7U
#define NVM_REQ_RESTORED_FROM_ROM   8U

typedef struct
{
    uint16      NvAddress[2];   /* two EEPROM copies (NVM_BLOCK_REDUNDANT) */
    uint8       Length;         /* user data, the CRC16 is stored after it */
    void       *RamBlock;       /* permanent RAM block */
    const void *RomBlock;       /* defaults used when both copies are corrupt */
} NvM_BlockDescriptorType;

void           NvM_Init(void);
void           NvM_ReadAll(void);   /* blocking, called once by EcuM before the OS starts */
Std_ReturnType NvM_WriteBlock(NvM_BlockIdType BlockId, const void *NvM_SrcPtr);
Std_ReturnType NvM_GetErrorStatus(NvM_BlockIdType BlockId, NvM_RequestResultType *RequestResultPtr);
void           NvM_MainFunction(void);

#endif /* NVM_H */
