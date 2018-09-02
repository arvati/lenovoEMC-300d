/*
 * f71808e.c
 */


#define DVT_BOARD 1

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/acpi.h>
#include "ich9-gpio.h"
#include "f71808e.h"

/* debug define */
#define PEGA_HWMON_F71808E_DEBUG 1
#if PEGA_HWMON_F71808E_DEBUG
#define dbg(fmt, ...) printk(KERN_DEBUG "F71808E-[%s] : " fmt, __func__, ##__VA_ARGS__)
#define err(fmt, ...) printk(KERN_ERR "F71808E-[%s] : " fmt, __func__, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#define err(fmt, ...)
#endif
#define info(fmt, ...) printk(KERN_INFO "F71808E-[%s] : " fmt, __func__, ##__VA_ARGS__)

#define DRVNAME "f71808e"

#define SIO_F71808E_LD_HWM	0x04	/* Hardware monitor logical device */
#define SIO_F71808E_LD_GPIO	0x06	/* GPIO logical device */
#define SIO_UNLOCK_KEY		0x87	/* Key to enable Super-I/O */
#define SIO_LOCK_KEY		0xAA	/* Key to diasble Super-I/O */

#define SIO_REG_LDSEL		0x07	/* Logical device select */
#define SIO_REG_DEVID		0x20	/* Device ID (2 bytes) */
#define SIO_REG_DEVREV		0x22	/* Device revision */
#define SIO_REG_MANID		0x23	/* Fintek ID (2 bytes) */
#define SIO_MULTI_FUN_EN	0x29	/* Multi-function select Register  device enable */
#define SIO_MULTI_FUN_EN2	0x2B	/* Multi-function select Register  device enable */
#define SIO_REG_ENABLE		0x30	/* Logical device enable */
#define SIO_REG_ADDR		0x60	/* Logical device address (2 bytes) */

#define SIO_FINTEK_ID		0x1934	/* Manufacturers ID */
#define SIO_F71808_ID		0x0901  /* Chipset ID */


#define REGION_LENGTH		8
#define ADDR_REG_OFFSET		5
#define DATA_REG_OFFSET		6

#define F71808E_REG_PECI		0x0A

#define F71808E_REG_IN(nr)		(0x20  + (nr))

#define F71808E_REG_IN0_HIGH		0x31 /* V0 over-voltage limit(V0_OVV_LIMIT, the unit is 9mv */
#define F71808E_REG_IN4_HIGH		0x35 /* V0 over-voltage limit(V0_OVV_LIMIT, the unit is 9mv */
#define F71808E_REG_IN5_HIGH		0x36 /* V0 over-voltage limit(V0_OVV_LIMIT, the unit is 9mv */


#define F71808E_REG_FAN(nr)		(0xA0 + (16 * (nr))) /* FAN(nr) count reading(A0->MSB, A1->LSB) */
#define F71808E_REG_FAN_TARGET(nr)	(0xA2 + (16 * (nr))) /* FAN(nr) RPM mode reading(A2->MSB, A3->LSB) */
#define F71808E_REG_FAN_FULL_SPEED(nr)	(0xA4 + (16 * (nr))) /* FAN(nr) full speed count reading(A4->MSB, A5->LSB) */
#define F71808E_REG_FAN_STATUS		0x92
#define F71808E_REG_FAN_BEEP		0x93

#define F71808E_REG_TEMP(nr)		(0x70 + 2 * (nr))
#define F71808E_REG_TEMP_OVT(nr)	(0x80 + 2 * (nr))
#define F71808E_REG_TEMP_HIGH(nr)	(0x81 + 2 * (nr))
#define F71808E_REG_TEMP_STATUS	0x62
#define F71808E_REG_TEMP_BEEP		0x63 /* for F71882fg */
#define F71808E_REG_TEMP_CONFIG	0x69 /* for F71805fg */
#define F71808E_REG_TEMP_HYST(nr)	(0x6C + (nr))
#define F71808E_REG_TEMP_TYPE		0x6B
#define F71808E_REG_TEMP_DIODE_OPEN	0x6F

#define F71808E_REG_PWM(nr)		(0xA3 + (16 * (nr)))
#define F71808E_REG_PWM_TYPE		0x94
#define F71808E_REG_PWM_ENABLE		0x96

#define F71808E_REG_FAN_HYST(nr)	(0x98 + (nr))

#define F71808E_REG_POINT_PWM(pwm, point)	(0xAA + (point) + (16 * (pwm)))
#define F71808E_REG_POINT_TEMP(pwm, point)	(0xA6 + (point) + (16 * (pwm)))
#define F71808E_REG_POINT_MAPPING(nr)		(0xAF + 16 * (nr))

#define	F71808E_REG_START		0x01

#define FAN_MIN_DETECT			366 /* Lowest detectable fanspeed */

#define LED_LOW	0x04
#define LED_MED 0x02
#define LED_HIG 0x01

//static struct mutex led_mutex_lck;
static unsigned short force_id;
module_param(force_id, ushort, 0);
MODULE_PARM_DESC(force_id, "Override the detected device ID");

enum chips { f71808e };

static const char *f71808e_names[] = {
	"f71808e",
};

static struct platform_device *f71808e_pdev;

/* Super-I/O Function prototypes */
static inline int superio_inb(int base, int reg);
static inline int superio_inw(int base, int reg);
static inline void superio_enter(int base);
static inline void superio_select(int base, int ld);
static inline void superio_exit(int base);
struct f71808e_sio_data {
	enum chips type;
};

