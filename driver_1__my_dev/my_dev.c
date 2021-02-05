#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>

static struct cdev my_cdev;      // internal cdev structure
static dev_t my_dev_num;         // MAJOR & MINOR

// device cb for device node open
static int my_dev_open(struct inode* inode, struct file* file)
{
    pr_info("My device open cb called\n");
    return 0;
}

// device cb for device node close
static int my_dev_close(struct inode* inode, struct file* file)
{
    pr_info("My device closed cb called\n");
    return 0;
}

// пdevice cb for device node ioctl
static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    pr_info("My device ioctl cb is called\n");
    return 0;
}

//  file operations structure
static const struct file_operations my_dev_fops = {
    .owner = THIS_MODULE,
    .open = my_dev_open,
    .release = my_dev_close,
    .unlocked_ioctl = my_dev_ioctl,
};


// Constructor
static int __init my_dev_init(void)
{
    int ret;    // for error info

    // get dynamically device identifier
    ret = alloc_chrdev_region(&my_dev_num, 0, 1, "my_dev");
    if(ret < 0) {
        pr_info("Unable get chrdev numbers\n");
        return ret;
    }

    pr_info("Hello from my_dev\n");
    pr_info("my_dev major number - %i\n", MAJOR(my_dev_num));
    pr_info("my_dev minor number - %i\n", MINOR(my_dev_num));

    // initialize the cdev structure and add it to kernel space
    cdev_init(&my_cdev, &my_dev_fops);
    ret = cdev_add(&my_cdev, my_dev_num, 1);
    if(ret < 0) {
        pr_info("Unable to add cdev\n");
        return ret;
    }

    return 0;
}

// Destructor
static void __exit my_dev_exit(void)
{
    pr_info("Goodbye from my_dev\n");
    cdev_del(&my_cdev);
    unregister_chrdev_region(my_dev_num, 1);
}

module_init(my_dev_init);
module_exit(my_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HeavyC1oud vheavyC1oud@gmail.com");
MODULE_DESCRIPTION("my_dev test driver");
