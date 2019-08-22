/*
 * ulogd_output_IPFIX.c
 *
 * ulogd IPFIX Exporter plugin.
 *
 * (C) 2009 by Holger Eitzenberger <holger@eitzenberger.org>, Astaro AG
 * (C) 2019 by Ander Juaristi <a@juaristi.eus>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ulogd/ulogd.h>
#include <ulogd/common.h>

#include "ipfix.h"

#define DEFAULT_MTU		512 /* RFC 5101, 10.3.3 */
#define DEFAULT_PORT		4739 /* RFC 5101, 10.3.4 */
#define DEFAULT_SPORT		4740
#define DEFAULT_SEND_TEMPLATE	"once"

enum {
	OID_CE = 0,
	HOST_CE,
	PORT_CE,
	PROTO_CE,
	MTU_CE,
	SEND_TEMPLATE_CE
};

#define oid_ce(x)		(x->ces[OID_CE])
#define host_ce(x)		(x->ces[HOST_CE])
#define port_ce(x)		(x->ces[PORT_CE])
#define proto_ce(x)		(x->ces[PROTO_CE])
#define mtu_ce(x)		(x->ces[MTU_CE])
#define send_template_ce(x)	(x->ces[SEND_TEMPLATE_CE])

static const struct config_keyset ipfix_kset = {
	.num_ces = 6,
	.ces = {
		{
			.key = "oid",
			.type = CONFIG_TYPE_INT,
			.u.value = 0
		},
		{
			.key = "host",
			.type = CONFIG_TYPE_STRING,
			.u.string = ""
		},
		{
			.key = "port",
			.type = CONFIG_TYPE_INT,
			.u.value = DEFAULT_PORT
		},
		{
			.key = "proto",
			.type = CONFIG_TYPE_STRING,
			.u.string = "tcp"
		},
		{
			.key = "mtu",
			.type = CONFIG_TYPE_INT,
			.u.value = DEFAULT_MTU
		},
		{
			.key = "send_template",
			.type = CONFIG_TYPE_STRING,
			.u.string = DEFAULT_SEND_TEMPLATE
		}
	}
};

struct ipfix_priv {
	struct ulogd_fd ufd;
	uint32_t seqno;
	struct ipfix_msg *msg;		/* current message */
	struct llist_head list;
	int tid;
	int proto;
	struct ulogd_timer timer;
	struct sockaddr_in sa;
};

enum {
	InIpSaddr = 0,
	InIpDaddr,
	InRawInPktCount,
	InRawInPktLen,
	InRawOutPktCount,
	InRawOutPktLen,
	InFlowStartSec,
	InFlowStartUsec,
	InFlowEndSec,
	InFlowEndUsec,
	InL4SPort,
	InL4DPort,
	InIpProto,
	InCtMark
};

static struct ulogd_key ipfix_in_keys[] = {
		[InIpSaddr] = {
			.type = ULOGD_RET_IPADDR,
			.name = "orig.ip.saddr"
		},
		[InIpDaddr] = {
			.type = ULOGD_RET_IPADDR,
			.name = "orig.ip.daddr"
		},
		[InRawInPktCount] = {
			.type = ULOGD_RET_UINT64,
			.name = "orig.raw.pktcount"
		},
		[InRawInPktLen] = {
			.type = ULOGD_RET_UINT64,
			.name = "orig.raw.pktlen"
		},
		[InRawOutPktCount] = {
			.type = ULOGD_RET_UINT64,
			.name = "reply.raw.pktcount"
		},
		[InRawOutPktLen] = {
			.type = ULOGD_RET_UINT64,
			.name = "reply.raw.pktlen"
		},
		[InFlowStartSec] = {
			.type = ULOGD_RET_UINT32,
			.name = "flow.start.sec"
		},
		[InFlowStartUsec] = {
			.type = ULOGD_RET_UINT32,
			.name = "flow.start.usec"
		},
		[InFlowEndSec] = {
			.type = ULOGD_RET_UINT32,
			.name = "flow.end.sec"
		},
		[InFlowEndUsec] = {
			.type = ULOGD_RET_UINT32,
			.name = "flow.end.usec"
		},
		[InL4SPort] = {
			.type = ULOGD_RET_UINT16,
			.name = "orig.l4.sport"
		},
		[InL4DPort] = {
			.type = ULOGD_RET_UINT16,
			.name = "orig.l4.dport"
		},
		[InIpProto] = {
			.type = ULOGD_RET_UINT8,
			.name = "orig.ip.protocol"
		},
		[InCtMark] = {
			.type = ULOGD_RET_UINT32,
			.name = "ct.mark"
		}
};