static int blink_delay = HZ/2;
struct timer_list blink_timer;
char pwr_white_led_status = 0, system_red_led_status = 0, system_white_led_status = 0, hdd_red_led_status = 0, blink_status = 0;

enum LED_STATE {
	LED_OFF=0,
	LED_ON,
	LED_BLINK,
};

static int nr_sata = 0;

struct f71808e_data {
	unsigned short addr;
	enum chips type;
	struct device *hwmon_dev;

	struct mutex update_lock;
	int temp_start;			/* temp numbering start (0 or 1) */
	char valid;			/* !=0 if following fields are valid */
	unsigned long last_updated;	/* In jiffies */
	unsigned long last_limits;	/* In jiffies */

	/* Register Values */
	u8	in[9];
	u8	in0_max;
	u8	in4_max;
	u8	in5_max;
	u16	fan[3];
	u16	fan_target[3];
	u16	fan_full_speed[3];
	u8	fan_status;
	u8	fan_beep;

	u16	temp[4];
	u8	temp_ovt[4];
	u8	temp_high[4];
	u8	temp_hyst[2]; /* 2 hysts stored per reg */
	u8	temp_type[4];
	u8	temp_status;
	u8	temp_diode_open;
	u8	pwm[4];
	u8	pwm_enable;
	u8	pwm_auto_point_hyst[2];
	u8	pwm_auto_point_mapping[4];
	u8	pwm_auto_point_pwm[4][5];
	u8	pwm_auto_point_temp[4][4];

	u8 	pwr_white_led; /* super io GPIO 24 */
	u8 	system_red_led; /* super io GPIO 13 */
	u8 	system_white_led; /* super io GPIO 12 */
	u8 	hdd_red_led; /* super io GPIO 11 */
	u8 	led_brightness;
	u8 	led_blinkrate;
	u8 	sata_link_led;
};

static ssize_t show_in(struct device *dev, struct device_attribute *devattr,
		char *buf);
static ssize_t show_temp(struct device *dev, struct device_attribute
		*devattr, char *buf);
static ssize_t show_fan(struct device *dev, struct device_attribute *devattr,
		char *buf);
static ssize_t show_fan_full_speed(struct device *dev,
		struct device_attribute *devattr, char *buf);
static ssize_t store_fan_full_speed(struct device *dev,
		struct device_attribute *devattr, const char *buf, size_t count);
static ssize_t show_pwm(struct device *dev, struct device_attribute *devattr,
		char *buf);
static ssize_t store_pwm(struct device *dev, struct device_attribute *devattr,
		const char *buf, size_t count);
static ssize_t show_pwm_enable(struct device *dev,
		struct device_attribute *devattr, char *buf);
static ssize_t store_pwm_enable(struct device *dev,
		struct device_attribute	*devattr, const char *buf, size_t count);
static ssize_t show_name(struct device *dev, struct device_attribute *devattr,
		char *buf);
static ssize_t show_led(struct device *dev, struct device_attribute *devattr, char *buf);
static ssize_t store_led(struct device *dev, struct device_attribute *devattr,
		const char *buf, size_t count);
static int f71808e_probe(struct platform_device * pdev);
static int f71808e_remove(struct platform_device *pdev);

static struct platform_driver f71808e_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= DRVNAME,
	},
	.probe		= f71808e_probe,
	.remove		= f71808e_remove,
};

static DEVICE_ATTR(name, S_IRUGO, show_name, NULL);

static struct sensor_device_attribute_2 f71808e_fan_attr[] = {
	SENSOR_ATTR_2(cpufan_input, S_IRUGO, show_fan, NULL, 0, 0),
	SENSOR_ATTR_2(cpufan_full_speed, S_IRUGO|S_IWUSR,
			show_fan_full_speed,
			store_fan_full_speed, 0, 0),
	SENSOR_ATTR_2(cpufan, S_IRUGO|S_IWUSR, show_pwm, store_pwm, 0, 0),
	SENSOR_ATTR_2(cpufan_enable, S_IRUGO|S_IWUSR, show_pwm_enable,
			store_pwm_enable, 0, 0),
	SENSOR_ATTR_2(systemfan1_input, S_IRUGO, show_fan, NULL, 0, 1),
	SENSOR_ATTR_2(systemfan1_full_speed, S_IRUGO|S_IWUSR,
			show_fan_full_speed,
			store_fan_full_speed, 0, 1),
	SENSOR_ATTR_2(systemfan1, S_IRUGO|S_IWUSR, show_pwm, store_pwm, 0, 1),
	SENSOR_ATTR_2(systemfan1_enable, S_IRUGO|S_IWUSR, show_pwm_enable,
			store_pwm_enable, 0, 1),
	SENSOR_ATTR_2(systemfan2_input, S_IRUGO, show_fan, NULL, 0, 2),
	SENSOR_ATTR_2(systemfan2_full_speed, S_IRUGO|S_IWUSR,
			show_fan_full_speed,
			store_fan_full_speed, 0, 2),
	SENSOR_ATTR_2(systemfan2, S_IRUGO|S_IWUSR, show_pwm, store_pwm, 0, 2),
	SENSOR_ATTR_2(systemfan2_enable, S_IRUGO|S_IWUSR, show_pwm_enable,
			store_pwm_enable, 0, 2),
};

