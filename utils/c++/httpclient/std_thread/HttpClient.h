/**********************************************************************
* Author:	jaron.ho
* Date:		2014-06-04
* Brief:	http �ͻ���
**********************************************************************/
#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#include <string>
#include <algorithm>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "CURLEx.h"

// http��������
class HttpObject
{
public:
	HttpObject() : tag(""), requesttype(HRT_POST), success(false), curlcode(-1), responsecode(-1) {}
	virtual void response(void) {}			//  http��Ϣ��������

public:
	// ����
	std::string tag;						// ��ǩ,���Ա�ʶÿ������
	std::string requesttype;				// ��������(�����ִ�Сд):"get","post","put","delete"
	CurlRequest requestdata;				// �������ݽṹ

	// ��Ӧ
	bool success;							// �Ƿ�����ɹ�
	int curlcode;							// ���󷵻ص�curl��
	int responsecode;						// http��Ӧ��,[200, 404]
	std::string errorbuffer;				// ��������,��responsecode!=200ʱ,��ֵ����������Ϣ
	std::vector<char> responsedata;			// ��Ӧ����,����ͨ��std::string str(responsedata->begin(), responsedata->end());�����ַ���
	std::vector<char> responseheader;		// ��Ӧͷ����,����ͨ��std::string str(responseheader->begin(), responseheader->end());�����ַ���
};

// http�ͻ���
class HttpClient
{
public:
	static HttpClient* getInstance(void);	// ��ȡ����
	static void destroyInstance(void);		// ɾ������
	void send(HttpObject* obj);				// ����http����
	void receive(void);						// ÿ֡ѭ������
};

#endif //_HTTP_CLIENT_H_