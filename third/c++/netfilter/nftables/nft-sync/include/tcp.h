#ifndef _TCP_H_
#define _TCP_H_

#include <arpa/inet.h>

struct tcp_conf {
	int ipproto;
	unsigned short port;
	union {
		struct {
			struct in_addr inet_addr;
		} ipv4;
		struct {
			struct in6_addr inet_addr6;
			int scope_id;
		} ipv6;
	} server;
	union {
		struct in_addr inet_addr;
		struct in6_addr inet_addr6;
	} client;
};

struct tcp_server;

struct tcp_server *tcp_server_create(struct tcp_conf *conf);
void tcp_server_destroy(struct tcp_server *c);
int tcp_server_get_fd(struct tcp_server *c);
int tcp_server_accept(struct tcp_server *c, struct sockaddr_in *addr);

struct tcp_client;

struct tcp_client *tcp_client_create(struct tcp_conf *conf);
void tcp_client_destroy(struct tcp_client *c);
int tcp_client_get_fd(struct tcp_client *c);
ssize_t tcp_client_send(struct tcp_client *c, const void *data, int size);
ssize_t tcp_client_recv(struct tcp_client *c, void *data, int size);
void tcp_client_set_data(struct tcp_client *c, void *data);
void *tcp_client_get_data(struct tcp_client *c);

#endif /*_TCP_H_ */
