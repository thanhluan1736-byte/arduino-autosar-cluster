/* S32K144_Regs.h - minimal register map of the S32K144 peripherals used by this project.
 * Written from the S32K1xx reference manual (addresses / bit positions); only what the MCAL needs. */
#ifndef S32K144_REGS_H
#define S32K144_REGS_H

#include <stdint.h>

#define REG32(addr)            (*(volatile uint32_t *)(addr))
#define REG8(addr)             (*(volatile uint8_t *)(addr))

/* ---------------- PCC: peripheral clock control ---------------- */
#define PCC_BASE               0x40065000UL
#define PCC_REG(index)         REG32(PCC_BASE + (4UL * (index)))
#define PCC_FTFC_INDEX         32U
#define PCC_LPIT_INDEX         55U
#define PCC_ADC0_INDEX         59U
#define PCC_PORTA_INDEX        73U
#define PCC_PORTB_INDEX        74U
#define PCC_PORTC_INDEX        75U
#define PCC_PORTD_INDEX        76U
#define PCC_PORTE_INDEX        77U
#define PCC_LPI2C0_INDEX       102U
#define PCC_LPUART1_INDEX      107U
#define PCC_CGC                (1UL << 30)
#define PCC_PCS(x)             (((uint32_t)(x) & 7UL) << 24)
#define PCC_PCS_FIRCDIV2       3U                     /* 48 MHz on this project */

/* ---------------- SCG: clock generation ---------------- */
#define SCG_FIRCDIV            REG32(0x40064304UL)
#define SCG_FIRCDIV1(x)        ((uint32_t)(x) & 7UL)
#define SCG_FIRCDIV2(x)        (((uint32_t)(x) & 7UL) << 8)

/* ---------------- WDOG ---------------- */
#define WDOG_CS                REG32(0x40052000UL)
#define WDOG_CNT               REG32(0x40052004UL)
#define WDOG_TOVAL             REG32(0x40052008UL)
#define WDOG_UNLOCK_KEY        0xD928C520UL

/* ---------------- RCM: reset reason ---------------- */
#define RCM_SRS                REG32(0x4007F008UL)
#define RCM_SRS_LVD            (1UL << 1)
#define RCM_SRS_WDOG           (1UL << 5)
#define RCM_SRS_PIN            (1UL << 6)
#define RCM_SRS_POR            (1UL << 7)
#define RCM_SRS_LOCKUP         (1UL << 9)
#define RCM_SRS_SW             (1UL << 10)

/* ---------------- PORT (pin mux) and GPIO ---------------- */
#define PORT_BASE(port)        (0x40049000UL + (0x1000UL * (port)))   /* A..E = 0..4 */
#define PORT_PCR(port, pin)    REG32(PORT_BASE(port) + (4UL * (pin)))
#define PORT_PCR_PS            (1UL << 0)            /* pull select: 1 = up */
#define PORT_PCR_PE            (1UL << 1)            /* pull enable */
#define PORT_PCR_MUX(x)        (((uint32_t)(x) & 7UL) << 8)

#define GPIO_BASE(port)        (0x400FF000UL + (0x40UL * (port)))
#define GPIO_PDOR(port)        REG32(GPIO_BASE(port) + 0x00UL)
#define GPIO_PSOR(port)        REG32(GPIO_BASE(port) + 0x04UL)
#define GPIO_PCOR(port)        REG32(GPIO_BASE(port) + 0x08UL)
#define GPIO_PTOR(port)        REG32(GPIO_BASE(port) + 0x0CUL)
#define GPIO_PDIR(port)        REG32(GPIO_BASE(port) + 0x10UL)
#define GPIO_PDDR(port)        REG32(GPIO_BASE(port) + 0x14UL)

/* ---------------- LPIT0 ---------------- */
#define LPIT_MCR               REG32(0x40037008UL)
#define LPIT_MSR               REG32(0x4003700CUL)
#define LPIT_MIER              REG32(0x40037010UL)
#define LPIT_TVAL(ch)          REG32(0x40037020UL + (0x10UL * (ch)))
#define LPIT_TCTRL(ch)         REG32(0x40037028UL + (0x10UL * (ch)))
#define LPIT_MCR_M_CEN         (1UL << 0)
#define LPIT_MCR_DBG_EN        (1UL << 3)
#define LPIT_TCTRL_T_EN        (1UL << 0)

/* ---------------- ADC0 ---------------- */
#define ADC0_SC1A              REG32(0x4003B000UL)
#define ADC0_CFG1              REG32(0x4003B040UL)
#define ADC0_CFG2              REG32(0x4003B044UL)
#define ADC0_RA                REG32(0x4003B048UL)
#define ADC0_SC2               REG32(0x4003B090UL)
#define ADC0_SC3               REG32(0x4003B094UL)
#define ADC_SC1_ADCH(x)        ((uint32_t)(x) & 0x1FUL)
#define ADC_SC1_AIEN           (1UL << 6)
#define ADC_SC1_COCO           (1UL << 7)
#define ADC_SC1_ADCH_DISABLED  0x1FUL
#define ADC_CFG1_MODE_12BIT    (1UL << 2)
#define ADC_CFG1_ADIV_4        (2UL << 5)
#define ADC_SC3_AVGS_4         0UL
#define ADC_SC3_AVGS_32        3UL
#define ADC_SC3_AVGE           (1UL << 2)
#define ADC_SC3_CAL            (1UL << 7)

