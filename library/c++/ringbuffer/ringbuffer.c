/**********************************************************************
* Author:	jaron.ho
* Date:		2020-03-19
* Brief:	ring buffer
**********************************************************************/
#include "ringbuffer.h"
#include <stdio.h>
#include <stdlib.h>

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

#define RINGBUFFER_EMPTY        0
#define RINGBUFFER_NORMAL	    1
#define RINGBUFFER_FULL		    2

static int getringbuffer(ringbuffer_st* ring, unsigned char* data) {
	if (!ring || RINGBUFFER_EMPTY == ring->state) {
        if (data) {
            *data = 0;
        }
		return 0;
	}
    if (data) {
        *data = ring->buf[ring->bottom];
    }
	ring->buf[ring->bottom] = 0;
	++ring->bottom;
	--ring->length;
	if (ring->bottom == ring->capacity) {
		ring->bottom = 0;
	}
	if (ring->bottom == ring->top) {
		ring->state = RINGBUFFER_EMPTY;
	} else {
        ring->state = RINGBUFFER_NORMAL;
    }
	return 1;
}

ringbuffer_st* ringbuffer_create(unsigned long capacity, int loop, int block) {
    ringbuffer_st* ring;
	if (capacity <= 0) {
		return NULL;
	}
    ring = (ringbuffer_st*)malloc(sizeof(*ring));
	ring->capacity = capacity;
    ring->length = 0;
    ring->bottom = 0;
    ring->top = 0;
    ring->state = RINGBUFFER_EMPTY;
	ring->loop = loop;
	ring->block = block;
    ring->buf = (unsigned char*)malloc(capacity * sizeof(unsigned char));
#ifdef RINGBUFFER_THREAD_SAFETY
    pthread_mutex_init(&ring->mutex, NULL);
	pthread_cond_init(&ring->cond, NULL);
#endif
    return ring;
}

int ringbuffer_destroy(ringbuffer_st* ring) {
	if (!ring) {
		return 1;
	}
#ifdef RINGBUFFER_THREAD_SAFETY
    pthread_mutex_destroy(&ring->mutex);
	pthread_cond_destroy(&ring->cond);
#endif
    free(ring->buf);
    free(ring);
	return 0;
}

unsigned long ringbuffer_capacity(ringbuffer_st* ring) {
	if (!ring) {
		return 0;
	}
    return ring->capacity;
}

unsigned long ringbuffer_length(ringbuffer_st* ring) {
	unsigned long length;
	if (!ring) {
		return 0;
	}
#ifdef RINGBUFFER_THREAD_SAFETY
	pthread_mutex_lock(&ring->mutex);
#endif
    length = ring->length;
#ifdef RINGBUFFER_THREAD_SAFETY
	pthread_mutex_unlock(&ring->mutex);
#endif
	return length;
}

int ringbuffer_write(ringbuffer_st* ring, unsigned char data) {
	if (!ring || !data) {
		return 1;
	}
#ifdef RINGBUFFER_THREAD_SAFETY
	pthread_mutex_lock(&ring->mutex);
#endif
    while (RINGBUFFER_FULL == ring->state) {
        if (ring->loop) {
            getringbuffer(ring, NULL);
        } else {
            if (ring->block) {
            #ifdef RINGBUFFER_THREAD_SAFETY
                pthread_cond_wait(&ring->cond, &ring->mutex);
            #endif
            } else {
            #ifdef RINGBUFFER_THREAD_SAFETY
                pthread_mutex_unlock(&ring->mutex);
            #endif
                return 2;
            }
        }
    }
	ring->buf[ring->top] = data;
	++ring->top;
	++ring->length;
	ring->state = RINGBUFFER_NORMAL;
	if (ring->top == ring->capacity) {
		ring->top = 0;
	}
	if (ring->top == ring->bottom) {
		ring->state = RINGBUFFER_FULL;
	}
#ifdef RINGBUFFER_THREAD_SAFETY
    if (ring->block) {
        pthread_cond_signal(&ring->cond);
    }
	pthread_mutex_unlock(&ring->mutex);
#endif
	return 0;
}

int ringbuffer_read(ringbuffer_st* ring, unsigned char* data) {
	if (!ring) {
		return 0;
	}
    int ok = 0;
#ifdef RINGBUFFER_THREAD_SAFETY
	pthread_mutex_lock(&ring->mutex);
#endif
    while (RINGBUFFER_EMPTY == ring->state) {
        if (ring->block) {
        #ifdef RINGBUFFER_THREAD_SAFETY
            pthread_cond_wait(&ring->cond, &ring->mutex);
        #endif
        } else {
        #ifdef RINGBUFFER_THREAD_SAFETY
            pthread_mutex_unlock(&ring->mutex);
        #endif
            return 2;
        }
    }
	ok = getringbuffer(ring, data);    
#ifdef RINGBUFFER_THREAD_SAFETY
    if (ring->block) {
        pthread_cond_signal(&ring->cond);
    }
	pthread_mutex_unlock(&ring->mutex);
#endif
	return (ok ? 0 : 2);
}
