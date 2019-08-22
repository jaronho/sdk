#ifndef _NFT_CONFIG_H_
#define _NFT_CONFIG_H_

#include <limits.h>
#include <stdbool.h>
#include "tcp.h"
#include "fd.h"
#include "proto.h"

#include <libmnl/libmnl.h>

enum nft_sync_mode {
	NFTS_MODE_SERVER	= (1 << 0),
	NFTS_MODE_CLIENT	= (1 << 1),
};

enum nft_sync_cmd {
	NFTS_CMD_NONE		= 0,
	NFTS_CMD_FETCH,
	NFTS_CMD_MAX
};

struct nft_sync_inst {
	enum nft_sync_mode	mode;
	enum nft_sync_cmd	cmd;
	bool			stop;
	struct {
		bool		color;
		int		type;
		char		filename[PATH_MAX];
		FILE		*fd;
	} log;
	struct tcp_conf		tcp;
	struct nft_fd		tcp_client_nfd;
	struct nft_fd		tcp_server_fd;
	struct mnl_socket	*nl_query_sock;
};

extern struct nft_sync_inst nfts_inst;

int nft_sync_config_parse(const char *filename);

#endif /* _NFT_CONFIG_H_ */
