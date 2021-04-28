/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  ipqmpd.h,v 1.12 2000/08/24 18:18:48 laforge Exp
 */

#ifndef _IPQMPD_H
#define _IPQMPD_H

#include <stdio.h>
#include <linux/netfilter.h>
#include "queue.h"

#ifdef DEBUG
#define DEBUGP(format, args...) fprintf(stderr, format, ## args)
#define ipqmpd_error(format, args...) fprintf(stderr, format, ## args)
#else
#define DEBUGP(format, args...)
#define ipqmpd_error(format, args...) fprintf(logfile, format, ## args)
#endif

#define IPQMPD_LOGFILE	"/var/log/ipqmpd.log"

/* maximum number of packet id's to track per peer */
#define IPQMPD_MAX_PKTIDS	256

/* maximumg address lenght */
#define PEER_ADDR_LEN		25

/* what to do with packets which can't be enqueued (wrong mark, queue full) */
#define IPQMPD_DEFAULT_VERDICT	NF_ACCEPT

/* some constants regarding sun_path */
#define IPQMPD_ADDR 		"\0ipqmpd"
#define IPQMPD_ADDR_LEN 	7
#define IPQMPD_PEER_FORMAT 	"X%s"
#define IPQMPD_DAEMON_FORMAT 	"XXXX%s"

/* input buffer for control socket */
#define IPQMPD_CTL_IBUF		1024

/* error types */
enum {
	ERRUNDEF = 1,
	ERROOM,		/* out of memory */
	ERRENQ,		/* error enqueuing packet */
	ERRFCNTL,	/* cannot set O_NONBLOCK */
	ERRBIND,	/* cannot bind to address */
	ERRCONN,	/* cannot connect to address */
	ERRSOCK,	/* error during socket call */
	ERREOF,		/* end of file during read */
	ERRMAGIC,	/* wrong magic in control message */
	ERRPKTID,	/* wrong packet id */
	ERRQEMP,	/* queue empty */
	ERRSEND,	/* error during send call */
	ERRTRACK,	/* error during packet tracking */
	ERRIPQ,		/* error from kernel queue */
	ERRRECV,	/* error during recv call */
	ERRMARK,	/* we already have a client for mark */
	ERRPERM,	/* permission denied */
};

/* valid states of a peer */
enum {
	PEER_STATE_NONE = 0,	/* stale, to be deleted */
	PEER_STATE_CONN,	/* ctl connected */
	PEER_STATE_DISC,	/* disconnect pending */
	PEER_STATE_REG,		/* datafd connected */
	PEER_STATE_AUTH,	/* authenticated */
};

typedef struct ipqmpd_pktid {
	unsigned long id;
} ipqmpd_pktid_t;

typedef struct ipqmpd_peer {
	struct ipqmpd_peer *prev;
	struct ipqmpd_peer *next;
	int		fd;
	int		ctlfd;
	u_int32_t	flags;
	u_int8_t	state;
	u_int8_t	copy_mode;
	size_t		copy_range;
	unsigned long	mark;
	ipqmpd_pktid_t	pkt_ids[IPQMPD_MAX_PKTIDS];
	char		addr[PEER_ADDR_LEN];
	queue_t *out_q;
	queue_t	*kernel_q;
	queue_t *ctl_q;
} ipqmpd_peer_t;

ipqmpd_peer_t *clients;

int hifd;

FILE *logfile;

#endif /* _IPQMPD_H */
