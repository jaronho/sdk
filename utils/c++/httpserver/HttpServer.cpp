/**********************************************************************
* Author:	jaron.ho
* Date:		2018-04-10
* Brief:	http server
**********************************************************************/
#include "HttpServer.h"
static int s_print_client_request_info = 0;
static bool startHttpServer(const char* ip, unsigned int port, void(*cb)(struct evhttp_request*, void*), void* arg, int pcri);
static const char* httpMethodName(evhttp_cmd_type method);
static void httpServerCallback(struct evhttp_request* req, void* arg);
static const char* handleHttpRequest(const char* method, const char* uri, const struct evkeyvalq* headers, const unsigned char* data);
//------------------------------------------------------------------------
static bool startHttpServer(const char* ip, unsigned int port, void (*cb)(struct evhttp_request*, void*), void* arg, int pcri) {
    if (!ip || 0 == strlen(ip) || 0 == port) {
        return false;
    }
	s_print_client_request_info = pcri;
    struct event_base* base = event_base_new();
    struct evhttp* http_server = evhttp_new(base);
    if (!http_server) {
        return false;
    }
    // bind address
    if (0 != evhttp_bind_socket(http_server, ip, port & 0xFFFF)) {
        return false;
    }
    // set http request handle callabck
    if (cb) {
        evhttp_set_gencb(http_server, cb, arg);
    }
    // start event loop, it will callback when http request triggered
	printf("start http server %s:%d ok ...\n", ip, port);
    event_base_dispatch(base);
    evhttp_free(http_server);
    return true;
}
//------------------------------------------------------------------------
static const char* httpMethodName(evhttp_cmd_type method) {
	if (EVHTTP_REQ_GET == method) {
		return "GET";
	} else if (EVHTTP_REQ_POST == method) {
		return "POST";
	} else if (EVHTTP_REQ_HEAD == method) {
		return "HEAD";
	} else if (EVHTTP_REQ_PUT == method) {
		return "PUT";
	} else if (EVHTTP_REQ_DELETE == method) {
		return "DELETE";
	} else if (EVHTTP_REQ_OPTIONS == method) {
		return "OPTIONS";
	} else if (EVHTTP_REQ_TRACE == method) {
		return "TRACE";
	} else if (EVHTTP_REQ_CONNECT == method) {
		return "CONNECT";
	} else if (EVHTTP_REQ_PATCH == method) {
		return "PATCH";
	}
	return "unknown";
}
//------------------------------------------------------------------------
static void httpServerCallback(struct evhttp_request* req, void* arg) {
    // cache client request info
	char major = req->major;													// http major version
	char minor = req->minor;													// http minor version
	const char* method = httpMethodName(evhttp_request_get_command(req));		// http method
	const char* host = evhttp_request_get_host(req);							// client host
	unsigned short port = req->remote_port;										// client port
	const struct evkeyvalq* headers = evhttp_request_get_input_headers(req);	// client headers
	struct evbuffer* buffer = evhttp_request_get_input_buffer(req);				// client buffer
	const unsigned char* body = evbuffer_pullup(buffer, -1);					// client body
	const char* uri = evhttp_request_get_uri(req);								// server uri
	// print client request info
	if (s_print_client_request_info) {
		printf("--------------------------------------------------[[\n");
		printf("HTTP / %d.%d\n", major, minor);
		printf("Receive a %s request from %s:%u\n", method, host, port);
		printf("Uri: %s\n", uri);
		printf("Headers:\n");
		if (headers) {
			struct evkeyval* header = headers->tqh_first;
			for (; header; header = header->next.tqe_next) {
				printf("    %s: %s\n", header->key, header->value);
			}
		}
		printf("Body:\n");
		printf("%s\n", body);
		printf("--------------------------------------------------]]\n");
	}
	// handle request
	const char* response = handleHttpRequest(method, uri, headers, body);
	// reply to client
	struct evbuffer* buf = evbuffer_new();
	if (!buf) {
		evhttp_send_error(req, HTTP_INTERNAL, "REPLY_ERROR");
		return;
	}
	if (response && strlen(response) > 0) {
		evbuffer_add_printf(buf, response);
	}
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}
//------------------------------------------------------------------------
static const char* handleHttpRequest(const char* method, const char* uri, const struct evkeyvalq* headers, const unsigned char* body) {
	std::map<std::string, std::string> datas;
	if (0 == strcmp("GET", method)) {
		struct evkeyvalq params;
		evhttp_parse_query(uri, &params);
		struct evkeyval* param = params.tqh_first;
		for (; param; param = param->next.tqe_next) {
			datas[param->key] = param->value;
		}
	} else if (0 == strcmp("POST", method)) {
		if (headers) {
			// parse content type
			std::string headerKey = "";
			std::string headerValue = "";
			struct evkeyval* header = headers->tqh_first;
			for (; header; header = header->next.tqe_next) {
				headerKey = header->key;
				std::transform(headerKey.begin(), headerKey.end(), headerKey.begin(), ::tolower);
				if ("content-type" == headerKey) {
					headerValue = header->value;
					std::transform(headerValue.begin(), headerValue.end(), headerValue.begin(), ::tolower);
					break;
				}
			}
			if ("application/x-www-form-urlencoded" == headerValue) {
				struct evkeyvalq params;
				evhttp_parse_query_str((const char*)body, &params);
				struct evkeyval* param = params.tqh_first;
				for (; param; param = param->next.tqe_next) {
					datas[param->key] = param->value;
				}
			} else {
				char buf[128] = { 0 };
				sprintf_s(buf, "now temporarily does not support multipart/form-data %s request, TODO", method);
				printf("%s\n", buf);
				return buf;
			}
		} else {
			char buf[128] = { 0 };
			sprintf_s(buf, "now temporarily does not support %s request without header, TODO", method);
			printf("%s\n", buf);
			return buf;
		}
	} else {
		char buf[128] = { 0 };
		sprintf_s(buf, "now temporarily does not support %s request, TODO", method);
		printf("%s\n", buf);
		return buf;
	}
	return HttpServer::getInstance()->handleRouter(method, uri, datas);
}
//------------------------------------------------------------------------
static HttpServer* mInstance = NULL;
//------------------------------------------------------------------------
HttpServer* HttpServer::getInstance(void) {
    if (!mInstance) {
        mInstance = new HttpServer();
		mInstance->mIsRunning = false;
    }
    return mInstance;
}
//------------------------------------------------------------------------
void HttpServer::destroyInstance(void) {
    if (mInstance) {
		std::map<std::string, HttpRouter*>::iterator iter = mInstance->mRouterMap.begin();
		for (; mInstance->mRouterMap.end() != iter; ++iter) {
			delete iter->second;
		}
        delete mInstance;
        mInstance = NULL;
    }
}
//------------------------------------------------------------------------
void HttpServer::addHeader(const std::string& name, const std::string& value) {
	if (name.empty() || value.empty()) {
		return;
	}
	mHeaderMap[name] = value;
}
//------------------------------------------------------------------------
void HttpServer::addRouter(const std::string& uri, HttpRouter* router) {
	if (uri.empty() || !router) {
		return;
	}
	if (mRouterMap[uri]) {
		delete router;
		printf("exist router for \"%s\"\n", uri.c_str());
	} else {
		mRouterMap[uri] = router;
	}
}
//------------------------------------------------------------------------
void HttpServer::addRouter(const std::string& uri, HTTP_ROUTER_CALLBACK callback) {
	if (uri.empty() || !callback) {
		return;
	}
	HttpRouter* router = new HttpRouter();
	router->support_get = true;
	router->support_post = true;
	router->callback = callback;
	addRouter(uri, router);
}
//------------------------------------------------------------------------
void HttpServer::addRouterGet(const std::string& uri, HTTP_ROUTER_CALLBACK callback) {
	if (uri.empty() || !callback) {
		return;
	}
	HttpRouter* router = new HttpRouter();
	router->support_get = true;
	router->support_post = false;
	router->callback = callback;
	addRouter(uri, router);
}
//------------------------------------------------------------------------
void HttpServer::addRouterPost(const std::string& uri, HTTP_ROUTER_CALLBACK callback) {
	if (uri.empty() || !callback) {
		return;
	}
	HttpRouter* router = new HttpRouter();
	router->support_get = false;
	router->support_post = true;
	router->callback = callback;
	addRouter(uri, router);
}
//------------------------------------------------------------------------
const char* HttpServer::handleRouter(const char* method, const char* uri, const std::map<std::string, std::string>& datas) {
	if (!method || 0 == strlen(method) || !uri || 0 == strlen(uri)) {
		return NULL;
	}
	if (!mRouterMap[uri]) {
		printf("can not find router for \"%s\"\n", uri);
		return NULL;
	}
	if (0 == strcmp("GET", method) && !mRouterMap[uri]->support_get) {
		printf("can not support GET for \"%s\"\n", uri);
		return NULL;
	}
	if (0 == strcmp("POST", method) && !mRouterMap[uri]->support_post) {
		printf("can not support POST for \"%s\"\n", uri);
		return NULL;
	}
	if (mRouterMap[uri]->callback) {
		return mRouterMap[uri]->callback(datas);
	}
	return NULL;
}
//------------------------------------------------------------------------
void HttpServer::run(const std::string& ip, unsigned int port, bool pcri /*= true*/) {
    if (ip.empty() || 0 == port || mIsRunning) {
        return;
    }
	mIsRunning = true;
#ifdef WIN32
	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		printf("WSA startup failed\n");
		return;
	}
#endif
	if (!startHttpServer(ip.c_str(), port, httpServerCallback, NULL, pcri ? 1 : 0)) {
		printf("start http server %s:%d failed ...\n", ip.c_str(), port);
	}
#ifdef WIN32
	WSACleanup();
#endif
}
//------------------------------------------------------------------------
