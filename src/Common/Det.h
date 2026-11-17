/* Det.h - Default Error Tracer (development errors) */
#ifndef DET_H
#define DET_H

#include "Std_Types.h"

void Det_Init(void);
Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);
uint16 Det_GetErrorCount(void);

#endif /* DET_H */
