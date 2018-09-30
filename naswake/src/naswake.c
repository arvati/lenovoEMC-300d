
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#define DEBUG 1
#include <linux/dmi.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/mc146818rtc.h>
#include "naswake.h"

#define DEVICE_NAME 	"naswake"


int naswake_open(struct inode * inode, struct file * file) {

	return 0;
}

long naswake_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	switch(cmd) {
		case WAKEONLAN_EN :
			irq_reg_writel(PM1_EN, \
				(irq_reg_readl(PM1_EN) & ~(1 << 14)));
			CMOS_WRITE(0x1, 0x58);
			break;

		case WAKEONLAN_DE :
			irq_reg_writel(PM1_EN, \
				(irq_reg_readl(PM1_EN) | (1 << 14)));
			CMOS_WRITE(0x0, 0x58);
			break;

	};
	pr_info("[%s] read index 58h : 0x%02x\n", __func__, CMOS_READ(0x58));

	return 0;
}


int naswake_close(struct inode * inode, struct file * file)
{
	return 0;
}

static struct file_operations naswake_fops = {
	.owner	= THIS_MODULE,
	.unlocked_ioctl	= naswake_ioctl,
	.compat_ioctl	= naswake_ioctl,
	.open		= naswake_open,
	.release	= naswake_close
};

static struct miscdevice naswake_dev = {
	MISC_DYNAMIC_MINOR,
	"naswake",
	&naswake_fops,
};


static int __init naswake_init(void)
{
	int ret = 0;
	ret = misc_register(&naswake_dev);
	if(ret) {
		pr_err("could not register the naswake driver.\n");
		return ret;
	}
	pr_info("naswake driver is register.\n");

	return ret;

}
static void __exit naswake_exit(void)
{
	misc_deregister(&naswake_dev);
}


MODULE_DESCRIPTION ("NAS Wake on LAN, and RTC Wake Driver");
MODULE_LICENSE("GPL");

module_init(naswake_init);
module_exit(naswake_exit);

