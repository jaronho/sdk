/*
 * (C) 2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Thanks to the NLnet Foundation <http://nlnet.nl> for making the bootstrap
 * of this project possible!
 */

#include <stdio.h>
#include <stdlib.h>
#include <fd.h>
#include <tcp.h>
#include <unistd.h>
#include <config.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include "init.h"
#include "logging.h"
#include "msg_buff.h"
#include "proto.h"
#include "mnl.h"

struct nft_sync_inst nfts_inst;

static void print_usage(const char *prog_name)
{
	fprintf(stderr,
		"%s (c) 2014 by Pablo Neira Ayuso <pablo@netfilter.org>\n"
		"Usage: %s [-h] [-c]\n"
		"	[ --help ]\n"
		"	[ --config=<FILE> ]\n"
		"	[ --fetch ]\n", prog_name, prog_name);
}

static const struct option options[] = {
	{ .name = "help",	.has_arg = false,	.val = 'h' },
	{ .name = "config",	.has_arg = false,	.val = 'c' },
	{ .name = "fetch",	.has_arg = false,	.val = 'f' },
	{ NULL },
};

#define NFT_SYNC_CONF_DEFAULT	"/etc/nft-sync.conf"

static int set_cmd(int cmd)
{
	if (nfts_inst.cmd) {
		fprintf(stderr,
			"Cannot specify multiple commands at the same time\n");
		return -1;
	}
	nfts_inst.cmd = cmd;
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE, c;
	const char *config = NFT_SYNC_CONF_DEFAULT;

	while ((c = getopt_long(argc, argv, "hc:f", options, NULL)) != -1) {
		switch (c) {
		case 'h':
			print_usage(argv[0]);
			return EXIT_SUCCESS;
		case 'c':
			config = optarg;
			break;
		case 'f':
			set_cmd(NFTS_CMD_FETCH);
			break;
		default:
			fprintf(stderr, "Unknown option -%c\n", c);
			return EXIT_FAILURE;
		}
	}

	if (nft_sync_config_parse(config) < 0)
		return EXIT_FAILURE;

	if (nft_sync_event_init() < 0) {
		fprintf(stderr, "Cannot start libev: %s\n", strerror(errno));
		goto err;
	}

	if (nft_sync_log_init(&nfts_inst) < 0) {
		fprintf(stderr, "Cannot start logging: %s\n", strerror(errno));
		goto err;
	}

	if (nfts_inst.mode & NFTS_MODE_SERVER) {
		if (tcp_server_start(&nfts_inst) < 0) {
			nfts_log(NFTS_LOG_FATAL,
				 "Cannot start TCP server: %s\n",
				 strerror(errno));
			goto err;
		}

		if (nfts_socket_open(&nfts_inst) < 0) {
			nfts_log(NFTS_LOG_FATAL,
				 "Cannot open Netlink query socket: %s\n",
				 strerror(errno));
			goto err;
		}

		nfts_log(NFTS_LOG_INFO, "listening at %s",
			 inet_ntoa(nfts_inst.tcp.server.ipv4.inet_addr));
	}

	if (nfts_inst.mode & NFTS_MODE_CLIENT) {
		if (!nfts_inst.cmd) {
			nfts_log(NFTS_LOG_FATAL,
				 "Client needs some command, eg. --fetch",
				 strerror(errno));
			goto err;
		}
		if (tcp_client_start(&nfts_inst) < 0) {
			nfts_log(NFTS_LOG_FATAL,
				 "Cannot start TCP client: %s",
				 strerror(errno));
			goto err;
		}
		nfts_log(NFTS_LOG_INFO, "connecting to %s",
			 inet_ntoa(nfts_inst.tcp.client.inet_addr));
	}

	/* TODO: add switch to allow to daemonize this process */

	nft_sync_event_loop();

	nft_sync_event_fini();

	if (nfts_inst.mode & NFTS_MODE_SERVER)
		nfts_socket_close(&nfts_inst);

	ret = EXIT_SUCCESS;
err:
	nft_sync_log_fini(&nfts_inst);

	return ret;
}
