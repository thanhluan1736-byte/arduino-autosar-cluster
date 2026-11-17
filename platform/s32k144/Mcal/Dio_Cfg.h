/* Dio_Cfg.h (S32K144) - symbolic Dio channels of the MaaZEDU board (J8 header pin in brackets) */
#ifndef DIO_CFG_H
#define DIO_CFG_H

#include "Port.h"

/* Joystick push button: GPIO0 (J8-5) = PTE0, active low with the internal pull-up */
#define DioConf_DioChannel_JOY_SW        PORT_PIN(PORT_E, 0U)
/* 28BYJ-48 stepper via ULN2003: IN1..IN4 = PTD9..PTD12 = GPIO7 (J8-19), GPIO6 (J8-17), GPIO5 (J8-15), GPIO4 (J8-13) */
#define DioConf_DioChannel_STEP_IN1      PORT_PIN(PORT_D, 9U)
#define DioConf_DioChannel_STEP_IN2      PORT_PIN(PORT_D, 10U)
#define DioConf_DioChannel_STEP_IN3      PORT_PIN(PORT_D, 11U)
#define DioConf_DioChannel_STEP_IN4      PORT_PIN(PORT_D, 12U)
/* Buzzer: GPIO3 (J8-11) = PTD13, square wave toggled by the Pwm driver */
#define DioConf_DioChannel_BUZZER        PORT_PIN(PORT_D, 13U)
/* Overspeed telltale = on-board RGB LED, red = PTD15, active low */
#define DioConf_DioChannel_LED_OVERSPEED PORT_PIN(PORT_D, 15U)
#define DIO_LED_OVERSPEED_ON_LEVEL       STD_LOW
/* other on-board RGB LED colours, kept off: green PTD0, blue PTD16 (active low) */
#define DioConf_DioChannel_LED_GREEN     PORT_PIN(PORT_D, 0U)
#define DioConf_DioChannel_LED_BLUE      PORT_PIN(PORT_D, 16U)

#endif /* DIO_CFG_H */
