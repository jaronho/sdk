/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  client.h,v 1.2 2000/08/21 13:17:08 laforge Exp
 */

#ifndef _CLIENT_H
#define _CLIENT_H

#include "ipqmpd.h"

typedef struct ipqmpd_verdict_msg {
	unsigned long id;
	unsigned int verdict;
	size_t data_len;
	unsigned char payload[0];
} ipqmpd_verdict_msg_t;


int client_inp(ipqmpd_peer_t *peer);
int client_outp(ipqmpd_peer_t *peer);

#endif
