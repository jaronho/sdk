/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  ctl.c,v 1.3 2000/08/24 13:36:17 laforge Exp
 */

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ipqmpd.h"
#include "ctl.h"
#include "ctlmsg.h"
#include "ipq.h"

static int mark_occupied(unsigned long mark)
{
	ipqmpd_peer_t *peer;

	for (peer = clients; peer; peer = peer->next)
	{
		if (peer->mark == mark) 
			return 0;
	}
	return 1;
}

static int send_msg_error(ipqmpd_peer_t *peer, int error)
{
	ipqmpd_ctl_msg_t *msg;
	struct ipqmpd_ctl_msg_error *errmsg;
	int len;
	
	len = sizeof(ipqmpd_ctl_msg_t) + sizeof(struct ipqmpd_ctl_msg_error);

	msg = (ipqmpd_ctl_msg_t *) malloc(len);
	if (!msg) {
		ipqmpd_error("send_msg_error: OOM malloc(%d)\n", len);
		return -ERROOM;
	}

	errmsg = (struct ipqmpd_ctl_msg_error *) msg->payload;
	ipqmpd_ctlmsg_prepare(msg, IPQMPD_MSG_ERROR, 
				sizeof(struct ipqmpd_ctl_msg_error));

	errmsg->error = error;	

	if (queue_enqueue(peer->ctl_q, msg, len)) {
		ipqmpd_error("send_msg_error: enqueue failed\n");
		return -ERRENQ;
	}

	return 0;
}

static int handle_msg_register(ipqmpd_peer_t *peer, 
		struct ipqmpd_ctl_msg_register *regmsg)
{
	int ret, flags;
	struct sockaddr_un sa, ra;
	ipqmpd_ctl_msg_t *ackmsg;
	int err = 0;

	peer->flags = regmsg->flags;
	peer->copy_mode = IPQ_COPY_NONE;
	peer->copy_range = 0;
	strncpy(peer->addr, regmsg->addr, PEER_ADDR_LEN);

	DEBUGP("handle_msg_register: flags=%d, mark=%lu, addr=%s\n",
		regmsg->flags, regmsg->mark, regmsg->addr);

	if (!mark_occupied(regmsg->mark)) {
		DEBUGP("handle_msg_register: we already have mark %lu\n",
			regmsg->mark);
		peer->state = PEER_STATE_DISC;
		err = -ERRMARK;
		goto handle_msg_register_err;
	}

	peer->mark = regmsg->mark;

	memset(&ra, 0, sizeof(ra));
	memset(&sa, 0, sizeof(sa));
	ra.sun_family = AF_UNIX;
	sa.sun_family = AF_UNIX;
	sprintf(ra.sun_path, IPQMPD_PEER_FORMAT, regmsg->addr);
	sprintf(sa.sun_path, IPQMPD_DAEMON_FORMAT, regmsg->addr);
	ra.sun_path[0] = '\0';
	sa.sun_path[0] = '\0';

	peer->fd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (peer->fd < 0) {
		ipqmpd_error("handle_msg_register: socket=%d\n", peer->fd);
		err = -ERRSOCK;
		goto handle_msg_register_err;
	}

	if (peer->fd > hifd)
		hifd = peer->fd;

	ret = bind(peer->fd, &sa, sizeof(sa));
	if (ret < 0) {
		ipqmpd_error("handle_msg_register: bind=%d\n", ret);
		err = -ERRBIND;
		goto handle_msg_register_err;
	}

	ret = connect(peer->fd, &ra, sizeof(ra));
	if (ret < 0) {
		ipqmpd_error("handle_msg_register: connect=%d\n", ret);
		err = -ERRCONN;
		goto handle_msg_register_err;
	}

	flags = fcntl(peer->fd, F_GETFL);
	flags |= O_NONBLOCK;
	ret = fcntl(peer->fd, F_SETFL, flags);
	if (ret < 0) {
		ipqmpd_error("handle_msg_register: fcntl=%d\n", ret);
		err = -ERRFCNTL;
		goto handle_msg_register_err;
	}


	/* send ack to client that we bound to the addr so he can
	 * connect (instead of the clumsy timeout) */

	ackmsg = (ipqmpd_ctl_msg_t *) malloc(sizeof(ipqmpd_ctl_msg_t));
	if (!ackmsg) {
		ipqmpd_error("handle_msg_register: OOM malloc ackmsg\n");
		err = -ERROOM;
		goto handle_msg_register_err;
	}
	ipqmpd_ctlmsg_prepare(ackmsg, IPQMPD_MSG_REGACK, 0);

