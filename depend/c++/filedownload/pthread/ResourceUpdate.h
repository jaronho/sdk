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
#include "FileDownload.h"


// �߳����ͣ��ڲ�ʹ�ã�
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


// ��Դ������Ϣ�ṹ���ڲ�ʹ�ã�
struct ResourceUpdateInfo
{
	std::string filename;				// �ļ���
	std::string md5value;				// md5ֵ
	long filesize;						// �ļ���С
};


// ��Դ���¼���������Ҫ�̳�ʵ�֣�
class ResourceUpdateListener
{
public:
	// �汾���ʧ��
	virtual void onCheckVersionFailed(const std::string& errorBuffer) {}
	// �����°汾
	virtual void onNewVersion(const std::string& curVersion, const std::string& newVersion) {}
	// û���°汾
	virtual void onNoNewVersion(const std::string& curVersion) {}

	// �����б���ʧ��
	virtual void onCheckUpdateListFailed(const std::string& errorBuffer) {}
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
	virtual void onError(const std::string& fileURL, const std::string& errorBuffer) {}
};


// ��Դ����
class ResourceUpdate : public FileDownloadListener
{
public:
	ResourceUpdate(const std::string& downloadDir, const std::string& versionNativeFile, const std::string& md5NativeFile, ResourceUpdateListener* listener = NULL);
	~ResourceUpdate(void);

	// ���ؽ���
	virtual void onProgress(FileDownloadCode code, const std::string& fileURL, const std::string& buffer, double totalToDownload, double nowDownloaded);

	// ���سɹ�
	virtual void onSuccess(FileDownloadCode code, const std::string& fileURL, const std::string& buffer);

	// ����ʧ��
	virtual void onError(FileDownloadCode code, const std::string& fileURL, const std::string& buffer);

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
	// ���ó�ʱ
	void setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout);

	// ÿ֡����
	void listen(void);

	// step1:�汾���,urlĩβ������'/'��β
	void checkVersion(const std::string& url, const std::string& versionCheckFile, const std::string& md5CheckFile);

	// step2:���¼��
	void checkUpdate(void);

	// step3:��ʼ����
	void startUpdate(void);

	// ��¼���ؽ���
	void record(void);

private:
	// ������Ϣ����
	void createMessageQueue(void);

	// ������Ϣ����
	void destroyMessageQueue(void);

	// ������Ϣ
	void sendMessage(RUPthreadType msg);

	// ������Ϣ
	void recvMessage(void);

	void handleVersionCheckFileDownloaded(bool downloaded, const std::string& errorBuffer);

	void handleMd5CheckFileDownloaded(bool downloaded, const std::string& errorBuffer);

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

	// ���������ļ��б�
	long parseUpdateFileList(const std::string& path, const std::string& checkFileName, const std::string& nativeFileName, std::vector<std::string>& updateVec);

	// �����Ƴ��ļ��б�
	void parseRemoveFileList(const std::string& path, const std::string& checkFileName, const std::string& nativeFileName, std::vector<std::string>& removeVec);

	// ����MD5�ļ��б�
	void parseFileInfo(const std::string& filename, std::map<std::string, ResourceUpdateInfo>& infoVec);

private:
	unsigned int mId;								// ����id
	FileDownload mFileDownload;						// �ļ����ض���
	ResourceUpdateListener *mListener;				// ������
	bool mIsInnerCreatedListener;					// �Ƿ�Ϊ�ڲ������ļ�����
	// 
	std::list<RUPthreadType> *mMessageList;			// ��Ϣ����
	pthread_mutex_t mMessageQueueMutex;				// ��Ϣ���л���
	pthread_t *mUpdatePthread;						// �����߳�
	pthread_t *mRemovePthread;						// �Ƴ��߳�
	pthread_t *mRecordPthread;						// ��¼�߳�
	// 
	std::string mDownloadDir;						// ��Դ����·��
	std::string mVersionNativeFile;					// �汾�����ļ�
	std::string mMd5NativeFile;						// MD5�����ļ�
	std::string mCurVersion;						// ��ǰ�汾��
	// 
	std::string mURL;								// ��Դ���ص�URL
	std::string mVersionCheckFile;					// �汾У���ļ�(�����ϵ��ļ�)
	std::string mMd5CheckFile;						// MD5У���ļ�(�����ϵ��ļ�)
	std::string mNewVersion;						// �°汾��
	std::vector<std::string> mDownloadedVec;		// �����ص��ļ�
	std::vector<std::string> mFileList;				// Ҫ���ص��ļ��б�
	long mTotalUpdateSize;							// �������ļ���С
	ResourceUpdateOperateStatus mUpdateStatus;		// ���²���״̬
};


#endif	// _RESOURCE_UPDATE_H_

