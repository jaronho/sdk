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
#include "FileDownload.h"

// �߳�����(�ڲ�ʹ��)
enum RUPthreadType
{
	RAUPT_NONE = 0,
	RAUPT_UPDATE_LIST,					// ���ָ����б�
	RAUPT_NO_UPDATE_LIST,				// û�и����б�
	RAUPT_UPDATE_LIST_DOWNLOADED		// �����б�ȫ���������
};

// ��Դ���²���״̬
enum ResourceUpdateOperateStatus
{
	RUOS_CAN_CHECK_VERSION,				// ��ִ�а汾���
	RUOS_IN_CHECK_VERSION,				// ��ִ�а汾���
	RUOS_CAN_CHECK_UPDATE,				// ��ִ�и��¼��
	RUOS_IN_CHECK_UPDATE,				// ��ִ�и��¼��
	RUOS_CAN_UPDATE,					// ��ִ�и��²���
	RUOS_IN_UPDATE,						// ��ִ�и��²���
};

// ��Դ������Ϣ�ṹ(�ڲ�ʹ��)
struct ResourceUpdateInfo
{
	std::string filename;				// �ļ���
	std::string md5value;				// md5ֵ
	long filesize;						// �ļ���С
};

// ��Դ���¼�����(��Ҫ�̳�ʵ��)
class ResourceUpdateListener
{
public:
	// �汾���ʧ��
	virtual void onCheckVersionFailed(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer) {}
	// �����°汾
	virtual void onNewVersion(const std::string& curVersion, const std::string& newVersion) {}
	// û���°汾
	virtual void onNoNewVersion(const std::string& curVersion) {}

	// �����б���ʧ��
	virtual void onCheckUpdateListFailed(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer) {}
	// ���ָ����б�
	virtual void onUpdateList(long updateCount, long updateSize) {}
	// û�и����б�
	virtual void onNoUpdateList(void) {}

	// �����ļ����ؽ���
	virtual void onPogress(const std::string& fileURL, double totalSize, double curSize) {}
	// �����ļ��������
	virtual void onSuccess(const std::string& fileURL) {}

	// �ļ��б����ؽ���
	virtual void onTotalProgress(const std::string& fileURL, int totalCount, int curCount) {}
	// �ļ��б��������
	virtual void onTotalSuccess(void) {}

	// �ļ����س���
	virtual void onError(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer) {}
};

// ��Դ����
class ResourceUpdate : public FileDownloadListener
{
public:
	ResourceUpdate(ResourceUpdateListener* listener = NULL);
	~ResourceUpdate(void);

	// ���ؽ���
	virtual void onProgress(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer, double totalToDownload, double nowDownloaded);

	// ���سɹ�
	virtual void onSuccess(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

	// ����ʧ��
	virtual void onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

public:
	// ���ػ��б�ʾ��
	static std::string newLineString(void);

	// �ָ��ַ���
	static std::vector<std::string> splitString(std::string str, const std::string& pattern);

	// �Ƿ�����ļ�
	static bool existFile(const std::string& file);

	// ��ȡ�ļ�����
	static char* getFileData(const std::string& file, unsigned long* fileSize);

	// ��ȡ�ļ�����,�ָ�ÿһ��
	static std::vector<std::string> getFileDataEx(const std::string& file);

	// д���ݵ��ļ�
	static bool writeDataToFile(const char* data, unsigned long dataSize, const std::string& file);

public:
	// ÿ֡����
	void listen(void);

	// ���ó�ʱ
	void setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout);

	// step1:���ñ���
	bool setNative(const std::string& path, const std::string& nativeVersionFile, const std::string& nativeMd5File);

	// step2:�汾���,urlĩβ������'/'��β
	bool checkVersion(const std::string& url, const std::string& checkVersionFile, const std::string& checkMd5File);

	// step3:���¼��,deepCheck:�汾�Ų�һ��ʱ,���޸����б�,true(�򲻱����°汾��,�´μ������汾�ź͸����б�),false(�����°汾��,�´�ֻ���汾��)
	bool checkUpdate(bool deepCheck = true);

	// step4:��ʼ����
	bool startUpdate(void);

	// ��¼���½���
	bool record(void);

private:
	// ������Ϣ����
	void createMessageQueue(void);

	// ������Ϣ����
	void destroyMessageQueue(void);

	// ������Ϣ
	void sendMessage(RUPthreadType msg);

	// ������Ϣ
	void recvMessage(void);

	void handleVersionCheckFileDownloaded(bool downloaded, const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer);

	void handleMd5CheckFileDownloaded(bool downloaded, const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer);

	void createUpdatePthread(void);

	void updateImpl(void);

	friend void* updateProcessFunc(void* ptr);

	// �������
	void handleUpdateListDownloaded(void);

	void removeImpl(void);

	friend void* removeProcessFunc(void* ptr);

	// ��¼����
	void recordImpl(void);

	friend void* recordProcessFunc(void* ptr);

	// �Ƴ�У���ļ�
	void removeCheckFile(void);

	// ����md5ֵ
	bool checkMd5Value(const std::string& fileURL);

	// ���������ļ��б�
	long parseUpdateFileList(std::vector<std::string>& updateVec);

	// �����Ƴ��ļ��б�
	void parseRemoveFileList(std::vector<std::string>& removeVec);

	// ����MD5�ļ��б�
	bool parseFileInfo(const std::string& filename, std::map<std::string, ResourceUpdateInfo>& infoVec);

private:
	unsigned int mId;											// ����id
	FileDownload mFileDownload;									// �ļ����ض���
	ResourceUpdateListener *mListener;							// ������
	bool mIsInnerCreatedListener;								// �Ƿ�Ϊ�ڲ������ļ�����
	// 
	std::list<RUPthreadType> *mMessageList;						// ��Ϣ����
	std::mutex mMessageQueueMutex;								// ��Ϣ���л���
	// 
	std::string mDownloadDir;									// ��Դ����·��
	std::string mNativeVersionFile;								// �汾�����ļ�
	std::string mNativeMd5File;									// MD5�����ļ�
	std::string mCurrVersion;									// ��ǰ�汾��
	std::map<std::string, ResourceUpdateInfo> mNativeInfoVec;	// �����ļ���Ϣ
	// 
	std::string mURL;											// ��Դ���ص�URL
	std::string mCheckVersionFile;								// �汾У���ļ�(�����ϵ��ļ�)
	std::string mCheckMd5File;									// MD5У���ļ�(�����ϵ��ļ�)
	std::string mNewVersion;									// �°汾��
	std::map<std::string, ResourceUpdateInfo> mCheckInfoVec;	// У���ļ���Ϣ
	std::vector<std::string> mDownloadedVec;					// �����ص��ļ�
	std::vector<std::string> mFileList;							// Ҫ���ص��ļ��б�
	long mTotalUpdateSize;										// �������ļ���С
	ResourceUpdateOperateStatus mUpdateStatus;					// ���²���״̬
	bool mDeepCheckUpdate;										// �Ƿ���ȼ�����״̬
};

#endif	// _RESOURCE_UPDATE_H_