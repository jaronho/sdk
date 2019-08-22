/* ulogd_output_JSON.c
 *
 * ulogd output target for logging to a file in JSON format.
 *
 * (C) 2014 by Eric Leblond <eric@regit.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <ulogd/ulogd.h>
#include <ulogd/conffile.h>
#include <jansson.h>

#ifndef ULOGD_JSON_DEFAULT
#define ULOGD_JSON_DEFAULT	"/var/log/ulogd.json"
#endif

#ifndef ULOGD_JSON_DEFAULT_DEVICE
#define ULOGD_JSON_DEFAULT_DEVICE "Netfilter"
#endif

#define host_ce(x)	(x->ces[JSON_CONF_HOST])
#define port_ce(x)	(x->ces[JSON_CONF_PORT])
#define mode_ce(x)	(x->ces[JSON_CONF_MODE])
#define file_ce(x)	(x->ces[JSON_CONF_FILENAME])
#define unlikely(x) __builtin_expect((x),0)

struct json_priv {
	FILE *of;
	int sec_idx;
	int usec_idx;
	long cached_gmtoff;
	char cached_tz[6];	/* eg +0200 */
	int mode;
	int sock;
};

enum json_mode {
	JSON_MODE_FILE = 0,
	JSON_MODE_TCP,
	JSON_MODE_UDP,
	JSON_MODE_UNIX
};

enum json_conf {
	JSON_CONF_FILENAME = 0,
	JSON_CONF_SYNC,
	JSON_CONF_TIMESTAMP,
	JSON_CONF_EVENTV1,
	JSON_CONF_DEVICE,
	JSON_CONF_BOOLEAN_LABEL,
	JSON_CONF_MODE,
	JSON_CONF_HOST,
	JSON_CONF_PORT,
	JSON_CONF_MAX
};

static struct config_keyset json_kset = {
	.num_ces = JSON_CONF_MAX,
	.ces = {
		[JSON_CONF_FILENAME] = {
			.key = "file",
			.type = CONFIG_TYPE_STRING,
			.options = CONFIG_OPT_NONE,
			.u = {.string = ULOGD_JSON_DEFAULT },
		},
		[JSON_CONF_SYNC] = {
			.key = "sync",
			.type = CONFIG_TYPE_INT,
			.options = CONFIG_OPT_NONE,
			.u = { .value = 0 },
		},
		[JSON_CONF_TIMESTAMP] = {
			.key = "timestamp",
			.type = CONFIG_TYPE_INT,
			.options = CONFIG_OPT_NONE,
			.u = { .value = 1 },
		},
		[JSON_CONF_EVENTV1] = {
			.key = "eventv1",
			.type = CONFIG_TYPE_INT,
			.options = CONFIG_OPT_NONE,
			.u = { .value = 0 },
		},
		[JSON_CONF_DEVICE] = {
			.key = "device",
			.type = CONFIG_TYPE_STRING,
			.options = CONFIG_OPT_NONE,
			.u = { .string = ULOGD_JSON_DEFAULT_DEVICE },
		},
		[JSON_CONF_BOOLEAN_LABEL] = {
			.key = "boolean_label",
			.type = CONFIG_TYPE_INT,
			.options = CONFIG_OPT_NONE,
			.u = { .value = 0 },
		},
		[JSON_CONF_MODE] = {
			.key = "mode",
			.type = CONFIG_TYPE_STRING,
			.options = CONFIG_OPT_NONE,
			.u = { .string = "file" },
		},
		[JSON_CONF_HOST] = {
			.key = "host",
			.type = CONFIG_TYPE_STRING,
			.options = CONFIG_OPT_NONE,
			.u = { .string = "127.0.0.1" },
		},
		[JSON_CONF_PORT] = {
			.key = "port",
			.type = CONFIG_TYPE_STRING,
			.options = CONFIG_OPT_NONE,
			.u = { .string = "12345" },
		},
	},
};

static void close_socket(struct json_priv *op) {
	if (op->sock != -1) {
		close(op->sock);
		op->sock = -1;
	}
}