/* ---------------- LPI2C0 (master) ---------------- */
#define LPI2C0_MCR             REG32(0x40066010UL)
#define LPI2C0_MSR             REG32(0x40066014UL)
#define LPI2C0_MIER            REG32(0x40066018UL)
#define LPI2C0_MCFGR1          REG32(0x40066024UL)
#define LPI2C0_MCCR0           REG32(0x40066048UL)
#define LPI2C0_MFCR            REG32(0x40066058UL)
#define LPI2C0_MFSR            REG32(0x4006605CUL)
#define LPI2C0_MTDR            REG32(0x40066060UL)
#define LPI2C_MCR_MEN          (1UL << 0)
#define LPI2C_MCR_RST          (1UL << 1)
#define LPI2C_MCR_DBGEN        (1UL << 3)
#define LPI2C_MCR_RTF          (1UL << 8)
#define LPI2C_MCR_RRF          (1UL << 9)
#define LPI2C_MSR_TDF          (1UL << 0)
#define LPI2C_MSR_SDF          (1UL << 9)
#define LPI2C_MSR_NDF          (1UL << 10)
#define LPI2C_MSR_ALF          (1UL << 11)
#define LPI2C_MSR_FEF          (1UL << 12)
#define LPI2C_MSR_PLTF         (1UL << 13)
#define LPI2C_MSR_W1C_ALL      0x00007F00UL
#define LPI2C_MSR_MBF          (1UL << 24)
#define LPI2C_MIER_TDIE        (1UL << 0)
#define LPI2C_MIER_SDIE        (1UL << 9)
#define LPI2C_MIER_NDIE        (1UL << 10)
#define LPI2C_MIER_ALIE        (1UL << 11)
#define LPI2C_MCFGR1_PRESCALE(x) ((uint32_t)(x) & 7UL)
#define LPI2C_MCCR0_VAL(lo, hi, sethold, datavd) \
    (((uint32_t)(lo) & 0x3FUL) | (((uint32_t)(hi) & 0x3FUL) << 8) | \
     (((uint32_t)(sethold) & 0x3FUL) << 16) | (((uint32_t)(datavd) & 0x3FUL) << 24))
#define LPI2C_MFCR_TXWATER(x)  ((uint32_t)(x) & 3UL)
#define LPI2C_MFSR_TXCOUNT     (LPI2C0_MFSR & 7UL)
#define LPI2C_FIFO_SIZE        4U
#define LPI2C_CMD_TX           (0UL << 8)
#define LPI2C_CMD_STOP         (2UL << 8)
#define LPI2C_CMD_START        (4UL << 8)

/* ---------------- LPUART1 ---------------- */
#define LPUART1_BAUD           REG32(0x4006B010UL)
#define LPUART1_STAT           REG32(0x4006B014UL)
#define LPUART1_CTRL           REG32(0x4006B018UL)
#define LPUART1_DATA           REG32(0x4006B01CUL)
#define LPUART_BAUD_SBR(x)     ((uint32_t)(x) & 0x1FFFUL)
#define LPUART_BAUD_OSR(x)     (((uint32_t)(x) & 0x1FUL) << 24)
#define LPUART_STAT_TDRE       (1UL << 23)
#define LPUART_CTRL_TE         (1UL << 19)
#define LPUART_CTRL_TIE        (1UL << 23)

/* ---------------- FTFC (flash controller, used for the D-flash of FlexNVM) ---------------- */
#define FTFC_FSTAT             REG8(0x40020000UL)
#define FTFC_FCNFG             REG8(0x40020001UL)
#define FTFC_FCNFG_EEERDY      (1U << 0)            /* FlexNVM partitioned as emulated EEPROM */
#define FTFC_FCCOB_BASE        0x40020004UL
#define FTFC_FSTAT_MGSTAT0     (1U << 0)
#define FTFC_FSTAT_FPVIOL      (1U << 4)
#define FTFC_FSTAT_ACCERR      (1U << 5)
#define FTFC_FSTAT_RDCOLERR    (1U << 6)
#define FTFC_FSTAT_CCIF        (1U << 7)
#define FTFC_CMD_PROGRAM_PHRASE 0x07U
#define FTFC_CMD_ERASE_SECTOR   0x09U
#define DFLASH_READ_BASE       0x10000000UL        /* D-flash as seen by the CPU */
#define DFLASH_CMD_BASE        0x00800000UL        /* D-flash address for FTFC commands */

/* ---------------- Cortex-M4 core ---------------- */
#define NVIC_ISER(n)           REG32(0xE000E100UL + (4UL * (n)))
#define NVIC_ICPR(n)           REG32(0xE000E280UL + (4UL * (n)))
#define NVIC_IPR8(irq)         REG8(0xE000E400UL + (irq))
#define DEMCR                  REG32(0xE000EDFCUL)
#define DEMCR_TRCENA           (1UL << 24)
#define DWT_CTRL               REG32(0xE0001000UL)
#define DWT_CYCCNT             REG32(0xE0001004UL)
#define DWT_CTRL_CYCCNTENA     (1UL << 0)

/* ---------------- IRQ numbers ---------------- */
#define IRQ_LPI2C0_MASTER      24U
#define IRQ_LPUART1_RXTX       33U
#define IRQ_ADC0               39U
#define IRQ_LPIT0_CH0          48U
#define IRQ_LPIT0_CH1          49U

static inline void Nvic_EnableIrq(uint32_t irq, uint8_t prio)
{
    NVIC_IPR8(irq) = (uint8_t)(prio << 4);          /* 4 priority bits */
    NVIC_ICPR(irq >> 5) = (1UL << (irq & 31UL));
    NVIC_ISER(irq >> 5) = (1UL << (irq & 31UL));
}

#define CORE_CLOCK_HZ          48000000UL          /* FIRC, the reset default */
#define PERIPH_CLOCK_HZ        48000000UL          /* FIRCDIV2 = /1 */

#endif /* S32K144_REGS_H */