static struct sensor_device_attribute_2 f71808e_in_temp_attr[] = {
	SENSOR_ATTR_2(VCC3V, S_IRUGO, show_in, NULL, 0, 0),
	SENSOR_ATTR_2(VCore, S_IRUGO, show_in, NULL, 0, 1),
	SENSOR_ATTR_2(+1.5V_core, S_IRUGO, show_in, NULL, 0, 2),
	SENSOR_ATTR_2(V3, S_IRUGO, show_in, NULL, 0, 3),
	SENSOR_ATTR_2(12V, S_IRUGO, show_in, NULL, 0, 4),
	SENSOR_ATTR_2(+5V, S_IRUGO, show_in, NULL, 0, 5),
	SENSOR_ATTR_2(VSB3V, S_IRUGO, show_in, NULL, 0, 7),
	SENSOR_ATTR_2(VBAT, S_IRUGO, show_in, NULL, 0, 8),
	SENSOR_ATTR_2(cputemp, S_IRUGO, show_temp, NULL, 0, 1),
	SENSOR_ATTR_2(systemtemp, S_IRUGO, show_temp, NULL, 0, 2),
};

static struct sensor_device_attribute_2 f71808e_led_attr[] = {
	SENSOR_ATTR_2(pwr_white_led, S_IRUGO|S_IWUSR, show_led, store_led, 0, 0),
	SENSOR_ATTR_2(system_red_led, S_IRUGO|S_IWUSR, show_led, store_led, 0, 1),
	SENSOR_ATTR_2(system_white_led, S_IRUGO|S_IWUSR, show_led, store_led, 0, 2),
	SENSOR_ATTR_2(hdd_red_led, S_IRUGO|S_IWUSR, show_led, store_led, 0, 3),
	SENSOR_ATTR_2(led_brightness, S_IRUGO|S_IWUSR, show_led, store_led, 0, 4),
	SENSOR_ATTR_2(led_blinkrate, S_IRUGO|S_IWUSR, show_led, store_led, 0, 5),
	SENSOR_ATTR_2(sata_link_led, S_IRUGO|S_IWUSR, show_led, store_led, 0, 6),
};


/* Super I/O functions */
static inline int superio_inb(int base, int reg)
{
	outb(reg, base);
	return inb(base + 1);
}

static int superio_inw(int base, int reg)
{
	int val;
	outb(reg++, base);
	val = inb(base + 1) << 8;
	outb(reg, base);
	val |= inb(base + 1);
	return val;
}

static inline void superio_outb(int base, int reg, u8 val)
{
	outb(reg, base);
	outb(val, base + 1);
}

static inline void superio_enter(int base)
{
	/* according to the datasheet the key must be send twice! */
	outb( SIO_UNLOCK_KEY, base);
	outb( SIO_UNLOCK_KEY, base);
}

static inline void superio_select( int base, int ld)
{
	outb(SIO_REG_LDSEL, base);
	outb(ld, base + 1);
}

static inline void superio_exit(int base)
{
	outb(SIO_LOCK_KEY, base);
}

void f71808e_set_attr(int pin, int state)
{
	int sioaddr = 0x2e;

	superio_enter(sioaddr);
	superio_select(sioaddr, SIO_F71808E_LD_GPIO);
	superio_outb(sioaddr, pin, state);
}
EXPORT_SYMBOL_GPL(f71808e_set_attr);

int f71808e_get_attr(int pin)
{
	int sioaddr = 0x2e;

	superio_enter(sioaddr);
	superio_select(sioaddr, SIO_F71808E_LD_GPIO);
	return superio_inb(sioaddr, pin);
}
EXPORT_SYMBOL_GPL(f71808e_get_attr);

static inline int fan_from_reg(u16 reg)
{
	return reg ? (1500000 / reg) : 0;
}

static inline u16 fan_to_reg(int fan)
{
	return fan ? (1500000 / fan) : 0;
}

static u8 f71808e_read8(struct f71808e_data *data, u8 reg)
{
	u8 val;

	outb(reg, data->addr + ADDR_REG_OFFSET);
	val = inb(data->addr + DATA_REG_OFFSET);

	return val;
}

static u16 f71808e_read16(struct f71808e_data *data, u8 reg)
{
	u16 val;

	outb(reg++, data->addr + ADDR_REG_OFFSET);
	val = inb(data->addr + DATA_REG_OFFSET) << 8;
	outb(reg, data->addr + ADDR_REG_OFFSET);
	val |= inb(data->addr + DATA_REG_OFFSET);

	return val;
}

static void f71808e_write8(struct f71808e_data *data, u8 reg, u8 val)
{
	outb(reg, data->addr + ADDR_REG_OFFSET);
	outb(val, data->addr + DATA_REG_OFFSET);
}

static void f71808e_write16(struct f71808e_data *data, u8 reg, u16 val)
{
	outb(reg++, data->addr + ADDR_REG_OFFSET);
	outb(val >> 8, data->addr + DATA_REG_OFFSET);
	outb(reg, data->addr + ADDR_REG_OFFSET);
	outb(val & 255, data->addr + DATA_REG_OFFSET);
}

static u16 f71808e_read_temp(struct f71808e_data *data, int nr)
{
	return f71808e_read8(data, F71808E_REG_TEMP(nr));
}

static struct f71808e_data *f71808e_update_device(struct device *dev)
{
	struct f71808e_data *data = dev_get_drvdata(dev);
	int nr, reg;
	int nr_fans = 2;
	int nr_ins = 9;

	if(nr_sata == 6)
		nr_fans = 3;

	mutex_lock(&data->update_lock);

	/* Update once every 60 seconds */

