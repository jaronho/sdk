/*
 * (C) 2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <syslog.h>

#include "config.h"
#include "logging.h"

static struct {
	const char	*text;
	const char	*color;
} logging[NFTS_LOG_MAX] = {
	[NFTS_LOG_DEBUG] = {
		.text	= "DEBUG",
		.color	= "\033[1;33m",
	},
        [NFTS_LOG_INFO] = {
		.text	= "INFO",
		.color	= "\033[1;32m",
	},
        [NFTS_LOG_NOTICE] = {
		.text	= "NOTICE",
		.color	= "\033[1;36m",
	},
        [NFTS_LOG_ERROR] = {
		.text	= "ERROR",
		.color	= "\033[1;33m",
	},
        [NFTS_LOG_FATAL] = {
		.text	= "FATAL",
		.color	= "\033[1;31m",
	},
};

int nft_sync_log_init(struct nft_sync_inst *inst)
{
	int ret = 0;

	switch (inst->log.type) {
	case NFTS_LOG_T_SYSLOG:
		break;
	case NFTS_LOG_T_FILE:
		if (inst->log.fd == NULL)
			inst->log.fd = stdout;
		else {
			inst->log.fd = fopen(inst->log.filename, "w+");
			if (inst->log.fd == NULL)
				return -1;
		}
		break;
	}

	return ret;
}

void nft_sync_log_fini(struct nft_sync_inst *inst)
{
	switch (inst->log.type) {
	case NFTS_LOG_T_SYSLOG:
		break;
	case NFTS_LOG_T_FILE:
		if (inst->log.fd != NULL)
			fclose(inst->log.fd);
		break;
	}
}

void nft_sync_log(struct nft_sync_inst *inst, int prio,
		  const char *format, ...)
{
	time_t t;
	char *timebuf = NULL;
	va_list args;

	switch (inst->log.type) {
	case NFTS_LOG_T_FILE:
		t = time(NULL);
		timebuf = ctime(&t);
		timebuf[strlen(timebuf) - 1]='\0';
		break;
	case NFTS_LOG_T_SYSLOG:
		break;
	}

	switch (inst->log.type) {
	case NFTS_LOG_T_FILE:
		va_start(args, format);
		fprintf(inst->log.fd, "%s[%s] [%s] ",
			inst->log.color ? logging[prio].color : "", timebuf,
			logging[prio].text);
		vfprintf(inst->log.fd, format, args);
		va_end(args);
		fprintf(inst->log.fd, "%s\n",
			inst->log.color ? "\033[1;0m" : "");
		fflush(inst->log.fd);
		break;
	case NFTS_LOG_T_SYSLOG:
		va_start(args, format);
		vsyslog(prio, format, args);
		va_end(args);
		break;
	}
}
