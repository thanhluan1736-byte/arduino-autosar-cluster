/* NvM.c - NvM subset: ReadAll with CRC check and redundant copies, WriteBlock processed by NvM_MainFunction */
#include <string.h>
#include "NvM.h"
#include "Eep.h"
#include "Crc.h"
#include "Det.h"
#include "Cdd_Log.h"

extern const NvM_BlockDescriptorType NvM_BlockDescriptor[NVM_NUM_USER_BLOCKS];

typedef enum
{
    NVM_JOB_IDLE = 0U,
    NVM_JOB_START_COPY0,
    NVM_JOB_WAIT_COPY0,
    NVM_JOB_START_COPY1,
    NVM_JOB_WAIT_COPY1
} NvM_JobStateType;

static boolean               NvM_Initialized = FALSE;
static NvM_RequestResultType NvM_Result[NVM_NUM_USER_BLOCKS];
static NvM_JobStateType      NvM_JobState = NVM_JOB_IDLE;
static uint8                 NvM_JobIndex = 0U;
static uint8                 NvM_Mirror[NVM_MAX_BLOCK_LENGTH + 2U];   /* data + CRC16 (big endian) */

static boolean NvM_IdToIndex(NvM_BlockIdType id, uint8 *idx)
{
    if ((id < NVM_FIRST_USER_BLOCK) || (id >= (NVM_FIRST_USER_BLOCK + NVM_NUM_USER_BLOCKS)))
    {
        return FALSE;
    }
    *idx = (uint8)(id - NVM_FIRST_USER_BLOCK);
    return TRUE;
}

static boolean NvM_ReadCopy(const NvM_BlockDescriptorType *d, uint8 copy)
{
    uint16 crc;

    if (Eep_Read(d->NvAddress[copy], NvM_Mirror, (Eep_LengthType)(d->Length + 2U)) != E_OK)
    {
        return FALSE;
    }
    crc = Crc_CalculateCRC16(NvM_Mirror, d->Length, 0U, TRUE);
    return ((NvM_Mirror[d->Length] == (uint8)(crc >> 8U)) && (NvM_Mirror[d->Length + 1U] == (uint8)crc)) ? TRUE : FALSE;
}

void NvM_Init(void)
{
    uint8 i;

    for (i = 0U; i < NVM_NUM_USER_BLOCKS; i++)
    {
        NvM_Result[i] = NVM_REQ_NOT_OK;
    }
    NvM_JobState = NVM_JOB_IDLE;
    NvM_Initialized = TRUE;
}

void NvM_ReadAll(void)
{
    uint8 i;
    uint8 copy;

    for (i = 0U; i < NVM_NUM_USER_BLOCKS; i++)
    {
        const NvM_BlockDescriptorType *d = &NvM_BlockDescriptor[i];
        boolean ok = FALSE;

        for (copy = 0U; (copy < 2U) && (ok == FALSE); copy++)
        {
            ok = NvM_ReadCopy(d, copy);
        }
        LOG_P("[NvM] block ");
        Cdd_Log_U32((uint32)i + NVM_FIRST_USER_BLOCK);
        if (ok == TRUE)
        {
            (void)memcpy(d->RamBlock, NvM_Mirror, d->Length);
            NvM_Result[i] = NVM_REQ_OK;
            if (copy == 1U) { LOG_P(": OK (copy 0)\r\n"); }               /* loop ended after copy 0 */
            else            { LOG_P(": OK (copy 0 corrupt, copy 1 used)\r\n"); }
        }
        else
        {
            (void)memcpy(d->RamBlock, d->RomBlock, d->Length);
            NvM_Result[i] = NVM_REQ_RESTORED_FROM_ROM;
            LOG_P(": both copies corrupt -> ROM defaults\r\n");
        }
    }
}

