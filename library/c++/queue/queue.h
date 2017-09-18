#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * queue_t is a thread-safe queue that has no limitation on the number of
 * threads that can call queue_put and queue_get simultaneously.
 * That is, it supports a multiple producer and multiple consumer model.
 */

/* queue_t implements a thread-safe queue using a fairly standard
 * circular buffer. */
typedef struct {
    /* An array of elements in the queue */
    void** buf;

    /* The position of the first element in the queue */
    unsigned int pos;

    /* The number of items currently in the queue.
     * When `length` = 0, queue_get will block.
     * When `length` = `capacity`, queue_put will block */
    unsigned int length;

    /* The total number of allowable items in the queue */
    unsigned int capacity;

    /* When 1, the queue has been closed. A run-time error will occur if a value is sent to a closed queue */
    int closed;
	
	/* When 1, the queue will auto replace new data to the oldest data */
	int auto_top_off;

    /* Guards the modification of `length` (a condition variable) and `pos` */
    pthread_mutex_t mutate;

    /* A condition variable that is pinged whenever `length` has changed or when the queue has been closed */
    pthread_cond_t cond_length;
} queue_t;

/* Allocates a new queue_t with a buffer size of the capacity given. */
extern queue_t* queue_create(unsigned int capacity, int auto_top_off);

/* Frees all data used to create a queue_t. It should only be called after
 * a call to queue_close to make sure all 'gets' are terminated before
 * destroying mutexes/condition variables.
 *
 * Note that the data inside the buffer is not freed. */
extern void queue_destroy(queue_t* queue);

/* Returns the current length (number of items) in the queue. */
extern int queue_length(queue_t* queue);

/* Returns the capacity of the queue. This is always equivalent to the
 * size of the initial buffer capacity. */
extern int queue_capacity(queue_t* queue);

/* Closes a queue. A closed queue cannot add any new values.
 *
 * When a queue is closed, an empty queue will always be empty.
 * Therefore, `queue_get` will return NULL and not block when
 * the queue is empty. Therefore, one can traverse the items in a queue
 * in a thread-safe manner with something like:
 *
 *  void *queue_item;
 *  while (NULL != (queue_item = queue_get(queue)))
 *      do_something_with(queue_item);
 */
extern void queue_close(queue_t* queue);

/* Adds new values to a queue (or "sends values to a consumer").
 * `queue_put` cannot be called with a queue that has been closed. If
 * it is, an assertion error will occur. 
 * If the queue is full, `queue_put` will block until it is not full,
 * in which case the value will be added to the queue. */
extern void queue_put(queue_t* queue, void* item);

/* Reads new values from a queue (or "receives values from a producer").
 * `queue_get` will block if the queue is empty until a new value has been
 * added to the queue with `queue_put`. In which case, `queue_get` will
 * return the next item in the queue.
 * `queue_get` can be safely called on a queue that has been closed (indeed,
 * this is probably necessary). If the queue is closed and not empty, the next
 * item in the queue is returned. If the queue is closed and empty, it will
 * always be empty, and therefore NULL will be returned immediately. */
extern void* queue_get(queue_t* queue);

#ifdef __cplusplus
}
#endif

#endif	// _QUEUE_H_