static int _connect_socket_unix(struct ulogd_pluginstance *pi)
{
	struct json_priv *op = (struct json_priv *) &pi->private;
	struct sockaddr_un u_addr;
	int sfd;

	close_socket(op);

	ulogd_log(ULOGD_DEBUG, "connecting to unix:%s\n",
		  file_ce(pi->config_kset).u.string);

	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd == -1) {
		return -1;
	}
	u_addr.sun_family = AF_UNIX;
	strncpy(u_addr.sun_path, file_ce(pi->config_kset).u.string,
		sizeof(u_addr.sun_path) - 1);
	if (connect(sfd, (struct sockaddr *) &u_addr, sizeof(struct sockaddr_un)) == -1) {
		close(sfd);
		return -1;
	}

	op->sock = sfd;

	return 0;
}

static int _connect_socket_net(struct ulogd_pluginstance *pi)
{
	struct json_priv *op = (struct json_priv *) &pi->private;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;

	close_socket(op);

	ulogd_log(ULOGD_DEBUG, "connecting to %s:%s\n",
		  host_ce(pi->config_kset).u.string,
		  port_ce(pi->config_kset).u.string);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = op->mode == JSON_MODE_UDP ? SOCK_DGRAM : SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	s = getaddrinfo(host_ce(pi->config_kset).u.string,
			port_ce(pi->config_kset).u.string, &hints, &result);
	if (s != 0) {
		ulogd_log(ULOGD_ERROR, "getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		int on = 1;

		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1)
			continue;

		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
			   (char *) &on, sizeof(on));

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;

		close(sfd);
	}

	freeaddrinfo(result);

	if (rp == NULL) {
		return -1;
	}

	op->sock = sfd;

	return 0;
}

static int _connect_socket(struct ulogd_pluginstance *pi)
{
	struct json_priv *op = (struct json_priv *) &pi->private;

	if (op->mode == JSON_MODE_UNIX)
		return _connect_socket_unix(pi);
	else
		return _connect_socket_net(pi);
}

static int json_interp_socket(struct ulogd_pluginstance *upi, char *buf, int buflen)
{
	struct json_priv *opi = (struct json_priv *) &upi->private;
	int ret = 0;

	if (opi->sock != -1)
		ret = send(opi->sock, buf, buflen, MSG_NOSIGNAL);
	free(buf);
	if (ret != buflen) {
		ulogd_log(ULOGD_ERROR, "Failure sending message: %s\n",
			  strerror(errno));
		if (ret == -1 || opi->sock == -1)
			return _connect_socket(upi);
		else
			return ULOGD_IRET_ERR;
	}

	return ULOGD_IRET_OK;
}

static int json_interp_file(struct ulogd_pluginstance *upi, char *buf)
{
	struct json_priv *opi = (struct json_priv *) &upi->private;

	fprintf(opi->of, "%s", buf);
	free(buf);

	if (upi->config_kset->ces[JSON_CONF_SYNC].u.value != 0)
		fflush(opi->of);

	return ULOGD_IRET_OK;
}

#define MAX_LOCAL_TIME_STRING 38

