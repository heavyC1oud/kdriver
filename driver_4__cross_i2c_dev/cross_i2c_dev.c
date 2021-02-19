#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>           // miscellaneous character module
#include <linux/kernel.h>
#include <linux/of.h>                   // Device Tree (of - open firmware)
#include <linux/i2c.h>
#include <linux/uaccess.h>


/*
 * &i2c1 {
 * 		pinctrl-names = "default";
 *		pinctrl-0 = <&smilebrd_i2c_pins>;
 *		clock-frequency = <100000>;
 *		status = "okay";
 *
 *		smilebrd_i2c@48 {
 *			compatible = "heavyc1oud,smilebrd_i2c";
 *			reg = <0x48>;
 *		};
 *	};
 */

// private device structure
struct smilebrd_i2c_dev {
    struct i2c_client* client;
    struct miscdevice miscdevice;
    char name[8];
};


// device cb for device node ioctl
static long smilebrd_i2c_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int retval;

    struct smilebrd_i2c_dev* dev = filp->private_data;

    pr_info("Smilebrd i2c ioctl cb is called, cmd=%i arg=%li\n", cmd, arg);

    // recover pointer to private structure to be able to access it members
    dev = container_of(filp->private_data, struct smilebrd_i2c_dev, miscdevice);

    retval = i2c_smbus_write_byte(dev->client, 0x48);
    if(retval < 0) {
        dev_err(&dev->client->dev, "Write error");
    }

    return 0;
}

// device cb for device node write
static ssize_t smilebrd_i2c_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *ppos)
{
    unsigned long val;
    char buf[4];
    struct smilebrd_i2c_dev * dev;

    // recover pointer to private structure to be able to access it members
    dev = container_of(filp->private_data, struct smilebrd_i2c_dev, miscdevice);

    if(copy_from_user(buf, userbuf, count)) {
        pr_info("Failed copy from user");
        return -EFAULT;
    }


    // convert char array to char string
    buf[count-1] = '\0';

    // convert the string to an unsigned long
    if(kstrtoul(buf, 0, &val)) {
        return -EINVAL;
    }

    i2c_smbus_write_byte(dev->client, val);

    return 0;
}


//  file operations structure
static const struct file_operations smilebrd_i2c_fops = {
    .owner = THIS_MODULE,
    .write = smilebrd_i2c_write,
    .unlocked_ioctl = smilebrd_i2c_ioctl,
};


static int smilebrd_i2c_probe(struct i2c_client* client, const struct i2c_device_id *id)
{
    int retval;
    struct smilebrd_i2c_dev* smilebrd_i2c;

    pr_info("smilebrd i2c probe start");

    // allocate mem for private structure
    smilebrd_i2c = devm_kzalloc(&client->dev, sizeof(struct smilebrd_i2c_dev), GFP_KERNEL);

    // store pointer to device structure in the bus
    i2c_set_clientdata(client, smilebrd_i2c);

    // store pointer to I2C client into private structure
    smilebrd_i2c->client = client;

    // initialize misc device
    smilebrd_i2c->miscdevice.name = "smilebrd_i2c";
    smilebrd_i2c->miscdevice.minor = MISC_DYNAMIC_MINOR;
    smilebrd_i2c->miscdevice.fops = &smilebrd_i2c_fops;

    // register miscdevice to kernel
    retval = misc_register(&smilebrd_i2c->miscdevice);
    if(retval != 0) {
        pr_err("could not register smilebrd i2c misc device");
        return retval;
    }

    pr_info("smilebrd i2c probed!\n");
    return 0;
}

static int smilebrd_i2c_remove(struct i2c_client* client)
{
    struct smilebrd_i2c_dev* smilebrd_i2c;

    // get device structure from device bus
    smilebrd_i2c = i2c_get_clientdata(client);

    // unregister miscdevice from kernel
    misc_deregister(&smilebrd_i2c->miscdevice);

    pr_info("smilebrd i2c remove");

    return 0;
}

static const struct of_device_id smilebrd_i2c_of_id[] = {
    { .compatible = "heavyc1oud,smilebrd_i2c", },
    {}
};

static const struct i2c_device_id smilebrd_i2c_i2cbus_id[] ={
    {"smilebrd_i2c", 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, smilebrd_i2c_i2cbus_id);

static struct i2c_driver smilebrd_i2c_driver = {
    .probe = smilebrd_i2c_probe,
    .remove = smilebrd_i2c_remove,
    .id_table = smilebrd_i2c_i2cbus_id,
    .driver = {
        .name = "smilebrd_i2c",
        .of_match_table = of_match_ptr(smilebrd_i2c_of_id),
        .owner = THIS_MODULE,
    },
};

module_i2c_driver(smilebrd_i2c_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HeavyC1oud vheavyC1oud@gmail.com");
MODULE_DESCRIPTION("I2C test driver");
