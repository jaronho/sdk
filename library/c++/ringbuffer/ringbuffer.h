/**********************************************************************
* Author:	jaron.ho
* Date:		2020-03-19
* Brief:	ring buffer
**********************************************************************/
#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#ifdef RINGBUFFER_THREAD_SAFETY
#include <pthread.h>
#include <semaphore.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ringbuffer_st {
	unsigned long capacity;
	unsigned long length;
	unsigned char* bptr;            /* begin ptr */
    unsigned char* eptr;            /* end ptr */
    unsigned char* wptr;            /* write ptr */
    unsigned char* rptr;            /* read ptr */
    int loop;
    int block;
#ifdef RINGBUFFER_THREAD_SAFETY
	sem_t sem;
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
 * Param:	buff - a ring buffer
 * Return:	int, 0.ok, 1.fail
 */
extern int ringbuffer_destroy(ringbuffer_st* buff);

/*
 * Brief:	get the capacity of a ring buffer
 * Param:	buff - a ring buffer
 * Return:	unsigned long
 */
extern unsigned long queue_capacity(ringbuffer_st* buff);

/*
 * Brief:	get the length of a ring buffer
 * Param:	buff - a ring buffer
 * Return:	unsigned long
 */
extern unsigned long queue_length(ringbuffer_st* buff);

/*
 * Brief:	write data to a ring buffer
 * Param:	buff - a ring buffer
 *			data - data
 * Return:	int, 0.ok, 1.fail, 2.ring buffer is full
 */
extern int ringbuffer_write(ringbuffer_st* buff, void* data);

/*
 * Brief:	read data from a ring buffer
 * Param:	buff - a ring buffer
 * Return:	void*
 */
extern void* ringbuffer_read(ringbuffer_st* buff);

#ifdef __cplusplus
}
#endif

#endif	// _RINGBUFFER_H_
