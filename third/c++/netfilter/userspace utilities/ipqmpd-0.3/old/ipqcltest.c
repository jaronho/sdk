/* small test program to simulate an ipqmpd client process,
 * beware: this doesn't use libipqmpd but interfaces directly with 
 * ipqmpd */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>

#include "ipqmpd.h"
/* because of the MAGIC constants */
#include "ctlmsg.c"

#if 1
#define DEBUGP(format, args...)	fprintf(stderr, format, ## args)
#else
#define DEBUGP(format, args...)
#endif

int wait4regack(int fd)
{
	ipqmpd_ctl_msg_t msg;
	int ret;

	while (1) {
		ret = recv(fd, &msg, sizeof(msg), 0);
		if (ret == 0)
			return 1;
		if (msg.type == IPQMPD_MSG_REGACK && 
				!memcmp(msg.magic, IPQMPD_CTL_MAGIC,
					IPQMPD_CTL_MAGIC_LEN))
			return 0;
	}
}

int send_register(int fd, u_int32_t flags, unsigned long mark)
{
	ipqmpd_ctl_msg_t *ctmsg;
	struct ipqmpd_ctl_msg_register *regmsg;
	size_t len;
	int ret, dfd;
	struct sockaddr_un sa, ra;

	len = sizeof(int) + sizeof(ipqmpd_ctl_msg_t) +
			sizeof(struct ipqmpd_ctl_msg_register);

	ctmsg = (ipqmpd_ctl_msg_t *) malloc(len);
			
	regmsg = (struct ipqmpd_ctl_msg_register *) ctmsg->payload;

	memcpy(ctmsg->magic, IPQMPD_CTL_MAGIC, IPQMPD_CTL_MAGIC_LEN);
	ctmsg->type = IPQMPD_MSG_REGISTER;
	ctmsg->len = sizeof(struct ipqmpd_ctl_msg_register);
	regmsg->flags = flags;
	regmsg->mark = mark;
	sprintf(regmsg->addr, "ipq_peer_%d", getpid());

	memset(&sa, 0, sizeof(sa));
	memset(&ra, 0, sizeof(ra));
	sa.sun_family = AF_UNIX;
	ra.sun_family = AF_UNIX;
	sprintf(sa.sun_path, "X%s", regmsg->addr);
	sprintf(ra.sun_path, "XXXX%s", regmsg->addr);
	sa.sun_path[0] = '\0';
	ra.sun_path[0] = '\0';
	
	dfd = socket(PF_UNIX, SOCK_DGRAM, 0);

	/* bind to local addr */
	bind(dfd, &sa, sizeof(sa));
	
	ret = send(fd, ctmsg, len, 0);

	if (wait4regack(fd)) {
		close(dfd);
		return 0;
	}

	connect(dfd, &ra, sizeof(ra));
	return dfd;
}

int main ()
{
	int sockfd, i, datafd;
	struct sockaddr_un ad;
	unsigned char recvbuf[10240];


	memset(&ad, 0, sizeof(struct sockaddr_un));
	ad.sun_family = AF_UNIX;
	memcpy(ad.sun_path,"\0ipqmpd", 7);
	sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (connect(sockfd, &ad, sizeof(ad))) {
		printf("%s\n", strerror(errno));
		exit(1);
	}

	datafd = send_register(sockfd, 1, 17);
	printf("send_register returned fd #%d\n", datafd);
	while (1) {
		i = recv(datafd, &recvbuf, 10240, 0);
		printf("received %d bytes from datafd\n", i);
		}

}
