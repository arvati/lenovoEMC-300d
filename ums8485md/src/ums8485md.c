
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#define UMS_DEBUG
#undef UMS_DEBUG

#include <linux/dmi.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/font.h>
#include <linux/miscdevice.h>
#include "ich9-gpio.h"
#include "f71808e.h"
#include "ums8485md.h"
#include "ilogo.h"

#define DRV_NAME 	"lcm"
#define DRV_VERSION 	"V0.4"
#define STATIC_LCM_MINOR	1
#define LCM_MINOR	128

#define LCD_SI		      	(1 << 6)  /* GPIO 6 */
#define LCD_SCL		      	(1 << 7)  /* GPIO 7 */
#define SYS_REBUILD     	(1 << 12) /* GPIO 12 */
#define SYS_HEALTH      	(1 << 13) /* GPIO 13 */
#define LCD_RS		      	(1 << 16) /* GPIO 16 */
#define LCD_A0		        (1 << 19) /* GPIO 19 */
#define LCD_CS1		        (1 << 21) /* GPIO 21 */

enum lcm_a0_state {
	LCM_CONTROL_DATA=0,
	LCM_DISPLAY_DATA,
};

long lcm_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static int lcm_initdata[] = { 0xa0, 0x2f, 0xa2, 0xc8, 0x27, 0x81, 0x17, 0xaf};
static int lcm_standby[] = { 0xad, 0x02, 0xaf, 0xa5, 0xa4};

void lcm_delay(unsigned char delay)
{
	unsigned timer;
	while(delay) {
		timer = 0x300;
		while(timer) timer --;
		delay--;
	}
}

void scl_hi(void)
{
	lcm_delay(1);
	set_ich9_gpio_attr(LCD_SCL, GP_LVL, 1);
	lcm_delay(1);
}

void scl_lo(void)
{
	lcm_delay(1);
	set_ich9_gpio_attr(LCD_SCL, GP_LVL, 0);
	lcm_delay(1);
}

void si_hi(void)
{
	lcm_delay(1);
	set_ich9_gpio_attr(LCD_SI, GP_LVL, 1);
	lcm_delay(1);
}

void si_lo(void)
{
	lcm_delay(1);
	set_ich9_gpio_attr(LCD_SI, GP_LVL, 0);
	lcm_delay(1);
}

void cs_hi(void)
{
	lcm_delay(1);
	set_ich9_gpio_attr(LCD_CS1, GP_LVL, 1);
	lcm_delay(1);
}

void cs_lo(void)
{
	lcm_delay(1);
	set_ich9_gpio_attr(LCD_CS1, GP_LVL, 0);
	lcm_delay(1);
}



int lcm_open(struct inode * inode, struct file * file) {

	return 0;
}

void write_lcm_data(int type, unsigned char data) {
	int i;
	cs_lo();
	if(type) { //display data
		lcm_delay(1);
		set_ich9_gpio_attr(LCD_A0, GP_LVL, LCM_DISPLAY_DATA); /* display data */
		lcm_delay(1);
	} else { //control data
		lcm_delay(1);
		set_ich9_gpio_attr(LCD_A0, GP_LVL, LCM_CONTROL_DATA); /* control data */
		lcm_delay(1);
	}

	for(i = 0; i < 8; i++) {
		scl_lo();
		if(data & 0x80)
			si_hi();
		else
			si_lo();
		scl_hi();
		lcm_delay(1);
		data <<= 1;
	}
	cs_hi();
}

void reload_logo(void)
{
	int i, j , col;

	col = 0;
	for(i = 0; i < 8; i++) {
		write_lcm_data(LCM_CONTROL_DATA, (0x0f & col));
		write_lcm_data(LCM_CONTROL_DATA, (0x10 | (0x0f & col >> 4)));
		write_lcm_data(LCM_CONTROL_DATA, (0xb0 + i));

		for(j = 0; j < 128; j++) {
			write_lcm_data(LCM_DISPLAY_DATA, ilogo[j+128*i]);
		}
	}
}