static int json_interp(struct ulogd_pluginstance *upi)
{
	struct json_priv *opi = (struct json_priv *) &upi->private;
	unsigned int i;
	char *buf;
	int buflen;
	json_t *msg;

	msg = json_object();
	if (!msg) {
		ulogd_log(ULOGD_ERROR, "Unable to create JSON object\n");
		return ULOGD_IRET_ERR;
	}

	if (upi->config_kset->ces[JSON_CONF_EVENTV1].u.value != 0)
		json_object_set_new(msg, "@version", json_integer(1));

	if (upi->config_kset->ces[JSON_CONF_TIMESTAMP].u.value != 0) {
		time_t now;
		char timestr[MAX_LOCAL_TIME_STRING];
		struct tm *t;
		struct tm result;
		struct ulogd_key *inp = upi->input.keys;


		if (pp_is_valid(inp, opi->sec_idx))
			now = (time_t) ikey_get_u64(&inp[opi->sec_idx]);
		else
			now = time(NULL);
		t = localtime_r(&now, &result);
		if (unlikely(*opi->cached_tz = '\0' || t->tm_gmtoff != opi->cached_gmtoff)) {
			snprintf(opi->cached_tz, sizeof(opi->cached_tz),
				 "%c%02d%02d",
				 t->tm_gmtoff > 0 ? '+' : '-',
				 abs(t->tm_gmtoff) / 60 / 60,
				 abs(t->tm_gmtoff) / 60 % 60);
		}

		if (pp_is_valid(inp, opi->usec_idx)) {
			snprintf(timestr, MAX_LOCAL_TIME_STRING,
					"%04d-%02d-%02dT%02d:%02d:%02d.%06u%s",
					t->tm_year + 1900, t->tm_mon + 1,
					t->tm_mday, t->tm_hour,
					t->tm_min, t->tm_sec,
					ikey_get_u32(&inp[opi->usec_idx]),
					opi->cached_tz);
		} else {
			snprintf(timestr, MAX_LOCAL_TIME_STRING,
					"%04d-%02d-%02dT%02d:%02d:%02d%s",
					t->tm_year + 1900, t->tm_mon + 1,
					t->tm_mday, t->tm_hour,
					t->tm_min, t->tm_sec,
					opi->cached_tz);
		}

		if (upi->config_kset->ces[JSON_CONF_EVENTV1].u.value != 0)
			json_object_set_new(msg, "@timestamp", json_string(timestr));
		else
			json_object_set_new(msg, "timestamp", json_string(timestr));
	}

	if (upi->config_kset->ces[JSON_CONF_DEVICE].u.string) {
		char *dvc = upi->config_kset->ces[JSON_CONF_DEVICE].u.string;
		json_object_set_new(msg, "dvc", json_string(dvc));
	}



	for (i = 0; i < upi->input.num_keys; i++) {
		struct ulogd_key *key = upi->input.keys[i].u.source;
		char *field_name;

		if (!key)
			continue;

		if (!IS_VALID(*key))
			continue;

		field_name = key->cim_name ? key->cim_name : key->name;

		switch (key->type) {
		case ULOGD_RET_STRING:
			json_object_set_new(msg, field_name, json_string(key->u.value.ptr));
			break;
		case ULOGD_RET_BOOL:
		case ULOGD_RET_INT8:
			json_object_set_new(msg, field_name, json_integer(key->u.value.i8));
			break;
		case ULOGD_RET_INT16:
			json_object_set_new(msg, field_name, json_integer(key->u.value.i16));
			break;
		case ULOGD_RET_INT32:
			json_object_set_new(msg, field_name, json_integer(key->u.value.i32));
			break;
		case ULOGD_RET_UINT8:
			if ((upi->config_kset->ces[JSON_CONF_BOOLEAN_LABEL].u.value != 0)
					&& (!strcmp(key->name, "raw.label"))) {
				if (key->u.value.ui8)
					json_object_set_new(msg, "action", json_string("allowed"));
				else
					json_object_set_new(msg, "action", json_string("blocked"));
				break;
			}
			json_object_set_new(msg, field_name, json_integer(key->u.value.ui8));
			break;
		case ULOGD_RET_UINT16:
			json_object_set_new(msg, field_name, json_integer(key->u.value.ui16));
			break;
		case ULOGD_RET_UINT32:
			json_object_set_new(msg, field_name, json_integer(key->u.value.ui32));
			break;
		case ULOGD_RET_UINT64:
			json_object_set_new(msg, field_name, json_integer(key->u.value.ui64));
			break;
		default:
			/* don't know how to interpret this key. */
			break;
		}
	}


	buf = json_dumps(msg, 0);
	json_decref(msg);
	if (buf == NULL) {
		ulogd_log(ULOGD_ERROR, "Could not create message\n");
		return ULOGD_IRET_ERR;
	}
	buflen = strlen(buf);
	buf = realloc(buf, sizeof(char)*(buflen+2));
	if (buf == NULL) {
		ulogd_log(ULOGD_ERROR, "Could not create message\n");
		return ULOGD_IRET_ERR;
	}
	strncat(buf, "\n", 1);
	buflen++;

	if (opi->mode == JSON_MODE_FILE)
		return json_interp_file(upi, buf);
	else
		return json_interp_socket(upi, buf, buflen);
}

static void reopen_file(struct ulogd_pluginstance *upi)
{
	struct json_priv *oi = (struct json_priv *) &upi->private;
	FILE *old = oi->of;

	ulogd_log(ULOGD_NOTICE, "JSON: reopening logfile\n");
	oi->of = fopen(upi->config_kset->ces[0].u.string, "a");
	if (!oi->of) {
		ulogd_log(ULOGD_ERROR, "can't open JSON "
				       "log file: %s\n",
			  strerror(errno));
		oi->of = old;
	} else {
		fclose(old);
	}
}