/* do some polishing and enqueue it */
static void enqueue_msg(struct ipfix_priv *priv, struct ipfix_msg *msg)
{
	struct ipfix_hdr *hdr = ipfix_msg_data(msg);

	if (!msg)
		return;

	hdr->time = htonl(time(NULL));
	hdr->seqno = htonl(priv->seqno += msg->nrecs);
	if (msg->last_set) {
		msg->last_set->id = htons(msg->last_set->id);
		msg->last_set->len = htons(msg->last_set->len);
		msg->last_set = NULL;
	}
	hdr->len = htons(ipfix_msg_len(msg));

	llist_add(&msg->link, &priv->list);
}

/**
 * @return %ULOGD_IRET_OK or error value
 */
static int send_msgs(struct ulogd_pluginstance *pi)
{
	struct ipfix_priv *priv = (struct ipfix_priv *) &pi->private;
	struct llist_head *curr, *tmp;
	struct ipfix_msg *msg;
	int ret = ULOGD_IRET_OK, sent;

	llist_for_each_prev(curr, &priv->list) {
		msg = llist_entry(curr, struct ipfix_msg, link);

		sent = send(priv->ufd.fd, ipfix_msg_data(msg), ipfix_msg_len(msg), 0);
		if (sent < 0) {
			ulogd_log(ULOGD_ERROR, "send: %m\n");
			ret = ULOGD_IRET_ERR;
			goto done;
		}

		/* TODO handle short send() for other protocols */
		if ((size_t) sent < ipfix_msg_len(msg))
			ulogd_log(ULOGD_ERROR, "short send: %d < %d\n",
					sent, ipfix_msg_len(msg));
	}

	llist_for_each_safe(curr, tmp, &priv->list) {
		msg = llist_entry(curr, struct ipfix_msg, link);
		llist_del(curr);
		msg->nrecs = 0;
		ipfix_msg_free(msg);
	}

done:
	return ret;
}

static int ipfix_ufd_cb(int fd, unsigned what, void *arg)
{
	struct ulogd_pluginstance *pi = arg;
	struct ipfix_priv *priv = (struct ipfix_priv *) pi->private;
	ssize_t nread;
	char buf[16];

	if (what & ULOGD_FD_READ) {
		nread = recv(priv->ufd.fd, buf, sizeof(buf), MSG_DONTWAIT);
		if (nread < 0) {
			ulogd_log(ULOGD_ERROR, "recv: %m\n");
		} else if (!nread) {
			ulogd_log(ULOGD_INFO, "connection reset by peer\n");
			ulogd_unregister_fd(&priv->ufd);
		} else
			ulogd_log(ULOGD_INFO, "unexpected data (%d bytes)\n", nread);
	}

	return 0;
}

static void ipfix_timer_cb(struct ulogd_timer *t, void *data)
{
	struct ulogd_pluginstance *pi = data;
	struct ipfix_priv *priv = (struct ipfix_priv *) &pi->private;

	if (priv->msg && priv->msg->nrecs > 0) {
		enqueue_msg(priv, priv->msg);
		priv->msg = NULL;
		send_msgs(pi);
	}
}

