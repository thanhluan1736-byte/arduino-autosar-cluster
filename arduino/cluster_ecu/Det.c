/* Det.c - counts development errors and prints them on the UART log */
#include "Det.h"
#include "Cdd_Log.h"

static uint16 Det_ErrorCount = 0U;

void Det_Init(void)
{
    Det_ErrorCount = 0U;
}

Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId)
{
    if (Det_ErrorCount < 0xFFFFU)
    {
        Det_ErrorCount++;
    }
    LOG_P("[DET] module=");
    Cdd_Log_U32(ModuleId);
    LOG_P(" inst=");
    Cdd_Log_U32(InstanceId);
    LOG_P(" api=0x");
    Cdd_Log_Hex8(ApiId);
    LOG_P(" err=0x");
    Cdd_Log_Hex8(ErrorId);
    LOG_P("\r\n");
    return E_OK;
}

uint16 Det_GetErrorCount(void)
{
    return Det_ErrorCount;
}