	if ( time_after(jiffies, data->last_limits + 60 * HZ ) ||
			!data->valid) {

		data->in0_max =
			f71808e_read8(data, F71808E_REG_IN0_HIGH);
		data->in4_max =
			f71808e_read8(data, F71808E_REG_IN4_HIGH);
		data->in5_max =
			f71808e_read8(data, F71808E_REG_IN5_HIGH);

		/* Get High & boundary temps*/
		for (nr = data->temp_start; nr < 3 + data->temp_start; nr++) {
			data->temp_ovt[nr] = f71808e_read8(data,
					F71808E_REG_TEMP_OVT(nr));
			data->temp_high[nr] = f71808e_read8(data,
					F71808E_REG_TEMP_HIGH(nr));
		}

		reg = f71808e_read8(data, F71808E_REG_PECI);
		if ((reg & 0x03) == 0x01)
			data->temp_type[1] = 6 /* PECI */;
		else if ((reg & 0x03) == 0x02)
			data->temp_type[1] = 5 /* AMDSI */;
		else
			data->temp_type[1] = 2; /* Only supports BJT */

		data->pwm_enable = f71808e_read8(data,
				F71808E_REG_PWM_ENABLE);
		data->pwm_auto_point_hyst[0] =
			f71808e_read8(data, F71808E_REG_FAN_HYST(0));
		data->pwm_auto_point_hyst[1] =
			f71808e_read8(data, F71808E_REG_FAN_HYST(1));

		for (nr = 0; nr < nr_fans; nr++) {
			data->pwm_auto_point_mapping[nr] =
				f71808e_read8(data,
						F71808E_REG_POINT_MAPPING(nr));
		}
		data->last_limits = jiffies;
	}

	/* Update every second */
	if (time_after(jiffies, data->last_updated + HZ) || !data->valid) {
		data->temp_status = f71808e_read8(data,
				F71808E_REG_TEMP_STATUS);
		data->temp_diode_open = f71808e_read8(data,
				F71808E_REG_TEMP_DIODE_OPEN);
		for (nr = data->temp_start; nr < 3 + data->temp_start; nr++)
			data->temp[nr] = f71808e_read_temp(data, nr);

		data->fan_status = f71808e_read8(data,
				F71808E_REG_FAN_STATUS);
		for (nr = 0; nr < nr_fans; nr++) {
			data->fan[nr] = f71808e_read16(data,
					F71808E_REG_FAN(nr));
			data->fan_target[nr] =
				f71808e_read16(data, F71808E_REG_FAN_TARGET(nr));
			data->fan_full_speed[nr] =
				f71808e_read16(data,
						F71808E_REG_FAN_FULL_SPEED(nr));
			data->pwm[nr] = fan_from_reg(f71808e_read8(data, F71808E_REG_PWM(nr)));
			//	data->pwm[nr] = fan_from_reg(f71808e_read16(data, F71808E_REG_FAN_TARGET(nr)));
		}

		for (nr = 0; nr < nr_ins; nr++)
			data->in[nr] = f71808e_read8(data,
					F71808E_REG_IN(nr));
		/* renew LED status */
		if(f71808e_get_attr(SIO_GPIO2_OD) & 0x10) data->pwr_white_led = 0;
		else data->pwr_white_led = 1;

		if(f71808e_get_attr(SIO_GPIO1_OD) & 0x08) data->system_red_led = 0;
		else data->system_red_led = 1;

		if(f71808e_get_attr(SIO_GPIO1_OD) & 0x04) data->system_white_led = 0;
		else data->system_white_led = 1;

		if(f71808e_get_attr(SIO_GPIO1_OD) & 0x02) data->hdd_red_led = 0;
		else data->hdd_red_led = 1;

#if DVT_BOARD
		data->sata_link_led = get_ich9_gpio_attr(2, GP_LVL2);
#else
		data->sata_link_led = 1;
#endif

		data->last_updated = jiffies;
		data->valid = 1;
	}

	mutex_unlock(&data->update_lock);

	return data;
}

/* Sysfs Interface */
static ssize_t show_in(struct device *dev, struct device_attribute *devattr,
		char *buf)
{
	struct f71808e_data *data = f71808e_update_device(dev);
	int nr = to_sensor_dev_attr_2(devattr)->index;

	if(nr == 0)
		return sprintf(buf, "%2d\n", data->in[nr] * 8 * 2);

	if(nr == 4)
		return sprintf(buf, "%d\n", data->in[nr] * 8 * 1009 / 100);

	if(nr == 5)
#if DVT_BOARD
		return sprintf(buf, "%d\n", data->in[nr] * 8 * 30 /10);
#else
	return sprintf(buf, "%d\n", data->in[nr] * 8 * 32 /10);
#endif

	if(nr == 7)
		return sprintf(buf, "%d\n", data->in[nr] * 8 * 2);

	if(nr == 8)
		return sprintf(buf, "%d\n", data->in[nr] * 8 * 2);

	return sprintf(buf, "%d\n", data->in[nr] * 8);
}

static ssize_t show_temp(struct device *dev, struct device_attribute *devattr,
		char *buf)
{
	struct f71808e_data *data = f71808e_update_device(dev);
	int nr = to_sensor_dev_attr_2(devattr)->index;
	int temp;

	temp = data->temp[nr];

	return sprintf(buf, "%d\n", temp);
}

static ssize_t show_fan(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct f71808e_data *data = f71808e_update_device(dev);
	int nr = to_sensor_dev_attr_2(devattr)->index;
	int speed = fan_from_reg(data->fan[nr]);

	if (speed == FAN_MIN_DETECT)
		speed = 0;

	return sprintf(buf, "%d\n", speed);
}

