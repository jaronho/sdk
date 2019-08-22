#ifndef _NFT_SYNC_LOGGING_H_
#define _NFT_SYNC_LOGGING_H_

enum nft_sync_logging_type {
	NFTS_LOG_T_FILE		= 0,
	NFTS_LOG_T_SYSLOG,
};

enum nft_sync_logging_prio {
	NFTS_LOG_DEBUG		= 0,
	NFTS_LOG_INFO,
	NFTS_LOG_NOTICE,
	NFTS_LOG_ERROR,
	NFTS_LOG_FATAL,
	NFTS_LOG_MAX
};

struct nft_sync_inst;

int nft_sync_log_init(struct nft_sync_inst *inst);
void nft_sync_log(struct nft_sync_inst *inst, int priority,
                  const char *format, ...);
void nft_sync_log_fini(struct nft_sync_inst *inst);

#include "config.h"

#define nfts_log(prio, fmt, args...) 	\
	nft_sync_log(&nfts_inst, prio, fmt, ##args)

#endif
