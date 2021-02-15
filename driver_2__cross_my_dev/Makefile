export ARCH = arm

# toolchain from ARM
export CROSS_COMPILE = $(HOME)/toolchain/gcc-arm-10.2-2020.11-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-

KDIR = $(HOME)/kernel/linux-5.4.92
PWD = $(shell pwd)
TARGET = cross_my_dev

obj-m := $(TARGET).o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
