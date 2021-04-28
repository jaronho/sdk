/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  ipqmpd.c,v 1.14 2000/08/24 18:18:48 laforge Exp
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ipqmpd.h"
#include "ctlmsg.h"

#include "ctlfd.h"
#include "ctl.h"
#include "ipq.h"
#include "client.h"

#ifdef DEBUG
#define NO_FORK
#endif

/* does a cleanup and frees all associated data structures */
static void peer_free(ipqmpd_peer_t *p)
{
	if (p->out_q) {
		DEBUGP("deleting output queue %lu\n", p->out_q);
		queue_destroy(p->out_q);
	}
	if (p->ctl_q) {
		DEBUGP("deleting control queue %lu\n", p->ctl_q);
		queue_destroy(p->ctl_q);
	}
	if (p->fd)
		close(p->fd);
	if (p->ctlfd)
		close(p->ctlfd);

	free(p);
}

static void peer_del(ipqmpd_peer_t *p)
{
	if (p->next)
		p->next->prev = p->prev;
	if (p->prev)
		p->prev->next = p->next;
	if (p == clients)
		clients = p->next;
	peer_free(p);
}

/*======================================================================
 * MAIN CODE
 *======================================================================
 */

int logfile_open(const char* name)
{
	logfile = fopen(name, "a");
	if (!logfile) { 
		return 1;	
	}
	return 0;
}


void main_loop(void)
{
	ipqmpd_peer_t *peer, *nextpeer;
	fd_set readset, writeset;
	while (1) {

		FD_ZERO(&readset);
		FD_ZERO(&writeset);

		/* create fdsets for select */	
		for (peer = clients; peer; peer = peer->next) {
			/* only if peer authenticated we want to 
			 * communicate */
			if (peer->state >= PEER_STATE_AUTH) {
				FD_SET(peer->fd, &readset);
				if (!queue_empty(peer->out_q))
					FD_SET(peer->fd, &writeset);
			}

			/* only if peer connected we have a 
			 * ctlfd and are able to read */
			if (peer->state >= PEER_STATE_CONN) {
				FD_SET(peer->ctlfd, &readset);
				if (!queue_empty(peer->ctl_q))
					FD_SET(peer->ctlfd, &writeset);
			}
		}

		/* we are always interested in what the kernel
		 * wants to tell us */
		FD_SET(qh->fd, &readset);

		/* the same for the master control socket */
		FD_SET(ctlfd, &readset);
		
		/* if we have something for the kernel */
		if (!queue_empty(kernelq))
			FD_SET(qh->fd, &writeset);


		select(hifd + 1, &readset, &writeset, NULL, NULL);

		/* handle incoming packets from the kernel */
		if (FD_ISSET(qh->fd, &readset))
			ipq_inp();

		/* get rid of outbound verdicts */
		if (FD_ISSET(qh->fd, &writeset))
			ipq_outp();

		/* a new connection has arrived? */
		if (FD_ISSET(ctlfd, &readset))
			ctlfd_inp();

		for (peer = clients; peer; peer = peer->next) {
			/* state could have changed, so we have
			 * to recheck :( */
			if (peer->state >= PEER_STATE_CONN) {
				if (FD_ISSET(peer->ctlfd, &readset))
					ctl_inp(peer);
				if (FD_ISSET(peer->ctlfd, &writeset))
					ctl_outp(peer);
			}
			if (peer->state >= PEER_STATE_AUTH) {
				if (FD_ISSET(peer->fd, &readset))
					client_inp(peer);
				if (FD_ISSET(peer->fd, &writeset))
					client_outp(peer);
			}
		}

		/* unlink and clean up all disconnected clients,
		 * call peer_del for each of them */
		for (peer = clients; peer; peer = nextpeer) {
			nextpeer = peer->next;
			if (peer->state == PEER_STATE_DISC) {
				if (queue_empty(peer->ctl_q))
					peer->state = PEER_STATE_NONE;
			}
			if (peer->state == PEER_STATE_NONE) {
				DEBUGP("deleting stale peer %s\n", peer->addr);
				peer_del(peer);
				ipqmpd_recalc_mode();
			}
		}
	}
}


int main()
{

	/* register a queue handle with libipq */
	qh = ipq_create_handle(IPQ_FLAG_PROMISC,0);
	if (!qh)
	{
		fprintf(stderr, "can't create netlink socket\n");
		ipq_perror(NULL);
		exit(1);
	}

	/* FIXME: we are unable to set IPQ_COPY_NONE because our 
	 * kernel module doesn't understand this :( */

#if 0
	if (ipq_set_mode(qh, IPQ_COPY_NONE, 0) < 0)
	{
		ipqmpd_error("can't set netlink mode\n");
		ipq_perror(NULL);
		exit(2);
	}
#endif

	/* register the master control socket */
	ctlfd = ctlfd_register();
	if (ctlfd < 0) 
		exit(1);

	hifd = ctlfd;

	
	kernelq = queue_init();
	if (!kernelq) {
		fprintf(stderr,"main: queue_init(kernelq) failed\n");
		exit(1);
	}

#ifdef NO_FORK
	main_loop();
#else
	if (logfile_open(IPQMPD_LOGFILE))
	{
		fprintf(stderr, "unable to open logfile %s\n", IPQMPD_LOGFILE);
		exit(1);
	}
	if (fork()) {
		/* we are parent */
		exit(0);	
	} else {
		/* we are child */	
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
		
		main_loop();
	}	
#endif

	return 0;
}
