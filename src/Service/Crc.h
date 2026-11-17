/* Crc.h - AUTOSAR Crc library (CRC-16/CCITT-FALSE) */
#ifndef CRC_H
#define CRC_H

#include "Std_Types.h"

/* poly 0x1021, init 0xFFFF, no reflection, no final XOR */
uint16 Crc_CalculateCRC16(const uint8 *Crc_DataPtr, uint32 Crc_Length, uint16 Crc_StartValue16, boolean Crc_IsFirstCall);

#endif /* CRC_H */
