/* EcuM.h - ECU state manager: startup sequence and periodic status */
#ifndef ECUM_H
#define ECUM_H

#include "Std_Types.h"

void EcuM_Init(void);           /* StartPreOS: init all BSW, ReadAll, RTE; then main() starts the OS */
void EcuM_MainFunction(void);   /* 1 s status line on the UART log */

#endif /* ECUM_H */
