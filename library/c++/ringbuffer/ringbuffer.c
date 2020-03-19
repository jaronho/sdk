/**********************************************************************
* Author:	jaron.ho
* Date:		2020-03-19
* Brief:	ring buffer
**********************************************************************/
#include "ringbuffer.h"
#include <stdio.h>
#include <stdlib.h>

#define RINGBUFFER_EMPTY        0
#define RINGBUFFER_NORMAL	    1
#define RINGBUFFER_FULL		    2

static void* getqueue(ringbuffer_st* buff) {
	void* retval;
	if (NULL == buff || RINGBUFFER_EMPTY == buff->state) {
		return NULL;
	}
	retval = buff->buf[buff->bottom];
	buff->buf[buff->bottom] = NULL;
	++buff->bottom;
	--buff->length;
	if (buff->bottom == buff->capacity) {
		buff->bottom = 0;
	}
	if (buff->bottom == buff->top) {
		buff->state = RINGBUFFER_EMPTY;
	}
	return retval;
}

ringbuffer_st* ringbuffer_create(unsigned long capacity, int loop, int block) {
    ringbuffer_st* buff;
	if (capacity <= 0) {
		return NULL;
	}
    buff = (ringbuffer_st*)malloc(sizeof(*buff));
	buff->capacity = capacity;
    buff->bottom = 0;
    buff->top = 0;
    buff->state = RINGBUFFER_EMPTY;
	buff->loop = loop;
	buff->block = block;
    buff->buf = (void**)malloc(capacity * sizeof(void*));
#ifdef RINGBUFFER_THREAD_SAFETY
	sem_init(&buff->sem, 0, 1);
    pthread_mutex_init(&buff->mutex, NULL);
	pthread_cond_init(&buff->cond, NULL);
#endif
    return buff;
}

int ringbuffer_destroy(ringbuffer_st* buff) {
	if (NULL == buff) {
		return 1;
	}
#ifdef RINGBUFFER_THREAD_SAFETY
	sem_destroy(&buff->sem);
    pthread_mutex_destroy(&buff->mutex);
	pthread_cond_destroy(&buff->cond);
#endif
    free(buff->buf);
    free(buff);
	return 0;
}

unsigned long queue_capacity(ringbuffer_st* buff) {
	if (NULL == buff) {
		return 0;
	}
    return buff->capacity;
}

unsigned long queue_length(ringbuffer_st* buff) {
	unsigned long length;
	if (NULL == buff) {
		return 0;
	}
#ifdef RINGBUFFER_THREAD_SAFETY
	pthread_mutex_lock(&buff->mutex);
#endif
    length = buff->length;
#ifdef RINGBUFFER_THREAD_SAFETY
	pthread_mutex_unlock(&buff->mutex);
#endif
	return length;
}

int ringbuffer_write(ringbuffer_st* buff, void* data) {
	void* retval = NULL;
	if (NULL == buff || NULL == data) {
		return 1;
	}
#ifdef RINGBUFFER_THREAD_SAFETY
	sem_wait(&buff->sem);
#endif
	int prev_state = buff->state;
	if (RINGBUFFER_FULL == prev_state) {
		if (!buff->loop) {
		#ifdef RINGBUFFER_THREAD_SAFETY
			sem_post(&buff->sem);
		#endif
			return 2;
		}
		retval = getqueue(buff);
		if (NULL != retval) {
			free(retval);
		}
	}
	buff->buf[buff->top] = data;
	++buff->top;
	++buff->length;
	buff->state = RINGBUFFER_NORMAL;
	if (buff->top == buff->capacity) {
		buff->top = 0;
	}
	if (buff->top == buff->bottom) {
		buff->state = RINGBUFFER_FULL;
	}
#ifdef RINGBUFFER_THREAD_SAFETY
	pthread_mutex_lock(&buff->mutex);
	if (RINGBUFFER_EMPTY == prev_state && buff->block) {
		pthread_cond_signal(&buff->cond);
	}
	pthread_mutex_unlock(&buff->mutex);
#endif
#ifdef RINGBUFFER_THREAD_SAFETY
	sem_post(&buff->sem);
#endif
	return 0;
}

void* ringbuffer_read(ringbuffer_st* buff) {
    void* retval;
	if (NULL == buff) {
		return NULL;
	}
#ifdef RINGBUFFER_THREAD_SAFETY
	pthread_mutex_lock(&buff->mutex);
	if (RINGBUFFER_EMPTY == buff->state && buff->block) {
		pthread_cond_wait(&buff->cond, &buff->mutex);
	}
	pthread_mutex_unlock(&buff->mutex);
#endif
#ifdef RINGBUFFER_THREAD_SAFETY
	sem_wait(&buff->sem);
#endif
	retval = getqueue(buff);
#ifdef RINGBUFFER_THREAD_SAFETY
	sem_post(&buff->sem);
#endif
	return retval;
}
