/*
 * libipqmpd.c
 *
 * IPQMPD library, (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 * Based on libipq from James Morris
 *
 * Designed to be compatible to libipq from James Morris
 *
 * Please note that this library is still developmental, and there may
 * be some API changes.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <libipqmpd/libipqmpd.h>
#include <linux/netfilter.h>

#include "ipqmpd.h"
#include "ctl.h"
#include "client.h"
#include "ctlmsg.h"

/****************************************************************************
 *
 * Private interface
 *
 ****************************************************************************/

enum {
	IPQ_ERR_NONE = 0,
	IPQ_ERR_IMPL,
	IPQ_ERR_HANDLE,
	IPQ_ERR_SOCKET,
	IPQ_ERR_BIND,
	IPQ_ERR_BUFFER,
	IPQ_ERR_RECV,
	IPQ_ERR_NLEOF,
	IPQ_ERR_ADDRLEN,
	IPQ_ERR_STRUNC,
	IPQ_ERR_RTRUNC,
	IPQ_ERR_NLRECV,
	IPQ_ERR_SEND,
	IPQ_ERR_SUPP,
	IPQ_ERR_FIXME,
	IPQ_ERR_PERM,
	IPQ_ERR_MARK,
	IPQ_ERR_PKTID,
	IPQ_ERR_RECVBUF
};
#define IPQ_MAXERR IPQ_ERR_RECVBUF

struct ipq_errmap_t {
	int errcode;
	char *message;
} ipq_errmap[] = {
	{ IPQ_ERR_NONE, "Unknown error" },
	{ IPQ_ERR_IMPL, "Implementation error" },
	{ IPQ_ERR_HANDLE, "Unable to create netlink handle" },
	{ IPQ_ERR_SOCKET, "Unable to create netlink socket" },
	{ IPQ_ERR_BIND, "Unable to bind netlink socket" },
	{ IPQ_ERR_BUFFER, "Unable to allocate buffer" },
	{ IPQ_ERR_RECV, "Failed to receive netlink message" },
	{ IPQ_ERR_NLEOF, "Received EOF on netlink socket" },
	{ IPQ_ERR_ADDRLEN, "Invalid peer address length" },
	{ IPQ_ERR_STRUNC, "Sent message truncated" },
	{ IPQ_ERR_RTRUNC, "Received message truncated" },
	{ IPQ_ERR_NLRECV, "Received error from netlink" },
	{ IPQ_ERR_SEND, "Failed to send netlink message" },
	{ IPQ_ERR_SUPP, "Operation not supported" },
	{ IPQ_ERR_FIXME, "FIXME!!!!" },
	{ IPQ_ERR_PERM, "Permission denied" },
	{ IPQ_ERR_MARK, "Cannot register for mark, already in use" },
	{ IPQ_ERR_PKTID, "Verdict for wrong packet ID" },
	{ IPQ_ERR_RECVBUF, "Receive buffer size invalid" }
};

static int ipq_errno = IPQ_ERR_NONE;

static ssize_t ipqmpd_recv(const struct ipq_handle *h,
                           unsigned char *buf, size_t len);

static char *ipq_strerror(int errcode);

static ssize_t ipqmpd_recv(const struct ipq_handle *h,
                                    unsigned char *buf, size_t len)
{
	int status;
	struct nlmsghdr *nlh;

	if (len < sizeof(struct nlmsgerr)) {
		ipq_errno = IPQ_ERR_RECVBUF;
		return -1;
	}

	status = recv(h->fd, buf, len, 0);

	if (status < 0) {
		ipq_errno = IPQ_ERR_RECV;
		return status;
	}

	if (status == 0) {
		ipq_errno = IPQ_ERR_NLEOF;
		return -1;
	}

	nlh = (struct nlmsghdr *)buf;
	if (nlh->nlmsg_flags & MSG_TRUNC || nlh->nlmsg_len > status) {
		ipq_errno = IPQ_ERR_RTRUNC;
		return -1;
	}
	return status;
}

