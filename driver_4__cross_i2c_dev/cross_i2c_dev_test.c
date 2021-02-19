/*
 * проверка операций модуля
 * open
 * ioctl
 * close
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    // уже должен быть создан device node -> sudo mknod my_dev c "MAJOR" "MINOR"
    int my_dev = open("/dev/smilebrd_i2c", 0);

    if(my_dev < 0) {
        perror("Fail to open device file: smilebrd_i2c");
    }
    else {
        ioctl(my_dev, 11, 72);
        close(my_dev);
    }

    return 0;
}