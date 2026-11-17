/* Eep.c - AUTOSAR Eep driver for ATmega328P (non-blocking writes) */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Eep.h"
#include "Det.h"
#include "SchM.h"

#define EEP_COMPARE_BUDGET  16U   /* unchanged bytes checked per main function call */

static MemIf_StatusType    Eep_Status = MEMIF_UNINIT;
static MemIf_JobResultType Eep_JobResult = MEMIF_JOB_OK;
static const uint8        *Eep_WriteSrc = NULL_PTR;
static Eep_AddressType     Eep_WriteAddr = 0U;
static Eep_LengthType      Eep_WriteRemaining = 0U;

static boolean Eep_HwBusy(void)
{
    return ((EECR & (uint8)(1U << EEPE)) != 0U) ? TRUE : FALSE;
}

static uint8 Eep_HwReadByte(Eep_AddressType addr)
{
    EEAR = addr;
    EECR |= (uint8)(1U << EERE);
    return EEDR;
}

static void Eep_HwWriteByte(Eep_AddressType addr, uint8 data)
{
    SchM_Enter_Exclusive();          /* EEPE must follow EEMPE within 4 cycles */
    EEAR = addr;
    EEDR = data;
    EECR = (uint8)(1U << EEMPE);     /* atomic erase + write mode */
    EECR |= (uint8)(1U << EEPE);
    SchM_Exit_Exclusive();
}

static boolean Eep_RangeOk(Eep_AddressType addr, Eep_LengthType len)
{
    return ((len != 0U) && (addr < EEP_SIZE) && (len <= (Eep_LengthType)(EEP_SIZE - addr))) ? TRUE : FALSE;
}

void Eep_Init(void)
{
    Eep_WriteRemaining = 0U;
    Eep_JobResult = MEMIF_JOB_OK;
    Eep_Status = MEMIF_IDLE;
}

Std_ReturnType Eep_Read(Eep_AddressType EepromAddress, uint8 *DataBufferPtr, Eep_LengthType Length)
{
    Eep_LengthType i;

    if (Eep_Status == MEMIF_UNINIT)
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_READ, EEP_E_UNINIT);
        return E_NOT_OK;
    }
    if (DataBufferPtr == NULL_PTR)
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_READ, EEP_E_PARAM_DATA);
        return E_NOT_OK;
    }
    if (Eep_RangeOk(EepromAddress, Length) == FALSE)
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_READ, EEP_E_PARAM_ADDRESS);
        return E_NOT_OK;
    }
    if ((Eep_Status == MEMIF_BUSY) || (Eep_HwBusy() == TRUE))
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_READ, EEP_E_BUSY);
        return E_NOT_OK;
    }
    for (i = 0U; i < Length; i++)
    {
        DataBufferPtr[i] = Eep_HwReadByte((Eep_AddressType)(EepromAddress + i));
    }
    Eep_JobResult = MEMIF_JOB_OK;
    return E_OK;
}

Std_ReturnType Eep_Write(Eep_AddressType EepromAddress, const uint8 *DataBufferPtr, Eep_LengthType Length)
{
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
    if (Eep_RangeOk(EepromAddress, Length) == FALSE)
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_WRITE, EEP_E_PARAM_ADDRESS);
        return E_NOT_OK;
    }
    if (Eep_Status == MEMIF_BUSY)
    {
        (void)Det_ReportError(EEP_MODULE_ID, 0U, EEP_SID_WRITE, EEP_E_BUSY);
        return E_NOT_OK;
    }
    Eep_WriteSrc = DataBufferPtr;          /* buffer must stay valid until the job ends */
    Eep_WriteAddr = EepromAddress;
    Eep_WriteRemaining = Length;
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
    uint8 budget = EEP_COMPARE_BUDGET;

    if (Eep_Status != MEMIF_BUSY)
    {
        return;
    }
    while (Eep_HwBusy() == FALSE)
    {
        uint8 data;

        if (Eep_WriteRemaining == 0U)
        {
            Eep_Status = MEMIF_IDLE;       /* last byte is programmed */
            Eep_JobResult = MEMIF_JOB_OK;
            return;
        }
        data = *Eep_WriteSrc;
        if (Eep_HwReadByte(Eep_WriteAddr) != data)
        {
            Eep_HwWriteByte(Eep_WriteAddr, data);
            Eep_WriteSrc++;
            Eep_WriteAddr++;
            Eep_WriteRemaining--;
            return;                         /* programming takes ~3.4 ms: continue in the next call */
        }
        Eep_WriteSrc++;                     /* byte already holds the value: skip it (saves wear) */
        Eep_WriteAddr++;
        Eep_WriteRemaining--;
        budget--;
        if (budget == 0U)
        {
            return;
        }
    }
}
