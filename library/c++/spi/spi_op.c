/**********************************************************************
* Author:	jaron.ho
* Date:		2020-03-26
* Brief:	spi operator
**********************************************************************/
#include "spi_op.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

typedef struct spi_st {
    int fd;                         /* 设备文件句柄 */
    unsigned char mode;             /* 设备对应的模式 */
    unsigned char lsb;              /* 设备传输的时候是否先传输低比特位,0.否(默认高比特位在前),1.是 */
    unsigned char bits;             /* 设备通信的字长 */
    unsigned int speed;             /* 设备通信的最大时钟频率,HZ */
} spi_st;

spi_st* spi_open(const char* device, int mode) {
    spi_st* sst;
    int fd = -1;
    if (!device || 0 == strlen(device)) {
        return NULL;
    }
    fd = open(device, mode);
    if (fd < 0) {
        return NULL;
    }
    sst = (spi_st*)malloc(sizeof(spi_st));
    sst->fd = fd;
    if (-1 == ioctl(fd, SPI_IOC_RD_MODE, &sst->mode)) {
        printf("can't get spi mode, device: %s\n", device);
    }
    if (-1 == ioctl(fd, SPI_IOC_RD_LSB_FIRST, &sst->lsb)) {
        printf("can't get spi lsb first, device: %s\n", device);
    }
    if (-1 == ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &sst->bits)) {
        printf("can't get spi bits per word, device: %s\n", device);
    }
    if (-1 == ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &sst->speed)) {
        printf("can't get spi max speed hz, device: %s\n", device);
    }
    return sst;
}

int spi_close(spi_st* sst) {
    if (!sst) {
        return 1;
    }
    close(sst->fd);
    free(sst);
    return 0;
}

int spi_set_mode(spi_st* sst, unsigned char mode) {
    if (!sst) {
        return 1;
    }
    if (-1 == ioctl(sst->fd, SPI_IOC_WR_MODE, &mode)) {
        return 2;
    }
    sst->mode = mode;
    return 0;
}

int spi_set_lsb_first(spi_st* sst, unsigned char lsb) {
    if (!sst) {
        return 1;
    }
    if (-1 == ioctl(sst->fd, SPI_IOC_WR_LSB_FIRST, &lsb)) {
        return 2;
    }
    sst->lsb = lsb;
    return 0;
}

int spi_set_bits_per_word(spi_st* sst, unsigned char bits) {
    if (!sst) {
        return 1;
    }
    if (-1 == ioctl(sst->fd, SPI_IOC_WR_BITS_PER_WORD, &bits)) {
        return 2;
    }
    sst->bits = bits;
    return 0;
}

int spi_set_max_speed(spi_st* sst, unsigned int speed) {
    if (!sst) {
        return 1;
    }
    if (-1 == ioctl(sst->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)) {
        return 2;
    }
    sst->speed = speed;
    return 0;
}

int spi_message(spi_st* sst, const unsigned char* txBuf, unsigned char* rxBuf, unsigned int bufLen, unsigned short delay) {
    struct spi_ioc_transfer tr;
    if (!sst) {
        return 1;
    }
	memset(&tr, 0, sizeof(struct spi_ioc_transfer));
    if (rxBuf) {
        memset(rxBuf, 0, bufLen);
    }
	tr.tx_buf = txBuf ? (unsigned long long)txBuf : 0;
	tr.rx_buf = rxBuf ? (unsigned long long)rxBuf : 0;
    tr.len = bufLen;
	tr.delay_usecs = delay;
	tr.speed_hz = sst->speed;
	tr.bits_per_word = sst->bits;
    if (-1 == ioctl(sst->fd, SPI_IOC_MESSAGE(1), &tr)) {
        return 2;
    }
    return 0;
}
