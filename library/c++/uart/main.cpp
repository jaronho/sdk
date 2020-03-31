#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "uart_op.h"

int main(void) {
    int fd = uart_open("/dev/ttyS1", O_RDWR);
    if (fd > 0) {
        printf("串口打开成功\n");
        printf("波特率: %d\n", uart_get_baud_rate(fd));
        printf("数据位: %d\n", uart_get_data_bits(fd));
        printf("校验位: %c\n", uart_get_parity(fd));
        printf("停止位: %d\n", uart_get_stop_bits(fd));
        char buf[10] = { 0 };
        int len = 0;
        while (1) {
            while ((len = uart_read(fd, buf, sizeof(buf) - 1)) > 0) {
                printf("%s\n", buf);
                uart_write(fd, buf, strlen(buf));
                len = 0;
                memset(buf, 0, sizeof(buf));
            }
        }
        uart_close(fd);
    } else {
        printf("串口打开失败\n");
    }
    return 0;
}

