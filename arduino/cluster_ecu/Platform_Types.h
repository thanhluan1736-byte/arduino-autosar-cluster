/* Platform_Types.h - AUTOSAR platform types for ATmega328P (8-bit AVR, avr-gcc) */
#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#define CPU_TYPE_8        8U
#define CPU_TYPE          CPU_TYPE_8

/* identical to the typedefs in the generated Rte_Type.h (C11 allows the repeat) */
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned long   uint32;
typedef signed char     sint8;
typedef signed short    sint16;
typedef signed long     sint32;
typedef uint8           boolean;

#ifndef TRUE
#define TRUE   ((boolean)1U)
#endif
#ifndef FALSE
#define FALSE  ((boolean)0U)
#endif

#endif /* PLATFORM_TYPES_H */
