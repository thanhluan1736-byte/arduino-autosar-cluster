/* Main.c - entry point: EcuM brings up the BSW, then the OS schedules everything */
#include "EcuM.h"
#include "Os.h"

int main(void)
{
    EcuM_Init();
    Os_Start();   /* never returns */
}
