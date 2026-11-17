/* Cdd_Lcd.h - complex driver for the kit's LCD1602 (HD44780, 4-bit mode through a PCF8574 I2C backpack) */
#ifndef CDD_LCD_H
#define CDD_LCD_H

#include "Std_Types.h"

#define CDD_LCD_MODULE_ID      253U
#define CDD_LCD_SID_INIT       0x00U
#define CDD_LCD_SID_MAIN       0x01U
#define CDD_LCD_E_NO_ACK       0x01U

#define CDD_LCD_I2C_ADDRESS    0x27U   /* kit lesson 18; some backpacks (PCF8574A) use 0x3F */
#define CDD_LCD_COLS           16U
#define CDD_LCD_ROWS           2U

void    Cdd_Lcd_Init(void);           /* blocking (~60 ms): call before the OS starts */
/* copies one line into the frame buffer (shorter text is padded with spaces); returns at once */
void    Cdd_Lcd_WriteLine(uint8 Row, const char *Text);
boolean Cdd_Lcd_IsPresent(void);
void    Cdd_Lcd_MainFunction(void);   /* every 2 ms: sends at most one changed character or cursor move */

#endif /* CDD_LCD_H */