static char *ipq_strerror(int errcode)
{
	if (errcode < 0 || errcode > IPQ_MAXERR)
		errcode = IPQ_ERR_IMPL;
	return ipq_errmap[errcode].message;
}

/* demux of control messages. Returns -1 on error, -2 if msg is regack */
static int _ipq_handle_ctl_msg(char *buf, int len)
{
	ipqmpd_ctl_msg_t *msg;
	struct ipqmpd_ctl_msg_error *errmsg;

	if (len < sizeof (ipqmpd_ctl_msg_t)) {
		ipq_errno = IPQ_ERR_RTRUNC;
		return -1;
	}
	msg = (ipqmpd_ctl_msg_t *) buf;

	if (ipqmpd_ctlmsg_verify(msg)) {
		LDEBUG("_ipq_handle_ctl_msg: magic in ctlmsg wrong\n");
		ipq_errno = IPQ_ERR_FIXME;
		return -1;
	}

	if (len < sizeof(ipqmpd_ctl_msg_t) + msg->len) {
		ipq_errno = IPQ_ERR_RTRUNC;
		return -1;
	}

	switch (msg->type) {
		case IPQMPD_MSG_REGACK:
			/* FIXME: impement regack here */
				return -2;
			break;

		case IPQMPD_MSG_ERROR:
			errmsg = (struct ipqmpd_ctl_msg_error *) msg->payload;
			switch (errmsg->error) {
				case -ERRPERM:
					ipq_errno = IPQ_ERR_PERM;
					break;

				case -ERRMARK:
					ipq_errno = IPQ_ERR_MARK;
					break;

				case -ERRPKTID:
					ipq_errno = IPQ_ERR_PKTID;
					break;

				case -ERRSOCK:
				case -ERRBIND:
				case -ERRCONN:
				case -ERRFCNTL:
				case -ERROOM:
				case -ERRENQ:
					ipq_errno = IPQ_ERR_SOCKET;
					break;
		
				default:	
					ipq_errno = IPQ_ERR_NONE;
					break;
			}

			return -1;
			break;

		default:
			LDEBUG("_ipq_handle_ctl_msg: unknown message\n");
			ipq_errno = IPQ_ERR_IMPL;
			return -1;
			break;
	}

}

/* waits for acknowledgement of our register request on ctlfd */
static int _ipq_wait4regack(int fd)
{
	ipqmpd_ctl_msg_t msg;
	int ret;

	while (1) {
		ret = recv(fd, &msg, sizeof(msg), 0);
		if (ret == 0) {
			ipq_errno = IPQ_ERR_FIXME;
			return 1;
		}
		if (_ipq_handle_ctl_msg((char *)&msg, ret) == -2) 
			return 0;
	}
}

static int _ipq_send_register(int fd, u_int32_t flags, unsigned long mark)
{
	size_t len = sizeof(ipqmpd_ctl_msg_t)+
		sizeof(struct ipqmpd_ctl_msg_register);
	char buf[len];
	ipqmpd_ctl_msg_t *msg;
	struct ipqmpd_ctl_msg_register *regmsg;

	struct sockaddr_un local, remote;
	int datafd, ret;

	msg = (ipqmpd_ctl_msg_t *) buf;
	ipqmpd_ctlmsg_prepare(msg, IPQMPD_MSG_REGISTER, 
			sizeof(struct ipqmpd_ctl_msg_register));

	regmsg = (struct ipqmpd_ctl_msg_register *) msg->payload;
	regmsg->flags = flags;
	regmsg->mark = mark;
	sprintf(regmsg->addr, "ipqmpd_peer_%d", getpid());

	memset(&local, 0, sizeof(local));
	memset(&remote, 0, sizeof(remote));
	local.sun_family = AF_UNIX;
	remote.sun_family = AF_UNIX;
	sprintf(local.sun_path, IPQMPD_PEER_FORMAT, regmsg->addr);
	sprintf(remote.sun_path, IPQMPD_DAEMON_FORMAT, regmsg->addr);
	local.sun_path[0] = '\0';
	remote.sun_path[0] = '\0';

	datafd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (datafd < 0) {
		close(datafd);
		ipq_errno = IPQ_ERR_FIXME;
		return -1;
	}

	if(bind(datafd, &local, sizeof(local))) {
		close(datafd);
		ipq_errno = IPQ_ERR_FIXME;
		return -1;
	}

	ret = send(fd, msg, len, 0);
	if (ret != len) {
		close(datafd);
		ipq_errno = IPQ_ERR_FIXME;
		return -1;
	}

	if (_ipq_wait4regack(fd)) {
		close(datafd);
		/* do not send ipq_errno, wait4regack already did */
		return -1;
	}

	if(connect(datafd, &remote, sizeof(remote))) {
		close(datafd);
		ipq_errno = IPQ_ERR_FIXME;
		return -1;
	}

	return datafd;	
}

