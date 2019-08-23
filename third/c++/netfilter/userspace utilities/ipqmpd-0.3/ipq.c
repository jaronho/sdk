/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  ipq.c,v 1.4 2000/08/24 18:18:48 laforge Exp
 */

/*======================================================================
 * Kernel QUEUE handling 
 *======================================================================
 */

#include <linux/netfilter.h>
#include <libipq/libipq.h>
#include "ipq.h"
#include "ipqmpd.h"
#include "pktid.h"
#include "client.h"

#define IPQ_BUF_SIZE 10240

/* we have to copy only what this particular peer requested */
static int ipq_enqueue_for(ipqmpd_peer_t *peer, unsigned char *buf,
			size_t len)
{
	size_t cpylen;
	unsigned char *qpkt;
	ipq_packet_msg_t *qpkth;

	qpkth = ipq_get_packet((unsigned char *) buf);
	if (!qpkth) {
		ipqmpd_error("ipq_enqueue_for: ipq_get_packet = NULL\n");
		return -ERRIPQ;
	}

	if (track_pktid(peer, qpkth->packet_id))
	{
		ipqmpd_error("ipq_enqueue_for: track_pktid failed\n");
		return -ERRTRACK;
	}

	switch(peer->copy_mode) {
		case IPQ_COPY_NONE:
			return 0;
			break;
		case IPQ_COPY_META:
			cpylen = len - qpkth->data_len;
			qpkth->data_len = 0;
			break;
		case IPQ_COPY_PACKET:
			if (len > peer->copy_range) {
				cpylen = peer->copy_range;
				qpkth->data_len = peer->copy_range;
			} else
				cpylen = len;
			break;
	}

	qpkt = malloc(len);
	if (!qpkt) {
		ipqmpd_error("ipq_enqueue_for: OOM malloc %d\n", len); 
		return -ERROOM;
	}

	memcpy(qpkt, buf, len);
	queue_enqueue(peer->out_q, qpkt, len);

	return 0;
}

/* input handler for kernel queue */
int ipq_inp(void)
{
	int len, mtype, qpktlen;
	ipq_packet_msg_t *qpkth;
	unsigned char buf[IPQ_BUF_SIZE];
	ipqmpd_peer_t *peer;
	int found = 0;

	len = recv(qh->fd, (unsigned char *) &buf, IPQ_BUF_SIZE, 0);
	if (len < 0) {
		ipqmpd_error("ipq_inp: recv= %d\n", len);
		return -ERRRECV;
	}

	mtype = ipq_message_type((unsigned char *) &buf);

	/* FIXME: handle error messages */

	qpkth = ipq_get_packet((unsigned char *) &buf);
	qpktlen = sizeof(ipq_packet_msg_t) + qpkth->data_len;

	DEBUGP("ipq_inp: id=%lu,mark=%lu,timestamp=%lu,indev=%s,datalen=%d\n",
		qpkth->packet_id, qpkth->mark, qpkth->timestamp_sec, 
		qpkth->indev_name, qpkth->data_len);

	for (peer = clients; peer; peer = peer->next)
	{
		if (peer->state < PEER_STATE_AUTH)
			continue;
		if (peer->mark == qpkth->mark) {
			DEBUGP("ipq_inp: enqueuing for %s\n", peer->addr);
			if (ipq_enqueue_for(peer, buf, len) == 0)
				found = 1;
			break;
		}
	}
	if (!found) {
		DEBUGP("no peer found, returning accept\n");
		/* FIXME: we could block here, so queue_enqueue(kernelq) it */
		ipq_set_verdict(qh, qpkth->packet_id, IPQMPD_DEFAULT_VERDICT, 
				qpkth->data_len, qpkth->payload);
	}
	return 0;
}

int ipq_outp(void)
{
	ipqmpd_verdict_msg_t *vmsg;
	int ret;
	void *payload;

	vmsg = queue_dequeue(kernelq, NULL);
	if (!vmsg) {
		ipqmpd_error("ipq_outp: queue empty\n");
		return -ERRQEMP;
	}

	if (vmsg->data_len == 0)
		payload = NULL;
	else
		payload = vmsg->payload;	

	DEBUGP("ipq_outp: setting verdict id %lu: %d\n",
			vmsg->id, vmsg->verdict);
	ret = ipq_set_verdict(qh, vmsg->id, vmsg->verdict, vmsg->data_len, 
				payload);

	free(vmsg);

	if (ret < 0) {
		ipqmpd_error("ipq_outp: packet id %lu caused error\n", 
				vmsg->id);
		ipq_perror(NULL);
		return -ERRIPQ;
	}
		
	return 0;
}

static u_int8_t ipqmpd_copy_mode = 0;
static size_t ipqmpd_copy_range = 0;

/* recalculate mode and copy_range, tell kernel if it changed */
int ipqmpd_recalc_mode(void)
{
	ipqmpd_peer_t *peer;
	u_int8_t newmode = 0;
	size_t newrange = 1;
	int ret;

	for (peer = clients; peer; peer = peer->next)
	{
		if (peer->state < PEER_STATE_AUTH)
			continue;

		if (peer->copy_mode > newmode)
			newmode = peer->copy_mode;

		if (newrange != 0) {
			if (peer->copy_range == 0)
				newrange = 0;
			else if (peer->copy_range > newrange)
				newrange = peer->copy_range;
		}
	}
	if (ipqmpd_copy_mode == newmode && ipqmpd_copy_range == newrange) {
		DEBUGP("ipqmpd_recalc_mode: no change needed\n");
		return -1;
	}

	DEBUGP("ipqmpd_recalc_mode: mode %d->%d, range %d->%d\n",
		ipqmpd_copy_mode, newmode, ipqmpd_copy_range, newrange);

	ret = ipq_set_mode(qh, newmode, newrange);

	if (ret < 0) {
		ipqmpd_error("ipqmpd_recalc_mode: can't set mode %d:%d\n",
				newmode, newrange);
		return -ERRIPQ;
	}

	ipqmpd_copy_mode = newmode;
	ipqmpd_copy_range = newrange;

	return ret;
	
}

