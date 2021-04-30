/**********************************************************************
* Author:	jaron.ho
* Date:		2015-12-01
* Brief:	��Դ����
**********************************************************************/
#ifndef _RESOURCE_DOWNLOAD_H_
#define _RESOURCE_DOWNLOAD_H_

#include <string>
#include <vector>
#include <functional>
#include "FileDownload.h"

// ��Դ���ػص����Ͷ���
typedef std::function<void(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)>	RDErrorCB;			// �����ļ����س���ص�����
typedef std::function<void(const std::string& fileURL, double totalSize, double currSize)>								RDProgressCB;		// �����ļ����ؽ��Ȼص�����
typedef std::function<void(const std::string& fileURL)>																	RDSuccessCB;		// �����ļ�������ɻص�����
typedef std::function<void(const std::string& fileURL, int totalCount, int currCount)>									RDTotalProgressCB;	// �ļ��б����ؽ��Ȼص�����
typedef std::function<void(void)>																						RDTotalSuccessCB;	// �ļ��б�������ɻص�����

class ResourceDownload : public FileDownloadListener {
public:
	ResourceDownload(void);
	~ResourceDownload(void);

	// ����ʧ��
	virtual void onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

	// ���ؽ���
	virtual void onProgress(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer, double totalToDownload, double nowDownloaded);

	// ���سɹ�
	virtual void onSuccess(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

public:
	// ��ȡid
	unsigned int getId(void);

	// ÿ֡����
	void listen(void);

	// ���ó�ʱ
	void setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout);

	// ���ûص�(�����ļ����س���)
	void setErrorCB(const RDErrorCB& callback);

	// ���ûص�(�����ļ����ؽ���)
	void setProgressCB(const RDProgressCB& callback);

	// ���ûص�(�����ļ��������)
	void setSuccessCB(const RDSuccessCB& callback);

	// ���ûص�(�ļ��б����ؽ���)
	void setTotalProgressCB(const RDTotalProgressCB& callback);

	// ���ûص�(�ļ��б��������)
	void setTotalSuccessCB(const RDTotalSuccessCB& callback);

	// ��������·��
	void setDownloadPath(const std::string& path);

	// ִ������
	bool excute(const std::vector<std::string>& fileUrlVec, const std::string& cacheSuffix = "");

	// �Ƿ���������
	bool isDownloading(void);

private:
	FileDownload mFileDownload;							// �ļ����ض���
	RDErrorCB mErrorCB;									// �����ļ����س���ص�
	RDProgressCB mProgressCB;							// �����ļ����ؽ��Ȼص�
	RDSuccessCB mSuccessCB;								// �����ļ�������ɻص�
	RDTotalProgressCB mTotalProgressCB;					// �ļ��б����ؽ��Ȼص�
	RDTotalSuccessCB mTotalSuccessCB;					// �ļ��б�������ɻص�
};

#endif	// _RESOURCE_DOWNLOAD_H_