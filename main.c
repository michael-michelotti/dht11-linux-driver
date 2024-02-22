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
#include <linux/timekeeping.h>
#include <linux/delay.h>
#include <linux/interrupt.h>



#define DRIVER_NAME                    "dht11-mm"
#define DHT11_DATA_VALID_TIME           2000000000  /* 2s in ns */
#define DHT11_EDGES_PREAMBLE            2
#define DHT11_BITS_PER_READ             40
/*
 * Note that when reading the sensor actually 84 edges are detected, but
 * since the last edge is not significant, we only store 83:
 */
#define DHT11_EDGES_PER_READ (2 * DHT11_BITS_PER_READ + \
                  DHT11_EDGES_PREAMBLE + 1)

#define DHT11_START_TRANSMISSION_MIN    1800000  /* us */
#define DHT11_START_TRANSMISSION_MAX    2000000  /* us */
#define DHT11_MIN_TIMERES               34000  /* ns */
#define DHT11_THRESHOLD                 49000  /* ns */
#define DHT11_AMBIG_LOW                 23000  /* ns */
#define DHT11_AMBIG_HIGH                30000  /* ns */

struct dht11_private_data
{
    struct device                       *dev;
    struct gpio_desc                    *gpiod;
    int                                 irq;
    struct completion                   completion;
    struct mutex                        lock;
    s64                                 timestamp;
    int                                 temperature;
    int                                 humidity;
    int                                 num_edges;
    struct { s64 ts; int value; }       edges[DHT11_EDGES_PER_READ];
};

static unsigned char dht11_decode_byte(char *bits)
{
    unsigned char ret = 0;
    int i;

    for (i = 0; i < 8; ++i) 
    {
        ret <<= 1;
        if (bits[i])
            ++ret;
    }

    return ret;
}

static int dht11_decode(struct dht11_private_data *dht11, int offset)
{
    int i, t;
    char bits[DHT11_BITS_PER_READ];
    unsigned char temp_int, temp_dec, hum_int, hum_dec, checksum;

    for (i = 0; i < DHT11_BITS_PER_READ; i++) 
    {
        t = dht11->edges[offset + 2 * i + 2].ts -
            dht11->edges[offset + 2 * i + 1].ts;
        if (!dht11->edges[offset + 2 * i + 1].value) 
        {
            dev_info(dht11->dev, "lost synchronisation at edge %d\n", offset + 2 * i + 1);
            return -EIO;
        }
        bits[i] = t > DHT11_THRESHOLD;
    }

    hum_int = dht11_decode_byte(bits);
    hum_dec = dht11_decode_byte(&bits[8]);
    temp_int = dht11_decode_byte(&bits[16]);
    temp_dec = dht11_decode_byte(&bits[24]);
    checksum = dht11_decode_byte(&bits[32]);

    if (((hum_int + hum_dec + temp_int + temp_dec) & 0xff) != checksum) 
    {
        dev_info(dht11->dev, "invalid checksum\n");
        return -EIO;
    }

    dht11->timestamp = ktime_get_boottime_ns();
    dht11->temperature = temp_int * 1000;
    dht11->humidity = hum_int * 1000;

    return 0;
}

static irqreturn_t dht11_handle_irq(int irq, void *data)
{
    struct device *dev = (struct device *) data;
    struct dht11_private_data *dht11 = dev_get_drvdata(dev);

    if (dht11->num_edges < DHT11_EDGES_PER_READ && dht11->num_edges >= 0)
    {
        dht11->edges[dht11->num_edges].ts = ktime_get_boottime_ns();
        dht11->edges[dht11->num_edges++].value = gpiod_get_value(dht11->gpiod);
        if (dht11->num_edges >= DHT11_EDGES_PER_READ)
        {
            complete(&dht11->completion);
        }
    }

    return IRQ_HANDLED;
}

