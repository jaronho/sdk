#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

#define EMPTY	0
#define NORMAL	1
#define FULL	2

static void* getqueue(queue_st* q) {
	void* retval;
	if (NULL == q || EMPTY == q->state) {
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
		q->state = EMPTY;
	}
	return retval;
}

queue_st* queue_create(unsigned long capacity, int closed_loop, int block) {
    queue_st* q;
	if (capacity <= 0) {
		return NULL;
	}
    q = (queue_st*)malloc(sizeof(*q));
	q->capacity = capacity;
    q->bottom = 0;
    q->top = 0;
    q->state = EMPTY;
	q->closed_loop = closed_loop;
	q->block = block;
    q->buf = malloc(capacity * sizeof(void*));
#if QUEUE_THREAD_SAFETY
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
	pthread_mutex_lock(&q->mutex);
#endif
	int prev_state = q->state;
	if (FULL == prev_state) {
		if (q->closed_loop) {
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
	q->state = NORMAL;
	if (q->top == q->capacity) {
		q->top = 0;
	}
	if (q->top == q->bottom) {
		q->state = FULL;
	}
#if QUEUE_THREAD_SAFETY
	if (q->block) {
		pthread_cond_signal(&q->cond);
	}
	pthread_mutex_unlock(&q->mutex);
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
	if (EMPTY == q->state && q->block) {
		pthread_cond_wait(&q->cond, &q->mutex);
	}
#endif
	retval = getqueue(q);
#if QUEUE_THREAD_SAFETY
	pthread_mutex_unlock(&q->mutex);
#endif
	return retval;
}
