# Dependencies & Setup

This project runs on a **BeagleBone** (or compatible Linux board) with I2C and ADC (IIO) interfaces available.

## System Packages

Update the package list and install the build toolchain:

```bash
sudo apt-get update
sudo apt-get install build-essential
```

Install the I2C tools, which provide the `i2c-dev` headers:

```bash
sudo apt-get install i2c-tools
```

Load the I2C kernel module:

```bash
sudo modprobe i2c-dev
```

## C Headers Used

The application relies on standard C and Linux headers:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
```

## Hardware Interfaces

- **I2C:** BMP390 sensor on `/dev/i2c-2` at address `0x76`. Verify it is detected with:
  ```bash
  i2cdetect -y -r 2
  ```
- **ADC (IIO):** soil-moisture and CO2 sensors read from
  `/sys/bus/iio/devices/iio:device0/in_voltage0_raw` and `in_voltage1_raw`.
  Confirm the IIO device path matches your board's configuration.

## Build

```bash
make
```
