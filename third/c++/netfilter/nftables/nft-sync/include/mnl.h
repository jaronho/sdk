#ifndef _MNL_H_
#define _MNL_H_

#include <libmnl/libmnl.h>
#include <libnftnl/common.h>
#include <libnftnl/ruleset.h>
#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/set.h>
#include <libnftnl/rule.h>

#include "config.h"

struct nftnl_rule_list *mnl_rule_dump(struct mnl_socket *nf_sock, int family);
struct nftnl_chain_list *mnl_chain_dump(struct mnl_socket *nf_sock, int family);
struct nftnl_table_list *mnl_table_dump(struct mnl_socket *nf_sock, int family);
struct nftnl_set_list *mnl_set_dump(struct mnl_socket *nf_sock, int family,
				    const char *table);
int mnl_setelem_get(struct mnl_socket *nf_sock, struct nftnl_set *nls);
struct nftnl_ruleset *mnl_ruleset_dump(struct mnl_socket *nf_sock);
int nfts_socket_open(struct nft_sync_inst *nfts_inst);
void nfts_socket_close(struct nft_sync_inst *nfts_inst);
const char *netlink_dump_ruleset(struct mnl_socket *s);

#endif /* _MNL_H_ */
