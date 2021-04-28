/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  queue.c,v 1.5 2000/08/24 18:18:48 laforge Exp
 */

#include <stdlib.h>
#include "queue.h"

queue_t *queue_init()
{
	queue_t *q;

	q = (queue_t *) malloc(sizeof(queue_t));

	if (!q)
		return NULL;

	q->head = NULL;
	q->tail = NULL;
	return q;
}

void queue_destroy(queue_t *q)
{
	q_element_t *e, *next;

	for (e = q->head; e; e = next)
	{
		next = e->next;
		free(e);
	}
	free(q);
}

int queue_enqueue(queue_t *q, void *data, ssize_t len)
{
	q_element_t *e;

	e = (q_element_t *) malloc(sizeof(q_element_t));

	if (!e) {
		return -1;
	}

	e->data = data;
	e->len = len;
	e->next = NULL;

	if (q->tail)	
		q->tail->next = e;

	if (!q->head)
		q->head = e;

	q->tail = e;

	return 0;
}

void *queue_dequeue(queue_t *q, ssize_t *len)
{
	q_element_t *e;
	void *ptr;

	if (!q->head)
		return NULL;

	e = q->head;
	q->head = q->head->next;
	ptr = e->data;
	if (len)
		*len = e->len;

	if (e == q->tail)
		q->tail = NULL;

	free(e);

	return ptr;
}

int queue_count(queue_t *q)
{
	q_element_t *e;
	int i = 0;

	for (e = q->head; e; e = e->next) {
		i++;
	}
	return i;
}
