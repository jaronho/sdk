/**********************************************************************
* Author:	jaron.ho
* Date:		2017-10-20
* Brief:	queue
**********************************************************************/
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
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

#define QUEUE_EMPTY		0
#define QUEUE_NORMAL	1
#define QUEUE_FULL		2

static void* getqueue(queue_st* q) {
	void* data;
	if (!q || QUEUE_EMPTY == q->state) {
		return NULL;
	}
	data = q->buf[q->bottom];
	q->buf[q->bottom] = NULL;
	++q->bottom;
	--q->length;
	if (q->bottom == q->capacity) {
		q->bottom = 0;
	}
	if (q->bottom == q->top) {
		q->state = QUEUE_EMPTY;
	} else {
        q->state = QUEUE_NORMAL;
    }
	return data;
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
    q->buf = (void**)malloc(capacity * sizeof(void*));
#ifdef QUEUE_THREAD_SAFETY
    pthread_mutex_init(&q->mutex, NULL);
	pthread_cond_init(&q->cond, NULL);
#endif
    return q;
}

int queue_destroy(queue_st* q) {
	if (!q) {
		return 1;
	}
#ifdef QUEUE_THREAD_SAFETY
    pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->cond);
#endif
    free(q->buf);
    free(q);
	return 0;
}

unsigned long queue_capacity(queue_st* q) {
	if (!q) {
		return 0;
	}
    return q->capacity;
}

unsigned long queue_length(queue_st* q) {
	unsigned long length;
	if (!q) {
		return 0;
	}
#ifdef QUEUE_THREAD_SAFETY
	pthread_mutex_lock(&q->mutex);
#endif
    length = q->length;
#ifdef QUEUE_THREAD_SAFETY
	pthread_mutex_unlock(&q->mutex);
#endif
	return length;
}

int queue_put(queue_st* q, void* data) {
	void* retval;
	if (!q || !data) {
		return 1;
	}
#ifdef QUEUE_THREAD_SAFETY
	pthread_mutex_lock(&q->mutex);
#endif
    while (QUEUE_FULL == q->state) {
        if (q->loop) {
            retval = getqueue(q);
		    if (retval) {
			    free(retval);
		    }
        } else {
            if (q->block) {
            #ifdef QUEUE_THREAD_SAFETY
                pthread_cond_wait(&q->cond, &q->mutex);
            #endif
            } else {
            #ifdef QUEUE_THREAD_SAFETY
                pthread_mutex_unlock(&q->mutex);
            #endif
                return 2;
            }
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
#ifdef QUEUE_THREAD_SAFETY
    if (q->block) {
        pthread_cond_signal(&q->cond);
    }
	pthread_mutex_unlock(&q->mutex);
#endif
	return 0;
}

void* queue_get(queue_st* q) {
    void* data;
	if (!q) {
		return NULL;
	}
#ifdef QUEUE_THREAD_SAFETY
	pthread_mutex_lock(&q->mutex);
#endif
    while (QUEUE_EMPTY == q->state) {
        if (q->block) {
        #ifdef QUEUE_THREAD_SAFETY
            pthread_cond_wait(&q->cond, &q->mutex);
        #endif
        } else {
        #ifdef QUEUE_THREAD_SAFETY
            pthread_mutex_unlock(&q->mutex);
        #endif
            return NULL;
        }
    }
	data = getqueue(q);
#ifdef QUEUE_THREAD_SAFETY
    if (q->block) {
        pthread_cond_signal(&q->cond);
    }
	pthread_mutex_unlock(&q->mutex);
#endif
	return data;
}
