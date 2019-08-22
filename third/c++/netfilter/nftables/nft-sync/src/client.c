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
#include <fd.h>
#include <tcp.h>
#include <unistd.h>
#include <config.h>
#include <string.h>
#include <errno.h>

#include "init.h"
#include "logging.h"
#include "msg_buff.h"
#include "proto.h"
#include "config.h"

static void print_payload(struct msg_buff *msgb)
{
	write(1, msgb_data(msgb) + sizeof(struct nft_sync_hdr),
	      msgb_len(msgb) - sizeof(struct nft_sync_hdr));
	write(1, "\n", 1);
}

static int process_response(struct msg_buff *msgb, int len)
{
	switch (nfts_inst.cmd) {
	case NFTS_CMD_NONE:
		break;
	case NFTS_CMD_FETCH:
		print_payload(msgb);
		/* We're done, stop running this process */
		nfts_inst.stop = true;
		return 0;
	/* TODO: We'll have a pull command at some point, the code to parse
	 *	 the xml/json ruleset should go here.
	 */
	default:
		break;
	}
	return -1;
}

static void tcp_client_established_cb(struct nft_fd *nfd, uint32_t mask)
{
	struct tcp_client *c = nfd->data;
	struct nft_sync_hdr *hdr;
	char buf[sizeof(struct nft_sync_hdr)];
	struct msg_buff *msgb = tcp_client_get_data(c);
	int ret, len;

	if (msgb == NULL) {
		/* Retrieve the header first to know the response length */
		ret = tcp_client_recv(c, buf, sizeof(buf));
		if (ret < 0) {
			nfts_log(NFTS_LOG_ERROR, "cannot received from socket");
			goto err1;
		} else if (ret == 0) {
			nfts_log(NFTS_LOG_ERROR,
				 "connection from server has been closed\n");
			/* FIXME retry every N seconds using a timer,
			 * otherwise this sucks up the CPU by retrying to
			 * connect very hard.
			 */
			goto err1;
		}

		hdr = (struct nft_sync_hdr *)buf;
		len = ntohl(hdr->len);

		/* Allocate a message for the entire response */
		msgb = msgb_alloc(len);
		if (msgb == NULL) {
			nfts_log(NFTS_LOG_ERROR, "OOM");
			goto err1;
		}
		memcpy(msgb_data(msgb), buf, sizeof(buf));
		msgb_put(msgb, sizeof(buf));

		/* Attach this message to the client */
		tcp_client_set_data(c, msgb);
	}

	/* Retrieve as much data as we can in this round */
	ret = tcp_client_recv(c, msgb_tail(msgb),
			      msgb_size(msgb) - msgb_len(msgb));
	if (ret < 0) {
		nfts_log(NFTS_LOG_ERROR, "cannot received from socket");
		goto err1;
	} else if (ret == 0) {
		nfts_log(NFTS_LOG_ERROR,
			 "connection from server has been closed\n");
		goto err1;
	}
	msgb_put(msgb, ret);

	/* Not enough data to process the response yet */
	if (msgb_len(msgb) < msgb_size(msgb))
		return;

	if (process_response(msgb, len) < 0) {
		nfts_log(NFTS_LOG_ERROR, "discarding malformed response");
		goto err1;
	}
	/* Detach this message from the client */
	tcp_client_set_data(c, NULL);
err1:
	msgb_free(msgb);
	close(tcp_client_get_fd(c));
	nft_fd_unregister(nfd);
	tcp_client_destroy(c);
}

static void tcp_client_connect_cb(struct nft_fd *nfd, uint32_t mask)
{
	struct nft_sync_hdr *hdr;
	struct tcp_client *c = nfd->data;
	struct msg_buff *msgb;
	int len;

	msgb = msgb_alloc(NFTS_MAX_REQUEST);
	if (msgb == NULL) {
		nfts_log(NFTS_LOG_ERROR, "OOM");
		return;
	}

	switch (nfts_inst.cmd) {
	case NFTS_CMD_FETCH:
		len = strlen("fetch") + sizeof(struct nft_sync_hdr);
		hdr = msgb_put(msgb, sizeof(struct nft_sync_hdr));
		hdr->len = htonl(len);
		memcpy(hdr->data, "fetch", strlen("fetch"));
		msgb_put(msgb, strlen("fetch"));
		break;
	default:
		nfts_log(NFTS_LOG_ERROR, "Unknown command");
		return;
	}

	if (tcp_client_send(c, msgb_data(msgb), msgb_len(msgb)) < 0) {
		nfts_log(NFTS_LOG_ERROR, "cannot send to socket: %s",
			 strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Now that we got connected, register the descriptor again to
	 * permanently listen for incoming data.
	 */
	nft_fd_setup(&nfts_inst.tcp_client_nfd, tcp_client_get_fd(c),
		     tcp_client_established_cb, c);
	nft_fd_register(nfd, EV_READ | EV_PERSIST);
}

int tcp_client_start(struct nft_sync_inst *inst)
{
	struct tcp_client *c;

	c = tcp_client_create(&inst->tcp);
	if (c == NULL) {
		fprintf(stderr, "cannot initialize TCP client\n");
		return -1;
	}

	nft_fd_setup(&inst->tcp_client_nfd, tcp_client_get_fd(c),
		     tcp_client_connect_cb, c);
	nft_fd_register(&inst->tcp_client_nfd, EV_WRITE);

	return 0;
}
