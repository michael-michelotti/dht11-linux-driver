#include <linux/module.h>

static int dht11_init(void)
{

}

static void dht11_deinit(void)
{

}

module_init(dht11_init);
module_exit(dht11_deinit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Michelotti <michael.michelotti4@gmail.com>");
MODULE_DESCRIPTION("DHT11 sensor character driver");
MODULE_INFO(board, "Beaglebone Black A5");
