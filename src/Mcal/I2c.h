/* I2c.h - I2C master on the TWI unit (SDA = A4, SCL = A5), 100 kHz, write transfers only.
 * AUTOSAR has no standard I2C MCAL; the API follows the usual vendor style (async job + status). */
#ifndef I2C_H
#define I2C_H

#include "Std_Types.h"

#define I2C_MODULE_ID        254U
#define I2C_SID_WRITE        0x01U
#define I2C_E_BUSY           0x01U
#define I2C_E_PARAM          0x02U

#define I2C_BUS_HZ           100000UL   /* the PCF8574 is rated for 100 kHz only */
#define I2C_MAX_LENGTH       8U

typedef enum
{
    I2C_IDLE = 0U,
    I2C_BUSY,
    I2C_ERROR          /* last transfer: no ACK (device missing) or bus error */
} I2c_StatusType;

void           I2c_Init(void);
/* blocking, polls the TWI flag: for init code that runs before interrupts are enabled */
Std_ReturnType I2c_WriteSync(uint8 Address7, const uint8 *Data, uint8 Length);
/* non-blocking: data is copied, the TWI interrupt does the transfer; poll I2c_GetStatus() */
Std_ReturnType I2c_WriteAsync(uint8 Address7, const uint8 *Data, uint8 Length);
I2c_StatusType I2c_GetStatus(void);

#endif /* I2C_H */
