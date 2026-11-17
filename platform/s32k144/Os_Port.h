/* Os_Port.h (S32K144 / Cortex-M4) - interrupt lock and idle sleep used by the portable Os.c */
#ifndef OS_PORT_H
#define OS_PORT_H

#define OS_PORT_INIT()          do { } while (0)
#define OS_PORT_DISABLE_IRQ()   __asm volatile ("cpsid i" ::: "memory")
#define OS_PORT_ENABLE_IRQ()    __asm volatile ("cpsie i" ::: "memory")
/* called with PRIMASK set: WFI still wakes on a pending interrupt, which then runs after cpsie */
#define OS_PORT_IDLE()          __asm volatile ("dsb\n wfi\n cpsie i" ::: "memory")

#endif /* OS_PORT_H */
