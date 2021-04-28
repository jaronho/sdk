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
 * Brief:	open uart device(default read block)
 * Param:	device - uart device name, e.g. "/dev/ttyS0"
 *          mode - uart device open mode, e.g. O_RDWR
 * Return:	int(fd), >=0.ok, -1.open failed, -2.device invalid
 */
extern int uart_open(const char* device, int mode);

/*
 * Brief:	close uart device
 * Param:	fd - uart device file handler
 * Return:	int, 0.ok, -1.fd invalid
 */
extern int uart_close(int fd);

/*
 * Brief:	config uart device
 * Param:	fd - uart device file handler
 *          baudRate - 0,50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400,
 *                     57600,115200,230400,460800,500000,576000,921600,1000000,1152000,
 *                     1500000,2000000,2500000,3000000,3500000,4000000
 *          dataBits - 5,6,7,8
 *          parity - 'N'=None,'O'=Odd,'E'=Even,'S'=Space,'M'=Mark
 *          stopBits - 1,2
 *          flowCtrl - 0=no,1=hard,2=soft
 * Return:	int, 0.ok, -1.fd invalid, -2.get attr fail, -3.set io speed fail, -4.set attr fail
 */
extern int uart_config(int fd, int baudRate, int dataBits, char parity, int stopBits, int flowCtrl);

/*
 * Brief:	set whether none block
 * Param:	fd - uart device file handler
 *          nonblock - 0.block,1.none block
 * Return:	int, 0.ok, -1.fd invalid, -2.set fail
 */
extern int uart_set_nonblock(int fd, int nonblock);

/*
 * Brief:	set wait time
 * Param:	fd - uart device file handler
 *          time - wait time(hundreds of milliseconds), [0, 255], e.g. 150 = 15 seconds
 * Return:	int, 0.ok, -1.fd invalid, -2.get attr fail, -3.set attr fail
 */
extern int uart_set_wait_time(int fd, unsigned char time);

/*
 * Brief:	set wait bytes
 * Param:	fd - uart device file handler
 *          bytes - wait bytes, [0, 255]
 * Return:	int, 0.ok, -1.fd invalid, -2.get attr fail, -3.set attr fail
 */
extern int uart_set_wait_bytes(int fd, unsigned char bytes);

/*
 * Brief:	get baud rate
 * Param:	fd - uart device file handler
 * Return:	int, [0,50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400,
 *                57600,115200,230400,460800,500000,576000,921600,1000000,1152000,
 *                1500000,2000000,2500000,3000000,3500000,4000000].ok, -1.fd invalid, -2.get attr fail, -3.invalid
 */
extern int uart_get_baud_rate(int fd);

/*
 * Brief:	get data bits
 * Param:	fd - uart device file handler
 * Return:	int, [5,6,7,8].ok, -1.fd invalid, -2.get attr fail, -3.invalid
 */
extern int uart_get_data_bits(int fd);

/*
 * Brief:	get parity
 * Param:	fd - uart device file handler
 * Return:	int, ['N'=None,'O'=Odd,'E'=Even,'S'=Space,'M'=Mark].ok, -1.fd invalid, -2.get attr fail
 */
extern char uart_get_parity(int fd);

/*
 * Brief:	get stop bits
 * Param:	fd - uart device file handler
 * Return:	int, [1,2].ok, -1.fd invalid, -2.get attr fail
 */
extern int uart_get_stop_bits(int fd);

/*
 * Brief:	read from uart device
 * Param:	fd - uart device file handler
 *          buf - read buffer
 *          bufLen - buffer length
 * Return:	int, >=0.read length, -1.fd invalid, -2.buf or bufLen invalid
 */
extern int uart_read(int fd, unsigned char* buf, unsigned int bufLen);

/*
 * Brief:	write to uart device
 * Param:	fd - uart device file handler
 *          buf - write buffer
 *          bufLen - buffer length
 * Return:	int, >=0.write length, -1.fd invalid, -2.buf or bufLen invalid
 */
extern int uart_write(int fd, const unsigned char* buf, unsigned int bufLen);

#ifdef __cplusplus
}
#endif

#endif  /* _UART_OP_H_ */
