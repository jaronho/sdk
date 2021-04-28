/**********************************************************************
* Author：jaron
* Date：2015-09-25
* Brief：socket tcp/ip handler
**********************************************************************/
#include "SocketTCP.h"
#include "SocketTCPHandler.h"

#define HEAD_SIZE	4							// 包头大小
#define RECV_SIZE	(1024 * 256) + HEAD_SIZE	// 接收大小
//--------------------------------------------------------------------------
SocketTCPHandler::SocketTCPHandler(void) {
	mRecvBuf = new char[RECV_SIZE];
	memset(mRecvBuf, 0, RECV_SIZE);
	mRecvBufTemp = NULL;
	mRecvPackSize = 0;
	mRecvCurrSize = 0;
}
//--------------------------------------------------------------------------
SocketTCPHandler::~SocketTCPHandler(void) {
	if (mRecvBuf) {
		delete[] mRecvBuf;
		mRecvBuf = NULL;
	}
	if (mRecvBufTemp) {
		delete[] mRecvBufTemp;
		mRecvBufTemp = NULL;
	}
}
//--------------------------------------------------------------------------
int SocketTCPHandler::send(int socketfd, ByteArray& bytes) {
	if (socketfd < 0) {
		return -1;
	}
	const char* pBuf = bytes.getContent();
	int length = bytes.getCurLength();
	if (0 == pBuf || length <= 0) {
		return -1;
	}
	// 解析包体大小,写入包头
	int packSize = ByteArray::swab32(length);
	char* sendBuf = new char[length + HEAD_SIZE];
	memset(sendBuf, 0, length + HEAD_SIZE);
	memcpy(sendBuf, &packSize, HEAD_SIZE);
	memcpy(sendBuf + HEAD_SIZE, pBuf, length);	// 数据添加到BUF尾
	int sendLen = SocketTCP::send(socketfd, sendBuf, length + HEAD_SIZE);
	delete[] sendBuf;
	sendBuf = NULL;
	return sendLen;
}
//--------------------------------------------------------------------------
int SocketTCPHandler::recv(int socketfd, ByteArray& bytes) {
	if (socketfd < 0) {
		return -1;
	}
	memset(mRecvBuf, 0, RECV_SIZE);
	int recvLen = SocketTCP::recv(socketfd, mRecvBuf, RECV_SIZE);
	if (recvLen <= 0) {
		return -1;
	}
	if (0 == mRecvPackSize) {
		if (recvLen <= HEAD_SIZE) {
			return 0;
		}
		// 读取包头,解析包体大小
		char head[HEAD_SIZE] = {0};
		memcpy(head, mRecvBuf, HEAD_SIZE);
		mRecvPackSize = ByteArray::swab32_array(head);
		if (0 == mRecvPackSize) {
			return 0;
		}
		if (recvLen < HEAD_SIZE + mRecvPackSize) {
			mRecvCurrSize = recvLen - HEAD_SIZE;
			mRecvBufTemp = new char[mRecvPackSize];
			memset(mRecvBufTemp, 0, mRecvPackSize);
			memcpy(mRecvBufTemp, mRecvBuf + HEAD_SIZE, mRecvCurrSize);
		} else if (recvLen == HEAD_SIZE + mRecvPackSize) {
			bytes.setContent(mRecvBuf + HEAD_SIZE, mRecvPackSize);
			mRecvPackSize = 0;
		}
	} else {
		if (recvLen < mRecvPackSize - mRecvCurrSize) {
			memcpy(mRecvBufTemp + mRecvCurrSize, mRecvBuf, recvLen);
			mRecvCurrSize += recvLen;
		} else if (recvLen == mRecvPackSize - mRecvCurrSize) {
			memcpy(mRecvBufTemp + mRecvCurrSize, mRecvBuf, mRecvPackSize - mRecvCurrSize);
			bytes.setContent(mRecvBufTemp, mRecvPackSize);
			delete[] mRecvBufTemp;
			mRecvBufTemp = NULL;
			mRecvPackSize = 0;
			mRecvCurrSize = 0;
		}
	}
	return recvLen;
}
//--------------------------------------------------------------------------