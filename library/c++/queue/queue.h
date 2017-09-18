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
} queue_t;

extern queue_t* queue_create(unsigned long capacity, int block);

extern int queue_destroy(queue_t* q);

extern unsigned long queue_capacity(queue_t* q);

extern unsigned long queue_length(queue_t* q);

extern int queue_put(queue_t* q, void* data);

extern void* queue_get(queue_t* q);

#ifdef __cplusplus
}
#endif

#endif	// _QUEUE_H_
