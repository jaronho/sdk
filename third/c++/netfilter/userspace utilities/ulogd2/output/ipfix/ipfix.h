/*
 * ipfix.h
 *
 * Holger Eitzenberger <holger@eitzenberger.org>, 2009.
 * Ander Juaristi <a@juaristi.eus>, 2019
 */
#ifndef IPFIX_H
#define IPFIX_H

#include <stdint.h>
#include <netinet/in.h>


struct ipfix_hdr {
#define IPFIX_VERSION			0xa
	uint16_t version;
	uint16_t len;
	uint32_t time;
	uint32_t seqno;
	uint32_t oid;				/* Observation Domain ID */
	uint8_t data[];
} __packed;

#define IPFIX_HDRLEN		sizeof(struct ipfix_hdr)

/*
 * IDs 0-255 are reserved for Template Sets.  IDs of Data Sets are > 255.
 */
struct ipfix_templ_hdr {
	uint16_t sid;
	uint16_t len;
	uint16_t tid;
	uint16_t cnt;
	uint8_t data[];
} __packed;

#define IPFIX_TEMPL_HDRLEN(nfields)	sizeof(struct ipfix_templ_hdr) + (sizeof(uint16_t) * 2 * nfields)

struct ipfix_set_hdr {
#define IPFIX_SET_TEMPL			2
#define IPFIX_SET_OPT_TEMPL		3
	uint16_t id;
	uint16_t len;
	uint8_t data[];
} __packed;

#define IPFIX_SET_HDRLEN		sizeof(struct ipfix_set_hdr)

struct ipfix_msg {
	struct llist_head link;
	uint8_t *tail;
	uint8_t *end;
	unsigned nrecs;
	int tid;
	struct ipfix_set_hdr *last_set;
	uint8_t data[];
};

struct vy_ipfix_data {
	struct in_addr saddr;
	struct in_addr daddr;
	uint32_t packets;
	uint32_t bytes;
	uint32_t start;				/* Unix time */
	uint32_t end;				/* Unix time */
	uint16_t sport;
	uint16_t dport;
	uint8_t l4_proto;
	uint32_t aid;				/* Application ID */
} __packed;

#define VY_IPFIX_SID		256

#define VY_IPFIX_FLOWS		36
#define VY_IPFIX_PKT_LEN	(IPFIX_HDRLEN + IPFIX_SET_HDRLEN \
							 + VY_IPFIX_FLOWS * sizeof(struct vy_ipfix_data))

/* message handling */
struct ipfix_msg *ipfix_msg_alloc(size_t, uint32_t, int);
void ipfix_msg_free(struct ipfix_msg *);
struct ipfix_hdr *ipfix_msg_hdr(const struct ipfix_msg *);
struct ipfix_templ_hdr *ipfix_msg_templ_hdr(const struct ipfix_msg *);
size_t ipfix_msg_len(const struct ipfix_msg *);
void *ipfix_msg_data(struct ipfix_msg *);
struct ipfix_set_hdr *ipfix_msg_add_set(struct ipfix_msg *, uint16_t);
void *ipfix_msg_add_data(struct ipfix_msg *, size_t);
int ipfix_dump_msg(const struct ipfix_msg *);

#endif /* IPFIX_H */
