/* Cdd_Lcd.c - frame buffer + incremental refresh: only characters that changed are sent, one per call,
 * so a full 32-character update never blocks the scheduler (each I2C job takes ~0.4 ms in the background). */
#include "Cdd_Lcd.h"
#include "Mcu.h"
#include "I2c.h"
#include "Det.h"
#include "Cdd_Log.h"

/* PCF8574 -> HD44780 wiring of the common backpack (same as the kit's LiquidCrystal_I2C library) */
#define LCD_RS         0x01U
#define LCD_EN         0x04U
#define LCD_BACKLIGHT  0x08U

#define LCD_CMD_CLEAR        0x01U
#define LCD_CMD_ENTRY_INC    0x06U   /* cursor moves right, no shift */
#define LCD_CMD_DISPLAY_ON   0x0CU   /* display on, cursor off, blink off */
#define LCD_CMD_FUNC_4BIT_2L 0x28U   /* 4-bit bus, 2 lines, 5x8 font */
#define LCD_CMD_SET_DDRAM    0x80U

#define LCD_CURSOR_UNKNOWN   0xFFU
#define LCD_CELLS            (CDD_LCD_ROWS * CDD_LCD_COLS)

static char    Cdd_Lcd_Frame[LCD_CELLS];   /* wanted content */
static char    Cdd_Lcd_Shown[LCD_CELLS];   /* what the LCD shows (as far as we know) */
static uint8   Cdd_Lcd_Cursor = LCD_CURSOR_UNKNOWN;
static uint8   Cdd_Lcd_ScanPos = 0U;
static uint8   Cdd_Lcd_PendingCell = LCD_CURSOR_UNKNOWN;   /* cell of the job in progress */
static boolean Cdd_Lcd_Present = FALSE;
static boolean Cdd_Lcd_ErrorReported = FALSE;

/* one byte = two nibbles latched on the falling edge of EN -> 5 I2C data bytes.
 * RS and the data are put on the bus BEFORE EN rises (HD44780 address setup time); setting RS and EN
 * in the same PCF8574 write can latch a character as a command (e.g. ' ' = 0x20 = "function set 1 line"). */
#define LCD_FRAME_LEN  5U
static void Cdd_Lcd_Encode(uint8 value, uint8 rs, uint8 out[LCD_FRAME_LEN])
{
    uint8 hi = (uint8)((value & 0xF0U) | rs | LCD_BACKLIGHT);
    uint8 lo = (uint8)((uint8)(value << 4U) | rs | LCD_BACKLIGHT);

    out[0] = hi;                    /* RS + data settle, EN low */
    out[1] = (uint8)(hi | LCD_EN);
    out[2] = hi;                    /* falling edge: high nibble latched */
    out[3] = (uint8)(lo | LCD_EN);
    out[4] = lo;                    /* falling edge: low nibble latched */
}

static boolean Cdd_Lcd_SendSync(uint8 value, uint8 rs)
{
    uint8 b[LCD_FRAME_LEN];
    Cdd_Lcd_Encode(value, rs, b);
    return (I2c_WriteSync(CDD_LCD_I2C_ADDRESS, b, LCD_FRAME_LEN) == E_OK) ? TRUE : FALSE;
}

static boolean Cdd_Lcd_NibbleSync(uint8 nibble)
{
    uint8 b[3];
    b[0] = (uint8)((uint8)(nibble << 4U) | LCD_BACKLIGHT);
    b[1] = (uint8)(b[0] | LCD_EN);
    b[2] = b[0];
    return (I2c_WriteSync(CDD_LCD_I2C_ADDRESS, b, 3U) == E_OK) ? TRUE : FALSE;
}

void Cdd_Lcd_Init(void)
{
    boolean ok;
    uint8 i;

    for (i = 0U; i < LCD_CELLS; i++)
    {
        Cdd_Lcd_Frame[i] = ' ';
        Cdd_Lcd_Shown[i] = ' ';
    }
    I2c_Init();
    Mcu_DelayUs(50UL * 1000UL);                      /* HD44780 power-on time */
    {
        uint8 idle = LCD_BACKLIGHT;     /* PCF8574 powers up with all pins high (EN = 1): drive them low first */
        (void)I2c_WriteSync(CDD_LCD_I2C_ADDRESS, &idle, 1U);
        Mcu_DelayUs(5UL * 1000UL);
    }

    /* HD44780 datasheet "initialization by instruction": 3 x 0x3 then 0x2 -> 4-bit mode */
    ok = Cdd_Lcd_NibbleSync(0x03U);
    Mcu_DelayUs(5UL * 1000UL);
    ok = (boolean)(ok & Cdd_Lcd_NibbleSync(0x03U));
    Mcu_DelayUs(5UL * 1000UL);
    ok = (boolean)(ok & Cdd_Lcd_NibbleSync(0x03U));
    Mcu_DelayUs(150UL);
    ok = (boolean)(ok & Cdd_Lcd_NibbleSync(0x02U));
    Mcu_DelayUs(150UL);
    ok = (boolean)(ok & Cdd_Lcd_SendSync(LCD_CMD_FUNC_4BIT_2L, 0U));
    ok = (boolean)(ok & Cdd_Lcd_SendSync(LCD_CMD_DISPLAY_ON, 0U));
    ok = (boolean)(ok & Cdd_Lcd_SendSync(LCD_CMD_CLEAR, 0U));
    Mcu_DelayUs(2UL * 1000UL);                       /* clear takes 1.52 ms */
    ok = (boolean)(ok & Cdd_Lcd_SendSync(LCD_CMD_ENTRY_INC, 0U));

    Cdd_Lcd_Present = ok;
    Cdd_Lcd_Cursor = 0U;                /* clear puts the cursor home */
    Cdd_Lcd_PendingCell = LCD_CURSOR_UNKNOWN;
    if (ok == FALSE)
    {
        (void)Det_ReportError(CDD_LCD_MODULE_ID, 0U, CDD_LCD_SID_INIT, CDD_LCD_E_NO_ACK);
        LOG_P("[Lcd] no ACK at I2C address 0x");
        Cdd_Log_Hex8(CDD_LCD_I2C_ADDRESS);
        LOG_P(" - check wiring / address\r\n");
    }
    else
    {
        LOG_P("[Lcd] LCD1602 ready\r\n");
    }
}

