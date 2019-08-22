/*
 * (C) 2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2011 by Intra2net AG <http://www.intra2net.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>

#include <libmnl/libmnl.h>
#include <libnetfilter_acct/libnetfilter_acct.h>
#include <linux/netfilter/nfnetlink_acct.h>
#include <linux/netfilter/nfnetlink.h>

enum {
	NFACCT_CMD_NONE = 0,
	NFACCT_CMD_LIST,
	NFACCT_CMD_ADD,
	NFACCT_CMD_DELETE,
	NFACCT_CMD_GET,
	NFACCT_CMD_FLUSH,
	NFACCT_CMD_VERSION,
	NFACCT_CMD_HELP,
	NFACCT_CMD_RESTORE,
	NFACCT_CMD_LISTEN,
};

static int nfacct_cmd_list(int argc, char *argv[]);
static int nfacct_cmd_add(int argc, char *argv[]);
static int nfacct_cmd_delete(int argc, char *argv[]);
static int nfacct_cmd_get(int argc, char *argv[]);
static int nfacct_cmd_flush(int argc, char *argv[]);
static int nfacct_cmd_version(int argc, char *argv[]);
static int nfacct_cmd_help(int argc, char *argv[]);
static int nfacct_cmd_restore(int argc, char *argv[]);
static int nfacct_cmd_monitor(int argc, char *argv[]);

static void usage(char *argv[])
{
	fprintf(stderr, "Usage: %s command [parameters]...\n", argv[0]);
}

static void nfacct_perror(const char *msg)
{
	if (errno == 0) {
		fprintf(stderr, "nfacct v%s: %s\n", VERSION, msg);
	} else {
		fprintf(stderr, "nfacct v%s: %s: %s\n",
			VERSION, msg, strerror(errno));
	}
}

static void duparg(const char *key)
{
	fprintf(stderr, "Error: duplicate \"%s\" or collision with another "
		"option \n", key);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int cmd = NFACCT_CMD_NONE, ret = 0;

	if (argc < 2) {
		usage(argv);
		exit(EXIT_FAILURE);
	}

	if (strncmp(argv[1], "list", strlen(argv[1])) == 0)
		cmd = NFACCT_CMD_LIST;
	else if (strncmp(argv[1], "add", strlen(argv[1])) == 0)
		cmd = NFACCT_CMD_ADD;
	else if (strncmp(argv[1], "delete", strlen(argv[1])) == 0)
		cmd = NFACCT_CMD_DELETE;
	else if (strncmp(argv[1], "get", strlen(argv[1])) == 0)
		cmd = NFACCT_CMD_GET;
	else if (strncmp(argv[1], "flush", strlen(argv[1])) == 0)
		cmd = NFACCT_CMD_FLUSH;
	else if (strncmp(argv[1], "version", strlen(argv[1])) == 0)
		cmd = NFACCT_CMD_VERSION;
	else if (strncmp(argv[1], "help", strlen(argv[1])) == 0)
		cmd = NFACCT_CMD_HELP;
	else if (strncmp(argv[1], "restore", strlen(argv[1])) == 0)
		cmd = NFACCT_CMD_RESTORE;
	else if (strncmp(argv[1], "monitor", strlen(argv[1])) == 0)
		cmd = NFACCT_CMD_LISTEN;
	else {
		fprintf(stderr, "nfacct v%s: Unknown command: %s\n",
			VERSION, argv[1]);
		usage(argv);
		exit(EXIT_FAILURE);
	}

	switch(cmd) {
	case NFACCT_CMD_LIST:
		ret = nfacct_cmd_list(argc, argv);
		break;
	case NFACCT_CMD_ADD:
		ret = nfacct_cmd_add(argc, argv);
		break;
	case NFACCT_CMD_DELETE:
		ret = nfacct_cmd_delete(argc, argv);
		break;
	case NFACCT_CMD_GET:
		ret = nfacct_cmd_get(argc, argv);
		break;
	case NFACCT_CMD_FLUSH:
		ret = nfacct_cmd_flush(argc, argv);
		break;
	case NFACCT_CMD_VERSION:
		ret = nfacct_cmd_version(argc, argv);
		break;
	case NFACCT_CMD_HELP:
		ret = nfacct_cmd_help(argc, argv);
		break;
	case NFACCT_CMD_RESTORE:
		ret = nfacct_cmd_restore(argc, argv);
		break;
	case NFACCT_CMD_LISTEN:
		ret = nfacct_cmd_monitor(argc, argv);
		break;
	}
	return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

static bool fmt_header = false;

static int nfacct_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nfacct *nfacct;
	char buf[4096];
	int *fmt = (int *)data;

	nfacct = nfacct_alloc();
	if (nfacct == NULL) {
		nfacct_perror("OOM");
		goto err;
	}

	if (nfacct_nlmsg_parse_payload(nlh, nfacct) < 0) {
		nfacct_perror("nfacct_parse_nl_msg");
		goto err_free;
	}

	switch (*fmt) {
	case NFACCT_SNPRINTF_T_XML:
		if (!fmt_header) {
			printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
				"<nfacct>\n");
			fmt_header = true;
		}
		break;
	case NFACCT_SNPRINTF_T_JSON:
		if (!fmt_header) {
			printf("{ \"timestamp\" : %lld, \n"
			       "  \"nfacct_counters\" : [\n",
			       (long long) time(NULL));
			fmt_header = true;
		} else
			printf(",\n");
		break;
	default:
		break;
	}

	nfacct_snprintf(buf, sizeof(buf), nfacct, *fmt,
			NFACCT_SNPRINTF_F_FULL);
	printf("%s", buf);
	switch (*fmt) {
	case NFACCT_SNPRINTF_T_JSON:
		/* no newline in JSON case */
		break;
	default:
		putchar('\n');
		break;
	}

