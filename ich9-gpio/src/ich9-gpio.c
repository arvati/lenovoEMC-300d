
#define DEBUG 0

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

#include <asm/io.h>

#include "ich9-gpio.h"


static struct pci_device_id ich9_lpc_pci_id[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_7), },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, ich9_lpc_pci_id);

static u32 ich9_pm_io_base;
static u32 ich9_gpio_io_base;
static u32 ich9_rtc_io_base;

static struct resource *gp_gpio_resource;

enum lcm_a0_state {
	LCM_CONTROL_DATA=0,
	LCM_DISPLAY_DATA,
};

static void __set_ich9_gpio_attr(u32 gpio_bit, u32 port, u32 value)
{
	u32 config_data;

	config_data = inl(ich9_gpio_io_base + port);
	if(value)
		config_data |= gpio_bit;
	else
		config_data &= ~gpio_bit;
	outl(config_data, ich9_gpio_io_base + port);
}


void set_ich9_gpio_attr(u32 gpio_bit, u32 port, u32 value)
{
	__set_ich9_gpio_attr(gpio_bit, port, value);
}
EXPORT_SYMBOL_GPL(set_ich9_gpio_attr);

u32 get_ich9_gpio_attr(u32 gpio_bit, u32 port)
{

	u32 gpio_state;

	gpio_state = inl(ich9_gpio_io_base + port);
	if(gpio_state & gpio_bit)
		return 1;
	return 0;
}
EXPORT_SYMBOL_GPL(get_ich9_gpio_attr);

void __set_ich9_gpio_reg(u32 reg, u32 value)
{
	u32 config_data;

	config_data = inl(ich9_gpio_io_base + reg);
	config_data |= value;
	outl(config_data, ich9_gpio_io_base + reg);
}

void set_ich9_gpio_reg(u32 reg, u32 value)
{
	__set_ich9_gpio_reg(reg, value);
}
EXPORT_SYMBOL_GPL(set_ich9_gpio_reg);

u32 get_ich9_gpio_reg(u32 regs)
{
	u32 reg_state;

	reg_state = inl(ich9_gpio_io_base + regs);
	if(reg_state)
		return reg_state;
	return 0;
}
EXPORT_SYMBOL_GPL(get_ich9_gpio_reg);

int pm_reg_read16(int regs)
{
	int regs16 = 0x0;

	regs16 = inw(ich9_pm_io_base + regs);
#if DEBUG
	pr_info("[%s] : 0x%02x = 0x%04x\n", __func__, regs, regs16);
#endif
	if(regs16)
		return regs16;
	return 0;
}
EXPORT_SYMBOL_GPL(pm_reg_read16);

void pm_reg_write16(u16 regs, u16 value)
{
	int regs16;
	regs16 = pm_reg_read16(regs);
	regs16 |= value;
	outw(regs16, ich9_pm_io_base + regs);
#if DEBUG
	regs16 = pm_reg_read16(regs);
	pr_info("[%s] : 0x%02x = 0x%04x\n", __func__, regs, regs16);
#endif
}
EXPORT_SYMBOL_GPL(pm_reg_write16);

u32 pm_reg_read32(u32 regs)
{
	u32 reg32 = 0x0;

	reg32 = inl(ich9_pm_io_base + regs);
#if DEBUG
	pr_info("[%s] : 0x%02x = 0x%08x\n", __func__, regs, reg32);
#endif
	if(reg32)
		return reg32;
	return 0;
}
EXPORT_SYMBOL_GPL(pm_reg_read32);

void pm_reg_write32(u32 regs, u32 value)
{
	u32 regs32;
	regs32 = pm_reg_read32(regs);
	regs32 |= value;
	outl(regs32, ich9_pm_io_base + regs);
}
EXPORT_SYMBOL_GPL(pm_reg_write32);

void pm_reg_clr16(u16 regs)
{
	u32 regs16;
	regs16 = pm_reg_read16(regs);
	pm_reg_write16(regs, regs16);
}

static void ich9_lpc_cleanup(struct device *dev)
{
	if (gp_gpio_resource) {
		dev_dbg(dev, ": Releasing GPIO I/O addresses\n");
		release_region(ich9_gpio_io_base, ICH9_GPIO_SIZE);
		gp_gpio_resource = NULL;
	}
}

static int
ich9_lpc_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	int status;
	u32 gc = 0;
	u32 acpi_config;

	/* enable pci device */
	status = pci_enable_device(dev);
	if (status) {
		dev_err(&dev->dev, "pci_enable_device failed\n");
		return -EIO;
	}

	/* get PM base address */
	status = pci_read_config_dword(dev, PMBASE, &ich9_pm_io_base);
	if (status)
		goto out;

	ich9_pm_io_base &= 0x00000ff80;
	dev_dbg(&dev->dev, ": PMBASE = 0x%08x\n", ich9_pm_io_base);

	/* check LPC interface, GPIO enable or disable */
	status = pci_read_config_dword(dev, GPIO_CTRL, &gc);
	if (!(GPIO_EN & gc)) {
		status = -EEXIST;
		dev_info(&dev->dev,
				"ERROR: The LPC GPIO Block has not been enabled.\n");
		goto out;
	}

	/* get GPIO base address */
	status = pci_read_config_dword(dev, GPIO_BASE, &ich9_gpio_io_base);
	if (0 > status) {
		dev_info(&dev->dev, "Unable to read GPIOBASE.\n");
		goto out;
	}
	ich9_gpio_io_base &= 0x00000ffc0;
	dev_dbg(&dev->dev, ": GPIOBASE = 0x%08x\n", ich9_gpio_io_base);

	/* request pci resource */
	gp_gpio_resource = request_region(ich9_gpio_io_base, ICH9_GPIO_SIZE,
			KBUILD_MODNAME);
	if (NULL == gp_gpio_resource) {
		dev_info(&dev->dev,
				"ERROR Unable to register GPIO I/O addresses.\n");
		status = -1;
		goto out;
	}
	if (status)
		goto out;

out:
	if (status) {
		ich9_lpc_cleanup(&dev->dev);
		pci_disable_device(dev);
	}
	return status;
}

static void ich9_lpc_remove(struct pci_dev *dev)
{
	ich9_lpc_cleanup(&dev->dev);
	pci_disable_device(dev);
}

static struct pci_driver ich9_gpio_pci_driver = {
	.name = KBUILD_MODNAME,
	.id_table = ich9_lpc_pci_id,
	.probe = ich9_lpc_probe,
	.remove = ich9_lpc_remove,
};

static int __init ich9_gpio_init(void)
{
	int ret = 0;
	u32 all_button_pin = 0;

	ret = pci_register_driver(&ich9_gpio_pci_driver);

	if (ret)
		return ret;
	return 0;
}
static void __exit ich9_gpio_exit(void)
{
	pci_unregister_driver(&ich9_gpio_pci_driver);
	pr_info("-------------------------------------------------------------------\n");
}


MODULE_DESCRIPTION ("Intel ICH9 GPIO Driver");
MODULE_LICENSE("GPL");

module_init(ich9_gpio_init);
module_exit(ich9_gpio_exit);
