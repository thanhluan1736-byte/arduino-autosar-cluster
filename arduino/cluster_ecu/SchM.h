/* SchM.h - exclusive areas (global interrupt lock, nestable) */
#ifndef SCHM_H
#define SCHM_H

#include "Std_Types.h"

void SchM_Enter_Exclusive(void);
void SchM_Exit_Exclusive(void);

#endif /* SCHM_H */
