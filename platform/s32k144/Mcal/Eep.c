/* Eep.c (S32K144) - D-flash through the FTFC command interface: erase sector, program phrase */
#include "Eep.h"
#include "Det.h"
#include "Cdd_Log.h"
#include "S32K144_Regs.h"

typedef enum
{
    EEP_JOB_NONE = 0U,
    EEP_JOB_ERASE,
    EEP_JOB_PROGRAM
} Eep_JobStepType;

static MemIf_StatusType    Eep_Status = MEMIF_UNINIT;
static MemIf_JobResultType Eep_JobResult = MEMIF_JOB_OK;
static Eep_JobStepType     Eep_Step = EEP_JOB_NONE;
static Eep_AddressType     Eep_JobAddr = 0U;
static uint8               Eep_JobBuf[EEP_MAX_WRITE];
static uint8               Eep_JobPhrases = 0U;
static uint8               Eep_JobPhrase = 0U;
static boolean             Eep_CmdRunning = FALSE;

/* command / address bytes: FCCOB0..3 are numbered big-endian inside the first word (FCCOB0 at offset 3) */
static void Eep_SetFccob(uint8 n, uint8 value)
{
    REG8(FTFC_FCCOB_BASE + (uint32)((n & 0xFCU) + (3U - (n & 3U)))) = value;
}

static boolean Eep_FtfcBusy(void)
{
    return ((FTFC_FSTAT & FTFC_FSTAT_CCIF) == 0U) ? TRUE : FALSE;
}

static void Eep_Launch(uint8 cmd, Eep_AddressType addr)
{
    uint32 a = DFLASH_CMD_BASE + addr;

    FTFC_FSTAT = (uint8)(FTFC_FSTAT_ACCERR | FTFC_FSTAT_FPVIOL | FTFC_FSTAT_RDCOLERR);   /* clear old errors */
    Eep_SetFccob(0U, cmd);
    Eep_SetFccob(1U, (uint8)(a >> 16U));
    Eep_SetFccob(2U, (uint8)(a >> 8U));
    Eep_SetFccob(3U, (uint8)a);
    FTFC_FSTAT = FTFC_FSTAT_CCIF;                       /* write 1: start the command */
    Eep_CmdRunning = TRUE;
}

static boolean Eep_RangeOk(Eep_AddressType addr, Eep_LengthType len)
{
    return ((len != 0U) && (addr < EEP_SIZE) && (len <= (EEP_SIZE - addr))) ? TRUE : FALSE;
}

void Eep_Init(void)
{
    PCC_REG(PCC_FTFC_INDEX) |= PCC_CGC;
    Eep_Step = EEP_JOB_NONE;
    Eep_CmdRunning = FALSE;
    Eep_JobResult = MEMIF_JOB_OK;
    if ((FTFC_FCNFG & FTFC_FCNFG_EEERDY) != 0U)
    {
        /* FlexNVM was partitioned for emulated EEPROM by an earlier project: the D-flash layout
         * assumed here may not exist. Stay uninitialised -> NvM uses its ROM defaults, nothing is written. */
        LOG_P("[Eep] FlexNVM is partitioned as EEPROM: D-flash storage disabled\r\n");
        Eep_Status = MEMIF_UNINIT;
        return;
    }
    Eep_Status = MEMIF_IDLE;
}

Std_ReturnType Eep_Read(Eep_AddressType EepromAddress, uint8 *DataBufferPtr, Eep_LengthType Length)
{
    const volatile uint8 *src = (const volatile uint8 *)(DFLASH_READ_BASE + EepromAddress);
    Eep_LengthType i;

    if (Eep_Status == MEMIF_UNINIT)
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_READ, EEP_E_UNINIT);
        return E_NOT_OK;
    }
    if ((DataBufferPtr == NULL_PTR) || (Eep_RangeOk(EepromAddress, Length) == FALSE))
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_READ, EEP_E_PARAM_ADDRESS);
        return E_NOT_OK;
    }
    if ((Eep_Status == MEMIF_BUSY) || (Eep_FtfcBusy() == TRUE))
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_READ, EEP_E_BUSY);
        return E_NOT_OK;   /* the D-flash cannot be read while it is being programmed */
    }
    for (i = 0U; i < Length; i++)
    {
        DataBufferPtr[i] = src[i];
    }
    Eep_JobResult = MEMIF_JOB_OK;
    return E_OK;
}

