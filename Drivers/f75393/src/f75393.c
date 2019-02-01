/*
 * f75393.c - driver for the Fintek F75393 hardware monitoring features
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/slab.h>

/* Addresses to scan */
static const unsigned short normal_i2c[] = {0x4c, I2C_CLIENT_END };

enum chips { f75393 };

/* Fintek F75393 registers  */
#define F75393_REG_CONFIG_READ		0x03
#define F75393_REG_CONFIG_WRITE		0x09
#define F75393_REG_STATUS		0x02
#define F75393_REG_CRR_READ		0x04 /*Conversion Rate Register */
#define F75393_REG_CRR_WRITE		0x0A
#define F75393_REG_ONE_SHOT		0x0F
#define F75393_REG_ALERT_Q_TIMEOUT	0x22
#define F75393_REG_STATUS_WITH_ARA	0x24
#define F75393_REG_CHIP_ID		0x5A
#define F75393_REG_VENDOR		0x5D
#define F75393_REG_SMBUS_ADDR		0xFA
#define F75393_REG_FINTEK_VID2		0xFE

#define F75393_REG_VERSION		0x5C
#define F75393_REG_FAN_TIMER		0x60

#define F75393_REG_TEMP1H		0x00
#define F75393_REG_TEMP1L		0x1A

#define F75393_REG_TEMP2H		0x01
#define F75393_REG_TEMP2L		0x10

struct f75393_data {
	unsigned short addr;
	struct device *hwmon_dev;

	const char *name;
	int kind;
	struct mutex update_lock; /* protect register access */
	char valid;
	unsigned long last_updated;	/* In jiffies */
	unsigned long last_limits;	/* In jiffies */

	s8 hddtemp[2];
};

static int f75393_detect(struct i2c_client *client,
		struct i2c_board_info *info);
static int f75393_probe(struct i2c_client *client,
		const struct i2c_device_id *id);
static int f75393_remove(struct i2c_client *client);

static const struct i2c_device_id f75393_id[] = {
	{ "f75393", f75393 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, f75393_id);

static struct i2c_driver f75393_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = "f75393",
	},
	.probe = f75393_probe,
	.remove = f75393_remove,
	.id_table = f75393_id,
	.detect = f75393_detect,
	.address_list = normal_i2c,
};

static inline int f75393_read8(struct i2c_client *client, u8 reg)
{
	return i2c_smbus_read_byte_data(client, reg);
}

/* in most cases, should be called while holding update_lock */
static inline u16 f75393_read16(struct i2c_client *client, u8 reg)
{
	return ((i2c_smbus_read_byte_data(client, reg) << 8)
			| i2c_smbus_read_byte_data(client, reg + 1));
}

static inline void f75393_write8(struct i2c_client *client, u8 reg,
		u8 value)
{
	i2c_smbus_write_byte_data(client, reg, value);
}

static inline void f75393_write16(struct i2c_client *client, u8 reg,
		u16 value)
{
	int err = i2c_smbus_write_byte_data(client, reg, (value << 8));
	if (err)
		return;
	i2c_smbus_write_byte_data(client, reg + 1, (value & 0xFF));
}

static struct f75393_data *f75393_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct f75393_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);

	/* Limit registers cache is refreshed after 60 seconds */
	if (time_after(jiffies, data->last_limits + 60 * HZ)
			|| !data->valid) {
		data->hddtemp[0] =
			f75393_read8(client, F75393_REG_TEMP1H);
		if(data->hddtemp[0] & 0x80)
			data->hddtemp[0] -= 256;

		data->hddtemp[1] =
			f75393_read8(client, F75393_REG_TEMP2H);
		if(data->hddtemp[1] & 0x80)
			data->hddtemp[1] -= 256;
	}

	mutex_unlock(&data->update_lock);
#if 0
	pr_info("[%s] update hddtemp HIGH: %d\n", __func__, data->hddtemp[0]);
	pr_info("[%s] update hddtemp LOW: %d\n", __func__, data->hddtemp[1]);
#endif
	return data;
}

static ssize_t show_hddtemp(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct f75393_data *data = f75393_update_device(dev);
	int nr = to_sensor_dev_attr(attr)->index;
	return sprintf(buf, "%d\n", data->hddtemp[nr]);
}

static SENSOR_DEVICE_ATTR(hddtemp, S_IRUGO, show_hddtemp, NULL, 0);
static SENSOR_DEVICE_ATTR(hddtemp2, S_IRUGO, show_hddtemp, NULL, 1);

static struct attribute *f75393_attributes[] = {
	&sensor_dev_attr_hddtemp.dev_attr.attr,
	&sensor_dev_attr_hddtemp2.dev_attr.attr,
	NULL
};

static const struct attribute_group f75393_group = {
	.attrs = f75393_attributes,
};
static int f75393_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct f75393_data *data;
	int err;

	pr_info("[%s] : ...\n", __func__);

	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	if (!(data = kzalloc(sizeof(struct f75393_data), GFP_KERNEL)))
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);
	data->kind = id->driver_data;

	if ((err = sysfs_create_group(&client->dev.kobj, &f75393_group)))
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);

	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &f75393_group);
exit_free:
	kfree(data);
	i2c_set_clientdata(client, NULL);
	return err;

}

static int f75393_remove(struct i2c_client *client)
{
	struct f75393_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &f75393_group);
	kfree(data);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static int f75393_detect(struct i2c_client *client,
		struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	u16 vendid, chipid;
	u8 version;
	const char *name;

	pr_info("[%s] : ...\n", __func__);
	vendid = f75393_read16(client, F75393_REG_VENDOR);
	chipid = f75393_read16(client, F75393_REG_CHIP_ID);
	if (chipid == 0x0707 && vendid == 0x1934)
		name = "f75393";
	else {
		pr_info("[%s] : can't fined Fintek F75393. \n", __func__);
		return -ENODEV;
	}
	version = f75393_read8(client, F75393_REG_VERSION);
	dev_info(&adapter->dev, "found %s version: %02X\n", name, version);
	dev_info(&adapter->dev, "found %s \n", name);
	strlcpy(info->type, name, I2C_NAME_SIZE);

	return 0;
}

static int __init sensors_f75393_init(void)
{
	return i2c_add_driver(&f75393_driver);
}

static void __exit sensors_f75393_exit(void)
{
	i2c_del_driver(&f75393_driver);
}

MODULE_AUTHOR("Riku Voipio");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("F75393 driver");

module_init(sensors_f75393_init);
module_exit(sensors_f75393_exit);
