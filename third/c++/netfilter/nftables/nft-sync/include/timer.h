#ifndef _NFT_SYNC_TIMER_H
#define _NFT_SYNC_TIMER_H_

#include <event.h>

struct nft_timer {
	struct event event;
	void (*callback)(struct nft_timer *);
	void *data;
};

void *nft_timer_data(struct nft_timer *timer);
void nft_timer_setup(struct nft_timer *timer, void (*cb)(struct nft_timer *),
		     void *data);
void nft_timer_add(struct nft_timer *timer, unsigned int sec,
		   unsigned int usec);
void nft_timer_del(struct nft_timer *timer);

#endif
