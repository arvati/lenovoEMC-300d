/*
 * f75383.c - driver for the Fintek F75383/F75384/F75393 temperature sensor
 *
 * Copyright (C) 2004-2006  Jean Delvare <khali@linux-fr.org>
 * Brian Beardall <brian@rapsure.net>
 * Based on the lm90 and lm63 driver.
 *
 * The F75383 is a sensor chip made by Fintek. It measures
 * two temperatures (its own and one external one). Complete datasheet can be
 * obtained from Fintek's website at:
 *   http://www.fintek.com.tw/files/productfiles/F75383_384_V030P.pdf
 *
 * The F75383 is a temperature sensors with one remote, and one local sensor.
 * the sensors behave the same.
 * The F75393 is identical to the F75383, with the addition of beta
 * compensation, which is not supported by this driver.
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
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon-sysfs.h>
#include <linux/hwmon.h>
#include <linux/err.h>
#include <linux/mutex.h>

/*
 * Addresses to scan
 * Address is fully defined internally and cannot be changed.
 */

static unsigned short normal_i2c[] = { 0x4c, 0x4d, I2C_CLIENT_END };

/*
 * Supported chips
 */

enum chips { f75383, f75393 };

/*
 * The F75383 registers
 */


#define F75383_REG_CONFIG_READ			0x03
#define F75383_REG_CONFIG_WRITE			0x09
#define F75383_STATUS				0x02
#define F75383_REG_CRR_READ			0x04 /*Conversion Rate Register */
#define F75383_REG_CRR_WRITE			0x0A
#define F75383_ONE_SHOT				0x0F
#define F75383_ALERT_TIMEOUT			0x22
#define F75383_STATUS_ARA			0x24

#define F75383_CHIP_ID1				0x5A /* Read to know that the driver is for it */
#define F75383_CHIP_ID2				0x5B

#define F75383_VENDOR_ID1_LOC1			0x5D
#define F75383_VENDOR_ID1_LOC2			0x5E
#define F75383_VENDOR_ID2			0xFE /* might be different */
/* #define F75383_VALUE_RAM			0x10 - 0x2F */
#define F75383_VERSION				0x5C
#define F75383_FAN_TIMER			0x60
#define F75383_VT1_HIGH				0x00
#define F75383_VT1_LOW				0x1A
#define F75383_VT2_HIGH				0x01
#define F75383_VT2_LOW				0x10
#define F75383_VT1_HIGH_LIMIT_HIGH_READ		0x05
#define F75383_VT1_HIGH_LIMIT_LOW_BYTE		0x1B
#define F75383_VT1_HIGH_LIMIT_HIGH_WRITE	0x0B
#define F75383_VT1_LOW_LIMIT_HIGH_READ		0x06
#define F75383_VT1_LOW_LIMIT_LOW_BYTE		0x1C
#define F75383_VT1_LOW_LIMIT_HIGH_WRITE		0x0C
#define F75383_VT2_HIGH_LIMIT_HIGH_READ		0x07
#define F75383_VT2_HIGH_LIMIT_LOW_BYTE		0x13
#define F75383_VT2_HIGH_LIMIT_HIGH_WRITE	0x0D
#define F75383_VT2_LOW_LIMIT_HIGH_READ		0x08
#define F75383_VT2_LOW_LIMIT_LOW_BYTE		0x14
#define F75383_VT2_LOW_LIMIT_HIGH_WRITE		0x0E
#define F75383_VT1_THERM_LIMIT			0x20
#define F75383_VT1_THERM_HYST			0x21
#define F75383_VT2_THERM_LIMIT			0x19
#define F75383_VT2_THERM_HYST			0x23

#define F75383_MFR_ID				0x1934
#define F75383_ID				0x0303
#define F75393_ID				0x0707



/* For the F75383 Both temperature sensors are read the same. VT1 is local,
 * and VT2 is external, and on ECS mainboards is
 * connected to the CPU for the temperature. There is no FAN circutry, and there
 * is only one method for calculating the temperatures. The High Byte is just a value
 * from 0 - 144 degrees C. For each increase in the number there is a corresponding change in
 * the temperature. The Low Byte uses the upper nibble for
 *temperature changes of .125 degrees C.
 */

#define TEMP_FROM_REG(reg)	((reg) / 32 * 125)
#define TEMP_TO_REG(val)	((val) <= 0 ? 0x0000 : \
				 (val) >= 140875 ? 0x8CE0 : \
				 (val) / 125 * 32)
#define HYST_FROM_REG(reg)	((reg) * 1000)
#define HYST_TO_REG(val)	((val) <= 0 ? 0 : \
				 (val) >= 140000 ? 140 : \
				 (val) / 1000)

/*
 * Functions declaration
 */

