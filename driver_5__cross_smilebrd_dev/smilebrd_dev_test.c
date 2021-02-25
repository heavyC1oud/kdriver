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
    unsigned int data;
    int retval;
    int fd;

    // уже должен быть создан device node -> sudo mknod smilebrd c "MAJOR" "MINOR"
    if((fd = open("/dev/smilebrd", 0)) == -1){
        perror("Fail to open device node smilebrd");
        return 1;
    }

    ioctl(fd, 11, 72);

    while(1) {
        retval = read(fd, &data, sizeof(unsigned int));
        if(retval < 0) {
            perror("Fail to read smilebrd");
            return retval;
        }

        printf("read: %i\n", data);
    }

    close(fd);

    return 0;
}