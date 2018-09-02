#ifndef __ICH9_H_INCLUDED
#define __ICH9_H_INCLUDED
#include <linux/device.h>
#define PMBASE			0x40 /* ACPI Base Address Register */
#define ACPI_CNTL		0x44 /* ACPI Conttrol register */
#define GPIO_BASE		0x48 /* GPIO Base Address Register */
#define GPIO_CTRL		0x4c /* GPIO Control Register */

#define RTC_RAM0		0x70
#define RTC_RAM1		0x71

#define GPIO_EN			0x010 /* GPIO Control Register Bit4=0 */

/* GPIO Registers */
#define GPIO_ROUT 		0xB8
#define SET_GPIO_ROUT_SCI(nr) 	(1 << (nr*2+1))
#define SET_GPIO_ROUT_SMI(nr) 	(1 << (nr*2))

/* Power Managerment I/O Registers */
#define PM1_STS			0x00  /* 16 bits */
#define PM1_EN			0x02
#define PM1_CNT			0x04
#define PM1_TMR			0x08
#define PROC_CNT		0x10
#define GPE0_STS		0x20
#define GPE0_EN			0x28
#define SMI_EN			0x30
#define SMI_STS			0x34
#define GPE_CNTL		0x42
#define DEVACT_STS		0x44

/* PM1_STS Register */
#define RTC_STS			(1 << 10)
#define PCIEXPWAKSTS		(1 << 14)

/* PM1_EN Register */
#define RTC_EN			(1 << 10)
#define PICEXPWAK_DIS		(1 << 14)
#define PWRBTN_EN		(1 << 8)

/* GPE0_STS Register */
#define GPIOn_STS(nr)		(1 << (16+nr))

/* GPE0_EN Register -> 28h 64 bits*/
#define GPIn_EN(nr)		(1 << (16+nr))


#define ICH9_GPIO_SIZE		64

#define LCD_SEL_GPIO		4
#define LCD_SCR_GPIO		5
#define LCD_SELECT	    	(1 << LCD_SEL_GPIO)  /* GPIO 4 */
#define LCD_SCROLL	    	(1 << LCD_SCR_GPIO)  /* GPIO 5 */


#define GPIO_USE_SEL		0x000
#define GP_IO_SEL		0x004
#define GP_LVL		    	0x00c
#define GPO_BLINK	    	0x018
#define GP_SER_BLINK		0x01c
#define GP_SB_CMDSTS		0x020
#define GP_SB_DATA		0x024
#define GPI_INV		    	0x02C
#define GPIO_USE_SEL2		0x030
#define GP_IO_SEL2	  	0x034
#define GP_LVL2		    	0x038

extern void set_ich9_gpio_attr(u32 , u32, u32);
extern u32 get_ich9_gpio_attr(u32, u32);
extern void set_ich9_gpio_reg(u32, u32);
extern u32 get_ich9_gpio_reg(u32);
extern int pm_reg_read16(int);
extern void pm_reg_write16(u16, u16);
extern u32 pm_reg_read32(u32);
extern void pm_reg_write32(u32, u32 );

#endif
