
#include <linux/dmi.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include "ich9-gpio.h"
#include <acpi/acpi.h>
#include "pega-btn.h"

#define PEGA_BTN_DEBUG 1
#if PEGA_BTN_DEBUG
#define dbg(fmt, ...) printk(KERN_DEBUG "[%s] : " fmt, __func__, ##__VA_ARGS__)
#define err(fmt, ...) printk(KERN_ERR "[%s] : " fmt, __func__, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#define err(fmt, ...)
#endif
#define info(fmt, ...) printk(KERN_INFO "[%s] : " fmt, __func__, ##__VA_ARGS__)

#define DRV_NAME	 	"btn"
#define DRV_VERSION	 	"0.4"
#define STATIC_BTN_MINOR	1

#define BTN_MINOR		129

#if 0
unsigned int btn_handler(void *context);
#endif

ssize_t btn_read(struct file *file, char *buf, size_t count, loff_t *ppos);
ssize_t btn_write(struct file * filp, const char __user *buf, size_t count,\
		loff_t * f_pos);
long btn_ioctl(struct file *file, unsigned int cmd, unsigned long arg) ;

int proc_btn_read(char *page, char **start, off_t offset, int count, int *eof,\
		void *data);

struct semaphore 	btn_semaphore;

typedef struct {
	struct list_head 	list;
	int			btn_index;
} btn_list;
static LIST_HEAD(pub_btn_list);

enum {
	DOWN=0,
	UP,
};

int pwr_up = UP;
int sel_up = UP;
int scr_up = UP;
int rst_up = UP;
int hdd_ovt_up = UP;
int cpu_ovt_up = UP;
wait_queue_head_t outq;
int flag = 0;

static struct work_struct pwr_wk;
static struct work_struct sel_wk;
static struct work_struct scr_wk;
static struct work_struct rst_wk;
static struct work_struct hdd_ovt_wk;
static struct work_struct cpu_ovt_wk;
static struct work_struct hdd_change_wk;

static irqreturn_t btn_isr(int irq, void *dev_id);
static struct file_operations btn_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= btn_ioctl,
	.compat_ioctl	= btn_ioctl,
};

static u32 btn_handler(void *context)
{
	return ACPI_INTERRUPT_HANDLED;
}

//acpi_gpe_handler btn_handler;
#if 0
unsigned int btn_handler(void *context)
{
	return ACPI_INTERRUPT_HANDLED;
}
#endif

long btn_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int		btn_index = WIX_NO_BTN; //no button
	btn_list 	*next;

	dbg("... \n");

	if (!access_ok(VERIFY_WRITE, arg, sizeof(int)))
		return -EFAULT;

	switch (cmd)
	{
		case READ_BUTTON_BLOCKING:
			do {
				if(wait_event_interruptible(outq, flag != 0)) {
					return -ERESTARTSYS;
				}

				if(down_interruptible(&btn_semaphore)) {
					return -ERESTARTSYS;
				}
				flag = 0;
			}while (list_empty(&pub_btn_list));
			break;
		case READ_BUTTON_NONBLOCKING:
			if(down_interruptible(&btn_semaphore))
				return -ERESTARTSYS;
			break;
		default:
			return -1;
	}

	if (!list_empty(&pub_btn_list))
	{
		next = list_entry(pub_btn_list.next, btn_list, list);
		if (copy_to_user((int *) arg, &next->btn_index, sizeof(int)))
		{
			up(&btn_semaphore);
			return -EFAULT;
		}
		list_del(&next->list);
		kfree(next);
	}
	else
	{
		if (copy_to_user((int *) arg, &btn_index, sizeof(int)))
		{
			up(&btn_semaphore);
			return -EFAULT;
		}
	}

	up(&btn_semaphore);
	return 0;
}

static struct miscdevice btn_dev = {
	BTN_MINOR,
	"buttons",
	&btn_fops,
};

int pwr_tk(struct work_struct *work)
{
	btn_list *next;

	dbg("...\n");

	if ((next = kmalloc(sizeof(btn_list), GFP_ATOMIC)) == NULL)
	{
		err("Unable to allocate memory !\n");
		return -ENOMEM;
	}

	next->btn_index = WIX_BTN_POWER;

	if(down_interruptible(&btn_semaphore))
		return -ERESTARTSYS;
	list_add_tail(&next->list, &pub_btn_list);
	up(&btn_semaphore);
	flag = 1;
	wake_up_interruptible(&outq);

	return 0;
}


