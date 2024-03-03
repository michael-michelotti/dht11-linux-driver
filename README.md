# dht11-linux-driver
A Linux driver for a BeagleBone Black (BBB) to read from a DHT11 humidity sensor.

<div align="center">
  <img src="https://github.com/michael-michelotti/dht11-linux-driver/blob/main/dht11_setup_image.JPG" alt="BeagleBone Black and DHT11 project setup" width="500"/>
</div>

## Overview
This is a demonstration Linux device driver for the [DHT11 humidity and temperature sensor](https://www.adafruit.com/product/386). 

There already [exists a driver](https://github.com/torvalds/linux/blob/master/drivers/iio/humidity/dht11.c) for this device in the official Linux kernel source code. The official code is written as an [Industrial Input/Output (IIO)](https://www.kernel.org/doc/html/v4.12/driver-api/iio/index.html) driver. 

As a demonstration, I converted this driver into a platform device driver which you interface with through the [sysfs psuedo file system](https://docs.kernel.org/filesystems/sysfs.html). 

I also converted the interrupt-based logic to polling-based logic, as the BeagleBone black runs a non-RTOS version of Linux, and was not servicing the DHT11 interrupts quickly enough. 

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
__1. (Optional) Flash your BBB with the latest image__

The BBB comes with an OS pre-flashed to the onboard eMMC memory. However, this may not be the latest available image. You can download the latest image [here](https://www.beagleboard.org/distros/am335x-12-2-2023-10-07-4gb-emmc-iot-flasher) (12.2 4GB IoT was used for this demo) and flash it to a microSD card. Insert the microSD card into the BBB and apply power. The new image will be loaded to the eMMC automatically if you downloaded the "Flasher" version of the image. After completion, the board will power off. Remove the microSD card and apply power to boot.

__2. Download the source code__
```
git clone git@github.com:michael-michelotti/dht11-linux-driver.git && cd dht11-linux-driver
```
__3. Compile the device tree overlay (DTBO) and module kernel object (this should result in several files, including `am335x-boneblack-mm-dht11.dtbo` and `dht11-mm.ko` in your project directory)__
```
make
```
__4. Hook the BBB up to your host machine with the Mini USB to USB A cable__

Wait for the BBB to boot, should take around 3 minutes.

__5. Once the BBB has booted, secure copy your module and device tree overlay to the BBB__
```
scp am335x-boneblack-mm-dht11.dtbo debian@192.168.6.2:/home/debian
scp dht11-mm.ko debian@192.168.6.2:/home/debian
```
__6. Insert the device tree overlay and reboot (login info is user: debian, password: temppwd)__
```
ssh debian@192.168.6.2
chmod +x am335x-boneblack-mm-dht11.dtbo
sudo chown root:root am335x-boneblack-mm-dht11.dtbo
sudo mv am335x-boneblack-mm-dht11.dtbo /boot/dtbs/5.10.168-ti-r72/overlays/
sudo reboot
```
__7. Wait for BBB to reboot. Log back in and insert driver module__
```
ssh debian@192.168.6.2
sudo insmod dht11-mm.ko
```
__8. Read from DHT11 sensor via sysfs interface!__
```
cd /sys/devices/platform/dht11
cat humidity
cat temperature
```
## Usage
