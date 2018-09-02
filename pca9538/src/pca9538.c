#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include "ich9-gpio.h"
#include <acpi/acpi.h>

/* debug define */
#define PEGA_HDDLINK_DEBUG 1
#if PEGA_HDDLINK_DEBUG
#define dbg(fmt, ...) printk(KERN_DEBUG "[%s] : " fmt, __func__, ##__VA_ARGS__)
#define err(fmt, ...) printk(KERN_ERR "[%s] : " fmt, __func__, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#define err(fmt, ...)
#endif
#define info(fmt, ...) printk(KERN_INFO "[%s] : " fmt, __func__, ##__VA_ARGS__)

#define DRV_NAME	 	"hdd-link"
#define DRV_VERSION	 	"0.2"

#define PCA9538_INPUT          0
#define PCA9538_OUTPUT         1
#define PCA9538_INVERT         2
#define PCA9538_DIRECTION      3

#define PCA9538_GPIOS	       0x00FF
#define PCA9538_INT	       0x0100

static const unsigned short normal_i2c[] = {0x70, I2C_CLIENT_END };

static ssize_t show_hdd_status(struct device *dev, struct device_attribute *devattr, char *buf);
static ssize_t show_hdd_info(struct device *dev, struct device_attribute *devattr, char *buf);
static int pca9538_detect(struct i2c_client *client, struct i2c_board_info *info);
static int pca9538_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int pca9538_remove(struct i2c_client *client);

static const struct i2c_device_id pca9538_id[] = {
	{ "pca9538", 8 | PCA9538_INT },
	{ }
};

MODULE_DEVICE_TABLE(i2c, pca9538_id);

static int nr_sata = 0;

struct pca9538_data {
	struct device *hwmon_dev;

	unsigned gpio_start;
	uint16_t reg_output;
	uint16_t reg_direction;

	struct mutex update_lock; /* protect register access */

	const char *const *names;
	int kind;
	struct i2c_client *client;

	u8 	hdd_info;
	u8 	hdd[6];
	unsigned long last_limits;	/* In jiffies */

};

static struct i2c_driver pca9538_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = "pca9538",
	},
	.probe = pca9538_probe,
	.remove = pca9538_remove,
	.id_table = pca9538_id,
	.detect = pca9538_detect,
	.address_list = normal_i2c,
};

static inline int i2c_read8(struct i2c_client *client, u8 reg)
{
	return i2c_smbus_read_byte_data(client, reg);
}

static inline u16 i2c_read16(struct i2c_client *client, u8 reg)
{
	return ((i2c_smbus_read_byte_data(client, reg) << 8)
			| i2c_smbus_read_byte_data(client, reg + 1));
}

static inline void i2c_write8(struct i2c_client *client, u8 reg,
		u8 value)
{
	i2c_smbus_write_byte_data(client, reg, value);
}

static inline void i2c_write16(struct i2c_client *client, u8 reg,
		u16 value)
{
	int err = i2c_smbus_write_byte_data(client, reg, (value << 8));
	if (err)
		return;
}

static struct sensor_device_attribute_2 pca9538_hdd_status_attr[] = {
	SENSOR_ATTR_2(hdd_info, S_IRUGO, show_hdd_info, NULL, 0, 0),
	SENSOR_ATTR_2(hdd1_link, S_IRUGO, show_hdd_status, NULL, 0, 0),
	SENSOR_ATTR_2(hdd2_link, S_IRUGO, show_hdd_status, NULL, 0, 1),
	SENSOR_ATTR_2(hdd3_link, S_IRUGO, show_hdd_status, NULL, 0, 2),
	SENSOR_ATTR_2(hdd4_link, S_IRUGO, show_hdd_status, NULL, 0, 3),
	SENSOR_ATTR_2(hdd5_link, S_IRUGO, show_hdd_status, NULL, 0, 4),
	SENSOR_ATTR_2(hdd6_link, S_IRUGO, show_hdd_status, NULL, 0, 5),
};

static int pca9538_create_sysfs_files(struct device *dev,
		struct sensor_device_attribute_2 *attr, int count)
{
	int err, i;

	for (i = 0; i < count+1; i++) {
		err = device_create_file(dev, &attr[i].dev_attr);
		if (err)
			return err;
	}
	return 0;
}

static struct pca9538_data *pca9538_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct pca9538_data *data = i2c_get_clientdata(client);
	int nr;


	mutex_lock(&data->update_lock);
		data->hdd_info = ~i2c_read8(client, 0x0);
		data->last_limits = jiffies;

	for(nr = 0; nr < nr_sata; nr++)
		data->hdd[nr] = ((i2c_read8(client, 0x0) >> nr) & 0x01);

	mutex_unlock(&data->update_lock);
	dbg("hdd_info : %d\n", data->hdd_info);

	return data;
}

static ssize_t show_hdd_status(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct pca9538_data *data = pca9538_update_device(dev);
	int nr = to_sensor_dev_attr_2(devattr)->index;
	int sta;

	sta = data->hdd[nr];
	if(sta)
		sta = 0;
	else
		sta = 1 ;

	return sprintf(buf, "%d\n", sta);
}

static ssize_t show_hdd_info(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct pca9538_data *data = pca9538_update_device(dev);
	return sprintf(buf, "%d\n", data->hdd_info);

}

static int pca9538_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct pca9538_data *data;
	int err;


	if(get_ich9_gpio_attr((1<<(56-32)), GP_LVL2)) {
		nr_sata = 6;
	} else {
		nr_sata = 4;
	}
	dbg("detect %d slot.\n", nr_sata);

	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	if (!(data = kzalloc(sizeof(struct pca9538_data), GFP_KERNEL)))
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);
	data->kind = id->driver_data;
	err = pca9538_create_sysfs_files(&client->dev,
			pca9538_hdd_status_attr,
			nr_sata);

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if(IS_ERR(data->hwmon_dev)) 
	{
		err = PTR_ERR(data->hwmon_dev);
		data->hwmon_dev = NULL;
		goto exit_free;
	}

	if(i2c_read8(client, 0x0) == 0xff) {
		dbg("no hdd install. \n");
	} else {
		dbg("hdd info : 0x%x\n", i2c_read8(client, 0x0));
	}

	return 0;

exit_free:
	kfree(data);
	i2c_set_clientdata(client, NULL);
	return err;

}

static int pca9538_remove(struct i2c_client *client)
{
	struct pca9538_data *data = i2c_get_clientdata(client);


	if(data->hwmon_dev)
		hwmon_device_unregister(data->hwmon_dev);

	kfree(data);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static int pca9538_detect(struct i2c_client *client,
		struct i2c_board_info *info)
{
	const char *name;

	name = "pca9538";
	strlcpy(info->type, name, I2C_NAME_SIZE);

	return 0;
}

static int __init sensors_pca9538_init(void)
{
	return i2c_add_driver(&pca9538_driver);

}

static void __exit sensors_pca9538_exit(void)
{
	i2c_del_driver(&pca9538_driver);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PCA9538 driver");

module_init(sensors_pca9538_init);
module_exit(sensors_pca9538_exit);
