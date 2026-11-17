/* Std_Types.h - AUTOSAR standard types */
#ifndef STD_TYPES_H
#define STD_TYPES_H

#include "Platform_Types.h"

typedef uint8 Std_ReturnType;

#define E_OK        ((Std_ReturnType)0U)
#define E_NOT_OK    ((Std_ReturnType)1U)

#define STD_LOW     0U
#define STD_HIGH    1U
#define STD_OFF     0U
#define STD_ON      1U

#ifndef NULL_PTR
#define NULL_PTR    ((void *)0)
#endif

#endif /* STD_TYPES_H */
