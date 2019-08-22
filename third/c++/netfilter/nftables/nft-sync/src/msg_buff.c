/*
 * (C) 2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "msg_buff.h"

struct msg_buff {
	uint16_t		len;
	unsigned char		*head;
	unsigned char		*data;
	unsigned char		*tail;
	unsigned char		*end;

	unsigned char		_data[0];
};

struct msg_buff *msgb_alloc(uint32_t size)
{
	struct msg_buff *msgb;

	msgb = malloc(sizeof(struct msg_buff) + size);
	if (msgb == NULL)
		return NULL;

	msgb->len = 0;
	msgb->head = msgb->_data;
	msgb->data = msgb->tail = msgb->_data;
	msgb->end = msgb->_data + size;

	return msgb;
}

void msgb_free(struct msg_buff *msgb)
{
	free(msgb);
}

uint32_t msgb_size(struct msg_buff *msgb)
{
	return msgb->end - msgb->head;
}

uint32_t msgb_len(struct msg_buff *msgb)
{
	return msgb->len;
}

void *msgb_put(struct msg_buff *msgb, uint32_t len)
{
	void *data = msgb->tail;

	msgb->len += len;
	msgb->tail += len;

	return data;
}

void *msgb_pull(struct msg_buff *msgb, uint32_t len)
{
	void *ptr = msgb->data;

	if (len > msgb->len)
		return NULL;

	msgb->len -= len;
	msgb->data += len;

	return ptr;
}

unsigned char *msgb_data(struct msg_buff *msgb)
{
	return msgb->data;
}

unsigned char *msgb_tail(struct msg_buff *msgb)
{
	return msgb->tail;
}

void msgb_burp(struct msg_buff *msgb)
{
	void *data = msgb->data;
	int len = msgb->len;

	msgb->data = msgb->head;
	memcpy(msgb->data, data, len);
}