err_free:
	nfacct_free(nfacct);
err:
	return MNL_CB_OK;
}

static void _print_footer(int fmt)
{
	if (fmt_header) {
		switch (fmt) {
		case NFACCT_SNPRINTF_T_XML:
			printf("</nfacct>\n");
			break;
		case NFACCT_SNPRINTF_T_JSON:
			printf("\n] }\n");
			break;
		default:
			break;
		}
	}
}

static int nfacct_cmd_list(int argc, char *argv[])
{
	bool zeroctr = false;
	int fmt = NFACCT_SNPRINTF_T_PLAIN;
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	unsigned int seq, portid;
	int ret, i;
	uint32_t mask = 0, value = 0;

	for (i=2; i<argc; i++) {
		if (strncmp(argv[i], "reset", strlen(argv[i])) == 0) {
			if (zeroctr)
				duparg(argv[i]);
			zeroctr = true;
		} else if (strncmp(argv[i], "xml", strlen(argv[i])) == 0) {
			if (fmt)
				duparg(argv[i]);
			fmt = NFACCT_SNPRINTF_T_XML;
		} else if (strncmp(argv[i], "json", strlen(argv[i])) == 0) {
			if (fmt)
				duparg(argv[i]);
			fmt = NFACCT_SNPRINTF_T_JSON;
		} else if (strncmp(argv[i], "counters", strlen(argv[i])) == 0) {
			if (mask || value)
				duparg(argv[i]);
			mask = NFACCT_F_QUOTA_BYTES | NFACCT_F_QUOTA_PKTS;
			value = 0;
		} else if (strncmp(argv[i], "bytes-quota", strlen(argv[i])) == 0) {
			if (mask || value)
				duparg(argv[i]);
			mask = NFACCT_F_QUOTA_BYTES;
			value = NFACCT_F_QUOTA_BYTES;
		} else if (strncmp(argv[i], "pkts-quota", strlen(argv[i])) == 0) {
			if (mask || value)
				duparg(argv[i]);
			mask = NFACCT_F_QUOTA_PKTS;
			value = NFACCT_F_QUOTA_PKTS;
		} else if (strncmp(argv[i], "overquota", strlen(argv[i])) == 0) {
			if (mask || value)
				duparg(argv[i]);
			mask = NFACCT_F_OVERQUOTA;
			value = NFACCT_F_OVERQUOTA;
		} else {
			nfacct_perror("unknown argument");
			return -1;
		}
	}

	seq = time(NULL);
	nlh = nfacct_nlmsg_build_hdr(buf, zeroctr ?
					NFNL_MSG_ACCT_GET_CTRZERO :
					NFNL_MSG_ACCT_GET,
				     NLM_F_DUMP, seq);
	if (mask || value) {
		struct nlattr *nest = mnl_attr_nest_start(nlh, NFACCT_FILTER);

		mnl_attr_put_u32(nlh, NFACCT_FILTER_MASK, htonl(mask));
		mnl_attr_put_u32(nlh, NFACCT_FILTER_VALUE, htonl(value));
		mnl_attr_nest_end(nlh, nest);
	}

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		nfacct_perror("mnl_socket_open");
		return -1;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		nfacct_perror("mnl_socket_bind");
		return -1;
	}
	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		nfacct_perror("mnl_socket_send");
		return -1;
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, nfacct_cb, &fmt);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		nfacct_perror("error");
		return -1;
	}
	mnl_socket_close(nl);

	_print_footer(fmt);

	return 0;
}

