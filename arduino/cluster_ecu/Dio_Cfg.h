/* Dio_Cfg.h - symbolic Dio channels of the cluster board (Arduino Uno pin in brackets) */
#ifndef DIO_CFG_H
#define DIO_CFG_H

#include "Mcal_Reg.h"

/* Joystick push button (A2), active low with pull-up */
#define DioConf_DioChannel_JOY_SW        MCAL_PIN(MCAL_PORT_C, 2U)
/* 28BYJ-48 stepper via ULN2003: IN1..IN4 = D8..D11 (PB0..PB3) - same pins as the kit lesson 19 */
#define DioConf_DioChannel_STEP_IN1      MCAL_PIN(MCAL_PORT_B, 0U)
#define DioConf_DioChannel_STEP_IN2      MCAL_PIN(MCAL_PORT_B, 1U)
#define DioConf_DioChannel_STEP_IN3      MCAL_PIN(MCAL_PORT_B, 2U)
#define DioConf_DioChannel_STEP_IN4      MCAL_PIN(MCAL_PORT_B, 3U)
/* Buzzer (D3 / PD3 = OC2B): driven by Pwm with a 4 kHz square wave, Port keeps it LOW when off */
#define DioConf_DioChannel_BUZZER        MCAL_PIN(MCAL_PORT_D, 3U)

/* Overspeed telltale = on-board LED (D13 / PB5) */
#define DioConf_DioChannel_LED_OVERSPEED MCAL_PIN(MCAL_PORT_B, 5U)
/* A4 / A5 are SDA / SCL of the I2C bus (LCD1602), driven by the TWI unit, not by Dio */

#endif /* DIO_CFG_H */
