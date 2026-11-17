/* Crc.c - bitwise CRC-16/CCITT-FALSE (small code, enough for a 10-byte NvM block) */
#include "Crc.h"

uint16 Crc_CalculateCRC16(const uint8 *Crc_DataPtr, uint32 Crc_Length, uint16 Crc_StartValue16, boolean Crc_IsFirstCall)
{
    uint16 crc = (Crc_IsFirstCall == TRUE) ? 0xFFFFU : Crc_StartValue16;
    uint32 i;
    uint8 b;

    for (i = 0U; i < Crc_Length; i++)
    {
        crc ^= (uint16)((uint16)Crc_DataPtr[i] << 8U);
        for (b = 0U; b < 8U; b++)
        {
            crc = ((crc & 0x8000U) != 0U) ? (uint16)((crc << 1U) ^ 0x1021U) : (uint16)(crc << 1U);
        }
    }
    return crc;
}
