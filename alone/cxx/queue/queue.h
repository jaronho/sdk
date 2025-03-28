/**********************************************************************
* Author:	jaron.ho
* Date:		2017-10-20
* Brief:	queue
**********************************************************************/
#ifndef _QUEUE_H_
#define _QUEUE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef QUEUE_THREAD_SAFETY
#include <pthread.h>
#endif

typedef struct queue_st {
	unsigned long capacity;
	unsigned long length;
	unsigned long bottom;
	unsigned long top;
	int state;
	int loop;
	int block;
	void** buf;
#ifdef QUEUE_THREAD_SAFETY
	pthread_mutex_t mutex;
	pthread_cond_t cond;
#endif
} queue_st;

/*
 * Brief:	create a queue
 * Param:	capacity - the max size of the queue
 *			loop - 1.if queue is full, put operate will replace and start from index 0
 *			block - 1.if in thread safe, get operate will wait until queue is not empty
 * Return:	queue_st*
 */
extern queue_st* queue_create(unsigned long capacity, int loop, int block);

/*
 * Brief:	destroy a queue
 * Param:	q - a queue
 * Return:	int, 0.ok, 1.fail
 */
extern int queue_destroy(queue_st* q);

/*
 * Brief:	get the capacity of a queue
 * Param:	q - a queue
 * Return:	unsigned long
 */
extern unsigned long queue_capacity(queue_st* q);

/*
 * Brief:	get the length of a queue
 * Param:	q - a queue
 * Return:	unsigned long
 */
extern unsigned long queue_length(queue_st* q);

/*
 * Brief:	put data to a queue
 * Param:	q - a queue
 *			data - data
 * Return:	int, 0.ok, 1.fail, 2.queue is full
 */
extern int queue_put(queue_st* q, void* data);

/*
 * Brief:	get data from a queue
 * Param:	q - a queue
 * Return:	void*
 */
extern void* queue_get(queue_st* q);

#ifdef __cplusplus
}
#endif

#endif	// _QUEUE_H_
