/* just some testing code for the netfilter packet queuing to userspace 
   (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>

   This code is licensed under GPL conditions 
*/
#include <stdio.h>
#include <stdlib.h>
#include <libipqmpd/libipqmpd.h>
#include <linux/netfilter.h>
#include <sys/socket.h>
#include <sys/un.h>

#define IPQ_BUF_SIZE 10240

#if 1
#define DEBUGP 	printf
#else
#define DEBUGP(format, args...)
#endif

static	struct ipq_handle *qh;

int handle_packet(unsigned char *packet)
{
	int ret;
	ipq_packet_msg_t *qpkth;

	qpkth = (ipq_packet_msg_t *) packet;
	DEBUGP("id: %lu, mark: %lu, timestamp: %lu, indev: %s, datalen: %d\n", qpkth->packet_id, qpkth->mark, qpkth->timestamp_sec, qpkth->indev_name, qpkth->data_len);

	ret = ipq_set_verdict(qh, qpkth->packet_id, NF_ACCEPT, qpkth->data_len, qpkth->payload);
	if (ret < 0)
	{
		printf("error setting verdict\n");
		ipq_perror(NULL);
		return 1;
	}
	return 0;
}

int register_ctrl_socket()
{
	int sockfd;
	struct sockaddr_un addr;

	sockfd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (sockfd)
	{
		addr.sun_family = AF_UNIX;
	//	addr.sun_path = "\0netfilter_ipq";
	
		bind(sockfd, &addr, sizeof(addr));
		
		return sockfd;	
	} 
	return 0;
}


int main()
{
	ssize_t len;
	unsigned char *buf;

	qh = ipq_create_handle(0,17);
	if (!qh)
	{
		printf("can't create netlink socket\n");
		ipq_perror(NULL);
		exit(1);
	}
	if (ipq_set_mode(qh, IPQ_COPY_PACKET, 0xFFFF) < 0)
	{
		printf("can't set netlink mode\n");
		ipq_perror(NULL);
		exit(2);
	}

	buf = (char *) malloc(IPQ_BUF_SIZE);
	while (1)
	{
		int ptype,error;
		unsigned char *packet;

		len = ipq_read(qh, buf, IPQ_BUF_SIZE, 0);
		if (len < 0) {
			printf("len < 0\n");
			break;
		} else if (len == 0) {
			printf("timeout exceeded\n");
		} else {
			ptype = ipq_message_type(buf);
			packet = (unsigned char *) ipq_get_packet(buf);
			DEBUGP("received packet, length=%d, type=%d\n", 
				len, ptype);
			switch (ptype) {
				case NLMSG_ERROR:
					error = -ipq_get_msgerr(buf);
					printf("NLMSG_ERROR: %d\n", error);
					exit(3);
					break;
	
				case IPQM_PACKET:
					handle_packet(packet);
					break;
			}
		}
	}
	ipq_perror(NULL);
	exit(1);
}

