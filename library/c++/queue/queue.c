#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

#define EMPTY	0
#define NORMAL	1
#define FULL	2

static void* getqueue(queue_st* q) {
	void* retval;
	if (NULL == q) {
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

queue_st* queue_create(unsigned long capacity, int block) {
    queue_st* q;
	if (capacity <= 0) {
		return NULL;
	}
    q = (queue_st*)malloc(sizeof(*q));
	q->capacity = capacity;
    q->bottom = 0;
    q->top = 0;
    q->state = EMPTY;
	q->block = block;
    q->buf = malloc(capacity * sizeof(void*));
	sem_init(&q->lock, 0, 1);
    pthread_mutex_init(&q->mutex, NULL);
	pthread_cond_init(&q->cond, NULL);
    return q;
}

int queue_destroy(queue_st* q) {
	if (NULL == q) {
		return 1;
	}
	sem_destroy(&q->lock);
    pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->cond);
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
    pthread_mutex_lock(&q->mutex);
    length = q->length;
    pthread_mutex_unlock(&q->mutex);
    return length;
}

int queue_put(queue_st* q, void* data) {
	void* retval = NULL;
	if (NULL == q || NULL == data) {
		return 1;
	}
	sem_wait(&q->lock);
	int prev_state = q->state;
	if (FULL == prev_state) {
		if (q->block) {
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
	pthread_mutex_lock(&q->mutex);
	if (EMPTY == prev_state) {
		pthread_cond_signal(&q->cond);
	}
	pthread_mutex_unlock(&q->mutex);
	sem_post(&q->lock);
	return 0;
}

void* queue_get(queue_st* q) {
    void* retval;
	if (NULL == q) {
		return NULL;
	}
	pthread_mutex_lock(&q->mutex);
	if (EMPTY == q->state) {
		pthread_cond_wait(&q->cond, &q->mutex);
	}
	pthread_mutex_unlock(&q->mutex);
	sem_wait(&q->lock);
	retval = getqueue(q);
	sem_post(&q->lock);
	return retval;
}
