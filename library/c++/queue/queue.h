#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <pthread.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
	unsigned long capacity;
	unsigned long length;
	unsigned long bottom;
	unsigned long top;
	int state;
	int block;
	void** buf;
	sem_t lock;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} queue_st;

extern queue_st* queue_create(unsigned long capacity, int block);

extern int queue_destroy(queue_st* q);

extern unsigned long queue_capacity(queue_st* q);

extern unsigned long queue_length(queue_st* q);

extern int queue_put(queue_st* q, void* data);

extern void* queue_get(queue_st* q);

#ifdef __cplusplus
}
#endif

#endif	// _QUEUE_H_
