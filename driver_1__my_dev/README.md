Testing simple character driver:

    creating kernel module my_dev.ko:
        sudo make

    insert module:
        sudo insmod ./my_dev.ko

    get major and minor numbers:
        dmesg | tail -2

    create device node file:
        sudo mknod my_dev c "major" "minor"

    create test program:
        gcc my_dev_test.c -o my_dev_test

    start test program:
        ./my_dev_test

    check dmesg for module messages:
        dmesg | tail -3

    remove module:
        sudo rmmod my_dev