/* send goodbye to ipqmpd */
static int _ipq_send_goodbye(int fd)
{
	ipqmpd_ctl_msg_t msg;

	ipqmpd_ctlmsg_prepare(&msg, IPQMPD_MSG_GOODBYE, 0);

	if (send(fd, &msg, sizeof(msg),0) != sizeof(msg))
		return -1;

	return 0;
}


/****************************************************************************
 *
 * Public interface
 *
 ****************************************************************************/

/*
 * Create and initialise an ipq handle.
 * FIXME: implement flags.
 */
struct ipq_handle *ipq_create_handle(u_int32_t flags, unsigned long mark)
{
	struct ipq_handle *h;

	struct sockaddr_un ad;

	h = (struct ipq_handle *)malloc(sizeof(struct ipq_handle));
	if (h == NULL) {
		ipq_errno = IPQ_ERR_HANDLE;
		return NULL;
	}
	memset(h, 0, sizeof(struct ipq_handle));

	/* control socket */
	memset(&ad, 0, sizeof(ad));
	ad.sun_family = AF_UNIX;
	memcpy(ad.sun_path, IPQMPD_ADDR, IPQMPD_ADDR_LEN);
	h->ctlfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (connect(h->ctlfd, &ad, sizeof(ad))) {
		ipq_errno = IPQ_ERR_SOCKET;
		close(h->ctlfd);
		free(h);
		return NULL;
	}

	h->fd = _ipq_send_register(h->ctlfd, flags, mark);

	if (h->fd < 0) {
		/* don't set ipq_errno here, ipq_send_register already did */
		close(h->ctlfd);
		free(h);
		return NULL;
	}
	h->mark = mark;
	return h;
}

/*
 * No error condition is checked here at this stage, but it may happen
 * if/when reliable messaging is implemented.
 */
int ipq_destroy_handle(struct ipq_handle *h)
{
	if (h) {
		_ipq_send_goodbye(h->ctlfd);
		close(h->ctlfd);
		close(h->fd);
		free(h);
	}
	return 0;
}

int ipq_set_mode(const struct ipq_handle *h,
                 u_int8_t mode, size_t range)
{
	struct {
		ipqmpd_ctl_msg_t msg;
		struct ipqmpd_ctl_msg_setmode sm;
	} req;
	int ret;

	ipqmpd_ctlmsg_prepare(&req.msg, IPQMPD_MSG_SETMODE, 0);

	req.sm.mode = mode;
	req.sm.range = range;

	ret = send(h->ctlfd, &req, sizeof(req), 0);
	if (send == 0) {
		ipq_errno = IPQ_ERR_NLEOF;
		ret = -1;
	}

	return ret;
}