static int _nfacct_cmd_add(char *name, uint64_t pkts, uint64_t bytes,
			  uint32_t flags, uint64_t quota)
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq;
	struct nfacct *nfacct;
	int ret;

	nfacct = nfacct_alloc();
	if (nfacct == NULL) {
		nfacct_perror("OOM");
		return -1;
	}

	nfacct_attr_set(nfacct, NFACCT_ATTR_NAME, name);

	nfacct_attr_set_u64(nfacct, NFACCT_ATTR_PKTS, pkts);
	nfacct_attr_set_u64(nfacct, NFACCT_ATTR_BYTES, bytes);
	if (flags) {
		nfacct_attr_set(nfacct, NFACCT_ATTR_FLAGS, &flags);
		nfacct_attr_set_u64(nfacct, NFACCT_ATTR_QUOTA, quota);
	}

	seq = time(NULL);
	nlh = nfacct_nlmsg_build_hdr(buf, NFNL_MSG_ACCT_NEW,
				     NLM_F_CREATE | NLM_F_ACK, seq);
	nfacct_nlmsg_build_payload(nlh, nfacct);

	nfacct_free(nfacct);

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		nfacct_perror("mnl_socket_open");
		return -1;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		nfacct_perror("mnl_socket_bind");
		return -1;
	}
	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		nfacct_perror("mnl_socket_send");
		return -1;
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		nfacct_perror("error");
		return -1;
	}
	mnl_socket_close(nl);

	return 0;
}

static int nfacct_cmd_add(int argc, char *argv[])
{
	int mode, ret;
	uint64_t quota;

	if (argc < 3) {
		nfacct_perror("missing object name");
		return -1;
	}

	if (argc == 3)
		return _nfacct_cmd_add(argv[2], 0, 0, 0, 0);

	if (argc == 4) {
		nfacct_perror("missing quota value");
		return -1;
	}

	if (argc == 5) {
		if (strcmp(argv[3], "byte") && strcmp(argv[3], "packet")) {
			nfacct_perror("argument must "
				      "\"byte\" or \"packet\"");
			return -1;
		}
	}

	if (argc > 5) {
		nfacct_perror("too many arguments");
		return -1;
	}

	mode = (strcmp(argv[3], "byte") == 0 ?
		NFACCT_F_QUOTA_BYTES : NFACCT_F_QUOTA_PKTS);

	ret = sscanf(argv[4], "%"PRIu64"", &quota);
	if (ret != 1) {
		nfacct_perror("error reading quota");
		return -1;
	}

	return _nfacct_cmd_add(argv[2], 0, 0, mode, quota);
}

static int nfacct_cmd_delete(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq;
	struct nfacct *nfacct;
	int ret;

	if (argc < 3) {
		nfacct_perror("missing object name");
		return -1;
	} else if (argc > 3) {
		nfacct_perror("too many arguments");
		return -1;
	}

	nfacct = nfacct_alloc();
	if (nfacct == NULL) {
		nfacct_perror("OOM");
		return -1;
	}

	nfacct_attr_set(nfacct, NFACCT_ATTR_NAME, argv[2]);

	seq = time(NULL);
	nlh = nfacct_nlmsg_build_hdr(buf, NFNL_MSG_ACCT_DEL,
				     NLM_F_ACK, seq);
	nfacct_nlmsg_build_payload(nlh, nfacct);

	nfacct_free(nfacct);

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		nfacct_perror("mnl_socket_open");
		return -1;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		nfacct_perror("mnl_socket_bind");
		return -1;
	}
	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		nfacct_perror("mnl_socket_send");
		return -1;
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		nfacct_perror("error");
		return -1;
	}

	mnl_socket_close(nl);

	return 0;
}

static int nfacct_cmd_get(int argc, char *argv[])
{
	bool zeroctr = false;
	int fmt = NFACCT_SNPRINTF_T_PLAIN;
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq;
	struct nfacct *nfacct;
	int ret, i;

	if (argc < 3) {
		nfacct_perror("missing object name");
		return -1;
	}
	for (i=3; i<argc; i++) {
		if (strncmp(argv[i], "reset", strlen(argv[i])) == 0) {
			zeroctr = true;
		} else if (strncmp(argv[i], "xml", strlen(argv[i])) == 0) {
			fmt = NFACCT_SNPRINTF_T_XML;
		} else if (strncmp(argv[i], "json", strlen(argv[i])) == 0) {
			fmt = NFACCT_SNPRINTF_T_JSON;
		} else {
			nfacct_perror("unknown argument");
			return -1;
		}
	}

	nfacct = nfacct_alloc();
	if (nfacct == NULL) {
		nfacct_perror("OOM");
		return -1;
	}
	nfacct_attr_set(nfacct, NFACCT_ATTR_NAME, argv[2]);

	seq = time(NULL);
	nlh = nfacct_nlmsg_build_hdr(buf, zeroctr ?
					NFNL_MSG_ACCT_GET_CTRZERO :
					NFNL_MSG_ACCT_GET,
				     NLM_F_ACK, seq);

	nfacct_nlmsg_build_payload(nlh, nfacct);

	nfacct_free(nfacct);

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		nfacct_perror("mnl_socket_open");
		return -1;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		nfacct_perror("mnl_socket_bind");
		return -1;
	}
	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		nfacct_perror("mnl_socket_send");
		return -1;
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, nfacct_cb, &fmt);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		nfacct_perror("error");
		return -1;
	}
	mnl_socket_close(nl);

	_print_footer(fmt);

	return 0;
}

