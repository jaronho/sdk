/*
 * (C) 2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include <tcp.h>
#include <timer.h>

#include "logging.h"

/*
 * TCP server side
 */

struct tcp_server {
	int fd;
	union {
		struct sockaddr_in ipv4;
		struct sockaddr_in6 ipv6;
	} addr;
};

#define TCP_SERVER_LISTEN 20

struct tcp_server *tcp_server_create(struct tcp_conf *conf)
{
	int ret, on = 1;
	struct tcp_server *c;
	socklen_t socklen = sizeof(int);

	c = calloc(1, sizeof(struct tcp_server));
	if (c == NULL)
		return NULL;

	switch (conf->ipproto) {
	case AF_INET:
		c->addr.ipv4.sin_family = AF_INET;
		c->addr.ipv4.sin_port = htons(conf->port);
		c->addr.ipv4.sin_addr = conf->server.ipv4.inet_addr;
		socklen = sizeof(struct sockaddr_in);
		break;

	case AF_INET6:
		c->addr.ipv6.sin6_family = AF_INET6;
		c->addr.ipv6.sin6_port = htons(conf->port);
		c->addr.ipv6.sin6_addr = conf->server.ipv6.inet_addr6;
		c->addr.ipv6.sin6_scope_id = conf->server.ipv6.scope_id;
		socklen = sizeof(struct sockaddr_in6);
		break;
	}

	c->fd = socket(conf->ipproto, SOCK_STREAM, 0);
	if (c->fd < 0)
		goto err1;

	ret = setsockopt(c->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	if (ret < 0)
		goto err2;

	ret = setsockopt(c->fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(int));
	if (ret < 0)
		goto err2;

	ret = bind(c->fd, (struct sockaddr *) &c->addr, socklen);
	if (ret < 0)
		goto err2;

	ret = listen(c->fd, TCP_SERVER_LISTEN);
	if (ret < 0)
		goto err2;

	ret = fcntl(c->fd, F_SETFL, O_NONBLOCK);
	if (ret < 0)
		goto err2;

	return c;
err2:
	close(c->fd);
err1:
	free(c);
	return NULL;
}

void tcp_server_destroy(struct tcp_server *c)
{
	close(c->fd);
	free(c);
}

int tcp_server_get_fd(struct tcp_server *c)
{
	return c->fd;
}

int tcp_server_accept(struct tcp_server *c, struct sockaddr_in *addr)
{
	int err, fd;
	socklen_t socklen = sizeof(struct sockaddr_in);

	err = accept(c->fd, (struct sockaddr *)addr, &socklen);
	if (err < 0 && errno != EAGAIN)
		return -1;

	fd = err;

	err = fcntl(fd, F_SETFL, O_NONBLOCK);
	if (err < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

/*
 * TCP client side
 */

enum tcp_client_state {
	TCP_DISCONNECTED	= 0,
	TCP_CONNECTING,
	TCP_CONNECTED
};

struct tcp_client {
	int			fd;
	enum tcp_client_state	state;
	union {
		struct sockaddr_in ipv4;
		struct sockaddr_in6 ipv6;
	} addr;
	socklen_t		socklen;
	struct nft_timer	timer;
	void			*data;
};

#define TCP_CONNECT_TIMEOUT	1

static int tcp_client_init(struct tcp_client *c, struct tcp_conf *conf)
{
	int ret = 0;

	c->fd = socket(conf->ipproto, SOCK_STREAM, 0);
	if (c->fd < 0)
		return -1;

	switch (conf->ipproto) {
	case AF_INET:
		c->addr.ipv4.sin_family = AF_INET;
		c->addr.ipv4.sin_port = htons(conf->port);
		c->addr.ipv4.sin_addr = conf->client.inet_addr;
		c->socklen = sizeof(struct sockaddr_in);
		break;
	case AF_INET6:
		c->addr.ipv6.sin6_family = AF_INET6;
		c->addr.ipv6.sin6_port = htons(conf->port);
		c->addr.ipv6.sin6_addr = conf->client.inet_addr6;
		c->socklen = sizeof(struct sockaddr_in6);
		break;
	default:
		ret = -1;
		break;
	}

	if (ret < 0)
		goto err1;

	ret = fcntl(c->fd, F_SETFL, O_NONBLOCK);
	if (ret < 0)
		goto err1;

	ret = connect(c->fd, (struct sockaddr *)&c->addr, c->socklen);
	if (ret < 0) {
		switch (errno) {
		case EINPROGRESS:
			c->state = TCP_CONNECTING;
			break;
		default: /* ECONNREFUSED */
			c->state = TCP_DISCONNECTED;
			goto err1;
		}
	} else {
		/* very unlikely at this stage. */
		c->state = TCP_CONNECTED;
	}
	return 0;
err1:
	close(c->fd);
	return ret;
}

int tcp_client_get_fd(struct tcp_client *c)
{
	return c->fd;
}

struct tcp_client *tcp_client_create(struct tcp_conf *conf)
{
	struct tcp_client *c;

	c = calloc(1, sizeof(struct tcp_client));
	if (c == NULL)
		return NULL;

	if (tcp_client_init(c, conf) < 0) {
		free(c);
		return NULL;
	}

	return c;
}

void tcp_client_destroy(struct tcp_client *c)
{
	close(c->fd);
	free(c);
}

ssize_t tcp_client_send(struct tcp_client *c, const void *data, int size)
{
	ssize_t ret = 0;

	switch (c->state) {
	case TCP_DISCONNECTED:
		ret = -1;
		break;
	case TCP_CONNECTING:
		ret = connect(c->fd, (struct sockaddr *)&c->addr, c->socklen);
		if (ret < 0)
			return ret;

		c->state = TCP_CONNECTED;
		/* fall through ... */
	case TCP_CONNECTED:
		ret = send(c->fd, data, size, 0);
		if (ret <= 0) {
			/* errno == EPIPE || errno == ECONNRESET */
			c->state = TCP_DISCONNECTED;
			return ret;
		}
		break;
	}
	return ret;
}

ssize_t tcp_client_recv(struct tcp_client *c, void *data, int size)
{
	ssize_t ret = 0;

	switch (c->state) {
	case TCP_DISCONNECTED:
		ret = -1;
		break;
	case TCP_CONNECTING:
		ret = connect(c->fd, (struct sockaddr *)&c->addr, c->socklen);
		if (ret < 0)
			return ret;

		c->state = TCP_CONNECTED;
		/* fall through ... */
	case TCP_CONNECTED:
		ret = recv(c->fd, data, size, 0);
		if (ret <= 0) {
			/* errno == ENOTCONN */
			c->state = TCP_DISCONNECTED;
			return ret;
		}
	}
	return ret;
}

void tcp_client_set_data(struct tcp_client *c, void *data)
{
	c->data = data;
}

void *tcp_client_get_data(struct tcp_client *c)
{
	return c->data;
}
