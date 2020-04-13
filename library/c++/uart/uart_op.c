/**********************************************************************
* Author:	jaron.ho
* Date:		2020-03-30
* Brief:	uart operator
**********************************************************************/
#include "uart_op.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int uart_open(const char* device, int mode) {
    if (!device || 0 == strlen(device)) {
        return -2;
    }
    return open(device, mode);
}

int uart_close(int fd) {
    if (fd < 0) {
        return -1;
    }
    close(fd);
    return 0;
}

int uart_config(int fd, int baudRate, int dataBits, char parity, int stopBits, int flow) {
    struct termios opt;
    unsigned int speed;
    unsigned char speedValid;
    speedValid = 1;
    if (fd < 0) {
        return -1;
    }
    /* 获取配置 */
    if (tcgetattr(fd, &opt) < 0) {
        return -2;
    }
    /* 设置波特率 */
    switch (baudRate) {
        case 0: speed = B0; break;
        case 50: speed = B50; break;
        case 75: speed = B75; break;
        case 110: speed = B110; break;
        case 134: speed = B134; break;
        case 150: speed = B150; break;
        case 200: speed = B200; break;
        case 300: speed = B300; break;
        case 600: speed = B600; break;
        case 1200: speed = B1200; break;
        case 1800: speed = B1800; break;
        case 2400: speed = B2400; break;
        case 4800: speed = B4800; break;
        case 9600: speed = B9600; break;
        case 19200: speed = B19200; break;
        case 38400: speed = B38400; break;
        case 57600: speed = B57600; break;
        case 115200: speed = B115200; break;
        case 230400: speed = B230400; break;
        case 460800: speed = B460800; break;
        case 500000: speed = B500000; break;
        case 576000: speed = B576000; break;
        case 921600: speed = B921600; break;
        case 1000000: speed = B1000000; break;
        case 1152000: speed = B1152000; break;
        case 1500000: speed = B1500000; break;
        case 2000000: speed = B2000000; break;
        case 2500000: speed = B2500000; break;
        case 3000000: speed = B3000000; break;
        case 3500000: speed = B3500000; break;
        case 4000000: speed = B4000000; break;
        default: speedValid = 0; break;
    }
    if (1 == speedValid) {
        if (cfsetispeed(&opt, speed) < 0 || cfsetospeed(&opt, speed) < 0) {
            return -3;
        }
    }
    /* 设置数据位, CSIZE: 数据位的位掩码 */
    opt.c_cflag &= ~CSIZE;      /* 清除数据位 */
    switch (dataBits) {
    case 5:
        opt.c_cflag |= CS5;
        break;
    case 6:
        opt.c_cflag |= CS6;
        break;
    case 7:
        opt.c_cflag |= CS7;
        break;
    case 8:
        opt.c_cflag |= CS8;
        break;
    default:
        break;
    }
    /* 设置校验位, PARENB: 校验位使能, PARODD: 使用奇校验而不使用偶校验, INPCK: 奇偶校验使能, ISTRIP: 除去奇偶校验位 */
    switch (parity) {
    case 'N':
        opt.c_cflag &= ~PARENB;     /* 取消校验位 */
        opt.c_cflag &= ~CSTOPB;     /* 使用1个停止位 */
        break;
    case 'O':
        opt.c_cflag |= PARENB;      /* 使能校验位 */
        opt.c_cflag |= PARODD;      /* 使用奇校验而不使用偶校验 */
        opt.c_cflag &= ~CSTOPB;     /* 使用1个停止位 */
        opt.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':
        opt.c_cflag |= PARENB;      /* 使能校验位 */
        opt.c_cflag &= ~PARODD;     /* 不使用奇校验即使用偶校验 */
        opt.c_cflag &= ~CSTOPB;     /* 使用1个停止位 */
        opt.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'S':   /* Space parity is set the same as None parity */
        opt.c_cflag &= ~PARENB;     /* 取消校验位 */
        opt.c_cflag &= ~CSTOPB;     /* 使用1个停止位 */
        break;
    case 'M':   /* Mark parity is simulated by using 2 Stop bits */
        opt.c_cflag &= ~PARENB;     /* 取消校验位 */
        opt.c_cflag |= CSTOPB;      /* 使用2个停止位 */
        break;
    default:
        break;
    }
    /* 设置停止位, CSTOPB: 2个停止位(不设则是1个停止位) */
    switch (stopBits){
    case 1:
        opt.c_cflag &= ~CSTOPB;     /* 不设即1个停止位 */
        break;
    case 2:
        opt.c_cflag |= CSTOPB;      /* 设置2个停止位 */
        break;
    default:
        break;
    }
    /* 设置流控 */
    switch(flow) {
    case 0:
    #ifdef __USE_MISC
        opt.c_cflag &= ~CRTSCTS;                /* 不使用流控制 */
    #endif
        break;
    case 1:
    #ifdef __USE_MISC
        opt.c_cflag |= CRTSCTS;                 /* 使用硬件流控制 */
    #endif
        break;    
    case 2:
        opt.c_cflag |= IXON | IXOFF | IXANY;    /* 使用软件流控制 */
        break;
    default:
        break;
    }
    /* 如果发生数据溢出,接收数据,但是不再读取,刷新收到的数据但是不读 */
    tcflush(fd, TCIFLUSH);
    /* 激活配置 */
    if (tcsetattr(fd, TCSANOW, &opt) < 0) {
        return -4;
    }
    return 0;
}

