Testing crosscompile simple character driver:

    crosscompile kernel module cross_my_dev.ko:
        make

    crosscopmile test program:
        cross_compile_gcc my_dev_test.c -static -o cross_my_dev_test

    copy crosscompiled kernel module to target
        sudo cp cross_my_dev.ko /media/$USER/rootfs/home

    copy crosscompiled test program to target
        sudo cp cross_my_dev_test /media/$USER/rootfs/home

    insert module on target:
        sudo insmod ./cross_my_dev.ko

    get major and minor numbers:
        dmesg | tail -2

    create device node file:
        sudo mknod my_dev c "major" "minor"

    start test program:
        ./my_dev_test

    check dmesg for module messages:
        dmesg | tail -3

    remove module:
        sudo rmmod my_dev
