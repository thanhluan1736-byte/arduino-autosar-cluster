/* Mcal_Reg.h - pin encoding and register access shared by Port and Dio (ATmega328P) */
#ifndef MCAL_REG_H
#define MCAL_REG_H

#include <avr/io.h>
#include "Std_Types.h"

#define MCAL_PORT_B      0U
#define MCAL_PORT_C      1U
#define MCAL_PORT_D      2U
#define MCAL_NUM_PORTS   3U

/* a pin / Dio channel is encoded as (port << 3) | bit, e.g. PB5 = 0x05, PC4 = 0x0C */
#define MCAL_PIN(port, bit)   ((uint8)(((uint8)(port) << 3U) | (uint8)(bit)))
#define MCAL_PIN_PORT(pin)    ((uint8)((uint8)(pin) >> 3U))
#define MCAL_PIN_MASK(pin)    ((uint8)(1U << ((uint8)(pin) & 7U)))
#define MCAL_PIN_VALID(pin)   (MCAL_PIN_PORT(pin) < MCAL_NUM_PORTS)

/* PINx, DDRx, PORTx are three consecutive registers; B, C, D follow each other (0x23, 0x26, 0x29) */
#define MCAL_REG_PIN(port)    (*(&PINB  + (3U * (uint8)(port))))
#define MCAL_REG_DDR(port)    (*(&DDRB  + (3U * (uint8)(port))))
#define MCAL_REG_PORT(port)   (*(&PORTB + (3U * (uint8)(port))))

#endif /* MCAL_REG_H */