static ssize_t show_fan_full_speed(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct f71808e_data *data = f71808e_update_device(dev);
	int nr = to_sensor_dev_attr_2(devattr)->index;
	int speed = fan_from_reg(data->fan_full_speed[nr]);
	return sprintf(buf, "%d\n", speed);
}

static ssize_t store_fan_full_speed(struct device *dev,
		struct device_attribute *devattr,
		const char *buf, size_t count)
{
	struct f71808e_data *data = dev_get_drvdata(dev);
	int nr = to_sensor_dev_attr_2(devattr)->index;
	long val = simple_strtol(buf, NULL, 10);

	val = SENSORS_LIMIT(val, 23, 1500000);
	val = fan_to_reg(val);

	mutex_lock(&data->update_lock);
	f71808e_write16(data, F71808E_REG_FAN_FULL_SPEED(nr), val);
	data->fan_full_speed[nr] = val;
	mutex_unlock(&data->update_lock);

	return count;
}

static ssize_t show_pwm(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct f71808e_data *data = f71808e_update_device(dev);
	int val, nr = to_sensor_dev_attr_2(devattr)->index;

//	mutex_lock(&data->update_lock);
	if (data->pwm_enable & (1 << (2 * nr)))
		//pwm_enable = 3
		val = fan_from_reg(data->fan[nr]);
	else {
		//pwm_enable = 2
		val = fan_from_reg(data->fan_target[nr]);
	}
//	mutex_unlock(&data->update_lock);
	return sprintf(buf, "%d\n", val);
}

static ssize_t store_pwm(struct device *dev,
		struct device_attribute *devattr, const char *buf,
		size_t count)
{

	struct f71808e_data *data = dev_get_drvdata(dev);
	int nr = to_sensor_dev_attr_2(devattr)->index;
	long val = simple_strtol(buf, NULL, 10);
	val = SENSORS_LIMIT(val, 0, 255);

	mutex_lock(&data->update_lock);
	data->pwm_enable = f71808e_read8(data, F71808E_REG_PWM_ENABLE);

	if (data->pwm_enable & (1 << (2 * nr))) {
		//pwm_enable = 3
		data->pwm[nr] = val;
		f71808e_write8(data, F71808E_REG_PWM(nr), val);
	} else {
		//pwm_enable = 2
		int target, full_speed;
		full_speed = f71808e_read16(data,
				F71808E_REG_FAN_FULL_SPEED(nr));
		target = fan_to_reg(val * fan_from_reg(full_speed) / 255);
		f71808e_write16(data, F71808E_REG_FAN_TARGET(nr), target);
		data->fan_target[nr] = target;
		data->fan_full_speed[nr] = full_speed;
	}
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t show_pwm_enable(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	int result = 0;
	struct f71808e_data *data = f71808e_update_device(dev);
	int nr = to_sensor_dev_attr_2(devattr)->index;

	if((data->pwm_enable >> 2 * nr) & 1) {
		if((data->pwm_enable >> (2 * nr + 1)) & 1) {
			result = 3;
		} else {
			result = 1;
		}
	}else {
		if((data->pwm_enable >> (2 * nr + 1)) & 1) {
			result = 2;
		} else {
			result = 0;
		}
	}
	return sprintf(buf, "%d\n", result);
}

static ssize_t store_pwm_enable(struct device *dev, struct device_attribute
		*devattr, const char *buf, size_t count)
{
	struct f71808e_data *data = dev_get_drvdata(dev);
	int nr = to_sensor_dev_attr_2(devattr)->index;
	long val = simple_strtol(buf, NULL, 10);

	mutex_lock(&data->update_lock);
	data->pwm_enable = f71808e_read8(data, F71808E_REG_PWM_ENABLE);
	switch(val) {
		case 0:
			data->pwm_enable &= ~(2 << (2 * nr));
			data->pwm_enable &= ~(1 << (2 * nr));
			break;
		case 1:
			data->pwm_enable &= ~(2 << (2 * nr));
			data->pwm_enable |=  (1 << (2 * nr));
			break;
		case 2:
			data->pwm_enable |=  (2 << (2 * nr));
			data->pwm_enable &= ~(1 << (2 * nr));
			break;
		case 3:
			data->pwm_enable |=  (2 << (2 * nr));
			data->pwm_enable |=  (1 << (2 * nr));
			break;
		default:
			count = -EINVAL;
			goto leave;
	}
	f71808e_write8(data, F71808E_REG_PWM_ENABLE, data->pwm_enable);
leave:
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t show_name(struct device *dev, struct device_attribute *devattr,
		char *buf)
{
	struct f71808e_data *data = dev_get_drvdata(dev);
	return sprintf(buf, "%s\n", f71808e_names[data->type]);
}

static ssize_t show_led(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct f71808e_data *data = dev_get_drvdata(dev);
	int nr = to_sensor_dev_attr_2(devattr)->index;
	int val = 0;

	switch(nr) {
		case 0://power
			if(f71808e_get_attr(SIO_GPIO2_OD) & 0x10) val = LED_OFF;
			else val = LED_ON;
			if(pwr_white_led_status) val = LED_BLINK;
			break;
		case 1://system_red_led
			if(f71808e_get_attr(SIO_GPIO1_OD) & 0x08) val = LED_OFF;
			else val = LED_ON;
			if(system_red_led_status) val = LED_BLINK;
			break;
		case 2://system_white_led
			if(f71808e_get_attr(SIO_GPIO1_OD) & 0x04) val = LED_OFF;
			else val = LED_ON;
			if(system_white_led_status) val = LED_BLINK;
			break;
		case 3://hdd
			if(f71808e_get_attr(SIO_GPIO1_OD) & 0x02) val = LED_OFF;
			else val = LED_ON;
			if(hdd_red_led_status) val = LED_BLINK;
			break;
		case 4: //brightness
			switch(f71808e_get_attr(SIO_GPIO3_OD)) {
				case 0x01:
					val = 0;
					break;
				case 0x02:
					val = 1;
					break;
#if DVT_BOARD
				case 0x04:
#else //EVT_BOARD
				case 0x03:
#endif
					val = 2;
					break;
			}
			break;
		case 5: //led_blinkrate
			val = data->led_blinkrate ;
			break;
		case 6: //sata_link_led
			val = data->sata_link_led ;
			break;
	}

	return sprintf(buf, "%d\n", val);
}
static void blink_timeout(unsigned long blink_status)
{

	unsigned int status = (unsigned int)blink_status;

	//mutex_lock(&led_mutex_lck);
	if(status == (unsigned int)LED_ON) {
		if(pwr_white_led_status) {
			f71808e_set_attr(SIO_GPIO2_OD, (f71808e_get_attr(SIO_GPIO2_OD) | 0x10));
		}

		if(system_red_led_status) {
			f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x04));
			f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x08));
		} else if(system_white_led_status) {
			f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD)  |0x08));
			f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x04));
		}

		if(hdd_red_led_status)  {
			f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x02));
		}

		status = LED_OFF;
	} else if(status == (unsigned int)LED_OFF) {
		if(pwr_white_led_status) {
			f71808e_set_attr(SIO_GPIO2_OD, (f71808e_get_attr(SIO_GPIO2_OD) & ~0x10));
		}

		if(system_red_led_status) {
			f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x04));
			f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) & ~0x08));
		} else if(system_white_led_status) {
			f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x08));
			f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) & ~0x04));
		}

		if(hdd_red_led_status)  {
			f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) & ~0x02));
		}

		status = LED_ON;
	}

	//mutex_unlock(&led_mutex_lck);
	blink_timer.expires = jiffies + blink_delay;blink_timer.data = (unsigned long)status;
	add_timer(&blink_timer);
}

