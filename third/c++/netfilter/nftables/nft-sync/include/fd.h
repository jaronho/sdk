#ifndef _NFT_SYNC_FD_H_
#define _NFT_SYNC_FD_H_

#include <event.h>
#include <stdint.h>

struct nft_fd {
	struct event		event;
	void			(*cb)(struct nft_fd *, uint32_t);
	int			fd;
	void			*data;
};

void nft_fd_setup(struct nft_fd *ofd, int fd,
		  void (*cb)(struct nft_fd *fd, uint32_t mask), void *data);
void nft_fd_register(struct nft_fd *fd, uint32_t events);
void nft_fd_unregister(struct nft_fd *fd);

struct nft_fd *nft_fd_alloc(void);
void nft_fd_free(struct nft_fd *nfd);

#endif
