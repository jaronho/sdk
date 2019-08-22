/*
 * (C) 2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */
#include <stdio.h>
#include <stdlib.h>
#include <event.h>
#include <signal.h>

#include "init.h"
#include "logging.h"

static int sigtype;
static struct event_base *ev_base;
static struct event sigterm_event, sigusr1_event, sigint_event;

static void sigterm_callback(int fd, short event, void *data)
{
	sigtype = SIGTERM;
}

static void sigint_callback(int fd, short event, void *data)
{
	sigtype = SIGINT;
}

static void sigusr1_callback(int fd, short event, void *data)
{
	sigtype = SIGUSR1;
}

int nft_sync_event_init(void)
{
	ev_base = event_init();
	if (ev_base == NULL)
		return -1;

	signal_set(&sigint_event, SIGINT, sigint_callback, NULL);
	signal_add(&sigint_event, NULL);
	signal_set(&sigterm_event, SIGTERM, sigterm_callback, NULL);
	signal_add(&sigterm_event, NULL);
	signal_set(&sigusr1_event, SIGUSR1, sigusr1_callback, NULL);
	signal_add(&sigusr1_event, NULL);

	return 0;
}

void nft_sync_event_loop(void)
{
	while (!sigtype && !nfts_inst.stop)
		event_loop(EVLOOP_ONCE);

	switch (sigtype) {
	case SIGINT:
		nfts_log(NFTS_LOG_NOTICE, "Received SIGINT, closing.");
		break;
	case SIGTERM:
		nfts_log(NFTS_LOG_NOTICE, "Received SIGTERM, closing.");
		break;
	case SIGUSR1:
		nfts_log(NFTS_LOG_NOTICE, "Received SIGUSR1");
		/* TODO: reload configuration file */
		break;
	default:
		nfts_log(NFTS_LOG_INFO, "Closing process");
		break;
	}
}

void nft_sync_event_fini(void)
{
	signal_del(&sigterm_event);
	signal_del(&sigusr1_event);
	signal_del(&sigint_event);
	event_base_free(ev_base);
}
