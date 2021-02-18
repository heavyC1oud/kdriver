#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>           // miscellaneous character module
#include <linux/kernel.h>
#include <linux/platform_device.h>      // platform devices
#include <linux/gpio/consumer.h>        // GPIO descriptor
#include <linux/interrupt.h>            // IRQ
#include <linux/of.h>                   // Device Tree (of - open firmware)


/*
 *    / {
 *          smilebrd {
 *          compatible = "heavyc1oud,smilebrd";
 *          pinctrl-names = "default";
 *          pinctrl-0 = <&smilebrd_pins>;
 *          button-gpios = <&gpio1 16 GPIO_ACTIVE_LOW>;
 *          led-gpios = <&gpio1 17 GPIO_ACTIVE_HIGH>;
 *          interrupt-parent = <&gpio1>;
 *          interrupts = <16 IRQ_TYPE_EDGE_FALLING>;
 *          status = "okay";
 *          };
 *    };
 */


// device cb for device node open
static int smilebrd_open(struct inode* inode, struct file* file)
{
    pr_info("Smilebrd open cb called\n");
    return 0;
}

// device cb for device node close
static int smilebrd_close(struct inode* inode, struct file* file)
{
    pr_info("Smilebrd closed cb called\n");
    return 0;
}

// device cb for device node ioctl
static long smilebrd_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    pr_info("Smilebrd ioctl cb is called, cmd=%i arg=%li\n", cmd, arg);
    return 0;
}

//  file operations structure
static const struct file_operations smilebrd_fops = {
    .owner = THIS_MODULE,
    .open = smilebrd_open,
    .release = smilebrd_close,
    .unlocked_ioctl = smilebrd_ioctl,
};

// miscdevice initialize
static struct miscdevice smilebrd_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "smilebrd",
    .fops = &smilebrd_fops,
};

// PLATFORM DEVICE
static struct gpio_desc* button;
static struct gpio_desc* led;
static unsigned int irq;

// static irq_handler_t smilebrd_irq_handler(int irq, void *dev_id)
static irqreturn_t smilebrd_irq_handler(int irq, void* dev_id)
{
    pr_info("wow! IRQ");

    // change led state
    gpiod_set_value(led, !(gpiod_get_value(led)));

    return IRQ_HANDLED;
}


static const struct of_device_id gpiod_dt_ids[] = {
    { .compatible = "heavyc1oud,smilebrd", },
    {}
};
MODULE_DEVICE_TABLE(of, gpiod_dt_ids);

static int smilebrd_probe(struct platform_device* pdev)
{
    int retval;
    struct device *dev = &pdev->dev;

    button = gpiod_get(dev, "button", 0);
    gpiod_direction_input(button);

    led = gpiod_get(dev, "led", 0);
    gpiod_direction_output(led, 0);

    retval = gpiod_set_debounce(button, 1000 * 5);        // time unit 1 us, 1000 us * 5 = 5 ms, MAX = 7 ms
    if(retval != 0) {
        pr_err("could not set debounce interval");
    }

    irq = gpiod_to_irq(button);
    retval = request_threaded_irq(irq, NULL, smilebrd_irq_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "smilepd_drv", NULL);
    if(retval != 0) {
        pr_err("could not register smilebrd irq handler");
        return retval;
    }

    // register miscdevice to kernel
    retval = misc_register(&smilebrd_miscdev);
    if(retval != 0) {
        pr_err("could not register misc device smilebrd");
        return retval;
    }

    pr_info("smilebrd probed!\n");
    return 0;
}

static int smilebrd_remove(struct platform_device* pdev)
{
    free_irq(irq, NULL);
    gpiod_put(button);
    gpiod_put(led);

    // unregister miscdevice from kernel
    misc_deregister(&smilebrd_miscdev);

    pr_info("smilebrd not used more\n");

    return 0;
}

static struct platform_driver pdrv_smilebrd = {
    .probe = smilebrd_probe,
    .remove = smilebrd_remove,
    .driver = {
        .name = "smilepd_drv",
        .of_match_table = of_match_ptr(gpiod_dt_ids),
        .owner = THIS_MODULE,
    },
};

module_platform_driver(pdrv_smilebrd);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("HeavyC1oud vheavyC1oud@gmail.com");
MODULE_DESCRIPTION("GPIO test driver");
