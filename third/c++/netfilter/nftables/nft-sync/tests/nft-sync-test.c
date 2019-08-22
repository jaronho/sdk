#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/tcp.h"
#include "../include/proto.h"
#include "../include/msg_buff.h"

int main(void)
{
	struct tcp_client *c;
	struct tcp_conf conf = {
		.ipproto	= AF_INET,
		.port		= 1234,
		.client		= {
			.inet_addr	= { inet_addr("127.0.0.1") },
		},
	};
	struct nft_sync_hdr *hdr;
	struct msg_buff *msgb;
	char buf[1024];
	fd_set fds;

	msgb = msgb_alloc(NFTS_MAX_REQUEST);
	if (msgb == NULL) {
		perror("msgb_alloc");
		exit(EXIT_FAILURE);
	}

	hdr = msgb_put(msgb, sizeof(struct nft_sync_hdr) + strlen("fetch"));
	hdr->len = htonl(sizeof(struct nft_sync_hdr) + strlen("fetch"));
	memcpy(hdr->data, "fetch", strlen("fetch"));

	c = tcp_client_create(&conf);
	if (c == NULL) {
		fprintf(stderr, "cannot initialize TCP client\n");
		exit(EXIT_FAILURE);
	}

	FD_ZERO(&fds);
	FD_SET(tcp_client_get_fd(c), &fds);
	/* Wait for connection ... */
	select(tcp_client_get_fd(c) + 1, NULL, &fds, NULL, NULL);

	if (tcp_client_send(c, msgb_data(msgb), msgb_len(msgb)) < 0) {
		perror("cannot send to socket");
		exit(EXIT_FAILURE);
	}

	FD_ZERO(&fds);
	FD_SET(tcp_client_get_fd(c), &fds);
	/* Wait to receive data after sending request ... */
	select(tcp_client_get_fd(c) + 1, &fds, NULL, NULL, NULL);

	if (tcp_client_recv(c, buf, sizeof(buf)) < 0) {
		perror("cannot send to socket");
		exit(EXIT_FAILURE);
	}
	printf("[TEST OK] Received: %s\n", buf + sizeof(struct nft_sync_hdr));
	tcp_client_destroy(c);
}