static int nfacct_cmd_flush(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq;
	int ret;

	if (argc > 2) {
		nfacct_perror("too many arguments");
		return -1;
	}

	seq = time(NULL);
	nlh = nfacct_nlmsg_build_hdr(buf, NFNL_MSG_ACCT_DEL, NLM_F_ACK, seq);

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		nfacct_perror("mnl_socket_open");
		return -1;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		nfacct_perror("mnl_socket_bind");
		return -1;
	}
	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		nfacct_perror("mnl_socket_send");
		return -1;
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		nfacct_perror("error");
		return -1;
	}

	mnl_socket_close(nl);

	return 0;
}

static const char version_msg[] =
	"nfacct v%s: utility for the Netfilter extended accounting "
	"infrastructure\n"
	"Copyright (C) 2011 Pablo Neira Ayuso <pablo@netfilter.org>\n"
	"Copyright (C) 2011 Intra2net AG <http://www.intra2net.com>\n"
	"This program comes with ABSOLUTELY NO WARRANTY.\n"
	"This is free software, and you are welcome to redistribute it under "
	"certain \nconditions; see LICENSE file distributed in this package "
	"for details.\n";

static int nfacct_cmd_version(int argc, char *argv[])
{
	printf(version_msg, VERSION);
	return 0;
}

static const char help_msg[] =
	"nfacct v%s: utility for the Netfilter extended accounting "
	"infrastructure\n"
	"Usage: %s command [parameters]...\n\n"
	"Commands:\n"
	"  list [reset]\t\tList the accounting object table (and reset)\n"
	"  add object-name\tAdd new accounting object to table\n"
	"  delete object-name\tDelete existing accounting object\n"
	"  get object-name\tGet existing accounting object\n"
	"  flush\t\t\tFlush accounting object table\n"
	"  restore\t\tRestore accounting object table reading 'list' output from stdin\n"
	"  monitor\t\tListens for quota attainment notifications\n"
	"  version\t\tDisplay version and disclaimer\n"
	"  help\t\t\tDisplay this help message\n";

static int nfacct_cmd_help(int argc, char *argv[])
{
	printf(help_msg, VERSION, argv[0]);
	return 0;
}

static int nfacct_cmd_restore(int argc, char *argv[])
{
	uint64_t pkts, bytes, quota;
	char name[512], mode[512], buffer[512];
	int ret, flags;

	while (fgets(buffer, sizeof(buffer), stdin)) {
		char *semicolon = strchr(buffer, ';');

		if (semicolon == NULL) {
			nfacct_perror("invalid line");
			return -1;
		}

		*semicolon = 0;
		ret = flags = 0;
		quota = 0;

		if (!strstr(buffer, "quota")) {
			ret = sscanf(buffer,
				     "{ pkts = %"PRIu64", "
				     "bytes = %"PRIu64" } = %s",
				     &pkts, &bytes, name);

			if (ret != 3) {
				nfacct_perror("error reading input");
				return -1;
			}
		} else {
			ret = sscanf(buffer, "{ pkts = %"PRIu64", "
				     "bytes = %"PRIu64", quota = %"PRIu64", "
				     "mode = %s } = %s",
				     &pkts, &bytes, &quota, mode, name);

			if (ret != 5) {
				nfacct_perror("error reading input");
				return -1;
			}

			flags = (strcmp(mode, "byte") == 0 ?
				 NFACCT_F_QUOTA_BYTES : NFACCT_F_QUOTA_PKTS);
		}

		ret = _nfacct_cmd_add(name, pkts, bytes, flags, quota);
		if (ret != 0)
			return ret;
	}
	return 0;
}

static int nfacct_cmd_monitor(int argc, char *argv[])
{
	struct mnl_socket *nl;
	int fmt = NFACCT_SNPRINTF_T_PLAIN;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	int ret, option = NFNLGRP_ACCT_QUOTA;

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		nfacct_perror("mnl_socket_open");
		return -1;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		nfacct_perror("mnl_socket_bind");
		return -1;
	}

	mnl_socket_setsockopt(nl, NETLINK_ADD_MEMBERSHIP,
			      &option, sizeof(int));

	while (1) {
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
		if (ret == -1) {
			perror("mnl_socket_recvfrom");
			break;
		}

		ret = mnl_cb_run(buf, ret, 0, 0, nfacct_cb, &fmt);
		if (ret <= 0)
			break;
	}

	mnl_socket_close(nl);

	return 0;
}