int sel_tk(struct work_struct *work)
{
	btn_list *next;

	dbg("...\n");

	if ((next = kmalloc(sizeof(btn_list), GFP_ATOMIC)) == NULL) {
		err("Unable to allocate memory !\n");
		return -ENOMEM;
	}

	next->btn_index = WIX_BTN_LCM_SELECT;

	if(down_interruptible(&btn_semaphore))
		return -ERESTARTSYS;
	list_add_tail(&next->list, &pub_btn_list);
	up(&btn_semaphore);
	flag = 1;
	wake_up_interruptible(&outq);

	return 0;
}

int scr_tk(struct work_struct *work)
{
	btn_list *next;

	dbg("...\n");

	if ((next = kmalloc(sizeof(btn_list), GFP_ATOMIC)) == NULL) {
		err("Unable to allocate memory !\n");
		return -ENOMEM;
	}

	next->btn_index = WIX_BTN_LCM_SCROLL;

	if(down_interruptible(&btn_semaphore))
		return -ERESTARTSYS;
	list_add_tail(&next->list, &pub_btn_list);
	up(&btn_semaphore);
	flag = 1;
	wake_up_interruptible(&outq);

	return 0;
}

int rst_tk(struct work_struct *work)
{
	btn_list *next;

	dbg("...\n");

	if ((next = kmalloc(sizeof(btn_list), GFP_ATOMIC)) == NULL) {

		err("Unable to allocate memory !\n");
		return -ENOMEM;
	}

	next->btn_index = WIX_BTN_RESET;

	if(down_interruptible(&btn_semaphore))
		return -ERESTARTSYS;
	list_add_tail(&next->list, &pub_btn_list);
	up(&btn_semaphore);
	flag = 1;
	wake_up_interruptible(&outq);

	return 0;
}

int hdd_ovt_tk(struct work_struct *work)
{
	btn_list *next;

	dbg("...\n");

	if ((next = kmalloc(sizeof(btn_list), GFP_ATOMIC)) == NULL)
	{
		err("Unable to allocate memory !\n");
		return -ENOMEM;
	}

	next->btn_index = WIX_BTN_HDD_OVT;

	if(down_interruptible(&btn_semaphore))
		return -ERESTARTSYS;
	list_add_tail(&next->list, &pub_btn_list);
	up(&btn_semaphore);
	flag = 1;
	wake_up_interruptible(&outq);

	return 0;
}

int cpu_ovt_tk(struct work_struct *work)
{
	btn_list *next;

	dbg("...\n");

	if ((next = kmalloc(sizeof(btn_list), GFP_ATOMIC)) == NULL)
	{
		err("Unable to allocate memory !\n");
		return -ENOMEM;
	}

	next->btn_index = WIX_BTN_CPU_OVT;

	if(down_interruptible(&btn_semaphore))
		return -ERESTARTSYS;
	list_add_tail(&next->list, &pub_btn_list);
	up(&btn_semaphore);
	flag = 1;
	wake_up_interruptible(&outq);

	return 0;
}

int hdd_change_tk(struct work_struct *work)
{
	btn_list *next;

	dbg("...\n");

	if ((next = kmalloc(sizeof(btn_list), GFP_ATOMIC)) == NULL)
	{
		err("Unable to allocate memory !\n");
		return -ENOMEM;
	}

	next->btn_index = WIX_BTN_HDD_CHANGE;

	if(down_interruptible(&btn_semaphore))
		return -ERESTARTSYS;
	list_add_tail(&next->list, &pub_btn_list);
	up(&btn_semaphore);
	flag = 1;
	wake_up_interruptible(&outq);

	return 0;
}

