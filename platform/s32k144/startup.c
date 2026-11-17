/* startup.c (S32K144) - vector table, flash configuration field, reset handler.
 * Own minimal startup (no NXP SDK): ECC RAM init, watchdog off, .data/.bss, main(). Core clock stays on FIRC 48 MHz. */
#include <stdint.h>
#include "S32K144_Regs.h"

/* the IRQ table is first filled with Default_Handler, then the used entries are overridden */
#pragma GCC diagnostic ignored "-Woverride-init"

extern uint32_t __StackTop;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern int main(void);

void Reset_Handler(void) __attribute__((naked, noreturn));
void Default_Handler(void);
static void Startup_C(void) __attribute__((noreturn, used));

/* handlers of the drivers (weak alias to Default_Handler when a driver is not linked) */
void NMI_Handler(void)                 __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)           __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)           __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)          __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)                 __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void LPI2C0_Master_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void LPUART1_RxTx_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void ADC0_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void LPIT0_Ch0_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void LPIT0_Ch1_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));

#define NUM_IRQS  123U   /* 139 vectors - 16 core vectors */

typedef void (*Vector_t)(void);

/* the IRQ table starts as all Default_Handler; the used IRQs are set with designated initialisers */
__attribute__((section(".isr_vector"), used))
const Vector_t Vector_Table[16U + NUM_IRQS] =
{
    [0]  = (Vector_t)(&__StackTop),
    [1]  = Reset_Handler,
    [2]  = NMI_Handler,
    [3]  = HardFault_Handler,
    [4]  = MemManage_Handler,
    [5]  = BusFault_Handler,
    [6]  = UsageFault_Handler,
    [11] = SVC_Handler,
    [12] = DebugMon_Handler,
    [14] = PendSV_Handler,
    [15] = SysTick_Handler,
    [16 ... (16U + NUM_IRQS - 1U)] = Default_Handler,
    [16U + IRQ_LPI2C0_MASTER] = LPI2C0_Master_IRQHandler,
    [16U + IRQ_LPUART1_RXTX]  = LPUART1_RxTx_IRQHandler,
    [16U + IRQ_ADC0]          = ADC0_IRQHandler,
    [16U + IRQ_LPIT0_CH0]     = LPIT0_Ch0_IRQHandler,
    [16U + IRQ_LPIT0_CH1]     = LPIT0_Ch1_IRQHandler,
};

/* Flash configuration field at 0x400: backdoor key, FPROT = unprotected, FSEC = 0xFE (UNSECURE),
 * FOPT = 0x7F, FEPROT / FDPROT = unprotected. A wrong FSEC would lock the chip. */
__attribute__((section(".flash_config"), used))
const uint8_t Flash_Config[16] =
{
    0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
    0xFFU, 0xFFU, 0xFFU, 0xFFU,
    0xFEU, 0x7FU, 0xFFU, 0xFFU
};

/* The SRAM has ECC: every word must be written once before a partial (byte) write or a read.
 * This runs before any C code uses the stack, so it is written in assembler. */
void Reset_Handler(void)
{
    __asm volatile (
        "cpsid i                  \n"
        /* watchdog off: unlock, 32-bit commands, LPO clock, disabled */
        "ldr  r0, =0x40052000     \n"
        "ldr  r1, =0xD928C520     \n"
        "str  r1, [r0, #4]        \n"
        "ldr  r1, [r0, #4]        \n"
        "ldr  r1, =0x0000FFFF     \n"
        "str  r1, [r0, #8]        \n"
        "ldr  r1, =0x00002100     \n"
        "str  r1, [r0, #0]        \n"
        /* zero SRAM_L + SRAM_U (0x1FFF8000 .. 0x20007000) */
        "ldr  r0, =0x1FFF8000     \n"
        "ldr  r1, =0x20007000     \n"
        "movs r2, #0              \n"
        "1:                       \n"
        "str  r2, [r0], #4        \n"
        "cmp  r0, r1              \n"
        "blo  1b                  \n"
        "ldr  r0, =__StackTop     \n"
        "mov  sp, r0              \n"
        "b    Startup_C           \n"
        ::: "r0", "r1", "r2", "memory");
}

static void Startup_C(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;

    while (dst < &_edata)
    {
        *dst = *src;
        dst++;
        src++;
    }
    for (dst = &_sbss; dst < &_ebss; dst++)
    {
        *dst = 0U;
    }
    (void)main();
    for (;;) { }
}

void Default_Handler(void)
{
    for (;;) { }   /* unexpected interrupt / fault: stop here (visible in the debugger) */
}
