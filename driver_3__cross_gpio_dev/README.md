Testing crosscompile simple GPIO driver:

    crosscompile kernel module cross_gpio_dev.ko:
        make

    copy crosscompiled kernel module to target
        sudo cp cross_gpio_dev.ko /media/$USER/rootfs/home

    compile new device tree for BeagleBone board, from kernel source root
        make ARCH=arm dtbs

    copy new blob file to target
        sudo cp am335x-boneblack-wireless.dtb /media/$USER/boot/

    insert module on target:
        sudo insmod ./cross_gpio_dev.ko

    test button and led, connected to board
        connect button on gpio1_16 pin [R13] to GND,
        led on gpio1_17 pin [V14] must switch on/off
        on each switch kernel message "wow! IRQ" appear

    remove module:
        sudo rmmod my_gpio_dev
