/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  pktid.h,v 1.3 2000/08/24 13:36:17 laforge Exp
 */

#ifndef _PKTID_H
#define _PKTID_H

int track_pktid(ipqmpd_peer_t *peer, unsigned long pktid);
int untrack_pktid(ipqmpd_peer_t *peer, unsigned long pktid);

#endif /* _PKTID_H */
