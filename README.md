# dht11-linux-driver
A Linux driver for a BeagleBone Black (BBB) to read from a DHT11 humidity sensor.

<div align="center">
  <img src="https://github.com/michael-michelotti/dht11-linux-driver/blob/main/readme_data/bbb_dht11_setup_diagram.png" alt="BeagleBone Black and DHT11 project setup" width="700"/>
</div>

## Overview
This is a demonstration Linux device driver for the [DHT11 humidity and temperature sensor](https://www.adafruit.com/product/386). 

There already [exists a driver](https://github.com/torvalds/linux/blob/master/drivers/iio/humidity/dht11.c) for this device in the official Linux kernel source code. The official code is written as an [Industrial Input/Output (IIO)](https://www.kernel.org/doc/html/v4.12/driver-api/iio/index.html) driver. 

As a demonstration, I converted this driver into a platform device driver which you interface with through the [sysfs psuedo file system](https://docs.kernel.org/filesystems/sysfs.html). 

I also converted the interrupt-based logic to polling-based logic, as the BeagleBone black runs a non-RTOS version of Linux, and was not servicing the DHT11 interrupts quickly enough. 

### About The Sensor
The sensor is a DHT11 humidity and temperature sensor. Here is the basic logic flow to read from it:
1. Pull the bus line low (it must be pulled up by default) to signal the sensor to start transmitting data.
2. DHT11 pulls line low for 80us, then high for 80us, this is the sensor's preamble.
3. DHT11 begins data transmit. Bits are transmitted by pulling line low for 50us, then high for either 20us or 70us. 20us indicates a 0, 70us indicates a 1.

When probed with a logic analyzer, the data transmission looks like this:
![DHT11 logic analyzer probe](https://github.com/michael-michelotti/dht11-linux-driver/blob/main/readme_data/dht11_logic_analyzer_decoded.png)

## Built With
This project was developed for a [BeagleBone Black](https://www.beagleboard.org/boards/beaglebone-black), which is a community-run development platform, similar to Raspberry Pi. The board features an [AM3358 CPU](https://www.ti.com/product/AM3358) by TI, which in turn features an ARM Cortex-A8 core. The BeagleBone Black offers two expansion headers for GPIO purposes. In this project, the 8th GPIO pin on header P8 interfaces with the single data line of the DHT11 sensor.

This project was built with the latest BeagleBone Black Internet of Things (IoT) Linux distribution, [version 12.2](https://www.beagleboard.org/distros/am335x-12-2-2023-10-07-4gb-emmc-iot-flasher), which features Linux kernel version [5.10.168-ti-r72](https://github.com/beagleboard/linux/tree/5.10.168-ti-r72) and U-Boot bootloader version [2022.04](https://openbeagle.org/beagleboard/u-boot/-/tree/v2022.04-bbb.io-am335x-am57xx).

The BeagleBone Black was connected to the DHT11 sensor via a breadboard and jumper wires. They are interfaced through a Hiletgo TXS0108E level shifter, as the GPIO pad on the AM3358 runs at 3.3V, and the DHT11 sensor runs at 5V.

## Getting Started
### Prerequisites
#### Hardware
- BeagleBone Black (BBB) development board
- microSD card reader
- microSD card (8GB or greater)
- DHT11 humidity/temperature sensor
- Breadboard
- Male to Male breadboard wires
- Mini USB to USB A wire
- 3.3V to 5V level shifter
#### Software
- Host machine running Linux (or VM running Linux)
- Linux kernel source code version 5.10.168-ti-r72
- BBB image (AM335x 12.2 2023-10-07 4GB eMMC IoT Flasher)

### Installation
#### Hardware Configuration
The hardware setup is pictured in the [dht11-linux-driver](#dht11-linux-driver) section. I will not enumerate every connection here, but here are the high level steps:
Here is the BBB pinout diagram:
![BeagleBone Black pinout diagram](https://github.com/michael-michelotti/dht11-linux-driver/blob/main/readme_data/bbb_pinout_headers.png)

* The 5V rail (bottom of the pictured breadboard) must be powered from SYS_5V (P9_08)
* The 3.3V rail (top) must be powered from VDD_3V3 (P9_04)
* GPIO 67 (P8_08) must be connected to the 3.3V side of the level shifter
* The data line of the DHT11 must be connected to the 5V side of the level shifter

### Software Configuration
__1. Flash your BBB with the latest image__

The BBB comes with an OS pre-loaded to the onboard eMMC memory. However, this may not be the image that this driver was designed to run on. You can download the proper image [here](https://www.beagleboard.org/distros/am335x-12-2-2023-10-07-4gb-emmc-iot-flasher) (12.2 4GB IoT was used for this demo) and flash it to a microSD card. Insert the microSD card into the BBB and apply power. The new image will be loaded to the eMMC automatically if you downloaded the "Flasher" version of the image. After completion, the board will power off. Remove the microSD card and apply power to boot.

__2. Download the source code__
```
git clone git@github.com:michael-michelotti/dht11-linux-driver.git && cd dht11-linux-driver
```

__3. Configure the BBB__

A couple things must happen before the BBB will interface with the DHT11 sensor.
* The provided DTS must be built into a DTBO (device tree overlay binary), pushed to the BBB, and registered with the bootloader
* The provided `dht11-mm.c` source code must be compiled into a kernel object `.ko`, pushed to the BBB, and reigstered with the kernel
* The BBB must be rebooted

All these steps are taken care of in the provided `Makefile`. You just need to run:
```
make
```
However, ensure that the BBB is plugged into your host machine. The `Makefile` will ask you for the password to the BBB several times. The password is `temppwd` by default.

__4. Wait for BBB to reboot. Log back in.__
```
ssh debian@192.168.6.2
```
__5. Read from DHT11 sensor via sysfs interface!__
```
cd /sys/devices/platform/dht11
cat humidity
cat temperature
```
## Usage
Once you have completed the installation steps above, you need: 
1. Hook up the hardware.
2. SSH into the BBB.
3. Interact with the sensor through sysfs at `/sys/devices/platform/dht11/<humidity/temperature>`

### SSH into the BBB
These steps assume you have already completed the [Hardware Configuration](#hardware-configuration) and [Software Configuration](#software-configuration) steps defined in this README.

__1. Plug the BBB into your host machine with your Mini USB to USB A wire.__
__2. SSH into the BBB at `192.168.7.2`__
```
ssh debian@192.168.7.2
```
__3. Change directory to the DHT11 sysfs location:__
```
cd /sys/devices/platform/dht11
```
__4. Request the temperature or humidity from the DHT11 sensor by reading the `temperature` or `humidity` file:__
```
cat humidity
cat temperature
```
Your output should look like this:

![DHT11 demo usage](https://github.com/michael-michelotti/dht11-linux-driver/blob/main/readme_data/dht11_cat_temp_hum_usage.png)

Note that the script is flaky because the read function can get preempted (non-RTOS Linux) and miss edges. You may have to try to read the file multiple times.
