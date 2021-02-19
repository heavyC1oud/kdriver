Testing crosscompile simple I2C driver:

    crosscompile kernel module cross_gpio_dev.ko:
        make

    copy crosscompiled kernel module to target
        sudo cp cross_i2c_dev.ko /media/$USER/rootfs/home

    compile new device tree for BeagleBone board, from kernel source root
        make ARCH=arm dtbs

    copy new blob file to target
        sudo cp am335x-boneblack.dtb /media/$USER/boot/

    insert module on target:
        sudo insmod ./cross_i2c_dev.ko

    test i2c
        witch oscilloscope can see i2c request on SDA/SCL pins

    remove module:
        sudo rmmod my_i2c_dev