ssize_t ipq_read(const struct ipq_handle *h,
                 unsigned char *buf, size_t len, int timeout)
{
	ssize_t ret, status;
	fd_set fds;
	struct timeval tmout;
	struct timeval *tm;
	ipq_packet_msg_t *qpkt;
	
	/* only select when user specifies timeot, to save one 
	 * syscall */

	FD_ZERO(&fds);
	FD_SET(h->fd, &fds);
	FD_SET(h->ctlfd, &fds);

	if (timeout == 0)
		tm = NULL;
	else {	
		tmout.tv_sec = timeout;
		tmout.tv_usec = 0;
		tm = &tmout;
	}

	LDEBUG("ipq_read: calling select\n");
	ret = select(h->fd + 1, &fds, NULL, NULL, tm);
	if (ret < 0) {
		LDEBUG("ipq_read: select returned value < 0\n");
		ipq_errno = IPQ_ERR_RECV;
		return -1;
	}

	if (ret == 0) {
		LDEBUG("ipq_read: timeout exceeded\n");
		return 0;
	}

	if (FD_ISSET(h->ctlfd, &fds)) {
		status = recv(h->ctlfd, buf, len, 0);
		if (status == 0)  {
			ipq_errno = IPQ_ERR_NLEOF;
			return -1;  
		}

		_ipq_handle_ctl_msg(buf, len);
		/* FIXME: evaluate control messages */
	}

	if (FD_ISSET(h->fd, &fds)) {	

		ret = ipqmpd_recv(h, buf, len);
		if (ret <= 0) {
			LDEBUG("ipq_read: recv returned %d\n", ret);
			return ret;
		}

		/* check whether the packets mark equals the mark of 
		 * the handle */
		qpkt = ipq_get_packet(buf);
		if (!qpkt) {
			/* packet is empty ??? */
			LDEBUG("ipq_read: packet is empty ???\n");
			ipq_errno = IPQ_ERR_RECV;
			return -1;
		}

		LDEBUG("qpkt->mark: %lu, h->mark: %lu\n", qpkt->mark, h->mark);

		if (qpkt->mark != h->mark) {
			LDEBUG("ipq_read: packet not for us\n");
		}

		LDEBUG("qpkt->data_len = %d\n", qpkt->data_len);
	}

	return ret;
}

int ipq_message_type(const unsigned char *buf)
{
	return ((struct nlmsghdr*)buf)->nlmsg_type;
}

int ipq_get_msgerr(const unsigned char *buf)
{
	struct nlmsghdr *h = (struct nlmsghdr *)buf;
	struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
	return -err->error;
}

ipq_packet_msg_t *ipq_get_packet(const unsigned char *buf)
{
	return NLMSG_DATA((struct nlmsghdr *)(buf));
}

int ipq_set_verdict(const struct ipq_handle *h,
                    unsigned long id,
                    unsigned int verdict,
                    size_t data_len,
                    unsigned char *buf)
{
	size_t vmsglen;
	ipqmpd_verdict_msg_t *vmsg;
	int ret;

	vmsglen = sizeof(ipqmpd_verdict_msg_t) + data_len;
	vmsg = malloc(vmsglen);
	vmsg->id = id;
	vmsg->verdict = verdict;
	vmsg->data_len = data_len;
	if (data_len)
		memcpy(vmsg->payload, buf, data_len);

	ret = send(h->fd, vmsg, vmsglen, 0);

	if (ret == 0) {
		ipq_errno = IPQ_ERR_NLEOF;
		ret = -1;
	} else if (ret < vmsglen) {
		ipq_errno = IPQ_ERR_FIXME;
		ret = -1;
	}	

	free(vmsg);
	return ret;
}

/* Not implemented yet */
int ipq_ctl(const struct ipq_handle *h, int request, ...)
{
	return 1;
}

void ipq_perror(const char *s)
{
	if (s)
		fputs(s, stderr);
	else
		fputs("ERROR", stderr);
	if (ipq_errno)
		fprintf(stderr, ": %s", ipq_strerror(ipq_errno));
	if (errno)
		fprintf(stderr, ": %s", strerror(errno));
	fputc('\n', stderr);
}
