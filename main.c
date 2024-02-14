#include <linux/module.h>


static int dht11_init(void)
{
    pr_info("Hello world\n");
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
