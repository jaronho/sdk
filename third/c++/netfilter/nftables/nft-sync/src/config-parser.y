%{
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
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <netinet/in.h>

#include "config.h"
#include "logging.h"

extern char *yytext;
extern int yylineno;

static int parse_addr(const char *text, struct in_addr *addr,
			 uint16_t *port)
{
	char *colon = strchr(text, ':');

	if (colon == NULL) {
		fprintf(stderr, "missing `:' to indicate port\n");
		return -1;
	}
	*colon = '\0';

	if (inet_pton(AF_INET, text, addr) < 0) {
		fprintf(stderr, "%s not valid IPv4 address\n", text);
		return -1;
	}
	*port = atoi(colon + 1);

	return 0;
}

%}

%union {
	int	val;
	char	*string;
}

%token T_LOCAL_ADDR
%token T_REMOTE_ADDR
%token T_ADDR
%token T_NUMBER
%token T_LOG
%token T_MODE

%token <string> T_STRING
%token <val>	T_INTEGER

%%

configfile	:
		| sections
		;

sections	: section
		| sections section
		;

section		: network
		| log
		;

network		: local_addr
		| remote_addr
		;

local_addr	: T_LOCAL_ADDR T_STRING
		{
			nfts_inst.tcp.ipproto = AF_INET;
			if (parse_addr($2,
				       &nfts_inst.tcp.server.ipv4.inet_addr,
				       &nfts_inst.tcp.port) < 0)
				break;

			nfts_inst.mode = NFTS_MODE_SERVER;
		}
		;

remote_addr	: T_REMOTE_ADDR T_STRING
		{
			nfts_inst.tcp.ipproto = AF_INET;
			if (parse_addr($2, &nfts_inst.tcp.client.inet_addr,
				       &nfts_inst.tcp.port) < 0)
				break;

			nfts_inst.mode = NFTS_MODE_CLIENT;
		}
		;

log		: T_LOG T_STRING
		{
			if (strcmp($2, "syslog") == 0) {
				nfts_inst.log.type = NFTS_LOG_T_SYSLOG;
			} else if (strcmp($2, "stdout") == 0) {
				nfts_inst.log.type = NFTS_LOG_T_FILE;
				nfts_inst.log.color = true;
			} else {
				nfts_inst.log.type = NFTS_LOG_T_FILE;
				strncpy(nfts_inst.log.filename, $2, PATH_MAX);
				nfts_inst.log.filename[PATH_MAX - 1] = '\0';
			}
		}
		;

%%

int __attribute__((noreturn)) yyerror(char *msg)
{
	fprintf(stderr, "parsing config file in line (%d), symbol '%s': %s\n",
			 yylineno, yytext, msg);
	exit(EXIT_FAILURE);
}

int nft_sync_config_parse(const char *filename)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "Cannot open configuration file %s\n",
			filename);
		return -1;
	}

	yyrestart(fp);
	yyparse();
	fclose(fp);

	return 0;
}
