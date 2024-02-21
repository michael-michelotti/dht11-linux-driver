obj-m := main.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
TARGET_KERN_DIR=/home/michael/linux-5.10.168-ti-r72
HOST_KERN_DIR = /lib/modules/$(shell uname -r)/build

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) clean

help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) help

host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules

dtbo:
	$(TARGET_KERN_DIR)/scripts/dtc -O dtb -o am335x-boneblack-mm-dht11.dtbo -b O -@ am335x-boneblack-mm-dht11.dts
