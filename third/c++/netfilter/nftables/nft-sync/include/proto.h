#ifndef _NFT_SYNC_PROTO_H_
#define _NFT_SYNC_PROTO_H_

struct nft_sync_hdr {
	uint32_t	len;
	char		data[0];
};

#define NFTS_MAX_REQUEST	1024

#endif