void Cdd_Lcd_WriteLine(uint8 Row, const char *Text)
{
    uint8 c;
    boolean end = FALSE;

    if ((Row >= CDD_LCD_ROWS) || (Text == NULL_PTR))
    {
        return;
    }
    for (c = 0U; c < CDD_LCD_COLS; c++)
    {
        if ((end == FALSE) && (Text[c] == '\0'))
        {
            end = TRUE;
        }
        Cdd_Lcd_Frame[(Row * CDD_LCD_COLS) + c] = (end == TRUE) ? ' ' : Text[c];
    }
}

boolean Cdd_Lcd_IsPresent(void)
{
    return Cdd_Lcd_Present;
}

void Cdd_Lcd_MainFunction(void)
{
    I2c_StatusType st = I2c_GetStatus();
    uint8 n;
    uint8 b[LCD_FRAME_LEN];

    if (Cdd_Lcd_Present == FALSE)
    {
        return;
    }
    if (st == I2C_BUSY)
    {
        return;                                   /* previous job still on the bus */
    }
    if (st == I2C_ERROR)
    {
        /* job lost (LCD unplugged?): resend that cell, re-position the cursor */
        if (Cdd_Lcd_PendingCell != LCD_CURSOR_UNKNOWN)
        {
            Cdd_Lcd_Shown[Cdd_Lcd_PendingCell] = '\0';
        }
        Cdd_Lcd_Cursor = LCD_CURSOR_UNKNOWN;
        if (Cdd_Lcd_ErrorReported == FALSE)
        {
            (void)Det_ReportError(CDD_LCD_MODULE_ID, 0U, CDD_LCD_SID_MAIN, CDD_LCD_E_NO_ACK);
            Cdd_Lcd_ErrorReported = TRUE;         /* report once, keep retrying */
        }
    }
    Cdd_Lcd_PendingCell = LCD_CURSOR_UNKNOWN;

    /* find the next cell that differs, round robin */
    for (n = 0U; n < LCD_CELLS; n++)
    {
        uint8 cell = (uint8)((Cdd_Lcd_ScanPos + n) % LCD_CELLS);
        if (Cdd_Lcd_Frame[cell] != Cdd_Lcd_Shown[cell])
        {
            if (Cdd_Lcd_Cursor != cell)
            {
                /* row 0 starts at DDRAM 0x00, row 1 at 0x40 */
                uint8 addr = (uint8)(((cell / CDD_LCD_COLS) * 0x40U) + (cell % CDD_LCD_COLS));
                Cdd_Lcd_Encode((uint8)(LCD_CMD_SET_DDRAM | addr), 0U, b);
                if (I2c_WriteAsync(CDD_LCD_I2C_ADDRESS, b, LCD_FRAME_LEN) == E_OK)
                {
                    Cdd_Lcd_Cursor = cell;
                }
                Cdd_Lcd_ScanPos = cell;
                return;                           /* the character goes in the next call */
            }
            Cdd_Lcd_Encode((uint8)Cdd_Lcd_Frame[cell], LCD_RS, b);
            if (I2c_WriteAsync(CDD_LCD_I2C_ADDRESS, b, LCD_FRAME_LEN) == E_OK)
            {
                Cdd_Lcd_Shown[cell] = Cdd_Lcd_Frame[cell];
                Cdd_Lcd_PendingCell = cell;
                /* the LCD auto-increments; past the end of a row the next cell needs a move */
                Cdd_Lcd_Cursor = (((cell + 1U) % CDD_LCD_COLS) == 0U) ? LCD_CURSOR_UNKNOWN : (uint8)(cell + 1U);
            }
            Cdd_Lcd_ScanPos = (uint8)((cell + 1U) % LCD_CELLS);
            return;
        }
    }
    Cdd_Lcd_ErrorReported = FALSE;                /* everything is shown */
}