static irqreturn_t btn_isr(int irq, void *dev_id)
{
	irqreturn_t	res = IRQ_NONE;
	int 		loop;
	unsigned int 	acpi_gpe_sts;
	int 		button_offset[] = {\
		WIX_BTN_POWER, WIX_BTN_LCM_SELECT, WIX_BTN_LCM_SCROLL, WIX_BTN_RESET, WIX_BTN_HDD_OVT, WIX_BTN_CPU_OVT, WIX_BTN_HDD_CHANGE, -1};

	dbg("...\n");

	acpi_gpe_sts = pm_reg_read32(GPE0_STS);
	for (loop = 0; button_offset[loop] != -1; loop++)
	{
		if ((acpi_gpe_sts >> (button_offset[loop] + 16)) & 0x01)
		{
			res = IRQ_HANDLED;
			switch (button_offset[loop])
			{
				case	WIX_BTN_POWER:
					if(pwr_up) {
						pwr_up = DOWN;
					} else {
						schedule_work(&pwr_wk);
						pwr_up = UP;
					}
					break;

				case	WIX_BTN_LCM_SELECT:
					if(sel_up) {
						sel_up = DOWN;
					} else {
						schedule_work(&sel_wk);
						sel_up = UP;
					}
					break;

				case	WIX_BTN_LCM_SCROLL:
					if(scr_up) {
						scr_up = DOWN;
					} else {
						schedule_work(&scr_wk);
						scr_up = UP;
					}
					break;

				case	WIX_BTN_RESET:
					if(rst_up) {
						rst_up = DOWN;
					} else {
						schedule_work(&rst_wk);
						rst_up = UP;
					}
					break;

				case	WIX_BTN_HDD_OVT:
					if(hdd_ovt_up) {
						hdd_ovt_up = DOWN;
					} else {
						schedule_work(&hdd_ovt_wk);
						hdd_ovt_up = UP;
					}
					break;

				case	WIX_BTN_CPU_OVT:
					if(cpu_ovt_up) {
						cpu_ovt_up = DOWN;
					} else {
						schedule_work(&cpu_ovt_wk);
						cpu_ovt_up = UP;
					}
					break;

				case	WIX_BTN_HDD_CHANGE:
					schedule_work(&hdd_change_wk);
					break;

				default:
					pr_info("[%s] no button\n", __func__);
					break;
			}
		}
	}

	return res;
}

