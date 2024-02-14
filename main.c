#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/module.h>


dev_t dev_number;
struct cdev dht11_cdev;

static int dht11_open(struct inode *inode, struct file *filep)
{
    return 0;
}

static int dht11_release(struct inode *inode, struct file *filep)
{
    return 0;
}

static ssize_t dht11_read(struct file *filep, char __user *buff, size_t count, loff_t *f_pos)
{
    return 0;
}

static ssize_t dht11_write(struct file *filep, const char __user *buff, size_t count, loff_t *f_pos)
{
    return 0;
}

struct file_operations dht11_fops = 
{
    .open = dht11_open,
    .release = dht11_release,
    .read = dht11_read,
    .write = dht11_write,
    .owner = THIS_MODULE
};

struct class *class_dht11;
struct device *dev_dht11;

static int dht11_init(void)
{
    alloc_chrdev_region(&dev_number, 0, 1, "dht11_devices");
    pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(dev_number), MINOR(dev_number));

    cdev_init(&dht11_cdev, &dht11_fops);
    dht11_cdev.owner = THIS_MODULE;
    cdev_add(&dht11_cdev, dev_number, 1);

    // class_create API was updated in kernel version >= 6.4 to have only name, no owner argument.
    // this code will have to be different on host vs. target!!! 
    class_dht11 = class_create("dht11_class");
    dev_dht11 = device_create(class_dht11, NULL, dev_number, NULL, "dht11");

    pr_info("Module init successful\n");
    return 0;
}

static void dht11_deinit(void)
{
    pr_info("Goodbye world\n");
}

module_init(dht11_init);
module_exit(dht11_deinit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Michelotti <michael.michelotti4@gmail.com>");
MODULE_DESCRIPTION("DHT11 sensor character driver");
MODULE_INFO(board, "Beaglebone Black A5");
