/**********************************************************************
* Author:	jaron.ho
* Date:		2014-02-14
* Brief:	��Դ����
**********************************************************************/
#ifndef _RESOURCE_UPDATE_H_
#define _RESOURCE_UPDATE_H_

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <functional>
#include "FileDownload.h"

// ��Դ���»ص����Ͷ���
typedef std::function<std::string(const std::string& fileName)>															RUMd5CheckFunc;			// �ļ�MD5��麯������
typedef std::function<void(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)>	RUUpdateListErrorCB;	// �����б����س���ص�����
typedef std::function<void(void)>																						RUUpdateListNotFoundCB;	// �޿ɸ����б�ص�����
typedef std::function<void(long updateCount, long updateSize)>															RUUpdateListCB;			// �пɸ����б�ص�����
typedef std::function<void(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)>	RUErrorCB;				// �����ļ����س���ص�����
typedef std::function<void(const std::string& fileURL, double totalSize, double currSize)>								RUProgressCB;			// �����ļ����ؽ��Ȼص�����
typedef std::function<void(const std::string& fileURL)>																	RUSuccessCB;			// �����ļ�������ɻص�����
typedef std::function<void(const std::string& fileURL, int totalCount, int currCount)>									RUTotalProgressCB;		// �ļ��б����ؽ��Ȼص�����
typedef std::function<void(void)>																						RUTotalSuccessCB;		// �ļ��б�������ɻص�����

// ��Դ����
class ResourceUpdate : public FileDownloadListener {
public:
	// �߳�����(�ڲ�ʹ��)
	enum RUThreadType {
		RUTT_NONE = 0,
		RUTT_UPDATE_LIST_NOT_FOUND,		// û�и����б�
		RUTT_UPDATE_LIST,				// ���ָ����б�
		RUTT_UPDATE_LIST_ENDED			// �����б����
	};

	// ��Դ������Ϣ�ṹ(�ڲ�ʹ��)
	struct ResourceUpdateInfo {
		std::string md5value;			// �ļ�md5ֵ
		std::string filename;			// �ļ�����
		long filesize;					// �ļ���С
	};

public:
	ResourceUpdate(void);
	~ResourceUpdate(void);

	// ����ʧ��
	virtual void onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

	// ���ؽ���
	virtual void onProgress(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer, double totalToDownload, double nowDownloaded);

	// ���سɹ�
	virtual void onSuccess(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

public:
	// ���ػ��б�ʾ��
	static std::string newLineString(void);

	// �ָ��ַ���
	static std::vector<std::string> splitString(std::string str, const std::string& pattern);

	// ��ȡ�ļ�����
	static char* getFileData(const std::string& file, unsigned long* fileSize);

	// ��ȡ�ļ�����,�ָ�ÿһ��
	static std::vector<std::string> getFileDataEx(const std::string& file);

	// д���ݵ��ļ�
	static bool writeDataToFile(const char* data, unsigned long dataSize, const std::string& file);

public:
	// ��ȡid
	unsigned int getId(void);

	// ÿ֡����
	void listen(void);

	// ���ó�ʱ
	void setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout);

	// �����ļ�MD5��麯��
	void setFileMd5CheckFunc(const RUMd5CheckFunc& func);

	// ���ûص�(�����б����س���)
	void setUpdateListErrorCB(const RUUpdateListErrorCB& callback);

	// ���ûص�(�޿ɸ����б�)
	void setUpdateListNotFoundCB(const RUUpdateListNotFoundCB& callback);

	// ���ûص�(�пɸ����б�)
	void setUpdateListCB(const RUUpdateListCB& callback);

	// ���ûص�(�����ļ����س���)
	void setErrorCB(const RUErrorCB& callback);

	// ���ûص�(�����ļ����ؽ���)
	void setProgressCB(const RUProgressCB& callback);

	// ���ûص�(�����ļ��������)
	void setSuccessCB(const RUSuccessCB& callback);

	// ���ûص�(�ļ��б����ؽ���)
	void setTotalProgressCB(const RUTotalProgressCB& callback);

	// ���ûص�(�ļ��б��������)
	void setTotalSuccessCB(const RUTotalSuccessCB& callback);