static int ipfix_configure(struct ulogd_pluginstance *pi, struct ulogd_pluginstance_stack *stack)
{
	struct ipfix_priv *priv = (struct ipfix_priv *) &pi->private;
	char *host, *proto, *send_template;
	int oid, port, mtu, ret;
	char addr[16];

	ret = config_parse_file(pi->id, pi->config_kset);
	if (ret < 0)
		return ret;

	oid = oid_ce(pi->config_kset).u.value;
	host = host_ce(pi->config_kset).u.string;
	port = port_ce(pi->config_kset).u.value;
	proto = proto_ce(pi->config_kset).u.string;
	mtu = mtu_ce(pi->config_kset).u.value;
	send_template = send_template_ce(pi->config_kset).u.string;

	if (!oid) {
		ulogd_log(ULOGD_FATAL, "invalid Observation ID\n");
		return ULOGD_IRET_ERR;
	}
	if (!host || !strcmp(host, "")) {
		ulogd_log(ULOGD_FATAL, "no destination host specified\n");
		return ULOGD_IRET_ERR;
	}

	if (!strcmp(proto, "udp")) {
		priv->proto = IPPROTO_UDP;
	} else if (!strcmp(proto, "tcp")) {
		priv->proto = IPPROTO_TCP;
	} else {
		ulogd_log(ULOGD_FATAL, "unsupported protocol '%s'\n", proto);
		return ULOGD_IRET_ERR;
	}

	memset(&priv->sa, 0, sizeof(priv->sa));
	priv->sa.sin_family = AF_INET;
	priv->sa.sin_port = htons(port);
	ret = inet_pton(AF_INET, host, &priv->sa.sin_addr);
	if (ret <= 0) {
		ulogd_log(ULOGD_FATAL, "inet_pton: %m\n");
		return ULOGD_IRET_ERR;
	}

	INIT_LLIST_HEAD(&priv->list);

	ulogd_init_timer(&priv->timer, pi, ipfix_timer_cb);

	priv->tid = (strcmp(send_template, "never") ? VY_IPFIX_SID : -1);

	ulogd_log(ULOGD_INFO, "using IPFIX Collector at %s:%d (MTU %d)\n",
		  inet_ntop(AF_INET, &priv->sa.sin_addr, addr, sizeof(addr)),
		  port, mtu);

	return ULOGD_IRET_OK;
}

