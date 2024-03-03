obj-m:=dht11-mm.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
TARGET_KERN=5.10.168-ti-r72
TARGET_KERN_DIR:=~/linux-$(TARGET_KERN)
HOST_KERN_DIR=/lib/modules/$(shell uname -r)/build

MODULE_FNAME=dht11-mm.ko

DTBO_FNAME=am335x-boneblack-mm-dht11.dtbo
DTS_FNAME=am335x-boneblack-mm-dht11.dts

BBB_IP=192.168.7.2
BBB_USER=debian
BBB_DIR=/home/debian

.PHONY: all ko dtbo target-reboot clean help host

all: ko dtbo target-reboot

ko: build-ko send-ko configure-ko

build-ko:
	@echo "Building BBB module .ko..."
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) modules

send-ko:
	@echo "Sending .ko to BBB..."
	scp $(MODULE_FNAME) $(BBB_USER)@$(BBB_IP):$(BBB_DIR)

configure-ko:
	@echo "Configuring module for insertion into kernel..."
	# Moves the driver .ko into place and informs the kernel to load the driver at boot via /etc/modules
	# 1. move dht11-mm.ko to the location where the kernel looks for loadable modules
	# 2. update the kernel's tree of which modules are available for load (modprobe)
	# 3. Add dht11-mm as a module to load a boot time
	ssh $(BBB_USER)@$(BBB_IP) 'sudo -S mv $(MODULE_FNAME) /usr/lib/modules/$(TARGET_KERN)/extra && \
								sudo -S depmod -a && \
								sudo -S echo "dht11-mm" | sudo tee -a /etc/modules'

dtbo: build-dtbo send-dtbo configure-dtbo

build-dtbo:
	@echo "Building device tree overlay .dtbo..."
	sudo $(TARGET_KERN_DIR)/scripts/dtc/dtc -O dtb -o $(DTBO_FNAME) -b O -@ $(DTS_FNAME)

send-dtbo:
	@echo "Sending .dtbo to BBB..."
	scp $(DTBO_FNAME) $(BBB_USER)@$(BBB_IP):$(BBB_DIR)

configure-dtbo:
	@echo "Configuring BBB to utilize device tree overlay..."
	# Moves the device tree overlay into place and informs UBoot to include it as an overlay
	# 1. Make the .dtbo executable
	# 2. Switch it so it's owned by root like all other .dtbos 
	# 3. Move .dtbo to location where UBoot looks for overlays
	# 4. Add the .dtbo as an overlay to load in uEnv.txt
	ssh $(BBB_USER)@$(BBB_IP) 'sudo -S chmod +x $(DTBO_FNAME) && \
								sudo -S chown root:root $(DTBO_FNAME) && \
								sudo -S mv $(BBB_DIR)/$(DTBO_FNAME) /boot/dtbs/$(TARGET_KERN)/overlays/ && \
								grep -q "#uboot_overlay_addr0=<file0>.dtbo" /boot/uEnv.txt && \
								sudo -S sed -i "s/#uboot_overlay_addr0=<file0>.dtbo/uboot_overlay_addr0=$(DTBO_FNAME)/" /boot/uEnv.txt'

target-reboot:
	@echo "Rebooting BBB..."
	ssh $(BBB_USER)@$(BBB_IP) 'sudo -S reboot'

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) clean

help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(TARGET_KERN_DIR) M=$(PWD) help

host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules
