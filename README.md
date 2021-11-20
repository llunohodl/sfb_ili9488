# Utils for use standart framebuffer driver with ili9488

> All tested on framebuffer driver from [4.14.111 kernel for NanoPi NEO Core](https://github.com/friendlyarm/linux/tree/sunxi-4.14.y)

## Setup driver module


Setup modules (check your GPIO connections, this config use spi0):

> ili9488 in SPI mode use 3 byte per pixel (bpp = 24), but default value in driver is 2 byte per pixel for correct work we add height 320*3/2=480

```
sudo modprobe fbtft_device name=flexfb gpios=reset:203,dc:1,led:0,cs:67 speed=40000000 &&\
sudo modprobe flexfb width=480 height=480 buswidth=8 init=\
-1,0x01,\
-2,120,\
-1,0x36,0xE8,\
-1,0x3A,0x66,\
-1,0x21,\
-1,0x11,\
-2,120,\
-1,0x29,\
-2,20,\
-1,0x13,\
-3
```
> Control symbols in init sequence : `-1` - send data / `-2` - delay ms / `-3` - end

Here is correct output after module add (run `dmesg`):

```
[ 4556.313774] fbtft_device: module is from the staging directory, the quality is unknown, you have been warned.
[ 4556.315604] fbtft_device: GPIOS used by 'flexfb':
[ 4556.315622] fbtft_device: 'reset' = GPIO203
[ 4556.315628] fbtft_device: 'dc' = GPIO1
[ 4556.315634] fbtft_device: 'led' = GPIO0
[ 4556.315640] fbtft_device: 'cs' = GPIO67
[ 4556.315666] spi spi0.0: flexfb spi0.0 40000kHz 8 bits mode=0x00
[ 4556.441204] flexfb: module is from the staging directory, the quality is unknown, you have been warned.
[ 4556.948816] graphics fb1: flexfb frame buffer, 480x480, 450 KiB video memory, 4 KiB buffer memory, fps=20, spi0.0 at 40 MHz
```

## Farmebuffer test C application

- [main.c](main.c)
- [Makefile](Makefile)

build:
```
make
```
runtest
```
./fbtest
```
