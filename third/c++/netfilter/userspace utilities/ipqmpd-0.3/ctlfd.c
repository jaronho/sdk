/* ipqmpd - ip queuing multiplex daemon
 *
 *  (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 *
 *  This code is released under the terms of GNU GPL
 *
 *  ctlfd.c,v 1.2 2000/08/21 13:17:08 laforge Exp
 */

/*====================================================================== 
 * Master Control Socket 
 *======================================================================
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ipqmpd.h"
#include "ctlfd.h"

/* generate ctlfd and bind it to our address */

int ctlfd_register(void)
{
	int sockfd, flags;
	struct sockaddr_un addr;

	sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0) {
		ipqmpd_error("ctlfd_register: socket returned %d\n", sockfd);
		return -ERRSOCK;
	}

	if (sockfd > hifd)
		hifd = sockfd;
	
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	memcpy(addr.sun_path, IPQMPD_ADDR, IPQMPD_ADDR_LEN);

	if(bind(sockfd, &addr, sizeof(addr)))
	{
		ipqmpd_error("ctlfd_register: bind failed\n");
		return -ERRBIND;
	}
	if (listen(sockfd,1)) {
		ipqmpd_error("ctlfd_register: listen failed\n");
		return -ERRBIND;
	}

	flags = fcntl(sockfd, F_GETFL);
	flags |= O_NONBLOCK;
	if (fcntl(sockfd, F_SETFL, flags)) {
		ipqmpd_error("ctlfd_register: fcntl failed\n");
		return -ERRFCNTL;
	}
		
	return sockfd;	
}

/* a new connection request arrived at ctlfd, we have to accept it */
int  ctlfd_inp()
{
	struct sockaddr_un from;
	size_t fromlen = sizeof(from);
	ipqmpd_peer_t *newpeer;
	int fd;

	newpeer = malloc(sizeof(ipqmpd_peer_t));
	if (!newpeer) {
		ipqmpd_error("ctlfd_inp: OOM malloc newpeer\n");
		return -ERROOM;
	}
	memset(newpeer, 0, sizeof(ipqmpd_peer_t));

	fd = accept(ctlfd, &from, &fromlen);
	if (fd < 0) {
		ipqmpd_error("ctlfd_inp: accept failed\n");
		return -ERRSOCK;
	}
	if (hifd < fd)
		hifd = fd;

	/* initialize with some sane values */	
	newpeer->ctlfd = fd;
	newpeer->out_q = queue_init();
	newpeer->ctl_q = queue_init();
	newpeer->state = PEER_STATE_CONN;

	/* prepend newpeer to clients list */
	if (clients)
		clients->prev = newpeer;
	newpeer->next = clients;
	clients = newpeer;	

	return 0;
}