	// step1:���ñ���,pathĩβ������'/'��β
	bool setNative(const std::string& path, const std::string& nativeMd5File);

	// step2:���¼��,urlĩβ������'/'��β
	bool checkUpdate(const std::string& url, const std::vector<std::string>& checkMd5FileList);

	// step3:���¿�ʼ
	bool startUpdate(const std::string& cacheSuffix = "", bool removeInvalidFileFlag = true);

	// �Ƿ���������
	bool isDownloading(void);

	// ��¼���½���
	bool record(bool immediatelyFlag = true);

private:
	// ������Ϣ����
	void createMessageQueue(void);

	// ������Ϣ����
	void destroyMessageQueue(void);

	// ������Ϣ
	void sendMessage(RUThreadType msg);

	// ������Ϣ
	void recvMessage(void);

	// ����׼��
	void handleUpdateListPrepare(void);

	friend void* updateListPrepareProcessFunc(void* ptr);

	void updateListPrepareImpl(void);

	// ���½���
	void handleUpdateListEnded(void);

	friend void* updateListEndedProcessFunc(void* ptr);

	void updateListEndedImpl(void);

	// ��¼����
	friend void* recordProcessFunc(void* ptr);

	void recordImpl(void);

	// ����md5ֵ
	bool checkMd5Value(const std::string& fileURL);

	// �Ƿ�MD5У���ļ�
	bool isCheckMd5File(const std::string& fileURL);

	// ��������б��ļ�
	void saveUpdateFileList(const std::map<std::string, ResourceUpdateInfo>& infoVec, const std::string& fileName);

	// ���������ļ��б�
	long parseUpdateFileList(std::vector<std::string>& updateVec);

	// ������Ч�ļ��б�
	void parseInvalidFileList(std::vector<std::string>& invalidVec);

	// ����MD5�ļ��б�
	bool parseFileInfo(const std::string& filename, std::map<std::string, ResourceUpdateInfo>& infoVec);

private:
	FileDownload mFileDownload;									// �ļ����ض���
	std::list<RUThreadType>* mMessageList;						// ��Ϣ����
	std::mutex mMessageQueueMutex;								// ��Ϣ���л���
	// 
	RUMd5CheckFunc mFileMd5CheckFunc;							// �ļ�MD5��麯��
	RUUpdateListErrorCB mUpdateListErrorCB;						// �����б����ش���ص�
	RUUpdateListNotFoundCB mUpdateListNotFoundCB;				// �޿ɸ����б�ص�
	RUUpdateListCB mUpdateListCB;								// �пɸ����б�ص�
	RUErrorCB mErrorCB;											// �����ļ����س���ص�
	RUProgressCB mProgressCB;									// �����ļ����ؽ��Ȼص�
	RUSuccessCB mSuccessCB;										// �����ļ�������ɻص�
	RUTotalProgressCB mTotalProgressCB;							// �ļ��б����ؽ��Ȼص�
	RUTotalSuccessCB mTotalSuccessCB;							// �ļ��б�������ɻص�
	// 
	std::string mDIR;											// ��Դ����·��
	std::string mNativeMd5File;									// MD5�����ļ�
	std::map<std::string, ResourceUpdateInfo> mNativeInfoVec;	// �����ļ���Ϣ
	// 
	std::string mURL;											// ��Դ��ȡURL
	std::vector<std::string> mCheckMd5FileList;					// MD5У���ļ��б�
	std::map<std::string, ResourceUpdateInfo> mCheckInfoVec;	// У���ļ���Ϣ
	// 
	std::string mCacheSuffix;									// �����׺
	bool mIsRemoveInvalidFile;									// �Ƿ�ɾ����Ч�ļ�
	std::vector<std::string> mUpdateFileList;					// Ҫ���µ��ļ��б�
	long mUpdateFileSize;										// Ҫ���µ��ļ���С
	std::vector<std::string> mHasUpdateFileList;				// �Ѹ��µ��ļ��б�
	size_t mHasRecordCount;										// �Ѽ�¼���ļ�����
};

#endif	// _RESOURCE_UPDATE_H_