int write_lcm(lcm_member_t buf)
{
	int i;
		write_lcm_data(LCM_CONTROL_DATA, (0xb0 + buf.page));
		write_lcm_data(LCM_CONTROL_DATA, (0x0f & buf.column));
		write_lcm_data(LCM_CONTROL_DATA, (0x10 | (0x0f & buf.column >> 4)));
	if(buf.ctrl != WIX_LCM_CMD_RESET) {
	for(i = 0; i < buf.size; i++)
		write_lcm_data(LCM_DISPLAY_DATA, buf.data[i]);
	}

	switch(buf.ctrl) {
		case WIX_LCM_CMD_PON :
			write_lcm_data(LCM_CONTROL_DATA, 0xaf);
			break;
		case WIX_LCM_CMD_POFF :
			write_lcm_data(LCM_CONTROL_DATA, 0xae);
			break;
		case WIX_LCM_CMD_RESET :
			for (i = 0; i < ARRAY_SIZE(lcm_initdata); i++) {
				write_lcm_data(LCM_CONTROL_DATA, lcm_initdata[i]);
			}
			reload_logo();
			break;
		case WIX_LCM_CMD_DISP_NORMAL :
			write_lcm_data(LCM_CONTROL_DATA, 0xa6);
			break;
		case WIX_LCM_CMD_DISP_REVERSE :
			write_lcm_data(LCM_CONTROL_DATA, 0xa7);
			break;
		case WIX_LCM_CMD_ENTIRE_DISP_ON :
			write_lcm_data(LCM_CONTROL_DATA, 0xa5);
			break;
		case WIX_LCM_CMD_ENTIRE_DISP_OFF :
			write_lcm_data(LCM_CONTROL_DATA, 0xa4);
			break;
		case WIX_LCM_CMD_ADC_SELECT_NORMAL :
			write_lcm_data(LCM_CONTROL_DATA, 0xa0);
			break;
		case WIX_LCM_CMD_ADC_SELECT_REVERSE :
			write_lcm_data(LCM_CONTROL_DATA, 0xa1);
			break;
		case WIX_LCM_CMD_OUTPUT_NORMAL :
			write_lcm_data(LCM_CONTROL_DATA, 0xc0);
			break;
		case WIX_LCM_CMD_OUTPUT_REVERSE :
			write_lcm_data(LCM_CONTROL_DATA, 0xc8);
			break;
		case WIX_LCM_CMD_WRITE_DATA:
			break;
		default :
			pr_info("[%s] Unknow ctrl command. \n", __func__);
			break;
	}
	return 0;
}

void data_dump(lcm_member_t buf)
{
	int i;

	pr_info("-------[%s] data dump -------\n", __func__);
	pr_info("ctrl : %d\n", buf.ctrl);
	pr_info("page : %d\n", buf.page);
	pr_info("column : %d\n", buf.column);
	pr_info("size : %d\n", buf.size);

	for(i = 0; i < 128; i++) {
		printk("%4d", buf.data[i]);
		if((i % 16) == 0)
			printk("\n");
	}
}

int data_check(lcm_member_t buf)
{
	if((buf.page < 0 ) || (buf.page > 7)) {
		return -1;
	}
	if((buf.column < 0 ) || (buf.column > 127))
		return -1;
	return 0;
}

long lcm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	lcm_member_t info;

	if (copy_from_user(&info, (lcm_member_t *) arg, sizeof(lcm_member_t)))
	{
		pr_err("[%s]->[%s] can't opy data from the user space!\n", DRV_NAME, __func__);
		return -EFAULT;
	}

	if(data_check(info))
		return -EFAULT;

#ifdef UMS_DEBUG
	data_dump(info);
#endif
	switch(cmd) {
		case IOCTL_DISPLAY_COMMAND :
			write_lcm(info);
			break;
		default :
			pr_info("[%s]->[%s] Unknown IOCTL command\n", DRV_NAME, __func__);
			break;
	};

	return 0;
}

int lcm_close(struct inode * inode, struct file * file)
{
	return 0;
}

static struct file_operations lcm_fops = {
	.owner	= THIS_MODULE,
	.unlocked_ioctl	= lcm_ioctl,
	.compat_ioctl	= lcm_ioctl,
	.open		= lcm_open,
	.release	= lcm_close
};

static struct miscdevice lcm_dev = {

#if STATIC_LCM_MINOR
	LCM_MINOR,
#else
	MISC_DYNAMIC_MINOR,
#endif
	"lcm",
	&lcm_fops,
};
static int lcm_gpio_init(void)
{

	int i;
	u32 all_lcm_pin = 0;

	/* Initial LCM GPIO pin */
	all_lcm_pin |= (LCD_SI | LCD_SCL | LCD_A0 | LCD_RS | LCD_CS1\
			| HDD_LED);

	set_ich9_gpio_attr(all_lcm_pin, GPIO_USE_SEL, 1);
	set_ich9_gpio_attr(all_lcm_pin, GP_IO_SEL, 0);
	set_ich9_gpio_attr(all_lcm_pin, GP_LVL, 0);
	set_ich9_gpio_attr(all_lcm_pin, GPO_BLINK, 0);
	set_ich9_gpio_attr(LCD_RS, GP_LVL, 0);
	mdelay(200);
	set_ich9_gpio_attr(LCD_RS, GP_LVL, 1);
	mdelay(200);
	set_ich9_gpio_attr(LCD_CS1, GP_LVL, 1);
	set_ich9_gpio_attr(LCD_SCL, GP_LVL, 1);

	for (i = 0; i < ARRAY_SIZE(lcm_initdata); i++) {
		write_lcm_data(LCM_CONTROL_DATA, lcm_initdata[i]);
	}
	reload_logo();
	return 0;
}



static int __init lcm_init(void)
{
	int ret = 0;
	ret = misc_register(&lcm_dev);
	if(ret) {
		pr_err("could not register the lcm driver.\n");
		return ret;
	}
	lcm_gpio_init();
	pr_info("lcm driver is register.\n");

	return ret;

}
static void __exit lcm_exit(void)
{
	misc_deregister(&lcm_dev);
}

MODULE_DESCRIPTION ("LCM Driver");
MODULE_LICENSE("GPL");

module_init(lcm_init);
module_exit(lcm_exit);

