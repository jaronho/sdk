/**********************************************************************
* Author:	jaron.ho
* Date:		2020-03-26
* Brief:	spi operator
**********************************************************************/
#ifndef _SPI_OP_H_
#define _SPI_OP_H_

#ifdef __cplusplus
extern "C"
{
#endif

struct spi_st;

/*
 * Brief:	open a spi device
 * Param:	device - spi device name, e.g. "/dev/spidev0.0"
 *			mode - device open mode
 * Return:	spi_st*
 */
extern spi_st* spi_open(const char* device, int mode);

/*
 * Brief:	close spi device
 * Param:	sst - a spi device
 * Return:	int, 0.ok, 1.fail
 */
extern int spi_close(spi_st* sst);

/*
 * Brief:	set spi mode
 * Param:	sst - a spi device
 *          mode - spi mode
 * Return:	int, 0.ok, 1.sst is NULL, 2.fail
 */
extern int spi_set_mode(spi_st* sst, unsigned char mode);

/*
 * Brief:	set spi lsb first
 * Param:	sst - a spi device
 *          lsb - 0.msb first, 1.lsb first
 * Return:	int, 0.ok, 1.sst is NULL, 2.fail
 */
extern int spi_set_lsb_first(spi_st* sst, unsigned char lsb);

/*
 * Brief:	set spi bits per word
 * Param:	sst - a spi device
 *          bits - bits, e.g. 8
 * Return:	int, 0.ok, 1.sst is NULL, 2.fail
 */
extern int spi_set_bits_per_word(spi_st* sst, unsigned char bits);

/*
 * Brief:	set spi max speed
 * Param:	sst - a spi device
 *          speed - max speed, e.g. 2*1000*1000 = 2M
 * Return:	int, 0.ok, 1.sst is NULL, 2.fail
 */
extern int spi_set_max_speed(spi_st* sst, unsigned int speed);

/*
 * Brief:	send/recv spi message
 * Param:	sst - a spi device
 *          txBuf - send buffer, can be NULL
 *          rxBuf - recv buffer, can be NULL
 *          bufLen - tx/rx buffer length
 *          delay - microseconds
 * Return:	int, 0.ok, 1.sst is NULL, 2.fail
 */
extern int spi_message(spi_st* sst, const unsigned char* txBuf, unsigned char* rxBuf, unsigned int bufLen, unsigned short delay);

#ifdef __cplusplus
}
#endif

#endif  /* _SPI_OP_H_ */
