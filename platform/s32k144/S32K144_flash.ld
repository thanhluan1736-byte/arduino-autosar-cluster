/* S32K144_flash.ld - code in P-flash, data/bss/stack in SRAM_U (own minimal linker script) */
ENTRY(Reset_Handler)

MEMORY
{
    m_interrupts   (RX)  : ORIGIN = 0x00000000, LENGTH = 0x00000400
    m_flash_config (RX)  : ORIGIN = 0x00000400, LENGTH = 0x00000010
    m_text         (RX)  : ORIGIN = 0x00000410, LENGTH = 0x0007FBF0   /* 512 KB P-flash */
    m_data         (RW)  : ORIGIN = 0x20000000, LENGTH = 0x00007000   /* SRAM_U 28 KB */
}

__StackTop = ORIGIN(m_data) + LENGTH(m_data);
__StackSize = 0x800;

SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > m_interrupts

    .flash_config :
    {
        KEEP(*(.flash_config))
    } > m_flash_config

    .text :
    {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > m_text

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > m_text

    _sidata = LOADADDR(.data);

    .data : ALIGN(4)
    {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > m_data AT > m_text

    .bss (NOLOAD) : ALIGN(4)
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > m_data

    /* keep room for the stack at the end of SRAM_U */
    ASSERT(_ebss + __StackSize <= __StackTop, "RAM overflow: no room left for the stack")
}