static int dht11_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct dht11_private_data *dht11 = dev_get_drvdata(dev);
    int ret, timeres, offset;

    mutex_lock(&dht11->lock);
    if (dht11->timestamp + DHT11_DATA_VALID_TIME < ktime_get_boottime_ns())
    {
        timeres = ktime_get_resolution_ns(); 
        dev_info(dev, "current time resolution: %dns\n", timeres);
        if (timeres > DHT11_MIN_TIMERES)
        {
            dev_err(dht11->dev, "time resolution %dns too low\n", timeres);
            ret = -EAGAIN;
            goto err;
        }

        reinit_completion(&dht11->completion);
        dht11->num_edges = 0;
        /* Start transmission - pull line low */
        dev_err(dht11->dev, "GPIO is active low: %d\n", gpiod_is_active_low(dht11->gpiod));
        dev_err(dht11->dev, "have GPIO %d\n", desc_to_gpio(dht11->gpiod));
        dev_err(dht11->dev, "current gpio direction: %d\n", gpiod_get_direction(dht11->gpiod));
        dev_err(dht11->dev, "current gpio value: %d\n", gpiod_get_value(dht11->gpiod));
        ret = gpiod_direction_output(dht11->gpiod, 0);
        if (ret)
        {
            goto err;
        }
        dev_err(dht11->dev, "return val: %d. new gpio direction: %d\n", ret, gpiod_get_direction(dht11->gpiod));
        dev_err(dht11->dev, "new gpio value: %d\n", gpiod_get_value(dht11->gpiod));

        dev_err(dht11->dev, "sleep start\n");
        usleep_range(DHT11_START_TRANSMISSION_MIN, DHT11_START_TRANSMISSION_MAX);
        dev_err(dht11->dev, "sleep end\n");

        /* Start reading from DHT11 sensor via interrupts */
        ret = gpiod_direction_input(dht11->gpiod);
        if (ret)
        {
            goto err;
        }

        ret = request_irq(dht11->irq, dht11_handle_irq, 
                            IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, 
                            dev->init_name, dev);
        if (ret)
        {
            goto err;
        }

        ret = wait_for_completion_killable_timeout(&dht11->completion, HZ);
        free_irq(dht11->irq, dev);
        if (ret == 0 && dht11->num_edges < DHT11_EDGES_PER_READ - 1)
        {
            dev_err(dev, "Only %d signal edges detected\n", dht11->num_edges);
            ret = -ETIMEDOUT;
        }

        if (ret < 0)
        {
            goto err;
        }

        offset = DHT11_EDGES_PREAMBLE + dht11->num_edges - DHT11_EDGES_PER_READ;
        for (;offset >= 0; offset--) 
        {
            ret = dht11_decode(dht11, offset);
            if (!ret)
            {
                break;
            }
        }
    }

err:
    dht11->num_edges = -1;
    mutex_unlock(&dht11->lock);
    return ret;
}

/****** SYSFS ATTRIBUTE FUNCTION IMPLEMENTATION *******/
static ssize_t temperature_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;
    struct dht11_private_data *dht11 = dev_get_drvdata(dev);

    ret = dht11_read(dev, attr, buf);
    if (ret)
    {
        return -EINVAL;
    }

    return sprintf(buf, "%d\n", dht11->temperature);
}

static ssize_t humidity_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;
    struct dht11_private_data *dht11 = dev_get_drvdata(dev);

    ret = dht11_read(dev, attr, buf);
    if (ret)
    {
        return -EINVAL;
    }

    return sprintf(buf, "%d\n", dht11->humidity);
}

/****** SYSFS ATTRIBUTE DECLARATION *******/
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

static int dht11_sysfs_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct dht11_private_data *dht11;
    int ret;

    dht11 = (struct dht11_private_data *) devm_kzalloc(dev, sizeof(struct dht11_private_data), GFP_KERNEL);
    if (!dht11)
    {
        dev_err(dev, "Failed to allocate memory\n");
        return -ENOMEM;
    }

    dht11->gpiod = devm_gpiod_get(dev, "data", GPIOD_IN);
    if (IS_ERR(dht11->gpiod))
    {
        return PTR_ERR(dht11->gpiod);
    }

    dht11->irq = gpiod_to_irq(dht11->gpiod);
    if (dht11->irq < 0)
    {
        dev_err(dev, "GPIO %d has no interrupt\n", desc_to_gpio(dht11->gpiod));
        return -EINVAL;
    }

    dev_set_drvdata(dev, dht11);
    dht11->dev = dev;
    init_completion(&dht11->completion);
    mutex_init(&dht11->lock);
    dht11->timestamp = ktime_get_boottime_ns();
    dht11->num_edges = -1;

    ret = sysfs_create_group(&dev->kobj, &dht11_attr_group);
    if (ret)
    {
        dev_err(dev, "Failed to create sysfs group\n");
        return ret;
    }

    return 0;
}

static int dht11_sysfs_remove(struct platform_device *pdev)
{
    sysfs_remove_group(&pdev->dev.kobj, &dht11_attr_group);
    return 0;
}

static const struct of_device_id dht11_dt_ids[] = 
{
    { .compatible = "michelotti,dht11" },
    { }
};
MODULE_DEVICE_TABLE(of, dht11_dt_ids);

static struct platform_driver dht11_sysfs_platform_driver = 
{
    .probe = dht11_sysfs_probe,
    .remove = dht11_sysfs_remove,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = dht11_dt_ids,
    }
};

module_platform_driver(dht11_sysfs_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Michelotti <michael.michelotti4@gmail.com>");
MODULE_DESCRIPTION("DHT11 humidity/temperature sensor driver");
