/*
 * ipfix.c
 *
 * Holger Eitzenberger, 2009.
 * Ander Juaristi, 2019
 */

/* These forward declarations are needed since ulogd.h doesn't like to be the first */
#include <ulogd/linuxlist.h>

#define __packed		__attribute__((packed))

#include "ipfix.h"

#include <ulogd/ulogd.h>
#include <ulogd/common.h>
#include <ulogd/ipfix_protocol.h>

struct ipfix_templ_elem {
	uint16_t id;
	uint16_t len;
};

struct ipfix_templ {
	unsigned int num_templ_elements;
	struct ipfix_templ_elem templ_elements[];
};

/* Template fields modeled after vy_ipfix_data */
static const struct ipfix_templ template = {
	.num_templ_elements = 10,
	.templ_elements = {
		{
			.id = IPFIX_sourceIPv4Address,
			.len = sizeof(uint32_t)
		},
		{
			.id = IPFIX_destinationIPv4Address,
			.len = sizeof(uint32_t)
		},
		{
			.id = IPFIX_packetTotalCount,
			.len = sizeof(uint32_t)
		},
		{
			.id = IPFIX_octetTotalCount,
			.len = sizeof(uint32_t)
		},
		{
			.id = IPFIX_flowStartSeconds,
			.len = sizeof(uint32_t)
		},
		{
			.id = IPFIX_flowEndSeconds,
			.len = sizeof(uint32_t)
		},
		{
			.id = IPFIX_sourceTransportPort,
			.len = sizeof(uint16_t)
		},
		{
			.id = IPFIX_destinationTransportPort,
			.len = sizeof(uint16_t)
		},
		{
			.id = IPFIX_protocolIdentifier,
			.len = sizeof(uint8_t)
		},
		{
			.id = IPFIX_applicationId,
			.len = sizeof(uint32_t)
		}
	}
};

struct ipfix_msg *ipfix_msg_alloc(size_t len, uint32_t oid, int tid)
{
	struct ipfix_msg *msg;
	struct ipfix_hdr *hdr;
	struct ipfix_templ_hdr *templ_hdr;
	struct ipfix_templ_elem *elem;
	unsigned int i = 0;

	if ((tid > 0 && len < IPFIX_HDRLEN + IPFIX_TEMPL_HDRLEN(template.num_templ_elements) + IPFIX_SET_HDRLEN) ||
	    (len < IPFIX_HDRLEN + IPFIX_SET_HDRLEN))
		return NULL;

	msg = malloc(sizeof(struct ipfix_msg) + len);
	memset(msg, 0, sizeof(struct ipfix_msg));
	msg->tid = tid;
	msg->end = msg->data + len;
	msg->tail = msg->data + IPFIX_HDRLEN;
	if (tid > 0)
		msg->tail += IPFIX_TEMPL_HDRLEN(template.num_templ_elements);

	/* Initialize message header */
	hdr = ipfix_msg_hdr(msg);
	memset(hdr, 0, IPFIX_HDRLEN);
	hdr->version = htons(IPFIX_VERSION);
	hdr->oid = htonl(oid);

	if (tid > 0) {
		/* Initialize template record header */
		templ_hdr = ipfix_msg_templ_hdr(msg);
		templ_hdr->sid = htons(2);
		templ_hdr->tid = htons(tid);
		templ_hdr->len = htons(IPFIX_TEMPL_HDRLEN(template.num_templ_elements));
		templ_hdr->cnt = htons(template.num_templ_elements);

		while (i < template.num_templ_elements) {
			elem = (struct ipfix_templ_elem *) &templ_hdr->data[i * 4];
			elem->id = htons(template.templ_elements[i].id);
			elem->len = htons(template.templ_elements[i].len);
			i++;
		}
	}

	return msg;
}

void ipfix_msg_free(struct ipfix_msg *msg)
{
	if (!msg)
		return;

	if (msg->nrecs > 0)
		ulogd_log(ULOGD_DEBUG, "%s: %d flows have been lost\n", __func__,
			msg->nrecs);

	free(msg);
}

struct ipfix_templ_hdr *ipfix_msg_templ_hdr(const struct ipfix_msg *msg)
{
	if (msg->tid > 0)
		return (struct ipfix_templ_hdr *) (msg->data + IPFIX_HDRLEN);

	return NULL;
}

struct ipfix_hdr *ipfix_msg_hdr(const struct ipfix_msg *msg)
{
	return (struct ipfix_hdr *)msg->data;
}

void *ipfix_msg_data(struct ipfix_msg *msg)
{
	return msg->data;
}

size_t ipfix_msg_len(const struct ipfix_msg *msg)
{
	return msg->tail - msg->data;
}

struct ipfix_set_hdr *ipfix_msg_add_set(struct ipfix_msg *msg, uint16_t sid)
{
	struct ipfix_set_hdr *shdr;

	if (msg->end - msg->tail < (int) IPFIX_SET_HDRLEN)
		return NULL;

	shdr = (struct ipfix_set_hdr *)msg->tail;
	shdr->id = sid;
	shdr->len = IPFIX_SET_HDRLEN;
	msg->tail += IPFIX_SET_HDRLEN;
	msg->last_set = shdr;
	return shdr;
}

struct ipfix_set_hdr *ipfix_msg_get_set(const struct ipfix_msg *msg)
{
	return msg->last_set;
}

/**
 * Add data record to an IPFIX message.  The data is accounted properly.
 *
 * @return pointer to data or %NULL if not that much space left.
 */
void *ipfix_msg_add_data(struct ipfix_msg *msg, size_t len)
{
	void *data;

	if (!msg->last_set) {
		ulogd_log(ULOGD_FATAL, "msg->last_set is NULL\n");
		return NULL;
	}

	if ((ssize_t) len > msg->end - msg->tail)
		return NULL;

	data = msg->tail;
	msg->tail += len;
	msg->nrecs++;
	msg->last_set->len += len;

	return data;
}

/* check and dump message */
int ipfix_dump_msg(const struct ipfix_msg *msg)
{
	const struct ipfix_hdr *hdr = ipfix_msg_hdr(msg);
	const struct ipfix_set_hdr *shdr = (struct ipfix_set_hdr *) hdr->data;

	if (ntohs(hdr->len) < IPFIX_HDRLEN) {
		ulogd_log(ULOGD_FATAL, "Invalid IPFIX message header length\n");
		return -1;
	}
	if (ipfix_msg_len(msg) != IPFIX_HDRLEN + ntohs(shdr->len)) {
		ulogd_log(ULOGD_FATAL, "Invalid IPFIX message length\n");
		return -1;
	}

	ulogd_log(ULOGD_DEBUG, "msg: ver=%#x len=%#x t=%#x seq=%#x oid=%d\n",
			  ntohs(hdr->version), ntohs(hdr->len), htonl(hdr->time),
			  ntohl(hdr->seqno), ntohl(hdr->oid));

	return 0;
}

/* template management */
size_t ipfix_rec_len(uint16_t sid)
{
	if (sid != htons(VY_IPFIX_SID)) {
		ulogd_log(ULOGD_FATAL, "Invalid SID\n");
		return 0;
	}

	return sizeof(struct vy_ipfix_data);
}
