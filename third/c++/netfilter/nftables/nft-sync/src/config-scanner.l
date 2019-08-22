%{
/*
 * (C) 2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <string.h>
#include "config-parser.h"
%}

%option yylineno
%option noinput
%option nounput

ws		[ \t]+
comment         #.*$
nl		[\n\r]

is_on		[o|O][n|N]
is_off		[o|O][f|F][f|F]
integer		[\-\+]?[0-9]+
string		[a-zA-Z0-9][a-zA-Z0-9\.\-\_\/\:]*

%%
"local-address"		{ return T_LOCAL_ADDR; }
"remote-address"	{ return T_REMOTE_ADDR; }
"logging"		{ return T_LOG; }
"mode"			{ return T_MODE; }

{integer}		{ yylval.val = atoi(yytext); return T_INTEGER; }
{string}		{ yylval.string = strdup(yytext); return T_STRING; }

{comment}		;
{ws}			;
{nl}			;

<<EOF>>			{ yyterminate(); }

.			{ return yytext[0]; }

%%

int
yywrap()
{
	return 1;
}