static struct f75383_data *f75383_update_device(struct device *dev);

static int f75383_detect(struct i2c_client *client, struct i2c_board_info *info);
static int f75383_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int f75383_remove(struct i2c_client *client);

static const struct i2c_device_id f75383_id[] = {
	{ "f75383", f75383 },
	{ "f75393", f75393 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, f75383_id);

/*
 * Driver data (common to all clients)
 */

static struct i2c_driver f75383_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "f75383",
	},
	.probe		= f75383_probe,
	.remove		= f75383_remove,
	.id_table	= f75383_id,
	.detect		= f75383_detect,
	.address_list	= normal_i2c,
};

/*
 * Client data (each client gets its own)
 */

struct f75383_data {
	struct i2c_client *client;
	struct device *hwmon_dev;
	struct mutex update_lock;
	int kind;
	char valid; /* zero until following fields are valid */
	unsigned long last_updated; /* in jiffies */

	/* registers values */
	u8 config;

	u16 temp1[3];	/* 0: VT1 input
			   1: VT1 low limit
			   2: VT1 high limit */
	u16 temp2[3];   /*
			   0: VT2 input
			   1: VT2 low limit
			   2: VT2 high limit */
	u8 temp1_hyst;
	u8 temp2_hyst;
	u8 temp1_crit;
	u8 temp2_crit;
	u8 alarms;
};

/*
 * Sysfs callback functions and files
 */


static ssize_t show_temp1(struct device *dev, struct device_attribute *devattr,
			   char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct f75383_data *data = f75383_update_device(dev);
	return sprintf(buf, "%d\n", TEMP_FROM_REG(data->temp1[attr->index]));
}

static ssize_t show_temp2(struct device *dev, struct device_attribute *devattr,
			   char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct f75383_data *data = f75383_update_device(dev);
	return sprintf(buf, "%d\n", TEMP_FROM_REG(data->temp2[attr->index]));
}

