/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  queue.h,v 1.3 2000/08/19 17:16:37 laforge Exp
 */


#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdlib.h>

typedef struct queue_element {
	struct queue_element 	*next;
	void			*data;
	ssize_t			len;
} q_element_t;

typedef struct queue {
	q_element_t *head;
	q_element_t *tail;
} queue_t;

queue_t *queue_init(void);
void queue_destroy(queue_t *q);
int queue_enqueue(queue_t *q, void *data, ssize_t len);
void *queue_dequeue(queue_t *q, ssize_t *len);
int queue_count(queue_t *q);

#define queue_empty(q)	(q->head == NULL)

#endif /* _QUEUE_H */