static int __init btn_init(void)
{
	int ret = 0;
	acpi_status 		status;
	int buf;

	dbg("... \n");

	/* retister misc device */
	ret = misc_register(&btn_dev);
	if(ret) {
		pr_err("could not register the button driver.\n");
		return ret;
	}
	//init_MUTEX_LOCKED(&btn_semaphore);
	sema_init(&btn_semaphore, 0);
	up(&btn_semaphore);
	dbg("up\n");

	/* PWR */
	buf = WIX_BTN_POWER;
	status = acpi_install_gpe_handler(NULL, WIX_BTN_POWER, ACPI_GPE_LEVEL_TRIGGERED, &btn_handler, &buf);
	if (status != AE_OK) {
		return -EINVAL;
	}
	dbg("install acpi_install_gpe_handler ... WIX_BTN_POWER\n");

	/* LCD_SELECT_BUTTON */
	buf = WIX_BTN_LCM_SELECT;
	status = acpi_install_gpe_handler(NULL, WIX_BTN_LCM_SELECT, ACPI_GPE_LEVEL_TRIGGERED, &btn_handler, &buf);
	if (status != AE_OK)
		goto remove_pwr_gpe;
	dbg("install acpi_install_gpe_handler ... LCD_SELECT_BUTTON\n");

	/* LCD_SCROLL_BUTTON */
	buf = WIX_BTN_LCM_SCROLL;
	status = acpi_install_gpe_handler(NULL, WIX_BTN_LCM_SCROLL, ACPI_GPE_LEVEL_TRIGGERED, &btn_handler,  &buf);
	if (status != AE_OK)
		goto remove_sel_gpe;
	dbg("install acpi_install_gpe_handler ... LCD_SCROLL_BUTTON\n");

	/* RESET_BUTTON */
	buf = WIX_BTN_RESET;
	status = acpi_install_gpe_handler(NULL, WIX_BTN_RESET, ACPI_GPE_LEVEL_TRIGGERED, &btn_handler,  &buf);
	if (status != AE_OK)
		goto remove_scr_gpe;
	dbg("install acpi_install_gpe_handler ... WIX_BTN_RESET\n");

	/* HDD OVT */
	buf = WIX_BTN_HDD_OVT;
	status = acpi_install_gpe_handler(NULL, WIX_BTN_HDD_OVT, ACPI_GPE_LEVEL_TRIGGERED, &btn_handler,  &buf);
	if (status != AE_OK)
		goto remove_rst_gpe;
	dbg("install acpi_install_gpe_handler ... WIX_BTN_HDD_OVT\n");

	/* CPU OVT */
	buf = WIX_BTN_CPU_OVT;
	status = acpi_install_gpe_handler(NULL, WIX_BTN_CPU_OVT, ACPI_GPE_LEVEL_TRIGGERED, &btn_handler,  &buf);
	if (status != AE_OK)
		goto remove_hdd_ovt_gpe;
	dbg("install acpi_install_gpe_handler ... WIX_BTN_CPU_OVT\n");

	/* HDD CHANGE */
	buf = WIX_BTN_HDD_CHANGE;
	status = acpi_install_gpe_handler(NULL, WIX_BTN_HDD_CHANGE, ACPI_GPE_LEVEL_TRIGGERED, &btn_handler,  &buf);
	if (status != AE_OK)
		goto remove_cpu_ovt_gpe;
	dbg("install acpi_install_gpe_handler ... WIX_BTN_HDD_CHANGE\n");

	INIT_WORK(&pwr_wk, (void *) pwr_tk);
	INIT_WORK(&sel_wk, (void *) sel_tk);
	INIT_WORK(&scr_wk, (void *) scr_tk);
	INIT_WORK(&rst_wk, (void *) rst_tk);
	INIT_WORK(&hdd_ovt_wk, (void *) hdd_ovt_tk);
	INIT_WORK(&cpu_ovt_wk, (void *) cpu_ovt_tk);
	INIT_WORK(&hdd_change_wk, (void *) hdd_change_tk);
	init_waitqueue_head(&outq);

	/* request irq */
	if (request_irq(9, btn_isr, IRQF_DISABLED | IRQF_SHARED, "button", btn_isr))
	{
		pr_info("[%s] Can't request IRQ: 9\n", __func__);
		goto sys_err;
	}
	wake_up_interruptible(&outq);

	info("btn driver is register.\n");

	return ret;
sys_err:
	/* unregister misc device */
	misc_deregister(&btn_dev);
	acpi_remove_gpe_handler(NULL, WIX_BTN_HDD_CHANGE, &btn_handler);
remove_cpu_ovt_gpe:
	acpi_remove_gpe_handler(NULL, WIX_BTN_CPU_OVT, &btn_handler);
remove_hdd_ovt_gpe:
	acpi_remove_gpe_handler(NULL, WIX_BTN_HDD_OVT, &btn_handler);
remove_rst_gpe:
	acpi_remove_gpe_handler(NULL, WIX_BTN_RESET, &btn_handler);
remove_scr_gpe:
	acpi_remove_gpe_handler(NULL, WIX_BTN_LCM_SCROLL, &btn_handler);
remove_sel_gpe:
	acpi_remove_gpe_handler(NULL, WIX_BTN_LCM_SELECT, &btn_handler);
remove_pwr_gpe:
	acpi_remove_gpe_handler(NULL, WIX_BTN_POWER, &btn_handler);
	return -EINVAL;
}
static void __exit btn_exit(void)
{
	btn_list	*next;

	free_irq(9, btn_isr);

	msleep(1000);

	acpi_remove_gpe_handler(NULL, WIX_BTN_HDD_CHANGE, &btn_handler);
	acpi_remove_gpe_handler(NULL, WIX_BTN_CPU_OVT, &btn_handler);
	acpi_remove_gpe_handler(NULL, WIX_BTN_HDD_OVT, &btn_handler);
	acpi_remove_gpe_handler(NULL, WIX_BTN_RESET, &btn_handler);
	acpi_remove_gpe_handler(NULL, WIX_BTN_LCM_SCROLL, &btn_handler);
	acpi_remove_gpe_handler(NULL, WIX_BTN_LCM_SELECT, &btn_handler);
	acpi_remove_gpe_handler(NULL, WIX_BTN_POWER, &btn_handler);
	down(&btn_semaphore);
	dbg("down\n");
	while (!list_empty(&pub_btn_list))
	{
		next = list_entry(pub_btn_list.next, btn_list, list);
		list_del(&next->list);
		kfree(next);
	}
	up (&btn_semaphore);
	misc_deregister(&btn_dev);

	pr_info("[%s] btn driver removed.\n", __func__);

}


MODULE_DESCRIPTION ("Button Driver");
MODULE_LICENSE("GPL");

module_init(btn_init);
module_exit(btn_exit);

