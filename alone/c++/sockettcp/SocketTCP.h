/**********************************************************************
* Author£ºjaron
* Date£º2015-09-18
* Brief£ºsocket tcp/ip
**********************************************************************/
#ifndef _SOCKET_TCP_H_
#define _SOCKET_TCP_H_

class SocketTCP {
public:
	// return:socketfd
	static int create(void);

	static void close(int socketfd, bool clean = true);

	static int setBlock(int socketfd, bool isBlock = true);

	// name:SO_DEBUG,SO_REUSEADDR,SO_TYPE,SO_ERROR,SO_DONTROUTE,SO_BROADCAST,SO_SNDBUF,SO_RCVBUF,SO_KEEPALIVE,SO_OOBINLINE,SO_LINGER
	static bool setOption(int socketfd, int name, const char* value, unsigned int valueLength);

	static bool startClient(int socketfd, const char* ip, unsigned int port);

	// backlog:1-5
	static bool startServer(int socketfd, const char* ip, unsigned int port, unsigned int backlog = 5);

	// return:socketfd
	static int accept(int socketfd, struct sockaddr_in* address);

	// return:send length
	static int send(int socketfd, const char* msg, unsigned int msgLength);

	// return:recv length
	static int recv(int socketfd, char* buf, unsigned int bufLength);

	// DNS parse
	static const char* parseDNS(const char* domain);
};

#endif	// _SOCKET_TCP_H_