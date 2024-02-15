#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/gpio/consumer.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>

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

static int dht11_sysfs_probe(struct platform_device *pdev)
{
    int ret;

    return 0;
}

static int dht11_sysfs_remove(struct platform_device *pdev)
{
    return 0;
}

static const struct of_device_id dht11_dt_ids[] = 
{
    { .compatible = "dht11_mm" },
    { }
};
MODULE_DEVICE_TABLE(of, dht11_dt_ids);

struct platform_driver dht11_sysfs_platform_driver = 
{
    .probe = dht11_sysfs_probe,
    .remove = dht11_sysfs_remove,
    .driver = {
        .name = "dht11_mm",
        .of_match_table = dht11_dt_ids,
    }
};

static int dht11_init(void)
{
    platform_driver_register(&dht11_sysfs_platform_driver);
    return 0;
}

static void dht11_deinit(void)
{
    platform_driver_unregister(&dht11_sysfs_platform_driver);
}

module_init(dht11_init);
module_exit(dht11_deinit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Michelotti <michael.michelotti4@gmail.com>");
MODULE_DESCRIPTION("DHT11 sensor character driver");
