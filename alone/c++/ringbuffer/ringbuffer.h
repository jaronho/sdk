/**********************************************************************
* Author:	jaron.ho
* Date:		2020-03-19
* Brief:	ring buffer
**********************************************************************/
#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef RINGBUFFER_THREAD_SAFETY
#include <pthread.h>
#endif

typedef struct ringbuffer_st {
	unsigned long capacity;
	unsigned long length;
	unsigned long bottom;
    unsigned long top;
    unsigned long state;
    int loop;
    int block;
    unsigned char* buf;
#ifdef RINGBUFFER_THREAD_SAFETY
	pthread_mutex_t mutex;
	pthread_cond_t cond;
#endif
} ringbuffer_st;

/*
 * Brief:	create a ring buffer
 * Param:	capacity - the max size of the ring buffer
 *			loop - 1.if ring buffer is full, put operate will replace and start from index 0
 *			block - 1.if in thread safe, read operate will wait until ring buffer is not empty
 * Return:	ringbuffer_st*
 */
extern ringbuffer_st* ringbuffer_create(unsigned long capacity, int loop, int block);

/*
 * Brief:	destroy a ring buffer
 * Param:	ring - a ring buffer
 * Return:	int, 0.ok, 1.fail
 */
extern int ringbuffer_destroy(ringbuffer_st* ring);

/*
 * Brief:	get the capacity of a ring buffer
 * Param:	ring - a ring buffer
 * Return:	unsigned long
 */
extern unsigned long ringbuffer_capacity(ringbuffer_st* ring);

/*
 * Brief:	get the length of a ring buffer
 * Param:	ring - a ring buffer
 * Return:	unsigned long
 */
extern unsigned long ringbuffer_length(ringbuffer_st* ring);

/*
 * Brief:	write data to a ring buffer
 * Param:	ring - a ring buffer
 *			data - data
 * Return:	int, 0.ok, 1.fail, 2.ring buffer is full
 */
extern int ringbuffer_write(ringbuffer_st* ring, unsigned char data);

/*
 * Brief:	read data from a ring buffer
 * Param:	ring - a ring buffer
 *          data - data
 * Return:	int, 0.ok, 1.fail, 2.ring buffer is empty
 */
extern int ringbuffer_read(ringbuffer_st* ring, unsigned char* data);

#ifdef __cplusplus
}
#endif

#endif	// _RINGBUFFER_H_
