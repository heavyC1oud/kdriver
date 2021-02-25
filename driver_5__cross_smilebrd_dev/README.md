Testing smilebrd dev:

    crosscompile kernel module smilebrd_dev.ko:
        make

    copy crosscompiled kernel module to target
        sudo cp smilebrd_dev.ko /media/$USER/rootfs/home

    compile new device tree for BeagleBone board, from kernel source root
        make ARCH=arm dtbs

    copy new blob file to target
        sudo cp am335x-boneblack.dtb /media/$USER/boot/

    insert module on target:
        sudo insmod ./smilebrd_dev.ko

    start test program
        ./test

    test module
        when connect button pin to gnd, led switches

    remove module:
        sudo rmmod smilebrd_dev
