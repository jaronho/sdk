/**********************************************************************
* Author:	jaron.ho
* Date:		2020-03-30
* Brief:	uart operator
**********************************************************************/
#ifndef _UART_OP_H_
#define _UART_OP_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Brief:	open uart device
 * Param:	device - uart device name, e.g. "/dev/ttyS0"
 *          mode - uart device open mode, e.g. O_RDWR
 * Return:	int, >=0.ok, -1.open failed, -2.device is error
 */
extern int uart_open(const char* device, int mode);

/*
 * Brief:	close uart device
 * Param:	fd - uart device file handler
 * Return:	int, 0.ok, -1.fd is error
 */
extern int uart_close(int fd);

/*
 * Brief:	get baud rate
 * Param:	fd - a uart device
 * Return:	int, [0,50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400,
 *                57600,115200,230400,460800,500000,576000,921600,1000000,1152000,
 *                1500000,2000000,2500000,3000000,3500000,4000000].ok, -1.fd is error, -2.fail
 */
extern int uart_get_baud_rate(int fd);

/*
 * Brief:	set baud rate
 * Param:	fd - a uart device
 *          baudRate - 0,50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400,
 *                     57600,115200,230400,460800,500000,576000,921600,1000000,1152000,
 *                     1500000,2000000,2500000,3000000,3500000,4000000
 * Return:	unsigned int, 0.ok, 1.fd is error, 2.baudRate out of range, 3.fail
 */
extern int uart_set_baud_rate(int fd, unsigned int baudRate);

/*
 * Brief:	get data bits
 * Param:	fd - a uart device
 * Return:	int, [5,6,7,8].ok, -1.fd is error, -2.fail
 */
extern int uart_get_data_bits(int fd);

/*
 * Brief:	set data bits
 * Param:	fd - a uart device
 *          bits - 5,6,7,8
 * Return:	int, 0.ok, -1.fd is error, -2.bits out of range, -3.fail
 */
extern int uart_set_data_bits(int fd, unsigned int bits);

/*
 * Brief:	get parity
 * Param:	fd - a uart device
 * Return:	int, [N="None",O="Odd",E="Even",S="Space",M="Mark"].ok, -1.fd is error, -2.fail
 */
extern char uart_get_parity(int fd);

/*
 * Brief:	set parity
 * Param:	fd - a uart device
 *          parity - N="None",O="Odd",E="Even",S="Space",M="Mark"
 * Return:	int, 0.ok, -1.fd is error, -2.parity out of range, -3.fail
 */
extern int uart_set_parity(int fd, char parity);

/*
 * Brief:	get stop bits
 * Param:	fd - a uart device
 * Return:	int, [1,2].ok, -1.fd is error, -2.fail
 */
extern int uart_get_stop_bits(int fd);

/*
 * Brief:	set stop bits
 * Param:	fd - a uart device
 *          bits - 1,2
 * Return:	int, 0.ok, -1.fd is error, -2.bits out of range, -3.fail
 */
extern int uart_set_stop_bits(int fd, int bits);

/*
 * Brief:	set wait time
 * Param:	fd - a uart device
 *          time - wait time(hundreds of milliseconds), [0, 255], e.g. 150 = 15 seconds
 * Return:	int, 0.ok, -1.fd is error, -2.bits out of range, -3.fail
 */
extern int uart_set_wait_time(int fd, unsigned char time);

/*
 * Brief:	set wait bytes
 * Param:	fd - a uart device
 *          bytes - wait bytes, [0, 255]
 * Return:	int, 0.ok, -1.fd is error, -2.bits out of range, -3.fail
 */
extern int uart_set_wait_bytes(int fd, unsigned char bytes);

/*
 * Brief:	read from uart device
 * Param:	fd - a uart device
 *          buf - read buffer
 *          bufLen - buffer length
 * Return:	int, >=0.read length, -1.fd is error, -2.buf is NULL or bufLen is empty
 */
extern int uart_read(int fd, char* buf, unsigned int bufLen);

/*
 * Brief:	write to uart device
 * Param:	fd - a uart device
 *          buf - write buffer
 *          bufLen - buffer length
 * Return:	int, >=0.write length, -1.fd is error, -2.buf is NULL or bufLen is empty
 */
extern int uart_write(int fd, const char* buf, unsigned int bufLen);

#ifdef __cplusplus
}
#endif

#endif  /* _UART_OP_H_ */
