/* SchM.c (S32K144) - exclusive areas: PRIMASK, restored on the outermost exit */
#include "SchM.h"

static uint32 SchM_Nesting = 0U;
static uint32 SchM_SavedPrimask = 0U;

void SchM_Enter_Exclusive(void)
{
    uint32 primask;

    __asm volatile ("mrs %0, primask" : "=r" (primask));
    __asm volatile ("cpsid i" ::: "memory");
    if (SchM_Nesting == 0U)
    {
        SchM_SavedPrimask = primask;
    }
    SchM_Nesting++;
}

void SchM_Exit_Exclusive(void)
{
    if (SchM_Nesting > 0U)
    {
        SchM_Nesting--;
        if ((SchM_Nesting == 0U) && (SchM_SavedPrimask == 0U))
        {
            __asm volatile ("cpsie i" ::: "memory");
        }
    }
}
