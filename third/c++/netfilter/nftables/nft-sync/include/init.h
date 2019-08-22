#ifndef _NFT_SYNC_EVENT_H_
#define _NFT_SYNC_EVENT_H_

int nft_sync_event_init(void);
void nft_sync_event_loop(void);
void nft_sync_event_fini(void);

struct nft_sync_inst;

int tcp_server_start(struct nft_sync_inst *);
int tcp_client_start(struct nft_sync_inst *inst);

#endif