static ssize_t store_led(struct device *dev, struct device_attribute *devattr,
		const char *buf, size_t count)
{
	struct f71808e_data *data = f71808e_update_device(dev);
	int nr = to_sensor_dev_attr_2(devattr)->index;
	long val = simple_strtol(buf, NULL, 10);

	//mutex_lock(&led_mutex_lck);
	val = SENSORS_LIMIT(val, 0, 3);
	mutex_lock(&data->update_lock);

	switch(nr) {
		case 0: //power_led
			switch(val) {
				case 0 :
					f71808e_set_attr(SIO_GPIO2_OD, (f71808e_get_attr(SIO_GPIO2_OD) | 0x10));
					pwr_white_led_status = 0;
					break;
				case 1:
					f71808e_set_attr(SIO_GPIO2_OD, (f71808e_get_attr(SIO_GPIO2_OD) & ~0x10));
					pwr_white_led_status = 0;
					break;
				case 2:
					pwr_white_led_status = 1;
					break;
			}
			data->pwr_white_led = val;
			break;
		case 1: //system_red_led
			switch(val) {
				case 0:
					system_red_led_status = 0;
					f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x0c));
					break;
				case 1:
						system_red_led_status = 0;
					f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x04));
					f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) & ~0x08));
					data->system_white_led = LED_OFF;
					system_white_led_status = 0;
					system_red_led_status = 0;
					break;
				case 2:
					system_red_led_status = 1;
					break;
			}

			if(val > 0) {
				data->system_white_led = LED_OFF;
				system_white_led_status = 0;
			}
			data->system_red_led = val;
			break;

		case 2: //system_white_led
			switch(val) {
				case 0:
					system_white_led_status = 0;
					f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x0c));
					break;
				case 1:
					system_white_led_status = 0;
					f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x08));
					f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) & ~0x04));
					break;
				case 2:
					system_white_led_status = 1;
					break;
			}

			if(val > 0) {
				data->system_red_led = LED_OFF;
				system_red_led_status = 0;
			}

			data->system_white_led = val;
			break;
		case 3: //hdd_red_led
			switch(val) {
				case 0:
#if DVT_BOARD
					set_ich9_gpio_attr(2, GP_LVL2, 0);
#endif
					f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x02));
					hdd_red_led_status = 0;
					break;
				case 1:
#if DVT_BOARD
					set_ich9_gpio_attr(2, GP_LVL2, 1);
#endif
					f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) & ~0x02));
					hdd_red_led_status = 0;
					break;
				case 2:
#if DVT_BOARD
					set_ich9_gpio_attr(2, GP_LVL2, 1);
#endif
					hdd_red_led_status = 1;
					break;
			}
			data->hdd_red_led = val;
			break;

		case 4: //led_brightness
			switch(val){
				case 0: //low
#if DVT_BOARD
					f71808e_set_attr(SIO_GPIO3_OD, 0x01);
#else
					f71808e_set_attr(SIO_GPIO3_OD, 0x02);
#endif
					break;
				case 1: //med
#if DVT_BOARD
					f71808e_set_attr(SIO_GPIO3_OD, 0x02);
#else
					f71808e_set_attr(SIO_GPIO3_OD, 0x01);
#endif
					break;
				case 2: //high
#if DVT_BOARD
					f71808e_set_attr(SIO_GPIO3_OD, 0x04);
#else
					f71808e_set_attr(SIO_GPIO3_OD, 0x03);
#endif
					break;
			}

			dbg("[%s] GPIO3 OD 0x%02x\n", __func__, f71808e_get_attr(SIO_GPIO3_OD));
			data->led_brightness = val;
			break;
		case 5: //led_blinkrate
			switch(val) {
				case 0:
					data->led_blinkrate = 0;
					blink_delay=HZ*2;
					break;
				case 1:
					data->led_blinkrate = 1;
					blink_delay=HZ;
					break;
				case 2:
					data->led_blinkrate = 2;
					blink_delay=HZ/2;
					break;
			}
			data->led_blinkrate = val;

			break;
		case 6: //sata_link_led
