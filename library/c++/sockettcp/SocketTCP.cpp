/**********************************************************************
* Author£ºjaron
* Date£º2015-09-18
* Brief£ºsocket tcp/ip
**********************************************************************/
#include "SocketTCP.h"
#include <stdio.h>
#include <fcntl.h>
#ifdef WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/un.h>
#endif
//--------------------------------------------------------------------------
int SocketTCP::create(void) {
#ifdef WIN32
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (0 != WSAStartup(wVersionRequested, &wsaData)) {
		WSACleanup();
		return -1;
	}
	if (2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion)) {
		WSACleanup();
		return -1;
	}
#endif
	int socketfd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd < 0) {
		WSACleanup();
		return -1;
	}
	int optAlive = 1;
	setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, (char*)(&optAlive), sizeof(optAlive));
	return socketfd;
}
//--------------------------------------------------------------------------
void SocketTCP::close(int socketfd, bool clean /*= true*/) {
	if (socketfd < 0) {
		return;
	}
#ifdef WIN32
	::closesocket(socketfd);
	if (clean) {
		WSACleanup();
	}
#else
	::close(socketfd);
#endif
}
//--------------------------------------------------------------------------
int SocketTCP::setBlock(int socketfd, bool isBlock /*= true*/) {
#ifdef WIN32
	unsigned long ul = isBlock ? 0 : 1;
	if (0 != ioctlsocket(socketfd, FIONBIO, &ul)) {
		return -1;
	}
#else
	int flag = fcntl(socketfd, F_GETFL, 0);
	if (fcntl(socketfd, F_SETFL, isBlock ? flag&~O_NONBLOCK : flag|O_NONBLOCK) < 0) {
		return -1;
	}
#endif
	return 0;
}
//--------------------------------------------------------------------------
bool SocketTCP::setOption(int socketfd, int name, const char* value, unsigned int valueLength) {
	if (socketfd < 0 || NULL == value) {
		return false;
	}
	if (::setsockopt(socketfd, SOL_SOCKET, name, value, valueLength) < 0) {
		return false;
	}
	return true;
}
//--------------------------------------------------------------------------
bool SocketTCP::startClient(int socketfd, const char* ip, unsigned int port) {
	if (socketfd < 0 || NULL == ip || 0 == strcmp("", ip) || 0 == port) {
		return false;
	}
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_addr.s_addr = inet_addr(ip);
	address.sin_port = htons(port);
	address.sin_family = AF_INET;
#ifndef WIN32
	bzero(&(address.sin_zero), 8);
#endif
	if (::connect(socketfd, (struct sockaddr*)&address, sizeof(struct sockaddr)) < 0) {
		return false;
	}
	return true;
}
//--------------------------------------------------------------------------
bool SocketTCP::startServer(int socketfd, const char* ip, unsigned int port, unsigned int backlog /*= 5*/) {
	if (socketfd < 0 || NULL == ip || 0 == strcmp("", ip) || 0 == port) {
		return false;
	}
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_addr.s_addr = inet_addr(ip);
	address.sin_port = htons(port);
	address.sin_family = AF_INET;
#ifndef WIN32
	bzero(&(address.sin_zero), 8);
#endif
	if (::bind(socketfd, (struct sockaddr*)&address, sizeof(struct sockaddr)) < 0) {
        return false;
    }
	if (::listen(socketfd, backlog) < 0) {
        return false;
    }
	return true;
}
//--------------------------------------------------------------------------
int SocketTCP::accept(int socketfd, struct sockaddr_in* address) {
	if (socketfd < 0 || NULL == address) {
		return -1;
	}
	int addressLength = sizeof(*address);
	return ::accept(socketfd, (struct sockaddr*)address, &addressLength);
}
//--------------------------------------------------------------------------
int SocketTCP::send(int socketfd, const char* msg, unsigned int msgLength) {
	if (socketfd < 0 || NULL == msg || 0 == msgLength) {
		return 0;
	}
	return ::send(socketfd, msg, msgLength, 0);
}
//--------------------------------------------------------------------------
int SocketTCP::recv(int socketfd, char* buf, unsigned int bufLength) {
	if (socketfd < 0 || NULL == buf || 0 == bufLength) {
		return 0;
	}
	return ::recv(socketfd, buf, bufLength, 0);
}
//--------------------------------------------------------------------------
const char* SocketTCP::parseDNS(const char* domain) {
	if (NULL == domain || 0 == strcmp("", domain)) {
		return "";
	}
	struct hostent* h = gethostbyname(domain);
	if (NULL == h) {
		return "";
	}
	static char ip[128] = {0};
	memset(ip, 0, strlen(ip));
	sprintf(ip, "%u.%u.%u.%u", (unsigned char)h->h_addr_list[0][0], (unsigned char)h->h_addr_list[0][1], (unsigned char)h->h_addr_list[0][2], (unsigned char)h->h_addr_list[0][3]);
	return ip;
}
//--------------------------------------------------------------------------