static int tcp_connect(struct ulogd_pluginstance *pi)
{
	struct ipfix_priv *priv = (struct ipfix_priv *) &pi->private;
	int ret = ULOGD_IRET_ERR;

	if ((priv->ufd.fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		ulogd_log(ULOGD_FATAL, "socket: %m\n");
		return ULOGD_IRET_ERR;
	}

	if (connect(priv->ufd.fd, (struct sockaddr *) &priv->sa, sizeof(priv->sa)) < 0) {
		ulogd_log(ULOGD_ERROR, "connect: %m\n");
		ret = ULOGD_IRET_ERR;
		goto err_close;
	}

	return ULOGD_IRET_OK;

err_close:
	close(priv->ufd.fd);
	return ret;
}

static int udp_connect(struct ulogd_pluginstance *pi)
{
	struct ipfix_priv *priv = (struct ipfix_priv *) &pi->private;

	if ((priv->ufd.fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		ulogd_log(ULOGD_FATAL, "socket: %m\n");
		return ULOGD_IRET_ERR;
	}

	if (connect(priv->ufd.fd, (struct sockaddr *) &priv->sa, sizeof(priv->sa)) < 0) {
		ulogd_log(ULOGD_ERROR, "connect: %m\n");
		return ULOGD_IRET_ERR;
	}

	return 0;
}

static int ipfix_start(struct ulogd_pluginstance *pi)
{
	struct ipfix_priv *priv = (struct ipfix_priv *) &pi->private;
	char addr[16];
	int port, ret;

	switch (priv->proto) {
	case IPPROTO_UDP:
		if ((ret = udp_connect(pi)) < 0)
			return ret;
		break;
	case IPPROTO_TCP:
		if ((ret = tcp_connect(pi)) < 0)
			return ret;
		break;

	default:
		break;
	}

	priv->seqno = 0;

	port = port_ce(pi->config_kset).u.value;
	ulogd_log(ULOGD_INFO, "connected to %s:%d\n",
			inet_ntop(AF_INET, &priv->sa.sin_addr, addr, sizeof(addr)),
			port);

	/* Register the socket FD */
	priv->ufd.when = ULOGD_FD_READ;
	priv->ufd.cb = ipfix_ufd_cb;
	priv->ufd.data = pi;

	if (ulogd_register_fd(&priv->ufd) < 0)
		return ULOGD_IRET_ERR;

	/* Add a 1 second timer */
	ulogd_add_timer(&priv->timer, 1);

	return ULOGD_IRET_OK;
}

static int ipfix_stop(struct ulogd_pluginstance *pi)
{
	struct ipfix_priv *priv = (struct ipfix_priv *) &pi->private;

	ulogd_unregister_fd(&priv->ufd);
	close(priv->ufd.fd);
	priv->ufd.fd = -1;

	ulogd_del_timer(&priv->timer);

	ipfix_msg_free(priv->msg);
	priv->msg = NULL;

	return 0;
}

static int ipfix_interp(struct ulogd_pluginstance *pi)
{
	struct ipfix_priv *priv = (struct ipfix_priv *) &pi->private;
	char saddr[16], daddr[16], *send_template;
	struct vy_ipfix_data *data;
	int oid, mtu, ret;

	if (!(GET_FLAGS(pi->input.keys, InIpSaddr) & ULOGD_RETF_VALID))
		return ULOGD_IRET_OK;

	oid = oid_ce(pi->config_kset).u.value;
	mtu = mtu_ce(pi->config_kset).u.value;
	send_template = send_template_ce(pi->config_kset).u.string;

again:
	if (!priv->msg) {
		priv->msg = ipfix_msg_alloc(mtu, oid, priv->tid);
		if (!priv->msg) {
			/* just drop this flow */
			ulogd_log(ULOGD_ERROR, "out of memory, dropping flow\n");
			return ULOGD_IRET_OK;
		}
		ipfix_msg_add_set(priv->msg, VY_IPFIX_SID);

		/* template sent - do not send it again the next time */
		if (priv->tid == VY_IPFIX_SID && strcmp(send_template, "once") == 0)
			priv->tid = -1;
	}

	data = ipfix_msg_add_data(priv->msg, sizeof(struct vy_ipfix_data));
	if (!data) {
		enqueue_msg(priv, priv->msg);
		priv->msg = NULL;
		/* can't loop because the next will definitely succeed */
		goto again;
	}

	data->saddr.s_addr = ikey_get_u32(&pi->input.keys[InIpSaddr]);
	data->daddr.s_addr = ikey_get_u32(&pi->input.keys[InIpDaddr]);

	data->packets = htonl((uint32_t) (ikey_get_u64(&pi->input.keys[InRawInPktCount])
						+ ikey_get_u64(&pi->input.keys[InRawOutPktCount])));
	data->bytes = htonl((uint32_t) (ikey_get_u64(&pi->input.keys[InRawInPktLen])
						+ ikey_get_u64(&pi->input.keys[InRawOutPktLen])));

	data->start = htonl(ikey_get_u32(&pi->input.keys[InFlowStartSec]));
	data->end = htonl(ikey_get_u32(&pi->input.keys[InFlowEndSec]));

	if (GET_FLAGS(pi->input.keys, InL4SPort) & ULOGD_RETF_VALID) {
		data->sport = htons(ikey_get_u16(&pi->input.keys[InL4SPort]));
		data->dport = htons(ikey_get_u16(&pi->input.keys[InL4DPort]));
	}

	data->aid = 0;
	if (GET_FLAGS(pi->input.keys, InCtMark) & ULOGD_RETF_VALID)
		data->aid = htonl(ikey_get_u32(&pi->input.keys[InCtMark]));

	data->l4_proto = ikey_get_u8(&pi->input.keys[InIpProto]);

	ulogd_log(ULOGD_DEBUG, "Got new packet (packets = %u, bytes = %u, flow = (%u, %u), saddr = %s, daddr = %s, sport = %u, dport = %u)\n",
		  ntohl(data->packets), ntohl(data->bytes), ntohl(data->start), ntohl(data->end),
		  inet_ntop(AF_INET, &data->saddr.s_addr, saddr, sizeof(saddr)),
		  inet_ntop(AF_INET, &data->daddr.s_addr, daddr, sizeof(daddr)),
		  ntohs(data->sport), ntohs(data->dport));

	if ((ret = send_msgs(pi)) < 0)
		return ret;

	return ULOGD_IRET_OK;
}

static struct ulogd_plugin ipfix_plugin = {
	.name = "IPFIX",
	.input = {
		.keys = ipfix_in_keys,
		.num_keys = ARRAY_SIZE(ipfix_in_keys),
		.type = ULOGD_DTYPE_PACKET | ULOGD_DTYPE_FLOW | ULOGD_DTYPE_SUM
	},
	.output = {
		.type = ULOGD_DTYPE_SINK
	},
	.config_kset = (struct config_keyset *) &ipfix_kset,
	.priv_size = sizeof(struct ipfix_priv),
	.configure = ipfix_configure,
	.start = ipfix_start,
	.stop = ipfix_stop,
	.interp = ipfix_interp,
	.version = VERSION,
};

void __attribute__ ((constructor)) init(void);

void init(void)
{
	ulogd_register_plugin(&ipfix_plugin);
}