static ssize_t set_temp1(struct device *dev, struct device_attribute *devattr,
			  const char *buf, size_t count)
{
	static const u8 reg[4]= {
		F75383_VT1_LOW_LIMIT_HIGH_WRITE,
		F75383_VT1_LOW_LIMIT_LOW_BYTE,
		F75383_VT1_HIGH_LIMIT_HIGH_WRITE,
		F75383_VT1_HIGH_LIMIT_LOW_BYTE,
	};

	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct f75383_data *data = i2c_get_clientdata(client);
	long val = simple_strtol(buf, NULL, 10);
	int nr = attr->index;

	mutex_lock(&data->update_lock);
	data->temp1[nr] = TEMP_TO_REG(val);
	i2c_smbus_write_byte_data(client, reg[(nr - 1) * 2],
				  data->temp1[nr] >> 8);
	i2c_smbus_write_byte_data(client, reg[(nr - 1) * 2 + 1],
				  data->temp1[nr] & 0xff);
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t set_temp2(struct device *dev, struct device_attribute *devattr,
			  const char *buf, size_t count)
{
	static const u8 reg[4]= {
		F75383_VT2_LOW_LIMIT_HIGH_WRITE,
		F75383_VT2_LOW_LIMIT_LOW_BYTE,
		F75383_VT2_HIGH_LIMIT_HIGH_WRITE,
		F75383_VT2_HIGH_LIMIT_LOW_BYTE,
	};

	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct f75383_data *data = i2c_get_clientdata(client);
	long val = simple_strtol(buf, NULL, 10);
	int nr = attr->index;

	mutex_lock(&data->update_lock);
	data->temp2[nr] = TEMP_TO_REG(val);
	i2c_smbus_write_byte_data(client, reg[(nr - 1) * 2],
				  data->temp2[nr] >> 8);
	i2c_smbus_write_byte_data(client, reg[(nr - 1) * 2 + 1],
				  data->temp2[nr] & 0xff);
	mutex_unlock(&data->update_lock);
	return count;
}

/* Show the THERM limit temperature. This is an eight bit value */

static ssize_t show_temp1_crit(struct device *dev, struct device_attribute *dummy,
				    char *buf)
{
	struct f75383_data *data = f75383_update_device(dev);
	return sprintf(buf, "%d\n", HYST_FROM_REG(data->temp1_crit));
}

static ssize_t show_temp2_crit(struct device *dev, struct device_attribute *dummy,
				    char *buf)
{
	struct f75383_data *data = f75383_update_device(dev);
	return sprintf(buf, "%d\n", HYST_FROM_REG(data->temp2_crit));
}


/* Set the THERM limit, I named the functions with crit because of the function
 * in libsensors
 */
static ssize_t set_temp1_crit(struct device *dev, struct device_attribute *dummy,
				   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct f75383_data *data = i2c_get_clientdata(client);
	long crit = simple_strtol(buf, NULL, 10);

	mutex_lock(&data->update_lock);
	i2c_smbus_write_byte_data(client, F75383_VT1_THERM_LIMIT,
				  HYST_TO_REG(crit));
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t set_temp2_crit(struct device *dev, struct device_attribute *dummy,
				   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct f75383_data *data = i2c_get_clientdata(client);
	long crit = simple_strtol(buf, NULL, 10);

	mutex_lock(&data->update_lock);
	i2c_smbus_write_byte_data(client, F75383_VT2_THERM_LIMIT,
				  HYST_TO_REG(crit));
	mutex_unlock(&data->update_lock);
	return count;
}

/* Hysteresis register holds a relative value, while we want to present
   an absolute to user-space */
static ssize_t show_temp1_hyst(struct device *dev, struct device_attribute *dummy,
				    char *buf)
{
	struct f75383_data *data = f75383_update_device(dev);
	return sprintf(buf, "%d\n", HYST_FROM_REG(data->temp1_hyst));
}

static ssize_t show_temp2_hyst(struct device *dev, struct device_attribute *dummy,
				    char *buf)
{
	struct f75383_data *data = f75383_update_device(dev);
	return sprintf(buf, "%d\n", HYST_FROM_REG(data->temp2_hyst));
}

/* And now the other way around, user-space provides an absolute
   hysteresis value and we have to store a relative one */
static ssize_t set_temp1_hyst(struct device *dev, struct device_attribute *dummy,
				   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct f75383_data *data = i2c_get_clientdata(client);
	long hyst = simple_strtol(buf, NULL, 10);

	mutex_lock(&data->update_lock);
	i2c_smbus_write_byte_data(client, F75383_VT1_THERM_HYST,
				  HYST_TO_REG(hyst));
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t set_temp2_hyst(struct device *dev, struct device_attribute *dummy,
				   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct f75383_data *data = i2c_get_clientdata(client);
	long hyst = simple_strtol(buf, NULL, 10);

	mutex_lock(&data->update_lock);
	i2c_smbus_write_byte_data(client, F75383_VT2_THERM_HYST,
				  HYST_TO_REG(hyst));
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t show_alarms(struct device *dev, struct device_attribute *dummy,
			   char *buf)
{
	struct f75383_data *data = f75383_update_device(dev);
	return sprintf(buf, "%u\n", data->alarms);
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, show_temp1, NULL, 0);
static SENSOR_DEVICE_ATTR(temp1_min, S_IWUSR | S_IRUGO, show_temp1,
	set_temp1, 1);
static SENSOR_DEVICE_ATTR(temp1_max, S_IWUSR | S_IRUGO, show_temp1,
	set_temp1, 2);
static DEVICE_ATTR(temp1_crit, S_IWUSR | S_IRUGO, show_temp1_crit,
	set_temp1_crit);
static DEVICE_ATTR(temp1_hyst, S_IWUSR | S_IRUGO, show_temp1_hyst,
	set_temp1_hyst);

static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO, show_temp2, NULL, 0);
static SENSOR_DEVICE_ATTR(temp2_min, S_IWUSR | S_IRUGO, show_temp2,
	set_temp2, 1);
static SENSOR_DEVICE_ATTR(temp2_max, S_IWUSR | S_IRUGO, show_temp2,
	set_temp2, 2);
static DEVICE_ATTR(temp2_crit, S_IWUSR | S_IRUGO, show_temp2_crit,
	set_temp2_crit);
static DEVICE_ATTR(temp2_hyst, S_IWUSR | S_IRUGO, show_temp2_hyst,
	set_temp2_hyst);

static DEVICE_ATTR(alarms, S_IRUGO, show_alarms, NULL);

static struct attribute *f75383_attributes[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp1_min.dev_attr.attr,
	&sensor_dev_attr_temp2_min.dev_attr.attr,
	&sensor_dev_attr_temp1_max.dev_attr.attr,
	&sensor_dev_attr_temp2_max.dev_attr.attr,
	&dev_attr_temp1_crit.attr,
	&dev_attr_temp2_crit.attr,
	&dev_attr_temp1_hyst.attr,
	&dev_attr_temp2_hyst.attr,
	&dev_attr_alarms.attr,
	NULL
};

static const struct attribute_group f75383_group = {
	.attrs = f75383_attributes,
};

/*
 * Real code
 */

/*
 * The following function does more than just detection. If detection
 * succeeds, it also registers the new chip.
 */
static int f75383_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	u16 man_id, chip_id;
	u8 version;
	const char *name = NULL;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

	/* detection and identification */
	man_id = (i2c_smbus_read_byte_data(client,
                        F75383_VENDOR_ID1_LOC1) << 8) |
                        i2c_smbus_read_byte_data(client,
                        F75383_VENDOR_ID1_LOC2);
	chip_id = (i2c_smbus_read_byte_data(client,
                        F75383_CHIP_ID1) << 8) |
                        i2c_smbus_read_byte_data(client,
                        F75383_CHIP_ID2);
	if (man_id < 0 || chip_id < 0)
		return -ENODEV;

	if (man_id != F75383_MFR_ID) {
		pr_info("[%s] : can't fined Fintek F75383/F75384/F75393. \n", __func__);
		return -ENODEV;
	}
	else if(chip_id == F75383_ID) {
		name = "f75383";
	}
	else if(chip_id == F75393_ID) {
		name = "f75393";
	}
	else {
		dev_dbg(&adapter->dev, "Unsupported chip id"
			"(chip_id=0x%04X).\n", chip_id);
	}
	version = i2c_smbus_read_byte_data(client, F75383_VERSION);
	dev_info(&adapter->dev, "found %s version: %02X\n", name, version);
	strlcpy(info->type, name, I2C_NAME_SIZE);

	return 0;
}

static int f75383_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct f75383_data *data;
	int err;

	data = devm_kzalloc(dev, sizeof(struct f75383_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	/* Set the device type */
	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);
	data->kind = id->driver_data;

	/* register sysfs nodes */
	if ((err = sysfs_create_group(&client->dev.kobj, &f75383_group)))
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);

	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &f75383_group);
exit_free:
	kfree(data);
	i2c_set_clientdata(client, NULL);
	return err;
}

