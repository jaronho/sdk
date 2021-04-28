/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  pktid.c,v 1.5 2000/08/24 18:18:48 laforge Exp
 */

#include "ipqmpd.h"

/* Packet ID tracking code. 
 *
 * - to ensure that each peer can set verdicts only for the packet ID's
 *   we sent him
 *  
 * possible extension:
 * - collect statistics about how many packet ID's we never got a verdict 
 * - kill/disconnect processes which do this, return NF_DROP 
 */

/* notice pktid as sent to peer */
int track_pktid(ipqmpd_peer_t *peer, unsigned long pktid)
{
	int i;
	
	for (i = 0; i <= IPQMPD_MAX_PKTIDS; i++) {
		if (peer->pkt_ids[i].id == 0) {
			DEBUGP("track_pktid: tracking pktid %lu for peer %s\n",
				pktid, peer->addr);
			peer->pkt_ids[i].id = pktid;
			return 0;
		}	
	}

	DEBUGP("track_pktid: no free pktid slots, cannot track %lu\n",
		pktid);	

	return 1;
}

/* untrack a pktid for this client */
int untrack_pktid(ipqmpd_peer_t *peer, unsigned long pktid)
{
	int i;

	for (i = 0; i <= IPQMPD_MAX_PKTIDS; i++) {
		if (peer->pkt_ids[i].id == pktid) {
			DEBUGP("untrack_pktid: untracking pktid %lu from peer %s\n", pktid, peer->addr);
			peer->pkt_ids[i].id = 0;
			return 0;
		}
	}

	DEBUGP("untrack_pktid: peer %s has not tracked id %lu\n", 
		peer->addr, pktid);

	return 1;
}

