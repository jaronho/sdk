/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  ctlmsg.h,v 1.1 2000/08/19 17:17:57 laforge Exp
 */

#ifndef _CTLMSG_H
#define _CTLMSG_H

#include "ctl.h"

/* writes magic, type and len into cmsg */
void ipqmpd_ctlmsg_prepare(ipqmpd_ctl_msg_t *cmsg, u_int16_t type,
				u_int16_t len);

/* returns 0 if cmsg is a valid ctl_msg_t */
int ipqmpd_ctlmsg_verify(ipqmpd_ctl_msg_t *cmsg);

#endif /* _CTLMSG_H */
