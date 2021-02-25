#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>           // miscellaneous character module
#include <linux/kernel.h>
#include <linux/of.h>                   // Device Tree (of - open firmware)
#include <linux/i2c.h>                  // i2c devices
#include <linux/uaccess.h>
#include <linux/platform_device.h>      // platform devices
#include <linux/gpio/consumer.h>        // GPIO descriptor
#include <linux/interrupt.h>            // IRQ
#include <linux/of.h>                   // Device Tree (of - open firmware)

#include <linux/wait.h>



// private smilebrd structure
struct smilebrd_dev {
    struct i2c_client* i2c_dev;
    struct platform_device* gpio_dev;
    struct gpio_desc* button;
    struct gpio_desc* led;
    unsigned int irq;
    unsigned int irq_f;

    struct miscdevice miscdevice;
    char name[8];
};

static struct smilebrd_dev* smilebrd;

static DECLARE_WAIT_QUEUE_HEAD(wq);


// device cb for device node ioctl
static long smilebrd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    pr_info("Smilebrd ioctl cb is called, cmd=%i arg=%li\n", cmd, arg);

    return 0;
}

// device cb for device node read
static ssize_t smilebrd_read(struct file *filp, char __user *userbuf, size_t count, loff_t *ppos)
{
    static unsigned int data = 0;

    wait_event_interruptible(wq, smilebrd->irq_f != 0);

    smilebrd->irq_f = 0;

    if(copy_to_user(userbuf, &data, sizeof(unsigned int)) != 0) {
        return -EIO;
    }

    data++;

    return sizeof(unsigned int);
}

// device cb for device node write
static ssize_t smilebrd_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *ppos)
{

    return 0;
}

static irqreturn_t smilebrd_gpio_irq_handler(int irq, void* dev_id)
{
    pr_info("wow! IRQ\n");

    // change led state
    gpiod_set_value(smilebrd->led, !(gpiod_get_value(smilebrd->led)));

    // send i2c data
    i2c_smbus_write_byte(smilebrd->i2c_dev, 0x48);

    // set irq flag
    smilebrd->irq_f = 1;
    wake_up_interruptible(&wq);

    return IRQ_HANDLED;
}

//  file operations structure
static const struct file_operations smilebrd_fops = {
    .owner = THIS_MODULE,
    .write = smilebrd_write,
    .read = smilebrd_read,
    .unlocked_ioctl = smilebrd_ioctl,
};


//  GPIO driver
static int smilebrd_gpio_probe(struct platform_device* pdev)
{
    int retval;
    smilebrd->gpio_dev = pdev;

    smilebrd->button = gpiod_get(&smilebrd->gpio_dev->dev, "button", 0);
    gpiod_direction_input(smilebrd->button);

    smilebrd->led = gpiod_get(&smilebrd->gpio_dev->dev, "led", 0);
    gpiod_direction_output(smilebrd->led, 0);

    retval = gpiod_set_debounce(smilebrd->button, 1000 * 5);        // time unit 1 us, 1000 us * 5 = 5 ms, MAX = 7 ms
    if(retval != 0) {
        pr_err("could not set debounce interval\n");
    }

    smilebrd->irq = gpiod_to_irq(smilebrd->button);
    retval = request_threaded_irq(smilebrd->irq, NULL, smilebrd_gpio_irq_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "smilepd_drv", NULL);
    if(retval != 0) {
        pr_err("could not register smilebrd irq handler\n");
        return retval;
    }

    pr_info("smilebrd gpio probed!\n");
    return 0;
}

static int smilebrd_gpio_remove(struct platform_device* pdev)
{
    free_irq(smilebrd->irq, NULL);
    gpiod_put(smilebrd->button);
    gpiod_put(smilebrd->led);

    pr_info("smilebrd gpio remove\n");

    return 0;
}


static const struct of_device_id smilebrd_gpio_dt_ids[] = {
    { .compatible = "heavyc1oud,smilebrd_gpio", },
    {}
};
MODULE_DEVICE_TABLE(of, smilebrd_gpio_dt_ids);  // ПРОВЕРИТЬ НУЖНО ЛИ ЭТО

static struct platform_driver smilebrd_gpio_drv = {
    .probe = smilebrd_gpio_probe,
    .remove = smilebrd_gpio_remove,
    .driver = {
        .name = "smilebrd_gpio",
        .of_match_table = of_match_ptr(smilebrd_gpio_dt_ids),
        .owner = THIS_MODULE,
    },
};

// I2C driver
static int smilebrd_i2c_probe(struct i2c_client* client, const struct i2c_device_id *id)
{
    // int retval;

    // store pointer to device structure in the bus
    i2c_set_clientdata(client, smilebrd);      // ПРОВЕРИТЬ НУЖНО ЛИ ЭТО

    // store pointer to I2C client into private structure
    smilebrd->i2c_dev = client;

    pr_info("smilebrd i2c probed!\n");

    return 0;
}

static int smilebrd_i2c_remove(struct i2c_client* client)
{
    // get device structure from device bus
    smilebrd = i2c_get_clientdata(client);      // ПРОВЕРИТЬ НУЖНО ЛИ ЭТО

    pr_info("smilebrd i2c remove\n");

    return 0;
}

static const struct of_device_id smilebrd_i2c_dt_ids[] = {
    { .compatible = "heavyc1oud,smilebrd_i2c", },
    {}
};

static const struct i2c_device_id smilebrd_i2c_i2cbus_id[] ={
    {"smilebrd_i2c", 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, smilebrd_i2c_i2cbus_id);

static struct i2c_driver smilebrd_i2c_drv = {
    .probe = smilebrd_i2c_probe,
    .remove = smilebrd_i2c_remove,
    .id_table = smilebrd_i2c_i2cbus_id,
    .driver = {
        .name = "smilebrd_i2c",
        .of_match_table = of_match_ptr(smilebrd_i2c_dt_ids),
        .owner = THIS_MODULE,
    },
};



static int __init smilebrd_init(void)
{
    int retval;

    // allocate mem for private structure
    smilebrd = kzalloc(sizeof(struct smilebrd_dev), GFP_KERNEL);

    // initialize misc device
    smilebrd->miscdevice.name = "smilebrd";
    smilebrd->miscdevice.minor = MISC_DYNAMIC_MINOR;
    smilebrd->miscdevice.fops = &smilebrd_fops;

    // register miscdevice
    if(misc_register(&smilebrd->miscdevice)) {
        pr_err("could not register smilebrd misc device\n");
        return EINVAL;
    }

    // register smilebrd gpio driver
    retval = platform_driver_register(&smilebrd_gpio_drv);

    // register smilebrd i2c driver
    retval = i2c_register_driver(THIS_MODULE, &smilebrd_i2c_drv);

    return 0;
}

static void __exit smilebrd_exit(void)
{
    // deregister miscdevice
    misc_deregister(&smilebrd->miscdevice);

    // deregister smilebrd gpio driver
    platform_driver_unregister(&smilebrd_gpio_drv);

    // deregister smilebrd i2c driver
    i2c_del_driver(&smilebrd_i2c_drv);


    // free mem previously allocated
    kfree(smilebrd);
}

module_init(smilebrd_init);
module_exit(smilebrd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HeavyC1oud vheavyC1oud@gmail.com");
MODULE_DESCRIPTION("smilebrd driver");
