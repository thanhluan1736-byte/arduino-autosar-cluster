/* SchM.c - exclusive areas: disable interrupts, restore the previous state on the outermost exit */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "SchM.h"

static uint8 SchM_Nesting = 0U;
static uint8 SchM_SavedSreg = 0U;

void SchM_Enter_Exclusive(void)
{
    uint8 sreg = SREG;
    cli();
    if (SchM_Nesting == 0U)
    {
        SchM_SavedSreg = sreg;
    }
    SchM_Nesting++;
}

void SchM_Exit_Exclusive(void)
{
    if (SchM_Nesting > 0U)
    {
        SchM_Nesting--;
        if (SchM_Nesting == 0U)
        {
            SREG = SchM_SavedSreg;
        }
    }
}
