/*
 * (C) 2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <event.h>
#include "timer.h"

void *nft_timer_data(struct nft_timer *timer)
{
	return timer->data;
}

static void nft_timer_callback(int fd, short mask, void *data)
{
	struct nft_timer *timer = data;

	timer->callback(timer);
}

void nft_timer_setup(struct nft_timer *timer, void (*cb)(struct nft_timer *),
		     void *data)
{
	// assert: evtimer_pending(timer->event, NULL) == 0;
	timer->callback	= cb;
}

void nft_timer_add(struct nft_timer *timer, unsigned int sec,
		   unsigned int usec)
{
	struct timeval tv = {
		.tv_sec		= sec,
		.tv_usec	= usec,
	};

	if (evtimer_pending(&timer->event, NULL))
		evtimer_del(&timer->event);

	evtimer_set(&timer->event, nft_timer_callback, timer);
	evtimer_add(&timer->event, &tv);
}

void nft_timer_del(struct nft_timer *timer)
{
	evtimer_del(&timer->event);
}
