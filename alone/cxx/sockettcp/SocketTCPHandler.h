/**********************************************************************
* Author£ºjaron
* Date£º2015-09-25
* Brief£ºsocket tcp/ip handler
**********************************************************************/
#ifndef _SOCKET_TCP_HANDLER_H_
#define _SOCKET_TCP_HANDLER_H_

#include "ByteArray.h"

class SocketTCPHandler {
public:
	SocketTCPHandler(void);

	~SocketTCPHandler(void);

	int send(int socketfd, ByteArray& bytes);

	int recv(int socketfd, ByteArray& bytes);

private:
	char* mRecvBuf;
	char* mRecvBufTemp;
	int mRecvPackSize;
	int mRecvCurrSize;
};

#endif	// _SOCKET_TCP_HANDLER_H_