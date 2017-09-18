#include "queue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

queue_t* queue_create(unsigned int capacity, int auto_top_off) {
    queue_t* queue;
    int errno;
    assert(capacity > 0);
    queue = malloc(sizeof(*queue));
    assert(queue);
    queue->pos = 0;
    queue->length = 0;
    queue->capacity = capacity;
    queue->closed = 0;
	queue->auto_top_off = auto_top_off;
    queue->buf = malloc(capacity * sizeof(*queue->buf));
    assert(queue->buf);
    if (0 != (errno = pthread_mutex_init(&queue->mutate, NULL))) {
        fprintf(stderr, "Could not create mutex. Errno: %d\n", errno);
        exit(1);
    }
    if (0 != (errno = pthread_cond_init(&queue->cond_length, NULL))) {
        fprintf(stderr, "Could not create cond var. Errno: %d\n", errno);
        exit(1);
    }
    return queue;
}

void queue_destroy(queue_t* queue) {
    int errno;
	assert(queue);
    if (0 != (errno = pthread_mutex_destroy(&queue->mutate))) {
        fprintf(stderr, "Could not destroy mutex. Errno: %d\n", errno);
        exit(1);
    }
    if (0 != (errno = pthread_cond_destroy(&queue->cond_length))) {
        fprintf(stderr, "Could not destroy cond var. Errno: %d\n", errno);
        exit(1);
    }
    free(queue->buf);
    free(queue);
}

int queue_length(queue_t* queue) {
    int len;
	assert(queue);
    pthread_mutex_lock(&queue->mutate);
    len = queue->length;
    pthread_mutex_unlock(&queue->mutate);
    return len;
}

int queue_capacity(queue_t* queue) {
	assert(queue);
    return queue->capacity;
}

void queue_close(queue_t* queue) {
	assert(queue);
    pthread_mutex_lock(&queue->mutate);
    queue->closed = 1;
    pthread_cond_broadcast(&queue->cond_length);
    pthread_mutex_unlock(&queue->mutate);
}

void queue_put(queue_t* queue, void* item) {
	void* tmp;
	assert(queue);
	assert(item);
    pthread_mutex_lock(&queue->mutate);
    assert(!queue->closed);
    while (queue->length == queue->capacity) {
		if (queue->auto_top_off) {
			tmp = queue->buf[queue->pos];
			queue->buf[queue->pos] = NULL;
			queue->pos = (queue->pos + 1) % queue->capacity;
			queue->length--;
			free(tmp);
		} else {
			pthread_cond_wait(&queue->cond_length, &queue->mutate);
		}
	}
    assert(!queue->closed);
    assert(queue->length < queue->capacity);
    queue->buf[(queue->pos + queue->length) % queue->capacity] = item;
    queue->length++;
    pthread_cond_broadcast(&queue->cond_length);
    pthread_mutex_unlock(&queue->mutate);
}

void* queue_get(queue_t* queue) {
    void* item;
	assert(queue);
    pthread_mutex_lock(&queue->mutate);
    while (0 == queue->length) {
        /* This is a bit tricky. It is possible that the queue has been closed
         * *and* has become empty while `pthread_cond_wait` is blocking.
         * Therefore, it is necessary to always check if the queue has been
         * closed when the queue is empty, otherwise we will deadlock. */
        if (queue->closed) {
            pthread_mutex_unlock(&queue->mutate);
            return NULL;
        }
        pthread_cond_wait(&queue->cond_length, &queue->mutate);
    }
    assert(queue->length <= queue->capacity);
    assert(queue->length > 0);
    item = queue->buf[queue->pos];
    queue->buf[queue->pos] = NULL;
    queue->pos = (queue->pos + 1) % queue->capacity;
    queue->length--;
    pthread_cond_broadcast(&queue->cond_length);
    pthread_mutex_unlock(&queue->mutate);
    return item;
}