Std_ReturnType Eep_Write(Eep_AddressType EepromAddress, const uint8 *DataBufferPtr, Eep_LengthType Length)
{
    const volatile uint8 *cur = (const volatile uint8 *)(DFLASH_READ_BASE + EepromAddress);
    boolean same = TRUE;
    Eep_LengthType i;

    if (Eep_Status == MEMIF_UNINIT)
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_WRITE, EEP_E_UNINIT);
        return E_NOT_OK;
    }
    if (DataBufferPtr == NULL_PTR)
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_WRITE, EEP_E_PARAM_DATA);
        return E_NOT_OK;
    }
    if ((Eep_RangeOk(EepromAddress, Length) == FALSE) || (Length > EEP_MAX_WRITE) ||
        ((EepromAddress % EEP_PHRASE_SIZE) != 0U) ||
        ((EepromAddress / EEP_SECTOR_SIZE) != ((EepromAddress + Length - 1U) / EEP_SECTOR_SIZE)))
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_WRITE, EEP_E_PARAM_ADDRESS);
        return E_NOT_OK;
    }
    if ((Eep_Status == MEMIF_BUSY) || (Eep_FtfcBusy() == TRUE))
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_WRITE, EEP_E_BUSY);
        return E_NOT_OK;
    }
    for (i = 0U; i < EEP_MAX_WRITE; i++)
    {
        Eep_JobBuf[i] = (i < Length) ? DataBufferPtr[i] : 0xFFU;   /* pad the last phrase with the erased value */
    }
    for (i = 0U; i < Length; i++)
    {
        if (cur[i] != Eep_JobBuf[i])
        {
            same = FALSE;
        }
    }
    if (same == TRUE)
    {
        Eep_JobResult = MEMIF_JOB_OK;   /* nothing to do: no erase, no wear */
        return E_OK;
    }
    Eep_JobAddr = EepromAddress;
    Eep_JobPhrases = (uint8)((Length + EEP_PHRASE_SIZE - 1U) / EEP_PHRASE_SIZE);
    Eep_JobPhrase = 0U;
    Eep_Step = EEP_JOB_ERASE;
    Eep_CmdRunning = FALSE;
    Eep_JobResult = MEMIF_JOB_PENDING;
    Eep_Status = MEMIF_BUSY;
    return E_OK;
}

MemIf_StatusType Eep_GetStatus(void)
{
    return Eep_Status;
}

MemIf_JobResultType Eep_GetJobResult(void)
{
    return Eep_JobResult;
}

void Eep_MainFunction(void)
{
    if ((Eep_Status != MEMIF_BUSY) || (Eep_FtfcBusy() == TRUE))
    {
        return;
    }
    if (Eep_CmdRunning == TRUE)
    {
        Eep_CmdRunning = FALSE;
        if ((FTFC_FSTAT & (FTFC_FSTAT_ACCERR | FTFC_FSTAT_FPVIOL | FTFC_FSTAT_MGSTAT0)) != 0U)
        {
            Eep_Step = EEP_JOB_NONE;
            Eep_JobResult = MEMIF_JOB_FAILED;    /* e.g. FlexNVM partitioned as EEPROM: no D-flash here */
            Eep_Status = MEMIF_IDLE;
            return;
        }
        if (Eep_Step == EEP_JOB_PROGRAM)
        {
            Eep_JobPhrase++;
        }
        else
        {
            Eep_Step = EEP_JOB_PROGRAM;          /* erase done */
        }
    }

    if (Eep_Step == EEP_JOB_ERASE)
    {
        Eep_Launch(FTFC_CMD_ERASE_SECTOR, Eep_JobAddr - (Eep_JobAddr % EEP_SECTOR_SIZE));
    }
    else if (Eep_JobPhrase < Eep_JobPhrases)
    {
        uint8 k;
        uint8 base = (uint8)(Eep_JobPhrase * EEP_PHRASE_SIZE);

        /* phrase data: byte k (lowest address first) goes to the FCCOB register at offset 4 + k,
         * in plain memory order - only the command / address bytes use the swapped FCCOB numbering */
        for (k = 0U; k < EEP_PHRASE_SIZE; k++)
        {
            REG8(FTFC_FCCOB_BASE + 4U + k) = Eep_JobBuf[base + k];
        }
        Eep_Launch(FTFC_CMD_PROGRAM_PHRASE, Eep_JobAddr + base);
    }
    else
    {
        Eep_Step = EEP_JOB_NONE;
        Eep_JobResult = MEMIF_JOB_OK;
        Eep_Status = MEMIF_IDLE;
    }
}
