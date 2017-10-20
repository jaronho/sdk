#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

#define QUEUE_EMPTY		0
#define QUEUE_NORMAL	1
#define QUEUE_FULL		2

static void* getqueue(queue_st* q) {
	void* retval;
	if (NULL == q || QUEUE_EMPTY == q->state) {
		return NULL;
	}
	retval = q->buf[q->bottom];
	q->buf[q->bottom] = NULL;
	++q->bottom;
	--q->length;
	if (q->bottom == q->capacity) {
		q->bottom = 0;
	}
	if (q->bottom == q->top) {
		q->state = QUEUE_EMPTY;
	}
	return retval;
}

queue_st* queue_create(unsigned long capacity, int loop, int block) {
    queue_st* q;
	if (capacity <= 0) {
		return NULL;
	}
    q = (queue_st*)malloc(sizeof(*q));
	q->capacity = capacity;
    q->bottom = 0;
    q->top = 0;
    q->state = QUEUE_EMPTY;
	q->loop = loop;
	q->block = block;
    q->buf = malloc(capacity * sizeof(void*));
#if QUEUE_THREAD_SAFETY
	sem_init(&q->sem, 0, 1);
    pthread_mutex_init(&q->mutex, NULL);
	pthread_cond_init(&q->cond, NULL);
#endif
    return q;
}

int queue_destroy(queue_st* q) {
	if (NULL == q) {
		return 1;
	}
#if QUEUE_THREAD_SAFETY
	sem_destroy(&q->sem);
    pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->cond);
#endif
    free(q->buf);
    free(q);
	return 0;
}

unsigned long queue_capacity(queue_st* q) {
	if (NULL == q) {
		return 0;
	}
    return q->capacity;
}

unsigned long queue_length(queue_st* q) {
	unsigned long length;
	if (NULL == q) {
		return 0;
	}
#if QUEUE_THREAD_SAFETY
	pthread_mutex_lock(&q->mutex);
#endif
    length = q->length;
#if QUEUE_THREAD_SAFETY
	pthread_mutex_unlock(&q->mutex);
#endif
	return length;
}

int queue_put(queue_st* q, void* data) {
	void* retval = NULL;
	if (NULL == q || NULL == data) {
		return 1;
	}
#if QUEUE_THREAD_SAFETY
	sem_wait(&q->sem);
#endif
	int prev_state = q->state;
	if (QUEUE_FULL == prev_state) {
		if (!q->loop) {
		#if QUEUE_THREAD_SAFETY
			sem_post(&q->sem);
		#endif
			return 2;
		}
		retval = getqueue(q);
		if (NULL != retval) {
			free(retval);
		}
	}
	q->buf[q->top] = data;
	++q->top;
	++q->length;
	q->state = QUEUE_NORMAL;
	if (q->top == q->capacity) {
		q->top = 0;
	}
	if (q->top == q->bottom) {
		q->state = QUEUE_FULL;
	}
#if QUEUE_THREAD_SAFETY
	pthread_mutex_lock(&q->mutex);
	if (QUEUE_EMPTY == prev_state && q->block) {
		pthread_cond_signal(&q->cond);
	}
	pthread_mutex_unlock(&q->mutex);
#endif
#if QUEUE_THREAD_SAFETY
	sem_post(&q->sem);
#endif
	return 0;
}

void* queue_get(queue_st* q) {
    void* retval;
	if (NULL == q) {
		return NULL;
	}
#if QUEUE_THREAD_SAFETY
	pthread_mutex_lock(&q->mutex);
	if (QUEUE_EMPTY == q->state && q->block) {
		pthread_cond_wait(&q->cond, &q->mutex);
	}
	pthread_mutex_unlock(&q->mutex);
#endif
#if QUEUE_THREAD_SAFETY
	sem_wait(&q->sem);
#endif
	retval = getqueue(q);
#if QUEUE_THREAD_SAFETY
	sem_post(&q->sem);
#endif
	return retval;
}
