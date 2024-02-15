#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/gpio/consumer.h>
#include <linux/completion.h>
#include <linux/mutex.h>


dev_t dev_number;
struct cdev dht11_cdev;

struct dht11_private_data
{
    struct device *dev;
    struct gpio_desc *gpiod;
    int irq;
    struct completion completion;
    struct mutex lock;
    /* struct s64 timestamp; */
    int temperature;
    int humidity;
    int num_edges;

};

static int temperature_show(struct device *dev, struct device_attribute *attr,char *buf)
{
    return 0;
}

static int humidity_show(struct device *dev, struct device_attribute *attr,char *buf)
{
    return 0;
}

DEVICE_ATTR_RO(temperature);
DEVICE_ATTR_RO(humidity);

static struct attribute *dht11_attrs[] =
{
    &dev_attr_temperature.attr,
    &dev_attr_humidity.attr,
    NULL
};

static struct attribute_group dht11_attr_group = 
{
    .attrs = dht11_attrs
};

static const struct attribute_group *dht11_attr_groups[] = 
{
    &dht11_attr_group,
    NULL
};

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

    /* class_create API was updated in kernel version >= 6.4 to have only name, no owner argument.
    this code will have to be different on host vs. target!!! */
    class_dht11 = class_create("dht11_class");
    dev_dht11 = device_create(class_dht11, NULL, dev_number, NULL, "dht11");

    pr_info("Module init successful\n");
    return 0;
}

static void dht11_deinit(void)
{
    device_destroy(class_dht11, dev_number);
    class_destroy(class_dht11);
    cdev_del(&dht11_cdev);
    unregister_chrdev_region(dev_number, 1);
    pr_info("DHT11 module unloaded.");
}

module_init(dht11_init);
module_exit(dht11_deinit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Michelotti <michael.michelotti4@gmail.com>");
MODULE_DESCRIPTION("DHT11 sensor character driver");
MODULE_INFO(board, "Beaglebone Black A5");
