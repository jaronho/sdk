/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  ctlmsg.c,v 1.1 2000/08/19 17:17:57 laforge Exp
 */


#include "ctlmsg.h"

#define IPQMPD_CTL_MAGIC 	"1102"
#define IPQMPD_CTL_MAGIC_LEN	strlen(IPQMPD_CTL_MAGIC)

void ipqmpd_ctlmsg_prepare(ipqmpd_ctl_msg_t *cmsg, u_int16_t type,
				u_int16_t len)
{
	memcpy(cmsg->magic, IPQMPD_CTL_MAGIC, IPQMPD_CTL_MAGIC_LEN);
	cmsg->type = type;
	cmsg->len = len;
}

int ipqmpd_ctlmsg_verify(ipqmpd_ctl_msg_t *cmsg)
{
	return memcmp(cmsg->magic, IPQMPD_CTL_MAGIC, IPQMPD_CTL_MAGIC_LEN);
}

