/*
 * (C) 2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <event.h>
#include <stdint.h>
#include <fd.h>

static void nft_fd_cb(int _fd, short mask, void *data)
{
	struct nft_fd *nfd = data;

	nfd->cb(nfd, mask);
}

void nft_fd_setup(struct nft_fd *nfd, int fd,
		  void (*cb)(struct nft_fd *fd, uint32_t mask), void *data)
{
	/* add assertion */

	nfd->fd = fd;
	nfd->cb = cb;
	nfd->data = data;
}

void nft_fd_register(struct nft_fd *nfd, uint32_t events)
{
	unsigned short mask = events;

	/* add assertion */

	event_set(&nfd->event, nfd->fd, mask, nft_fd_cb, nfd);
	event_add(&nfd->event, NULL);
}

void nft_fd_unregister(struct nft_fd *fd)
{
	/* add assertion */
	event_del(&fd->event);
	fd->fd = -1;
}

struct nft_fd *nft_fd_alloc(void)
{
	return calloc(1, sizeof(struct nft_fd));
}

void nft_fd_free(struct nft_fd *nfd)
{
	free(nfd);
}
