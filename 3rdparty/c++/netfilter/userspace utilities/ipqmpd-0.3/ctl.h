/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  ctl.h,v 1.2 2000/08/21 13:17:08 laforge Exp
 */

#ifndef _CTL_H
#define _CTL_H

#include "ipqmpd.h"

/* control message types */
#define IPQMPD_MSG_REGISTER 	0x0001
#define IPQMPD_MSG_REGACK 	0x0002
#define IPQMPD_MSG_GOODBYE 	0x0003
#define IPQMPD_MSG_SETMODE	0x0004
#define IPQMPD_MSG_ERROR	0x0005

typedef struct ipqmpd_ctl_msg {
	char magic[4];
	u_int16_t type;
	u_int16_t len;
	unsigned char payload[0];
} ipqmpd_ctl_msg_t;

struct ipqmpd_ctl_msg_register {
	u_int32_t	flags;
	unsigned long 	mark;
	char		addr[PEER_ADDR_LEN];
};

struct ipqmpd_ctl_msg_setmode {
	u_int8_t	mode;
	size_t		range;
};

struct ipqmpd_ctl_msg_error {
	u_int32_t	error;
};


/* handles input from one of the established control sockets to a peer */
int ctl_inp(ipqmpd_peer_t *peer);

/* dequeues a packet from peer->out_q and sends it to peer->ctl_fd */
void ctl_outp(ipqmpd_peer_t *peer);

#endif /* _CTL_H */
