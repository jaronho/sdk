/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  client.c,v 1.5 2000/08/24 18:18:48 laforge Exp
 */

/*======================================================================
 * Client descriptor code
 *======================================================================
 */

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

#include "client.h"
#include "ipq.h"
#include "pktid.h"

#define IPQMPD_CLIENT_IBUF 10240
/* an incoming verdict from one of our peers is pending */
int client_inp(ipqmpd_peer_t *peer)
{
	unsigned char buf[IPQMPD_CLIENT_IBUF];
	size_t len;
	ipqmpd_verdict_msg_t *vmsg, *vmsgk;

	len = recv(peer->fd, &buf, sizeof(buf), 0);
	if (len == 0) {
		DEBUGP("client_inp: EOF on fd of peer %s\n", peer->addr);
		close(peer->fd);
		close(peer->ctlfd);
		peer->fd = 0;
		peer->state = PEER_STATE_NONE;
		return -ERREOF;
	}

	vmsg = (ipqmpd_verdict_msg_t *) buf;

	DEBUGP("client_inp: received verdict from peer %s for id %lu\n",
		peer->addr, vmsg->id);
	
	if (untrack_pktid(peer, vmsg->id)) {
		ipqmpd_error("client_inp: wrong pktid %lu from %s\n",
				vmsg->id, peer->addr);
		/* FIXME: send ctlmsg_pktid to client */
		return -ERRPKTID;
	}

	vmsgk = (ipqmpd_verdict_msg_t *) malloc(len);
	if (!vmsgk) {
		ipqmpd_error("client_inp: OOM malloc(%d)\n", len);
		return -ERROOM;
	}
	memcpy(vmsgk, vmsg, len);
	if(queue_enqueue(kernelq, vmsgk, len))
	{
		ipqmpd_error("client_inp: enqueue failed\n");
		return -ERRENQ;
	}

	return 0;
}

/* we have enqueued packets for this peer, send them out */
int client_outp(ipqmpd_peer_t *peer)
{
	size_t size;
	int ret;
	unsigned char *buf;

	buf = queue_dequeue(peer->out_q, &size);
	if (!buf) {
		ipqmpd_error("client_outp called, but queue empty\n");
		return -ERRQEMP;
	}
	DEBUGP("client_oupt: sending %d bytes to %s\n", size, peer->addr);

	ret = send(peer->fd, buf, size, 0);
	if (ret != size) {
		ipqmpd_error("client_outp: ret != size: %d %d\n", ret, size);
		if (ret == -1)
			ipqmpd_error("client_outp: %s\n", strerror(errno));

		/* FIXME: we were unable to send the packet out, so we have to
		 * re-enqueue it and re-track it */

		return -ERRSEND;
	}
	free(buf);

	return 0;
}

