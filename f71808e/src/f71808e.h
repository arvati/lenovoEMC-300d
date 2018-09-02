#ifndef __F71808E_H_INCLUDED
#define __F71808E_H_INCLUDED
#define SIO_GPIRQ		0x70	/* GPIRQ Channel Select Register */
#define SIO_GPIO3_OE		0xC0	/* GPIO3 Output Enable Register */
#define SIO_GPIO3_OD		0xC1	/* GPIO3 Output Data Register */
#define SIO_GPIO3_PS		0xC2	/* GPIO3 Pin Status Register */
#define SIO_GPIO3_DE		0xC3	/* GPIO3 Drive Enable Register */

#define SIO_GPIO2_OE		0xD0	/* GPIO2 Output Eanble Register */
#define SIO_GPIO2_OD		0xD1	/* GPIO2 Output Data Register */
#define SIO_GPIO2_PS		0xD2	/* GPIO2 Pin Status Register */
#define SIO_GPIO2_DE		0xD3	/* GPIO2 Drive Enable Register */
#define SIO_GPIO2_PME		0xD4	/* GPIO2 PME Enable Register */
#define SIO_GPIO2_IDS		0xD5	/* GPIO2 Input Detection select Register */
#define SIO_GPIO2_ES		0xD6	/* GPIO2 Event Status Register */
#define SIO_GPIO2_OMS		0xD7	/* GPIO2 Output Mode Status Register */
#define SIO_GPIO2_PW_SEL	0xD8	/* GPIO2 Pulse Width of Pule Mode Status Register */

#define SIO_GPIO1_OE		0xE0	/* GPIO1 Output Enable Register */
#define SIO_GPIO1_OD		0xE1	/* GPIO1 Output Data Register */
#define SIO_GPIO1_PS		0xE2	/* GPIO1 Pin Status Register */
#define SIO_GPIO1_DE		0xE3	/* GPIO1 Drive Enable Register */

#define SIO_GPIO0_OE		0xF0	/* GPIO0 Output Enable Register */
#define SIO_GPIO0_OD		0xF1	/* GPIO0 Output Data Register */
#define SIO_GPIO0_PS		0xF2	/* GPIO0 Pin Status Register */
#define SIO_GPIO0_DE		0xF3	/* GPIO0 Drive Enable Register */

#define	F71808E_REG_START		0x01
//#define POWER_LED			( /* GPIO 24 */
#define HDD_RED_LED				1 /* GPIO 11 */
#define SYS_WHITE_LED			2 /* GPIO 12 */
#define SYS_RED_LED			3 /* GPIO 13 */

#define STAT_SEL_A			0 /* GPIO 30 */
#define STAT_SEL_B			1 /* GPIO 31 */
#define STAT_SEL_C			2 /* GPIO 32 */

#define HDD_LED				(1 << 1) /* GPIO 11 */
#define SYS_REBU_LED			(1 << 2) /* GPIO 12 */
#define SYS_HEAL_LED			(1 << 3) /* GPIO 13 */

extern void f71808e_set_attr(int, int);

extern int f71808e_get_attr(int);

#endif