	if (queue_enqueue(peer->ctl_q, ackmsg, sizeof(ipqmpd_ctl_msg_t))) {
		ipqmpd_error("handle_msg_register: enqueue failed\n");
		err = -ERRENQ;
		goto handle_msg_register_err;
	}

	peer->state = PEER_STATE_REG;

	/* FIXME: Set state only to PEER_STATE_REG and have some 
	 * authentication */ 

	peer->state = PEER_STATE_AUTH;

	return 0;

handle_msg_register_err:

	send_msg_error(peer, err);
	return err;
}

static int handle_msg_setmode(ipqmpd_peer_t *peer, 
				struct ipqmpd_ctl_msg_setmode *msg)
{
	DEBUGP("handle_msg_setmode: mode=%d, range=%d, addr=%s\n",
			msg->mode, msg->range, peer->addr);

	if (peer->state < PEER_STATE_AUTH) {
		DEBUGP("handle_msg_setmode: permission denied\n");
		send_msg_error(peer, -ERRPERM);
		return -ERRPERM;
	}

	peer->copy_mode = msg->mode;
	peer->copy_range = msg->range;
	ipqmpd_recalc_mode();
	return 0;
}

/* handles input from one of the established control sockets to a peer */
int ctl_inp(ipqmpd_peer_t *peer)
{
	int ret;
	unsigned char *buf;
	ipqmpd_ctl_msg_t *msg;
	struct ipqmpd_ctl_msg_register *regmsg;

	buf = (unsigned char *) malloc(IPQMPD_CTL_IBUF);
	if (!buf) {
		ipqmpd_error("ctl_inp: OOM malloc(%d)\n", IPQMPD_CTL_IBUF);
		return -ERROOM;
	}

	ret = recv(peer->ctlfd, buf, IPQMPD_CTL_IBUF, 0);

	if (ret == 0) {
		DEBUGP("ctl_inp: EOF on fd for client %s\n", peer->addr);
		close(peer->ctlfd);
		peer->ctlfd = 0;

		/* we cannot delete peer here, because main_loop
		 * ist still iterating over the linked list */
		peer->state = PEER_STATE_NONE;
		ret = -ERREOF;
		goto ctl_inp_err;
	}
		

	msg = (ipqmpd_ctl_msg_t *) buf;
	if (ipqmpd_ctlmsg_verify(msg)) {
		ipqmpd_error("ctl_inp: magic in msg wrong\n");
		ret = -ERRMAGIC;
		goto ctl_inp_err;
	}
	switch (msg->type) {
		case IPQMPD_MSG_REGISTER:
			DEBUGP("ctl_inp: msg_register detected\n");
			
			regmsg = (struct ipqmpd_ctl_msg_register *) 
				msg->payload;
			handle_msg_register(peer, regmsg);
			break;
		case IPQMPD_MSG_GOODBYE:
			DEBUGP("ctl_inp: msg_goodbye detected\n");
			/* FIXME: implementation */

			break;
		case IPQMPD_MSG_SETMODE:
			DEBUGP("ctl_inp: msg_setmode detected\n");
			handle_msg_setmode(peer, 
					(struct ipqmpd_ctl_msg_setmode *)
					msg->payload);

			break;
		default:
			DEBUGP("ctl_inp: unknown message type %d\n",
					msg->type);
	}

	free(buf);
	return 0;

ctl_inp_err:
	free(buf);
	return ret;
}

void ctl_outp(ipqmpd_peer_t *peer)
{
	size_t len, ret;
	ipqmpd_ctl_msg_t *msg;

	msg = (ipqmpd_ctl_msg_t *) queue_dequeue(peer->ctl_q, NULL);
	if (!msg) {
		DEBUGP("ctl_outp: called for peer %s, but queue empty\n",
			peer->addr);
		return;
	}
	len = sizeof(ipqmpd_ctl_msg_t) + msg->len;

	DEBUGP("ctl_outp: sending %d bytes to %s\n", len, peer->addr);

	ret = send(peer->ctlfd, msg, len, 0);
		
	if (ret != len) {
		ipqmpd_error("ctl_outp: sent only %d of %d bytes",
				ret, len);
	}
	free(msg);
}