static void reopen_socket(struct ulogd_pluginstance *upi)
{
	ulogd_log(ULOGD_NOTICE, "JSON: reopening socket\n");
	if (_connect_socket(upi) < 0) {
		ulogd_log(ULOGD_ERROR, "can't open JSON "
				       "socket: %s\n",
			  strerror(errno));
	}
}

static void sighup_handler_print(struct ulogd_pluginstance *upi, int signal)
{
	struct json_priv *oi = (struct json_priv *) &upi->private;

	switch (signal) {
	case SIGHUP:
		if (oi->mode == JSON_MODE_FILE)
			reopen_file(upi);
		else
			reopen_socket(upi);
		break;
	default:
		break;
	}
}

static int json_configure(struct ulogd_pluginstance *upi,
			    struct ulogd_pluginstance_stack *stack)
{
	struct json_priv *op = (struct json_priv *) &upi->private;
	char *mode_str = mode_ce(upi->config_kset).u.string;
	int ret;

	ret = ulogd_wildcard_inputkeys(upi);
	if (ret < 0)
		return ret;

	ret = config_parse_file(upi->id, upi->config_kset);
	if (ret < 0)
		return ret;

	if (!strcasecmp(mode_str, "udp")) {
		op->mode = JSON_MODE_UDP;
	} else if (!strcasecmp(mode_str, "tcp")) {
		op->mode = JSON_MODE_TCP;
	} else if (!strcasecmp(mode_str, "unix")) {
		op->mode = JSON_MODE_UNIX;
	} else if (!strcasecmp(mode_str, "file")) {
		op->mode = JSON_MODE_FILE;
	} else {
		ulogd_log(ULOGD_ERROR, "unknown mode '%s'\n", mode_str);
		return -EINVAL;
	}

	return 0;
}

static int json_init_file(struct ulogd_pluginstance *upi)
{
	struct json_priv *op = (struct json_priv *) &upi->private;

	op->of = fopen(upi->config_kset->ces[0].u.string, "a");
	if (!op->of) {
		ulogd_log(ULOGD_FATAL, "can't open JSON log file: %s\n",
			strerror(errno));
		return -1;
	}

	return 0;
}

static int json_init_socket(struct ulogd_pluginstance *upi)
{
	struct json_priv *op = (struct json_priv *) &upi->private;

	if (host_ce(upi->config_kset).u.string == NULL)
		return -1;
	if (port_ce(upi->config_kset).u.string == NULL)
		return -1;

	op->sock = -1;
	return _connect_socket(upi);
}

static int json_init(struct ulogd_pluginstance *upi)
{
	struct json_priv *op = (struct json_priv *) &upi->private;
	unsigned int i;

	/* search for time */
	op->sec_idx = -1;
	op->usec_idx = -1;
	for (i = 0; i < upi->input.num_keys; i++) {
		struct ulogd_key *key = upi->input.keys[i].u.source;
		if (!strcmp(key->name, "oob.time.sec"))
			op->sec_idx = i;
		else if (!strcmp(key->name, "oob.time.usec"))
			op->usec_idx = i;
	}

	*op->cached_tz = '\0';

	if (op->mode == JSON_MODE_FILE)
		return json_init_file(upi);
	else
		return json_init_socket(upi);
}

static void close_file(FILE *of) {
	if (of != stdout)
		fclose(of);
}

static int json_fini(struct ulogd_pluginstance *pi)
{
	struct json_priv *op = (struct json_priv *) &pi->private;

	if (op->mode == JSON_MODE_FILE)
		close_file(op->of);
	else
		close_socket(op);

	return 0;
}

static struct ulogd_plugin json_plugin = {
	.name = "JSON",
	.input = {
		.type = ULOGD_DTYPE_PACKET | ULOGD_DTYPE_FLOW | ULOGD_DTYPE_SUM,
	},
	.output = {
		.type = ULOGD_DTYPE_SINK,
	},
	.configure = &json_configure,
	.interp	= &json_interp,
	.start 	= &json_init,
	.stop	= &json_fini,
	.signal = &sighup_handler_print,
	.config_kset = &json_kset,
	.version = VERSION,
};

void __attribute__ ((constructor)) init(void);

void init(void)
{
	ulogd_register_plugin(&json_plugin);
}
