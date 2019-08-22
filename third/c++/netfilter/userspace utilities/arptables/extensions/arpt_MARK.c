/*
 * (C) 2014 by Gao Feng <gaofeng@cn.fujitsu.com>
 *
 * arpt_MARK.c -- arptables extension to set mark for arp packet
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <getopt.h>
#include <arptables.h>
#include <linux/netfilter/xt_mark.h>
#include <linux/netfilter/xt_MARK.h>
#include <linux/netfilter/x_tables.h>

static void help(void)
{
	printf(
"MARK target v%s options:\n"
"--set-mark mark : set the mark value\n"
"--and-mark value : binary AND the mark with value\n"
"--or-mark value : binary OR the mark with value\n",
	ARPTABLES_VERSION);
}

#define MARK_OPT 1
#define AND_MARK_OPT 2
#define OR_MARK_OPT 3

static struct option opts[] = {
	{ .name = "set-mark", .has_arg = required_argument, .flag = 0, .val = MARK_OPT },
	{ .name = "and-mark", .has_arg = required_argument, .flag = 0, .val = AND_MARK_OPT },
	{ .name = "or-mark", .has_arg = required_argument, .flag = 0, .val =  OR_MARK_OPT },
	{ .name = NULL}
};

static void init(struct arpt_entry_target *t)
{
	struct xt_mark_tginfo2 *info = (struct xt_mark_tginfo2 *) t->data;

	info->mark = 0;
}

static int parse(int c, char **argv, int invert, unsigned int *flags,
		 const struct arpt_entry *e, struct arpt_entry_target **t)
{
	struct xt_mark_tginfo2 *info = (struct xt_mark_tginfo2 *)(*t)->data;
	int i;

	switch (c) {
	case MARK_OPT:
		if (sscanf(argv[optind-1], "%x", &i) != 1) {
			exit_error(PARAMETER_PROBLEM,
				"Bad mark value `%s'", optarg);
			return 0;
		}
		info->mark = i;
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
				"MARK: Can't specify --set-mark twice");
		*flags = 1;
		break;
	case AND_MARK_OPT:
		if (sscanf(argv[optind-1], "%x", &i) != 1) {
			exit_error(PARAMETER_PROBLEM,
				"Bad mark value `%s'", optarg);
			return 0;
		}
		info->mark = 0;
		info->mask = ~i;
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
				"MARK: Can't specify --and-mark twice");
		*flags = 1;
		break;
	case OR_MARK_OPT:
		if (sscanf(argv[optind-1], "%x", &i) != 1) {
			exit_error(PARAMETER_PROBLEM,
				"Bad mark value `%s'", optarg);
			return 0;
		}
		info->mark = info->mask = i;
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
				"MARK: Can't specify --or-mark twice");
		*flags = 1;
		break;
	default:
		return 0;
	}
	return 1;
}

static void final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM, "MARK: Parameter --set-mark/--and-mark/--or-mark is required");
}

static void print(const struct arpt_arp *ip,
		  const struct arpt_entry_target *target, int numeric)
{
	struct xt_mark_tginfo2 *info = (struct xt_mark_tginfo2 *)(target->data);

	if (info->mark == 0)
		printf("--and-mark %x", (unsigned int)(uint32_t)~info->mask);
	else if (info->mark == info->mask)
		printf("--or-mark %x", info->mark);
	else
		printf("--set-mark %x", info->mark);
}

static void save(const struct arpt_arp *ip,
		 const struct arpt_entry_target *target)
{
}

static struct arptables_target mark = {
	.next          = NULL,
	.name          = "MARK",
	.version       = ARPTABLES_VERSION,
	.size          = ARPT_ALIGN(sizeof(struct xt_mark_tginfo2)),
	.userspacesize = ARPT_ALIGN(sizeof(struct xt_mark_tginfo2)),
	.revision      = 2,
	.help          = help,
	.init          = init,
	.parse         = parse,
	.final_check   = final_check,
	.print         = print,
	.save          = save,
	.extra_opts    = opts
};

static void _init(void) __attribute__ ((constructor));
static void _init(void)
{
	register_target(&mark);
}
