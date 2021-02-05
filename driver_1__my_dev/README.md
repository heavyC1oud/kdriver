Testing simple character driver:

    creating kernel module my_dev.ko:
        sudo make

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