#if DVT_BOARD
			if(val)
				set_ich9_gpio_attr(2, GP_LVL2, 0);
			else
				set_ich9_gpio_attr(2, GP_LVL2, 1);
#endif
			break;
	}
	mutex_unlock(&data->update_lock);
	//mutex_unlock(&led_mutex_lck);
	return count;
}

static int f71808e_create_sysfs_files(struct platform_device *pdev,
		struct sensor_device_attribute_2 *attr, int count)
{
	int err, i;

	for (i = 0; i < count; i++) {
		err = device_create_file(&pdev->dev, &attr[i].dev_attr);
		if (err)
			return err;
	}
	return 0;
}

static int f71808e_probe(struct platform_device *pdev)
{
	struct f71808e_data *data;
	struct f71808e_sio_data *sio_data = pdev->dev.platform_data;
	int err, nr_fans = 2;
	int gpio_config_pin = 0;
	u8 start_reg;

		if(nr_sata == 6)
			nr_fans=3;

	data = kzalloc(sizeof(struct f71808e_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->addr = platform_get_resource(pdev, IORESOURCE_IO, 0)->start;
	data->type = sio_data->type;
	data->temp_start = 1;
	mutex_init(&data->update_lock);
	//mutex_init(&led_mutex_lck);

	platform_set_drvdata(pdev, data);

	start_reg = f71808e_read8(data, F71808E_REG_START);
	if (start_reg & 0x04) {
		dev_warn(&pdev->dev, "Hardware monitor is powered down\n");
		err = -ENODEV;
		goto exit_free;
	}
	if (!(start_reg & 0x03)) {
		dev_warn(&pdev->dev, "Hardware monitoring not activated\n");
		err = -ENODEV;
		goto exit_free;
	}
	dbg("start_reg is  %08x\n", start_reg);

	/* Register sysfs interface files */
	err = device_create_file(&pdev->dev, &dev_attr_name);
	if (err)
		goto exit_unregister_sysfs;
	if (start_reg & 0x01) {
		err = f71808e_create_sysfs_files(pdev,
				f71808e_in_temp_attr,
				ARRAY_SIZE(f71808e_in_temp_attr));
		if (err)
			goto exit_unregister_sysfs;
		/* fall through! */
		if (err)
			goto exit_unregister_sysfs;
	}
	if (start_reg & 0x02) {
		data->pwm_enable =
			f71808e_read8(data, F71808E_REG_PWM_ENABLE);

		err = 0;

		err = f71808e_create_sysfs_files(pdev,
				f71808e_fan_attr,
				ARRAY_SIZE(f71808e_fan_attr));
		if (err)
			goto exit_unregister_sysfs;
		/* fall through! */

		err = f71808e_create_sysfs_files(pdev,
				f71808e_led_attr,
				ARRAY_SIZE(f71808e_led_attr));
		if (err)
			goto exit_unregister_sysfs;

#ifdef FDEBUG
		for (i = 0; i < nr_fans; i++)
			dev_info(&pdev->dev, "Fan: %d is in %s mode\n", i + 1,
					(data->pwm_enable & (1 << 2 * i)) ?
					"duty-cycle" : "RPM");
#endif
	}
	data->hwmon_dev = hwmon_device_register(&pdev->dev);
	//data->hwmon_dev = nasmonitor_device_register(&pdev->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		data->hwmon_dev = NULL;
		goto exit_unregister_sysfs;
	}

	/* initial lcm backlight */
	f71808e_set_attr(SIO_MULTI_FUN_EN2, (f71808e_get_attr(SIO_MULTI_FUN_EN2 ) | 0x10));

#if DVT_BOARD
	gpio_config_pin = 0x07 ;
#else
	gpio_config_pin = 0x03 ;
#endif
	f71808e_set_attr(SIO_MULTI_FUN_EN, gpio_config_pin);
	f71808e_set_attr(SIO_GPIO3_OE, gpio_config_pin);
	f71808e_set_attr(SIO_GPIO3_OD, gpio_config_pin);

	/* initial LED state */

	/*config power led is output */
	f71808e_set_attr(SIO_GPIO2_OE, (f71808e_get_attr(SIO_GPIO2_OE) | 0x10));
	f71808e_set_attr(SIO_GPIO2_OD, (f71808e_get_attr(SIO_GPIO2_OD) & ~0x10));

	/* config system_white_led system_red_led hdd_erroris output */
	f71808e_set_attr(SIO_GPIO1_OE, (f71808e_get_attr(SIO_GPIO1_OE) | 0x0e));
	f71808e_set_attr(SIO_GPIO1_OD, (f71808e_get_attr(SIO_GPIO1_OD) | 0x0e));
#if DVT_BOARD
	/* config  hdd_blue_led and brightness select_c output */
	set_ich9_gpio_attr(2, GPIO_USE_SEL2, 1);
	set_ich9_gpio_attr(2, GP_IO_SEL2, 0);
#if 0
	set_ich9_gpio_attr(2, GP_LVL2, 0);
#endif
#endif
	/* detect 4port / 6port GPIO 56*/
	set_ich9_gpio_attr((1<<(56-32)), GPIO_USE_SEL2, 1);
	set_ich9_gpio_attr((1<<(56-32)), GP_IO_SEL2, 1);

	if(get_ich9_gpio_attr((1<<(56-32)), GP_LVL2)) {
		nr_sata = 6;
	} else {
		nr_sata = 4;
	}
	info("[%s] %d port sata \n", __func__, nr_sata);

	/* add timer */
	init_timer(&blink_timer);
	blink_timer.function=blink_timeout;
	blink_timer.data = (unsigned long)blink_status;
	blink_timer.expires = jiffies + blink_delay;
	add_timer(&blink_timer);
	return 0;

exit_unregister_sysfs:
	f71808e_remove(pdev); /* Will unregister the sysfs files for us */
	return err; /* f71808e_remove() also frees our data */

exit_free:
	//mutex_destroy(&led_mutex_lck);
	mutex_destroy(&data->update_lock);
	kfree(data);
	return err;
}

static int f71808e_remove(struct platform_device *pdev)
{
	struct f71808e_data *data = platform_get_drvdata(pdev);

	del_timer(&blink_timer);
	//mutex_destroy(&led_mutex_lck);
	mutex_destroy(&data->update_lock);
	platform_set_drvdata(pdev, NULL);
	if (data->hwmon_dev)
		hwmon_device_unregister(data->hwmon_dev);

	/* Note we are not looping over all attr arrays we have as the ones
	   below are supersets of the ones skipped. */
	device_remove_file(&pdev->dev, &dev_attr_name);
	kfree(data);

	return 0;
}

/* f71808e_find will be return the ioport addres from device. */
static int __init f71808e_find(int sioaddr, unsigned short *address,
		struct f71808e_sio_data *sio_data)
{
	int err = -ENODEV;
	u16 devid;

	superio_enter(sioaddr);

	devid = superio_inw(sioaddr, SIO_REG_MANID);
	if (devid != SIO_FINTEK_ID) {
		info("Not a Fintek device\n");
		goto exit;
	} else {
		/* will find 0x1934 is FINTEK_ID */
		dbg("find device %08x\n", devid);
	}

	devid = force_id ? force_id : superio_inw(sioaddr, SIO_REG_DEVID);
	switch (devid) {
		case SIO_F71808_ID:
			sio_data->type = f71808e; /* ==0 */
			break;
		default:
			info("Unsupported Fintek device\n");
			goto exit;
	}

	superio_select(sioaddr, SIO_F71808E_LD_HWM);

	if (!(superio_inb(sioaddr, SIO_REG_ENABLE) & 0x01)) {
		err("Device not activated\n");
		goto exit;
	}

	*address = superio_inw(sioaddr, SIO_REG_ADDR);
	if (*address == 0)
	{
		err("Base address not set\n");
		goto exit;
	}
	*address &= ~(REGION_LENGTH - 1);	/* Ignore 3 LSB */

	err = 0;
	dbg("Found %s chip at %#x, revision %d\n",f71808e_names[sio_data->type],
			(unsigned int)*address,	(int)superio_inb(sioaddr, SIO_REG_DEVREV));
exit:
	superio_exit(sioaddr);
	return err;
}

static int __init f71808e_device_add(unsigned short address,
		const struct f71808e_sio_data *sio_data)
{
	struct resource res = {
		.start	= address,
		.end	= address + REGION_LENGTH - 1,
		.flags	= IORESOURCE_IO,
	};
	int err;

	f71808e_pdev = platform_device_alloc(DRVNAME, address);
	if (!f71808e_pdev)
		return -ENOMEM;

	res.name = f71808e_pdev->name;
	err = acpi_check_resource_conflict(&res);
	if (err)
		goto exit_device_put;

	err = platform_device_add_resources(f71808e_pdev, &res, 1);
	if (err) {
		err("Device resource addition failed\n");
		goto exit_device_put;
	}
	err = platform_device_add_data(f71808e_pdev, sio_data,
			sizeof(struct f71808e_sio_data));
	if (err) {
		err("Platform data allocation failed\n");
		goto exit_device_put;
	}
	err = platform_device_add(f71808e_pdev);
	if (err) {
		err("Device addition failed\n");
		goto exit_device_put;
	}

	return 0;

exit_device_put:
	platform_device_put(f71808e_pdev);

	return err;
}

static int __init f71808e_init(void)
{
	int err = -ENODEV;
	unsigned short address;
	struct f71808e_sio_data sio_data;

	memset(&sio_data, 0, sizeof(sio_data));

	if (f71808e_find(0x2e, &address, &sio_data) &&
			f71808e_find(0x4e, &address, &sio_data))
		goto exit;

	/* address is /proc/ioports : 0a00-0a07 */
	dbg("address is  %08x\n", address);

	err = platform_driver_register(&f71808e_driver);
	if (err)
		goto exit;

	err = f71808e_device_add(address, &sio_data);
	if (err)
		goto exit_driver;
	return 0;

exit_driver:
	platform_driver_unregister(&f71808e_driver);
exit:
	return err;
}

static void __exit f71808e_exit(void)
{
	platform_device_unregister(f71808e_pdev);
	platform_driver_unregister(&f71808e_driver);
}

MODULE_DESCRIPTION("F71808E Hardware Monitoring Driver");
MODULE_LICENSE("GPL");

module_init(f71808e_init);
module_exit(f71808e_exit);