int uart_set_nonblock(int fd, int nonblock) {
    if (fd < 0) {
        return -1;
    }
    if (1 == nonblock) {
        if (fcntl(fd, F_SETFL, FNDELAY) < 0) {
            return -2;
        }
    } else {
        if (fcntl(fd, F_SETFL, 0) < 0) {
            return -2;
        }
    }
    return 0;
}

int uart_set_wait_time(int fd, unsigned char time) {
    struct termios opt;
    if (fd < 0) {
        return -1;
    }
    if (tcgetattr(fd, &opt) < 0) {
        return -2;
    }
    opt.c_cc[VTIME] = time;
    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &opt) < 0) {
        return -3;
    }
    return 0;
}

int uart_set_wait_bytes(int fd, unsigned char bytes) {
    struct termios opt;
    if (fd < 0) {
        return -1;
    }
    if (tcgetattr(fd, &opt) < 0) {
        return -2;
    }
    opt.c_cc[VMIN] = bytes;
    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &opt) < 0) {
        return -3;
    }
    return 0;
}

int uart_get_baud_rate(int fd) {
    struct termios opt;
    unsigned int baudRate;
    if (fd < 0) {
        return -1;
    }
    if (tcgetattr(fd, &opt) < 0) {
        return -2;
    }
    switch (cfgetispeed(&opt)) {
        case B0: baudRate = 0; break;
        case B50: baudRate = 50; break;
        case B75: baudRate = 75; break;
        case B110: baudRate = 110; break;
        case B134: baudRate = 134; break;
        case B150: baudRate = 150; break;
        case B200: baudRate = 200; break;
        case B300: baudRate = 300; break;
        case B600: baudRate = 600; break;
        case B1200: baudRate = 1200; break;
        case B1800: baudRate = 1800; break;
        case B2400: baudRate = 2400; break;
        case B4800: baudRate = 4800; break;
        case B9600: baudRate = 9600; break;
        case B19200: baudRate = 19200; break;
        case B38400: baudRate = 38400; break;
        case B57600: baudRate = 57600; break;
        case B115200: baudRate = 115200; break;
        case B230400: baudRate = 230400; break;
        case B460800: baudRate = 460800; break;
        case B500000: baudRate = 500000; break;
        case B576000: baudRate = 576000; break;
        case B921600: baudRate = 921600; break;
        case B1000000: baudRate = 1000000; break;
        case B1152000: baudRate = 1152000; break;
        case B1500000: baudRate = 1500000; break;
        case B2000000: baudRate = 2000000; break;
        case B2500000: baudRate = 2500000; break;
        case B3000000: baudRate = 3000000; break;
        case B3500000: baudRate = 3500000; break;
        case B4000000: baudRate = 4000000; break;
        default: return -3;
    }
    return baudRate;
}

int uart_get_data_bits(int fd) {
    struct termios opt;
    if (fd < 0) {
        return -1;
    }
    if (tcgetattr(fd, &opt) < 0) {
        return -2;
    }
    if ((opt.c_cflag & ~CS5) == (opt.c_cflag & ~CSIZE)) {
        return 5;
    } else if ((opt.c_cflag & ~CS6) == (opt.c_cflag & ~CSIZE)) {
        return 6;
    } else if ((opt.c_cflag & ~CS7) == (opt.c_cflag & ~CSIZE)) {
        return 7;
    } else if ((opt.c_cflag & ~CS8) == (opt.c_cflag & ~CSIZE)) {
        return 8;
    }
    return -3;
}

char uart_get_parity(int fd) {
    struct termios opt;
    if (fd < 0) {
        return -1;
    }
    if (tcgetattr(fd, &opt) < 0) {
        return -2;
    }
    if (PARENB == (opt.c_cflag & PARENB)) {         /* 使能校验位 */
        if (PARODD == (opt.c_cflag & PARODD)) {     /* 使用奇校验而不使用偶校验 */
            return 'O';
        }
        return 'E';
    }
    if (CSTOPB == (opt.c_cflag & CSTOPB)) {         /* 使用2个停止位 */
        return 'M';
    }
    return 'N';
}

int uart_get_stop_bits(int fd) {
    struct termios opt;
    if (fd < 0) {
        return -1;
    }
    if (tcgetattr(fd, &opt) < 0) {
        return -2;
    }
    if (CSTOPB == (opt.c_cflag & CSTOPB)) {
        return 2;
    }
    return 1;
}

int uart_read(int fd, unsigned char* buf, unsigned int bufLen) {
    int len;
    if (fd < 0) {
        return -1;
    }
    if (!buf || 0 == bufLen) {
        return -2;
    }
    memset(buf, 0, bufLen);
    len = read(fd, buf, bufLen);
    return (len < 0 ? 0 : len);
}

int uart_write(int fd, const unsigned char* buf, unsigned int bufLen) {
    int len;
    if (fd < 0) {
        return -1;
    }
    if (!buf || 0 == bufLen) {
        return -2;
    }
    len = write(fd, buf, bufLen);
    return (len < 0 ? 0 : len);
}