static int f75383_remove(struct i2c_client *client)
{
	struct f75383_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &f75383_group);
	kfree(data);
	i2c_set_clientdata(client, NULL);

	return 0;

}


static struct f75383_data *f75383_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct f75383_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ) || !data->valid) {

		/* order matters for temp2_input */ // Modify this
		data->temp1[0] = i2c_smbus_read_byte_data(client,
				  F75383_VT1_HIGH) << 8;
		data->temp1[0] |= i2c_smbus_read_byte_data(client,
				   F75383_VT1_LOW);
		data->temp1[1] = (i2c_smbus_read_byte_data(client,
				  F75383_VT1_LOW_LIMIT_HIGH_READ) << 8)
				| i2c_smbus_read_byte_data(client,
				  F75383_VT1_LOW_LIMIT_LOW_BYTE);
		data->temp1[2] = (i2c_smbus_read_byte_data(client,
				  F75383_VT1_HIGH_LIMIT_HIGH_READ) << 8)
				| i2c_smbus_read_byte_data(client,
				  F75383_VT1_HIGH_LIMIT_LOW_BYTE);
		data->temp1_hyst = i2c_smbus_read_byte_data(client,
					F75383_VT1_THERM_HYST);
		data->temp1_crit = i2c_smbus_read_byte_data(client,
					F75383_VT1_THERM_LIMIT);

		data->temp2[0] = i2c_smbus_read_byte_data(client,
				  F75383_VT2_HIGH) << 8;
		data->temp2[0] |= i2c_smbus_read_byte_data(client,
				   F75383_VT2_LOW);
		data->temp2[1] = (i2c_smbus_read_byte_data(client,
				  F75383_VT2_LOW_LIMIT_HIGH_READ) << 8)
				| i2c_smbus_read_byte_data(client,
				  F75383_VT2_LOW_LIMIT_LOW_BYTE);
		data->temp2[2] = (i2c_smbus_read_byte_data(client,
				  F75383_VT2_HIGH_LIMIT_HIGH_READ) << 8)
				| i2c_smbus_read_byte_data(client,
				  F75383_VT2_HIGH_LIMIT_LOW_BYTE);
		data->temp2_hyst = i2c_smbus_read_byte_data(client,
					F75383_VT2_THERM_HYST);
		data->temp2_crit = i2c_smbus_read_byte_data(client,
					F75383_VT2_THERM_LIMIT);

		data->alarms = i2c_smbus_read_byte_data(client,
			       F75383_STATUS) & 0x7F;

		data->last_updated = jiffies;
		data->valid = 1;
	}

	mutex_unlock(&data->update_lock);

	return data;
}

static int __init sensors_f75383_init(void)
{
	return i2c_add_driver(&f75383_driver);
}

static void __exit sensors_f75383_exit(void)
{
	i2c_del_driver(&f75383_driver);
}

MODULE_AUTHOR("Jean Delvare <khali@linux-fr.org> Brian Beardall <brian@rapsure.net>");
MODULE_DESCRIPTION("F75383 driver");
MODULE_LICENSE("GPL");

module_init(sensors_f75383_init);
module_exit(sensors_f75383_exit);
