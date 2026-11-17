/* Os_Port.h (AVR) - interrupt lock and idle sleep used by the portable Os.c */
#ifndef OS_PORT_H
#define OS_PORT_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define OS_PORT_INIT()          set_sleep_mode(SLEEP_MODE_IDLE)
#define OS_PORT_DISABLE_IRQ()   cli()
#define OS_PORT_ENABLE_IRQ()    sei()
/* called with interrupts disabled: sei + sleep is atomic on AVR, so no tick is missed */
#define OS_PORT_IDLE()          do { sleep_enable(); sei(); sleep_cpu(); sleep_disable(); } while (0)

#endif /* OS_PORT_H */