Std_ReturnType NvM_WriteBlock(NvM_BlockIdType BlockId, const void *NvM_SrcPtr)
{
    uint8 idx;
    uint16 crc;
    const NvM_BlockDescriptorType *d;

    if (NvM_Initialized == FALSE)
    {
        (void)Det_ReportError(NVM_MODULE_ID, 0U, NVM_SID_WRITE_BLOCK, NVM_E_NOT_INITIALIZED);
        return E_NOT_OK;
    }
    if (NvM_IdToIndex(BlockId, &idx) == FALSE)
    {
        (void)Det_ReportError(NVM_MODULE_ID, 0U, NVM_SID_WRITE_BLOCK, NVM_E_PARAM_BLOCK_ID);
        return E_NOT_OK;
    }
    if (NvM_JobState != NVM_JOB_IDLE)
    {
        return E_NOT_OK;   /* single job slot: the caller retries later */
    }
    d = &NvM_BlockDescriptor[idx];
    /* snapshot now, so the SWC can keep updating its RAM block while the EEPROM is written */
    (void)memcpy(NvM_Mirror, (NvM_SrcPtr != NULL_PTR) ? NvM_SrcPtr : d->RamBlock, d->Length);
    crc = Crc_CalculateCRC16(NvM_Mirror, d->Length, 0U, TRUE);
    NvM_Mirror[d->Length] = (uint8)(crc >> 8U);
    NvM_Mirror[d->Length + 1U] = (uint8)crc;
    NvM_JobIndex = idx;
    NvM_Result[idx] = NVM_REQ_PENDING;
    NvM_JobState = NVM_JOB_START_COPY0;
    return E_OK;
}

Std_ReturnType NvM_GetErrorStatus(NvM_BlockIdType BlockId, NvM_RequestResultType *RequestResultPtr)
{
    uint8 idx;

    if ((RequestResultPtr == NULL_PTR) || (NvM_IdToIndex(BlockId, &idx) == FALSE))
    {
        (void)Det_ReportError(NVM_MODULE_ID, 0U, NVM_SID_GET_ERROR_STATUS, NVM_E_PARAM_BLOCK_ID);
        return E_NOT_OK;
    }
    *RequestResultPtr = NvM_Result[idx];
    return E_OK;
}

void NvM_MainFunction(void)
{
    const NvM_BlockDescriptorType *d = &NvM_BlockDescriptor[NvM_JobIndex];
    Eep_LengthType len = (Eep_LengthType)(d->Length + 2U);

    switch (NvM_JobState)
    {
    case NVM_JOB_START_COPY0:
    case NVM_JOB_START_COPY1:
        if (Eep_GetStatus() == MEMIF_IDLE)
        {
            uint8 copy = (NvM_JobState == NVM_JOB_START_COPY0) ? 0U : 1U;
            if (Eep_Write(d->NvAddress[copy], NvM_Mirror, len) == E_OK)
            {
                NvM_JobState = (copy == 0U) ? NVM_JOB_WAIT_COPY0 : NVM_JOB_WAIT_COPY1;
            }
            else
            {
                NvM_Result[NvM_JobIndex] = NVM_REQ_NOT_OK;
                NvM_JobState = NVM_JOB_IDLE;
            }
        }
        break;

    case NVM_JOB_WAIT_COPY0:
    case NVM_JOB_WAIT_COPY1:
        if (Eep_GetStatus() == MEMIF_IDLE)
        {
            if (Eep_GetJobResult() != MEMIF_JOB_OK)
            {
                NvM_Result[NvM_JobIndex] = NVM_REQ_NOT_OK;
                NvM_JobState = NVM_JOB_IDLE;
            }
            else if (NvM_JobState == NVM_JOB_WAIT_COPY0)
            {
                NvM_JobState = NVM_JOB_START_COPY1;   /* copy 0 is safe: now write copy 1 */
            }
            else
            {
                NvM_Result[NvM_JobIndex] = NVM_REQ_OK;
                NvM_JobState = NVM_JOB_IDLE;
            }
        }
        break;

    case NVM_JOB_IDLE:
    default:
        break;
    }
}
